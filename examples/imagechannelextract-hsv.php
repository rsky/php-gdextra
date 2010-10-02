<?php
include dirname(__FILE__) . '/_init.php';

$im = imagecreatefromjpeg('images/mosaic.jpg');
$channels = imagechannelextract($im, IMAGE_EX_COLORSPACE_HSV);
imagepng($channels[0], 'output/hsv-h.png');
imagepng($channels[1], 'output/hsv-s.png');
imagepng($channels[2], 'output/hsv-v.png');
