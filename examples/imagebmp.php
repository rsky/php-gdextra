<?php
include dirname(__FILE__) . '/_init.php';

// 32bits/pixel BMP will be created from the truecolor image which has alpha channel.
imagebmp(imagecreatefrompng('images/rgba-32bit.png'), 'output/rgba-32bit.bmp');

// 24bits/pixel BMP will be created from the truecolor image.
imagebmp(imagecreatefrompng('images/rgb-24bit.png'), 'output/rgb-24bit.bmp');

// 8bits/pixel BMP will be created from the indexed color image (> 4bits/pizel).
imagebmp(imagecreatefromgif('images/rgba-8bit.gif'), 'output/rgb-8bit.bmp');

// 4bits/pixel BMP will be created from the indexed color image (<= 4bits/pixel).
imagebmp(imagecreatefromgif('images/rgb-4bit.gif'), 'output/rgb-4bit.bmp');

// 1bit/pixel BMP will be created from the indexed color image (= 1bit/pixel).
imagebmp(imagecreatefromwbmp('images/mono.wbmp'), 'output/mono.bmp');
