<?php
include dirname(__FILE__) . '/_init.php';

$im = imagecreatefromjpeg('images/mutzig.jpg');
imagecolorcorrect($im, array(
    'l' => array('gamma' => 0.7),
), IMAGE_EX_COLORSPACE_HSL);
imagejpeg($im, 'output/gamma-l1.jpg');

$im = imagecreatefromjpeg('images/mutzig.jpg');
imagecolorcorrect($im, array(
    'l' => array('gamma' => 1.4),
), IMAGE_EX_COLORSPACE_HSL);
imagejpeg($im, 'output/gamma-l2.jpg');
