<?php
include dirname(__FILE__) . '/_init.php';

if (!defined('IMAGE_EX_SCALE_CARVE')) {
    echo "Seam carving (by using liblqr) is not enabled.\n";
    exit(1);
}

$im = imagecreatefromjpeg('images/mutzig.jpg');

$im1 = imagescale($im, 100, 100, IMAGE_EX_SCALE_CARVE);
imagejpeg($im1, 'output/scale-carve-1.jpg');

$im2 = imagescale($im, 1000, 1000, IMAGE_EX_SCALE_CARVE);
imagejpeg($im2, 'output/scale-carve-2.jpg');
