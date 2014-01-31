<?php
/*
   PHPlot / contrib / prune_labels
   $Id: prune_labels.php 454 2009-12-09 03:45:55Z lbayuk $
   PHPlot contrib code - public domain - no copyright - use as you wish

Reduce the number of data labels along the X axis,  when the density is too
high.  This simply blanks out M-1 of every M labels in the data array.
There are other ways to do this, but we need to keep the labels uniformly
spaced.  You select the target maximum label count (maxlabels), and you will
get no more than maxlabels data labels.

  Arguments:
     $data  - The PHPlot data array (reference variable)
     $maxlabels - The maximum number of data labels you are willing to have,
  Returns: Nothing
     Modifies the $data array in place to remove some of the labels.

   Notes:
     The data array and its rows must be 0-based integer indexed arrays.
*/
function prune_labels(&$data, $maxlabels)
{
    # Do nothing if there are not already too many labels:
    if (($n = count($data)) <= $maxlabels) return;

    # Compute how many labels to erase. Keep 1 of every $m labels.
    $m = (int)ceil($n / $maxlabels);

    # Process the data array, zapping M-1 of every M labels:
    $k = 0;
    for ($i = 0; $i < $n; $i++) {
       if ($k > 0) $data[$i][0] = '';
       if (++$k >= $m) $k = 0;
    }
}
