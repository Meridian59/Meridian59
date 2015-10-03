<?php
require_once '../partials/config.php';
header('Content-type: application/json');

$response = array();

// Check connection
if (! $mysqli->connect_error) {
    $get_avg_damage = "SELECT avg(player_damaged_applied) pvpdamagetaken, player_damaged_weapon weapon
                                FROM player_damaged
                                WHERE player_damaged_weapon != 'monster attack' AND
                                player_damaged_attacker != 'sandstorm' AND
                                player_damaged_attacker != 'Amulet of Shadows'
                                group by player_damaged_weapon order by pvpdamagetaken desc";

    $result = $mysqli->query($get_avg_damage);
    if ($result->num_rows > 0) {

        $response['status'] = 'success';

        // output data of each row
        while($row = $result->fetch_assoc()) {
            $response['data'][] = $row;
        }


        echo json_encode($response);
    }else{
        $response['status'] = 'empty';
        echo json_encode($response);
    }

    mysqli_close($mysqli);
    exit;
}
$response['status'] = 'error';
$response['message'] = 'Unable to Establish Connection';
echo json_encode($response);
exit;
