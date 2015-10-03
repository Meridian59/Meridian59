<?php
require_once '../partials/config.php';
header('Content-type: application/json');

$response = array();
if(! empty($_REQUEST['idguild'])) {

    // Check connection
    if (! $mysqli->connect_error) {
        $idguild = mysqli_real_escape_string($mysqli, $_REQUEST['idguild']);

        //get from `guild table`
        $get_guild = 'SELECT * FROM `guild` WHERE `idguild` = '.$idguild;
        $result = $mysqli->query($get_guild);

        if (! empty($result) && $result->num_rows > 0) {

            // output data of each row
            while($row = $result->fetch_assoc()) {
                $response['data']['guild'] = $row;
            }

        }

        /*
         * TODO: Setup some means of tracking pvp for guilds.
         * TODO: Perhaps track each player's kills on their table and each kill as a member of a guild in either the guilds table or a guild_pvp table
        //get from `guild_death` table
        $get_deaths = 'SELECT * FROM `guild_death` WHERE `guild_death_victim` = "'.$response['data']['guild']['guild_name'].'" ORDER BY guild_death_time DESC LIMIT 50';
        $result = $mysqli->query($get_deaths);

        if (! empty($result) && $result->num_rows > 0) {

            // output data of each row
            while($row = $result->fetch_assoc()) {
                $response['data']['deaths'][] = $row;
            }
        }
        */

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