<?php
include dirname(__FILE__) . '/_init.php';

$im = imagecreatefromjpeg('images/mutzig.jpg');

$im1 = imagescale($im, 100, 100, IMAGE_EX_SCALE_NONE);
imagejpeg($im1, 'output/scale-none-1.jpg');

$im2 = imagescale($im, 1000, 1000, IMAGE_EX_SCALE_NONE);
imagejpeg($im2, 'output/scale-none-2.jpg');
