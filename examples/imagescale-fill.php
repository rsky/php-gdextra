<?php
include dirname(__FILE__) . '/_init.php';

$im = imagecreatefromjpeg('images/mutzig.jpg');

$im1 = imagescale($im, 100, 100, IMAGE_EX_SCALE_FILL);
imagejpeg($im1, 'output/scale-fill-1.jpg');

$im2 = imagescale($im, 1000, 1000, IMAGE_EX_SCALE_FILL);
imagejpeg($im2, 'output/scale-fill-2.jpg');
