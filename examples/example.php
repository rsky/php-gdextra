<?php
extension_loaded('gdextra') || dl('gdextra.so') || exit(1);

// 32bits/pixel BMP will be created from the truecolor image which has alpha channel.
imagebmp_ex(imagecreatebymagick_ex('rgba-32bit.tif'), 'rgba-32bit.bmp');

// 24bits/pixel BMP will be created from the truecolor image.
imagebmp_ex(imagecreatebymagick_ex('rgb-24bit.tif'), 'rgb-24bit.bmp');

// 8bits/pixel BMP will be created from the indexed color image (> 4bits/pizel).
imagebmp_ex(imagecreatefromgif('rgba-8bit.gif'), 'rgb-8bit.bmp');

// 4bits/pixel BMP will be created from the indexed color image (<= 4bits/pixel).
imagebmp_ex(imagecreatefromgif('rgb-4bit.gif'), 'rgb-4bit.bmp');

// 1bit/pixel BMP will be created from the indexed color image (= 1bit/pixel).
imagebmp_ex(imagecreatefromwbmp('mono.wbmp'), 'mono.bmp');