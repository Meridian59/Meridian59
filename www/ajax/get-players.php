<?php
require_once '../partials/config.php';
header('Content-type: application/json');

$response = array();
if(! empty($_REQUEST['name'])) {

    // Check connection
    if (! $mysqli->connect_error) {
        $name = mysqli_real_escape_string($mysqli, $_REQUEST['name']);

        $get_players = 'SELECT * FROM player WHERE player_name LIKE "%'.$name.'%" ORDER BY player_name ASC';
        $result = $mysqli->query($get_players);
        if ($result->num_rows > 0) {

            $response['status'] = 'success';

            // output data of each row
            while($row = $result->fetch_assoc()) {
                $response['data'][] = array( 'name' => $row['player_name'], 'id' => $row['idplayer'],  'suicide' => $row['player_suicide'],);
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
}
$response['status'] = 'error';
$response['message'] = 'No Name provided';

echo json_encode($response);
exit;