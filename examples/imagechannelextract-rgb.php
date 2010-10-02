<?php
include dirname(__FILE__) . '/_init.php';

$im = imagecreatefromjpeg('images/mosaic.jpg');
$channels = imagechannelextract($im);
imagepng($channels[0], 'output/rgb-r.png');
imagepng($channels[1], 'output/rgb-g.png');
imagepng($channels[2], 'output/rgb-b.png');
