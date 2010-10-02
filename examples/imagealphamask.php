<?php
include dirname(__FILE__) . '/_init.php';

$mask = imagecreatefrompng('images/alpha-star.png');
$im = imagecreatefromjpeg('images/mutzig.jpg');
imagealphamask($im, $mask);
imagepng($im, 'output/masked-1.png');

$mask = imagecreatefrompng('images/alpha-star.png');
$im = imagecreatefromjpeg('images/mutzig.jpg');
imagealphamask($im, $mask, IMAGE_EX_MASK_TILE);
imagepng($im, 'output/masked-2.png');

$mask = imagecreatefrompng('images/alpha-star.png');
$im = imagecreatefromjpeg('images/mutzig.jpg');
imagealphamask($im, $mask, IMAGE_EX_MASK_TILE | IMAGE_EX_MASK_NOT);
imagepng($im, 'output/masked-3.png');
