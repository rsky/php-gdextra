<?php
include dirname(__FILE__) . '/_init.php';

$im = imagecreatefromjpeg('images/mosaic.jpg');
$channels = imagechannelextract($im, IMAGE_EX_COLORSPACE_HSL);
imagepng($channels[0], 'output/hsl-h.png');
imagepng($channels[1], 'output/hsl-s.png');
imagepng($channels[2], 'output/hsl-l.png');
