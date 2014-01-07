<?php
# $Id: data_table.example3.php 999 2011-08-05 19:00:48Z lbayuk $
# phplot / contrib / data_table example 3:  Pie chart with data table
require_once 'phplot.php';
require_once 'data_table.php';

// Note column 0, labels, are not used by PHPlot itself, but are
// displayed in the data table, and extracted for the legend.
$data = array(
   array('Gold', 20),
   array('Silver', 40),
   array('Platinum', 30),
   array('Tin', 70),
);

// The $settings array configures the data table:
// Width and height are auto-calculated, and position defaults to 0,0.
$settings = array(
    'headers' => array('Metal', 'Amount'),
    'data' => $data,
);

$plot = new PHPlot(600, 400);
$plot->SetTitle('Pie Chart with Legend and Data Table');
$plot->SetDataValues($data);
$plot->SetDataType('text-data-single');
$plot->SetPlotType('pie');
foreach ($data as $row) $plot->SetLegend($row[0]);
$plot->SetCallback('draw_graph', 'draw_data_table', $settings);
$plot->DrawGraph();
