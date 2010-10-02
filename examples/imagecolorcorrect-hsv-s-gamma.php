<?php
include dirname(__FILE__) . '/_init.php';

$im = imagecreatefromjpeg('images/mosaic.jpg');
imagecolorcorrect($im, array(
    's' => array('gamma' => 0.7),
), IMAGE_EX_COLORSPACE_HSV);
imagejpeg($im, 'output/gamma-s1.jpg');

$im = imagecreatefromjpeg('images/mosaic.jpg');
imagecolorcorrect($im, array(
    's' => array('gamma' => 1.4),
), IMAGE_EX_COLORSPACE_HSV);
imagejpeg($im, 'output/gamma-s2.jpg');
