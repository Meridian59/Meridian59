<?php
define('M59DEBUG', true);

//TODO: Might want to build a "Query" function to reduce the redundency in the ajax calls.

if(M59DEBUG){
    error_reporting(E_ALL);
    ini_set('display_errors', 1);
}else{
    error_reporting(0);
    ini_set('display_errors', 0);
}

define('DB_NAME', 'meridian');
define('DB_USER', 'root');
define('DB_PASSWORD', '');
define('DB_HOST', '127.0.0.1');

$mysqli = mysqli_connect(DB_HOST, DB_USER, DB_PASSWORD, DB_NAME);

/**
 * @desc used to look at the contents of a variable for debugging.
 *
 * @param $data any variable
 */
function m59_data_print($data){
    echo '<pre>';
    print_r($data);
    echo '</pre>';
}