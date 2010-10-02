<?php
include dirname(__FILE__) . '/_init.php';

$im = imagecreatefrompng('images/rgba-16x16.png');
imagesavealpha($im, false);
imagetruecolortopalette($im, true, 256);
imageicon($im, 'output/favicon.ico');
