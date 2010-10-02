<?php
include dirname(__FILE__) . '/_init.php';

$im = imagecreatefromjpeg('images/mutzig.jpg');

$im1 = imagescale($im, 100, 100, IMAGE_EX_SCALE_FIT);
imagejpeg($im1, 'output/scale-fit-1.jpg');

$im2 = imagescale($im, 1000, 1000, IMAGE_EX_SCALE_FIT);
imagejpeg($im2, 'output/scale-fit-2.jpg');
