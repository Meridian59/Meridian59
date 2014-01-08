<?php
# $Id: data_table.example1.php 999 2011-08-05 19:00:48Z lbayuk $
# phplot / contrib / data_table example 1:  Stand-alone data tables
# This example does not use PHPlot. Output is a PNG file with multiple
# data tables, with varying parameters.
require_once 'data_table.php';

$data = array(
  array('January',  1,  10, 1e05),
  array('February',  2,  20, 1e04),
  array('March',  3,  30, 1e03),
  array('April',  4,  40, 1e02),
  array('May',  5,  '', 1e01),
  array('June',  6,  60, 1e00),
  array('July',  7,  70,    0),
  array('August',  8,  80, 1e-1),
  array('September',  9,  90, 1e-2),
  array('October', 10, 100, 1e-3),
  array('November', 11, 110, 1e-4),
  array('December', 12, 120, 1e-5),
);

$img_width = 800;
$img_height = 600;
$cx = $img_width / 2;
$cy = $img_height / 2;
$tbl_width = $cx - 20;
$tbl_height = $cy - 20;

$base_settings = array(
    'headers' => array("Label", 'Y1', 'Y2', 'Y3'),
    'column_widths' => array(3, 1, 2, 3),
    'column_formats' => array('%s', '%d', '%5.2f', '%6g'),
    'width' => $tbl_width,
    'height' => $tbl_height,
    'data' => $data,
    'column_alignments' => array('C', 'R', 'R', 'R'),
);

$img = imagecreate($img_width, $img_height);
$white = imagecolorresolve($img, 255, 255, 255);
$red = imagecolorresolve($img, 255, 0, 0);
imageline($img, 0, $cy, $img_width-1, $cy, $red);
imageline($img, $cx, 0, $cx, $img_height-1, $red);

draw_data_table($img, array_merge($base_settings, array(
    'position' => array(10, 10),
   )));
draw_data_table($img, array_merge($base_settings, array(
    'position' => array($cx + 10, 10),
    'cellpadding' => 12,
    'column_alignments' => array('R', 'L', 'R', 'R'),
   )));
draw_data_table($img, array_merge($base_settings, array(
    'position' => array(10, $cy + 10),
    'color' => array(0x66, 0x66, 0xcc),
   )));
draw_data_table($img, array_merge($base_settings, array(
    'position' => array($cx + 10, $cy + 10),
    'font' => 4,
   )));

imagepng($img);
