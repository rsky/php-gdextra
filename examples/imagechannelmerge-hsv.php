<?php
include dirname(__FILE__) . '/_init.php';

$im = imagechannelmerge(array(
    imagecreatefrompng('output/hsv-h.png'),
    imagecreatefrompng('output/hsv-s.png'),
    imagecreatefrompng('output/hsv-v.png'),
), IMAGE_EX_COLORSPACE_HSV);
imagejpeg($im, 'output/merge-hsv.jpg');
