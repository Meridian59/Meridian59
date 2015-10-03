<?php
# PHPlot / contrib / color_range : Test 1, make a picture
# $Id: color_range.test1.php 451 2009-12-09 03:45:49Z lbayuk $
# This creates a PNG file on output with a color gradient.

require_once 'color_range.php';

function usage()
{
   fwrite(STDERR, "Usage: color_range.test1.php color1 color2 number_of_colors
Each color is of the form rrggbb with 2 digit hex color components.
");
   exit(1);
}

# Split color "rrggbb" into separate components. Code is from PHPlot.
function rgb($color)
{
  return array(hexdec(substr($color, 1, 2)),
               hexdec(substr($color, 3, 2)),
               hexdec(substr($color, 5, 2)));
}

if ($_SERVER['argc'] != 4) usage();

$color1 = rgb($_SERVER['argv'][1]);
$color2 = rgb($_SERVER['argv'][2]);
$n_col = (int)$_SERVER['argv'][3];
if ($n_col < 2) usage();

# Build a color map from colors[0]=color1 to colors[$n_col-1]=color2.
$colors = color_range($color1, $color2, $n_col);

# Make a picture:
$w = 800;
$h = 800;
$im = imagecreate($w, $h);
$background = imagecolorresolve($im, 0, 0, 0);
for ($col = 0; $col < $n_col; $col++) {
    list($r, $g, $b) = $colors[$col];
    $colmap[$col] = imagecolorresolve($im, $r, $g, $b);
}

$margin = 20;
$bar_width = (int)(($w - 2 * $margin) / $n_col);
$x1 = $margin;
$x2 = $x1 + $bar_width;
$y1 = $margin;
$y2 = $h - $margin;
for ($col = 0; $col < $n_col; $col++) {
    imagefilledrectangle($im, $x1, $y1, $x2, $y2, $colmap[$col]);
    $x1 = $x2;
    $x2 += $bar_width;
}
imagepng($im);
