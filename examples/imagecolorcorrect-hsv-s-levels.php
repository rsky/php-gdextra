<?php
include dirname(__FILE__) . '/_init.php';

$im = imagecreatefromjpeg('images/mosaic.jpg');
imagecolorcorrect($im, array(
    's' => array('levels' => array(51, 204)),
), IMAGE_EX_COLORSPACE_HSV);
imagejpeg($im, 'output/levels-s1.jpg');

$im = imagecreatefromjpeg('images/mosaic.jpg');
imagecolorcorrect($im, array(
    's' => array('levels' => array(0, 255, 51, 204)),
), IMAGE_EX_COLORSPACE_HSV);
imagejpeg($im, 'output/levels-s2.jpg');
