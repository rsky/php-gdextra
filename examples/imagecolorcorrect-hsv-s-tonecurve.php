<?php
include dirname(__FILE__) . '/_init.php';

$im = imagecreatefromjpeg('images/mosaic.jpg');
imagecolorcorrect($im, array(
    's' => array('tonecurve' => array(
        array(0.5, 0.75)
    )),
), IMAGE_EX_COLORSPACE_HSV);
imagejpeg($im, 'output/tonecurve-s1.jpg');

$im = imagecreatefromjpeg('images/mosaic.jpg');
imagecolorcorrect($im, array(
    's' => array('tonecurve2' => array(
        array(0.3, 0.2), array(0.5, 0.5), array(0.7, 0.8)
    )),
), IMAGE_EX_COLORSPACE_HSV);
imagejpeg($im, 'output/tonecurve-s2.jpg');
