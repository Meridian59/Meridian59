<?php
# $Id: data_table.example2.php 999 2011-08-05 19:00:48Z lbayuk $
# phplot / contrib / data_table example 2: Line plot with data table on the side
require_once 'phplot.php';
require_once 'data_table.php';

$data = array();
for ($i = 0; $i < 20; $i++)
  $data[] = array('', $i, 2 * $i, $i * $i);

// The $settings array configures the data table:
$settings = array(
    'headers' => array(NULL, 'X', '2Y', 'Y^2'),
    'position' => array(640, 20),
    'width' => 150,
    'data' => $data,
    'font' => 3,
);

$plot = new PHPlot(800, 600);
$plot->SetTitle('Line Plot with Data Table on Right Side');
$plot->SetDataValues($data);
$plot->SetDataType('data-data');
$plot->SetPlotType('linepoints');
$plot->SetPlotAreaPixels(NULL, NULL, 630, NULL);
$plot->SetCallback('draw_graph', 'draw_data_table', $settings);
$plot->SetLegend(array('2Y', 'Y^2'));
$plot->DrawGraph();
