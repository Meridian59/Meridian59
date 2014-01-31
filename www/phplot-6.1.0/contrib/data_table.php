<?php
/* $Id: data_table.php 1000 2011-08-06 01:19:27Z lbayuk $
phplot / contrib / data_table.php: Draw a table of data values

     Copyright (c) 2011, lbayuk -at- users.sourceforge.net
     All rights reserved.

     Redistribution and use in source and binary forms, with or without
     modification, are permitted provided that the following conditions are met:

         * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
         * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
     AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
     ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
     LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
     CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
     SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
     INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
     ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
     POSSIBILITY OF SUCH DAMAGE.

This function draws a data table. That is, using PHP GD functions, it draws
a table containing elements from a 2-dimensional array of text and number data.

This is meant to be used as a PHPlot callback, but does not use PHPlot.
It takes 2 arguments: a PHP GD image resource, and an array of settings. It
draws the table into the image and returns TRUE.  On error, it uses
trigger_error(); if an error handler returns, it returns FALSE.

The following keys are required in the settings array:

    'data' => A PHPlot-style data array. Array[0..n-1] of rows, each
      row entry is an array with values (strings or numbers). The
      'headers' entry (see below) determines which columns are used in
      the table. Other settings control how the values are formatted and
      drawn in the table.
      Note: Multi-line string values (with newlines) do not work.

    'headers' => Array of headers for the data array.
      There must be an entry for each data array column. The entry is the
      header label for that column.  NULL means do not include this data array
      column in the table.
      Header labels are centered within the header row of the table.
      Multi-line entries (with newlines) are not supported and do not work.

The following keys are optional in the settings array:

    'position' => Array (x, y) giving the GD coordinates for the upper left
      corner of the table. The default is (0,0), which is the upper left
      corner of the image.

    'height' => Height in pixels of the table.
       If not given or empty or 0, the table height is calculated as the minimum
       value, taking into account the number of rows, font height, and
       cellpadding value.

    'width' => Width in pixels of the table
       If not given or empty or 0, the table width is calculated to just fit
       the widest value in each column (after formatting).
       If width is not provided, column_widths is ignored.

    'column_widths' => Array of relative column width weights.
       For example: array(4,2,1,1) means the 1st column is 2 times
       as wide as the 2nd, and 4 times as wide as the 3rd and 4th.
       If missing, all columns get equal width. Ignored if the overall
       table width is not given.

    'column_formats' => Array of printf formats for each column.
       An empty string means no formatting for that column - the value is
       drawn into the table using PHP's defaults.
       If missing, there is no formatting for any column.

    'column_alignments' => Array of L C or R for horizontal alignment, meaning
       left align, center, or right align.
       An empty string means automatic: align right if numeric, else left.
       Default if missing is automatic for all columns.

    'font' => GD font number 1-5. 1 is smallest, 5 is largest. Default is 2.

    'color' => Array with R, G, B color specification. Default is (0,0,0)=black.
            White would be (255,255,255). This color is used for grid lines
            and text in the table.

    'cellpadding' => Line to text gap in pixels. Default is 4.

Note that the arrays 'column_widths', 'column_formats', and 'column_alignments'
have one entry for each column in the table which will be drawn. This is
different from 'headers', which has one column for each column in the
data array. These sizes will be the same only if 'headers' has no NULL
entries - that is, no data array columns are being skipped.

*/
function draw_data_table($img, $settings)
{
    // Apply defaults, then extract all settings as variables named 'o_*':
    extract(array_merge(array(
            'color' => array(0, 0, 0),  // Default black
            'font' => 2,                // Default GD font
            'position' => array(0, 0),  // Default to upper left corner
            'width' => 0,               // Default auto width calculation
            'height' => 0,              // Default auto height calculation
            'cellpadding' => 4,         // Default line-to-text spacing
        ), $settings), EXTR_PREFIX_ALL, 'o');
    list($x, $y) = $o_position; // Expand to separate variables

    // Check for mandatory settings:
    if (!isset($o_data, $o_headers)) {
        trigger_error("draw_data_table error: 'headers' and 'data' are required");
        return FALSE; // In case error handler returns
    }

    // Font and color setup:
    $char_width = imagefontwidth($o_font);
    $char_height = imagefontheight($o_font);
    $color = imagecolorresolve($img, $o_color[0], $o_color[1], $o_color[2]);
    $pad2 = 2 * $o_cellpadding; // Pad all 4 sides of cells

    // Calculate the number of rows and columns in the table:
    $n_rows = count($o_data) + 1; // Add 1 for header row
    // Count non-skipped columns:
    $n_cols = 0;
    foreach ($o_headers as $h) if (!is_null($h)) $n_cols++;
    // Number of columns in the data array and in $o_headers:
    $n_data_cols = count($o_headers);

    // Default column weights so all columns have equal width.
    if (empty($o_column_widths))
        $o_column_widths = array_fill(0, $n_cols, 1);

    // Default column formats to no formatting:
    if (empty($o_column_formats))
        $o_column_formats = array_fill(0, $n_cols, '');

    // Default column alignments to auto align:
    if (empty($o_column_alignments))
        $o_column_alignments = array_fill(0, $n_cols, '');

    // Make sure there are the right number of entries.
    if (count($o_column_widths) != $n_cols
     || count($o_column_formats) != $n_cols
     || count($o_column_alignments) != $n_cols) {
        trigger_error("draw_data_table error: Mismatch in size of column spec arrays");
        return FALSE; // In case error handler returns
    }

    // If the table height is not supplied, calculate the space needed.
    if (empty($o_height))
        $o_height = $n_rows * ($char_height + $pad2);
    // Then calculate the height of each row.
    $row_height = $o_height / $n_rows;

    // If the table width is not supplied, calculate the space needed based
    // on the widest value for each column (including header). The column width
    // factors are ignored, since each column will be as wide as needed.
    if (empty($o_width)) {
        $o_width = 0;
        $col = 0; // Index to unskipped columns 
        for ($i = 0; $i < $n_data_cols; $i++) { // Index to all columns
            if (is_null($o_headers[$i])) continue; // Skip column
            // Find the longest string in this column, post-formatting.
            $len = strlen($o_headers[$i]); // Start with the header
            for ($row = 1; $row < $n_rows; $row++) {
                if (($cell = $o_data[$row - 1][$i]) !== '') { // Non-empty
                    // Apply cell format if specified:
                    if (($fmt = $o_column_formats[$col]) != '')
                        $cell = sprintf($fmt, $cell);
                    if (($this_len = strlen($cell)) > $len)
                        $len = $this_len;
                }
            }
            // Assign column width, and accumulate the total:
            $o_width += $col_width[$col++] = $len * $char_width + $pad2;
        }
    } else { // Table width, and optionally column width factors, are supplied.
        // Calculate the width of each column.
        $col_width_scale = $o_width / array_sum($o_column_widths);
        for ($col = 0; $col < $n_cols; $col++)
            $col_width[$col] = $o_column_widths[$col] * $col_width_scale;
    }

    // Calculate the column start positions within the table:
    $col_start[0] = 0;
    for ($i = 1; $i < $n_cols; $i++)
        $col_start[$i] = $col_start[$i-1] + $col_width[$i-1];

    // Draw the table grid (without outer border)
    $x2 = $x + $o_width - 1;
    for ($row = 1; $row < $n_rows; $row++) {
        $y0 = $y + $row_height * $row; // Avoid accumulating errors.
        imageline($img, $x, $y0, $x2, $y0, $color);
    }
    $y2 = $y + $o_height - 1;
    for ($col = 1; $col < $n_cols; $col++) {
        $x0 = $x + $col_start[$col];
        imageline($img, $x0, $y, $x0, $y2, $color);
    }

    // Draw the header row, then the data rows
    for ($row = 0; $row < $n_rows; $row++) {

        // Vertically center the cell contents within the cell:
        $y0 = $y + $row_height * ($row + 0.5) - $char_height / 2;

        if ($row == 0) $cells = $o_headers; // Header row
        else $cells = $o_data[$row - 1]; // -1 accounts for header row.

        $col = 0; // Index to unskipped columns 
        for ($i = 0; $i < $n_data_cols; $i++) { // Index to all columns
           
            if (is_null($o_headers[$i])) continue; // NULL header => skip column

            if (($cell = $cells[$i]) !== '') { // Empty cell?
                if ($row == 0) {
                    $alg = 'C'; // Header row forces center alignment
                } else {
                    // Apply cell format if specified:
                    if (($fmt = $o_column_formats[$col]) != '')
                        $cell = sprintf($fmt, $cell);
                    // Get cell alignment:
                    $alg = $o_column_alignments[$col];
                }

                // Calculate upper left position for this cell's text:
                if (empty($alg)) // Default alignment: numbers right, else left.
                    $alg = is_numeric($cell) ? 'R' : 'L';
                $x0 = $x + $col_start[$col];
                if ($alg == 'R') {
                    $x0 += $col_width[$col] - strlen($cell) * $char_width - $o_cellpadding;
                } elseif ($alg == 'C') {
                    $x0 += ($col_width[$col] - strlen($cell) * $char_width) / 2;
                } else { // Default, assume L
                    $x0 += $o_cellpadding;
                }
                imagestring($img, $o_font, $x0, $y0, $cell, $color);
            }
            $col++;
        }
    }
    return TRUE;
}
