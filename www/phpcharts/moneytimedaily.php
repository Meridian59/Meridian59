<?php
require_once 'phplot-6.1.0/phplot.php';

ini_set('display_errors', 'off');
$link = mysql_connect('localhost', 'root', '');
$max = 0;
$min = 0;
/*
$result = mysql_query("SELECT Max(player_money_total_amount) max,min(player_money_total_amount) min FROM meridian103.player_money_total;");
while ($row = mysql_fetch_assoc($result)) {
    $max = $row['max'];
    $min = $row['min'];
}
$min -= 1000000;
mysql_free_result($result);
*/

$result = mysql_query("SELECT player_money_total_amount amount, player_money_total_time time from meridian.player_money_total where date(player_money_total_time) = date(now()) order by player_money_total_time ASC;");
while ($row = mysql_fetch_assoc($result)) {
    $data[] = array( $row['time'],$row['amount'] );
}
mysql_free_result($result);
mysql_close($link);

//Define the object
$plot = new PHPlot();

$plot->SetTitle("Server 103 Total Shillings over Time - Period 1 Day");
$plot->SetFontGD('y_label', 5);
$plot->SetFontGD('x_label', 5);
//$plot->SetPlotAreaWorld(NULL,$min,NULL,$max);
$plot->SetXLabelAngle(90);
$plot->SetYDataLabelPos('plotin');
$plot->SetDrawYDataLabelLines('false');
//$plot->SetFontGD('y_title', 5);
//$plot->SetFontGD('x_title', 5);


$plot->SetDataValues($data);

//Turn off X axis ticks and labels because they get in the way:
$plot->SetXTickLabelPos('none');
$plot->SetXTickPos('none');

//Draw it
$plot->DrawGraph();
?>