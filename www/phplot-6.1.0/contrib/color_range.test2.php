<?php
/*
  PHPlot / contrib / color_range : Unit tests
  $Id: color_range.test2.php 452 2009-12-09 03:45:51Z lbayuk $

  Tests color.range.php functions:
      color_range($color_a, $color_b, $n_steps)
      count_data_sets($data, $data_type)

*/
require_once 'color_range.php';


# Testing count_data_sets()
function test_count_data_sets($data, $data_type, $expected)
{
  $n = count_data_sets($data, $data_type);
  if ($n == $expected) $result = "Pass";
  else $result = "FAIL: Expected $expected but got";
  echo "$result: $n data sets, $data_type with " . count($data) . " records.\n";
}

function test_driver_count_data_sets()
{
    echo "\nTesting count_data_sets():\n";
    $data1 = array(array('a', 1, 2, 3), array('b', 2, 2, 3));
    $data2 = array(array('a', 1, 2, 3, 4, 5, 6, 7), array('b', 2, 4, 5, 6));
    $data3 = array(array('', 1), array('', 2), array('', 3), array('', 4));
    $data4 = array(array('', 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13),
                   array('', 1, 2, 3, 4, 5, 6, 7),
                   array('', 1, 2, 3, 4));

    test_count_data_sets($data1, 'text-data', 3);
    test_count_data_sets($data1, 'data-data', 2);
    test_count_data_sets($data2, 'data-data-error', 2);
    test_count_data_sets($data3, 'text-data-single', 4);

    test_count_data_sets($data4, 'text-data', 13);
    test_count_data_sets($data4, 'data-data', 12);
    test_count_data_sets($data4, 'data-data-error', 4);
    test_count_data_sets($data4, 'text-data-single', 3);
}

# Testing color_range()
function test_color_range($color1, $color2, $nsteps, $expected)
{
    $info = 'From (' . implode(', ', $color1)
       . ') To (' . implode(', ', $color2) . ") with $nsteps steps";

    $colors = color_range($color1, $color2, $nsteps);
    if ($colors == $expected) echo "Pass: $info\n";
    else echo "FAIL: $info\n" . print_r($colors, True) . "\n";
    if (($n = count($colors)) != $nsteps)
        echo "FAIL: Bad count $n expecting $nsteps\n";
}

function test_driver_color_range()
{
    echo "\nTesting color_range():\n";
    test_color_range(array(0,0,0), array(255,255,255), 3,
        array(array(0,0,0), array(127,127,127), array(255,255,255)));

    test_color_range(array(255,0,0), array(0,255,0), 2,
        array(array(255,0,0), array(0,255,0)));

    test_color_range(array(100,0,100), array(0,100,0), 6,
        array(array(100,0,100), array(80,20,80), array(60,40,60),
              array(40,60,40), array(20,80,20), array(0,100,0)));
}


test_driver_count_data_sets();
test_driver_color_range();
