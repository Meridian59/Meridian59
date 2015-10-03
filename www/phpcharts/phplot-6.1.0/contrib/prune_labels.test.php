<?php
# PHPlot / contrib / prune_labels : Test
# $Id: prune_labels.test.php 455 2009-12-09 03:45:57Z lbayuk $
# Test driver for contrib / prune_labels

require_once 'prune_labels.php';

/* Testing the prune_labels function: */
function test($count, $maxlabels)
{
    # Make an array of count records, like PHPlot uses, with labels:
    $data = array();
    for ($i = 0; $i < $count; $i++) {
        $data[] = array("Row $i", $i, 100, 200, 300);
    }

    prune_labels($data, $maxlabels);

    # See how many labels are non-blank now:
    $line = '';
    $non_blank = 0;
    for ($i = 0; $i < $count; $i++) {
        if (!empty($data[$i][0])) {
            $non_blank++;
            $line .= '*';
        } else {
            $line .= '_';
        }
    }
    $status = ($non_blank <= $maxlabels) ? 'PASS' : 'FAIL';
    echo "$status: $count rows, maxlabels=$maxlabels => $non_blank labels\n";
    echo substr($line, 0, 80) . "\n";  # Only show first 80 chars.
}

/* Test cases for prune_labels */
for ($n = 7; $n <= 1000; $n *= 2) test($n, 10);
for ($g = 5; $g <= 40; $g++) test(72, $g);
# Edge cases
test(80, 41);
test(80, 40);
test(80, 39);
