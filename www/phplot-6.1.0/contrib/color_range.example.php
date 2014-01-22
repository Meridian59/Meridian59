<?php
# PHPlot / contrib / color_range : Example
# $Id: color_range.example.php 449 2009-12-09 03:45:45Z lbayuk $
# This is a bar chart with a color gradient for the bars in each group.

require_once 'phplot.php';
require_once 'color_range.php';

$bars_per_group = 10;
$x_values = 4;

mt_srand(1);
$data = array();
for ($i = 0; $i < $x_values; $i++) {
    $row = array($i);
    for ($j = 0; $j < $bars_per_group; $j++) $row[] = mt_rand(0, 100);
    $data[] = $row;
}

$p = new PHPlot(800, 600);
$p->SetTitle('Example - Bar Chart with gradient colors');
$p->SetDataType('text-data');
$p->SetDataValues($data);
$p->SetPlotAreaWorld(0, 0, $x_values, 100);

# This isn't necessary, as we do know how many data sets (bars_per_group):
$n_data = count_data_sets($data, 'text-data');
# Make a gradient color map:
$colors = color_range($p->SetRGBColor('SkyBlue'),
                      $p->SetRGBColor('DarkGreen'), $n_data);
$p->SetDataColors($colors);
$p->SetXTickLabelPos('none');
$p->SetXTickPos('none');
$p->SetPlotType('bars');
$p->DrawGraph();
