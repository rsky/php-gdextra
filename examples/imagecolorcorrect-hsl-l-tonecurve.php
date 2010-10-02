<?php
include dirname(__FILE__) . '/_init.php';

$im = imagecreatefromjpeg('images/mutzig.jpg');
imagecolorcorrect($im, array(
    'l' => array('tonecurve' => array(
        array(0.5, 0.75)
    )),
), IMAGE_EX_COLORSPACE_HSL);
imagejpeg($im, 'output/tonecurve1.jpg');

$im = imagecreatefromjpeg('images/mutzig.jpg');
imagecolorcorrect($im, array(
    'l' => array('tonecurve2' => array(
        array(0.3, 0.2), array(0.5, 0.5), array(0.7, 0.8)
    )),
), IMAGE_EX_COLORSPACE_HSL);
imagejpeg($im, 'output/tonecurve2.jpg');
