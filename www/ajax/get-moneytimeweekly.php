<?php
require_once '../partials/config.php';
header('Content-type: application/json');

$response = array();

// Check connection
if (! $mysqli->connect_error) {
    $get_avg_damage = "SELECT player_money_total_amount amount, Date(player_money_total_time) date FROM player_money_total GROUP BY date ORDER by date ASC LIMIT 7";

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
