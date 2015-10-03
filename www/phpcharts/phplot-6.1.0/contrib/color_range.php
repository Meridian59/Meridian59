<?php
/*
   PHPlot / contrib / color_range
   $Id: color_range.php 974 2011-07-26 17:20:50Z lbayuk $
   PHPlot contrib code - public domain - no copyright - use as you wish

   Original contribution from: Josep Sanz <josep dot sans at w3 dot es>
      "I wrote this code to calculate the range of colors between 2 colors
      to plot using the range from color_a to color_b..."

   I have changed the code and repackaged it, but the idea is the same.
   Given 2 colors and number of data sets, computes an array of colors
   that make up a gradient between the two provided colors, for use
   with SetDataColors().

   Provides the following functions:
       $colors = color_range($color_a, $color_b, $n_intervals)
              Returns a color array for SetDataColors.

       $n = count_data_sets($data, $data_type)
              Counts the number of data sets in a data array.
              This can be used to provide $n_intervals in color_range().
*/



/*
   Fill a color map with a gradient step between two colors.
  Arguments:
    $color_a : Starting color for the gradient. Array of (r, g, b)
    $color_b : Ending color for the gradient. Array of (r, g, b)
    $n_steps : Total number of color steps, including color_a and color_b.

  Returns: A color map array with n_steps colors in the form
           $colors[i][3], suitable for SetDataColors().

  Notes:
    You may use the PHPlot internal function $plot->SetRGBColor($color)
    to convert a color name or #rrggbb notation into the required array
    of 3 values (r, g, b) for color_a and color_b.

    Newer versions of PHPlot use 4 components (r, g, b, a) arrays for color.
    This script ignores the alpha component in those arrays.

*/
function color_range($color_a, $color_b, $n_steps)
{
    if ($n_steps < 2) $n_steps = 2;
    $nc = $n_steps - 1;
    # Note: $delta[] and $current[] are kept as floats. $colors is integers.
    for ($i = 0; $i < 3; $i++)
        $delta[$i] = ($color_b[$i] - $color_a[$i]) / $nc;
    $current = $color_a;
    for ($col = 0; $col < $nc; $col++) {
        for ($i = 0; $i < 3; $i++) {
            $colors[$col][$i] = (int)$current[$i];
            $current[$i] += $delta[$i];
        }
    }
    $colors[$nc] = $color_b;  # Make sure the last color is exact.
    return $colors;
}


/*
    Determine the number of data sets (plot lines, bars per group, pie
    segments, etc.) contained in a data array.
    This can be used to determine n_steps for $color_range.

  Arguments:
    $data : PHPlot data array
    $data_type : PHPlot data type, describing $data. (e.g. 'data-data')
  Returns: The number of data sets in the data array.
  Notes:
    This has to scan the entire data array. Don't use this unless you
    really don't have a better way to determine the number of data sets.

    This does NOT require that the data array be integer indexed.

*/
function count_data_sets($data, $data_type)
{

    if ($data_type == 'text-data-single')
        return count($data); # Pie chart, 1 segment per record

    # Get the longest data record:
    $max_row = 0;
    foreach ($data as $row)
        if (($n = count($row)) > $max_row) $max_row = $n;

   if ($data_type == 'text-data' || $data_type == 'text-data-yx')
      return ($max_row - 1);  # Each record is (label Y1 Y2...)

   if ($data_type == 'data-data' || $data_type == 'data-data-yx')
      return ($max_row - 2); # Each record is (label X Y1 Y2...)

   if ($data_type == 'data-data-error')
      return (($max_row - 2) / 3); # Each record is (label X Y1 Y1+ Y1-...)

   # Not a recognized data type... Just return something sane.
   return $max_row;
}
