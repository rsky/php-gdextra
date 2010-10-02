<?php
include dirname(__FILE__) . '/_init.php';

$im = imagecreatefromjpeg('images/mosaic.jpg');
$channels = imagechannelextract($im, IMAGE_EX_COLORSPACE_CMYK);
imagepng($channels[0], 'output/cmyk-c.png');
imagepng($channels[1], 'output/cmyk-m.png');
imagepng($channels[2], 'output/cmyk-y.png');
imagepng($channels[3], 'output/cmyk-k.png');
