<?php
include dirname(__FILE__) . '/_init.php';

$im = imagecreatefromjpeg('images/mutzig.jpg');
imagecolorcorrect($im, array(
    'l' => array('levels' => array(51, 204)),
), IMAGE_EX_COLORSPACE_HSL);
imagejpeg($im, 'output/levels-l1.jpg');

$im = imagecreatefromjpeg('images/mutzig.jpg');
imagecolorcorrect($im, array(
    'l' => array('levels' => array(0, 255, 51, 204)),
), IMAGE_EX_COLORSPACE_HSL);
imagejpeg($im, 'output/levels-l2.jpg');
