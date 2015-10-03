<?php
require_once '../partials/config.php';
header('Content-type: application/json');

$response = array();
if(! empty($_REQUEST['idplayer'])) {

    // Check connection
    if (! $mysqli->connect_error) {
        $idplayer = mysqli_real_escape_string($mysqli, $_REQUEST['idplayer']);

        //get from `player table`
        $get_player = 'SELECT * FROM `player` WHERE `idplayer` = '.$idplayer;
        $result = $mysqli->query($get_player);

        if (! empty($result) && $result->num_rows > 0) {

            // output data of each row
            while($row = $result->fetch_assoc()) {
                $response['data']['player'] = $row;
            }

        }

        //get from `player_death` table
        $get_deaths = 'SELECT * FROM `player_death` WHERE `player_death_victim` = "'.$response['data']['player']['player_name'].'" ORDER BY player_death_time DESC LIMIT 50';
        $result = $mysqli->query($get_deaths);

        if (! empty($result) && $result->num_rows > 0) {

            // output data of each row
            while($row = $result->fetch_assoc()) {
                $response['data']['deaths'][] = $row;
            }
        }

        //getting information from player table
        $get_players = 'SELECT * FROM `player_logins` WHERE `player_logins_character_name` = "'.$response['data']['player']['player_name'].'" ORDER BY player_logins_time DESC LIMIT 50';
        $result = $mysqli->query($get_players);

        if (! empty($result) && $result->num_rows > 0) {

            // output data of each row
            $i = 0;
            while($row = $result->fetch_assoc()) {
                $response['data']['logins'][] = $row;
                if($result->num_rows - 1 == $i++){
                    $response['data']['player']['player_last_login'] = $row['player_logins_time'];
                }
            }

        }

        if(empty($response)){
            $response['status'] = 'empty';

        }else{
            $response['status'] = 'success';
        }

        echo json_encode($response);

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