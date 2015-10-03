<?php
require_once 'phplot-6.1.0/phplot.php';

ini_set('display_errors', 'off');
$link = mysql_connect('localhost', 'root', '');
$max = 0;
$min = 0;

$result = mysql_query("SELECT avg(player_damaged_applied) pvpdamagetaken, player_damaged_weapon weapon FROM meridian.player_damaged
WHERE player_damaged_weapon != 'monster attack' AND
player_damaged_attacker != 'sandstorm' AND
player_damaged_attacker != 'Amulet of Shadows'
group by player_damaged_weapon order by pvpdamagetaken desc");
while ($row = mysql_fetch_assoc($result)) {
    $data[] = array( $row['weapon'],$row['pvpdamagetaken'] );
}
mysql_free_result($result);
mysql_close($link);

//Define the object
$plot = new PHPlot(800,400);

$plot->SetTitle("Server 103 Average PvP Damage");
$plot->SetFontGD('y_label', 5);
$plot->SetFontGD('x_label', 5);

$plot->SetXLabelAngle(90);
$plot->SetYDataLabelPos('plotin');
$plot->SetDrawYDataLabelLines('false');
$plot->SetPrecisionY(2);

$plot->SetPlotType('bars');
$plot->SetDataValues($data);

//Turn off X axis ticks and labels because they get in the way:
$plot->SetXTickLabelPos('none');
$plot->SetXTickPos('none');

//Draw it
$plot->DrawGraph();
?>