<?php
include dirname(__FILE__) . '/_init.php';

// Read 32-bit PNG images.
$im16 = imagecreatefrompng('images/rgba-16x16.png');
$im32 = imagecreatefrompng('images/rgba-32x32.png');
$im48 = imagecreatefrompng('images/rgba-48x48.png');

// Create a 32-bit Icon.
imageicon(array($im16, $im32, $im48), 'output/rgba.ico');

// Convert to 8-bit image.
imagesavealpha($im16, false);
imagesavealpha($im32, false);
imagesavealpha($im48, false);
imagetruecolortopalette($im16, true, 256);
imagetruecolortopalette($im32, true, 256);
imagetruecolortopalette($im48, true, 256);

// Create a 8-bit Icon.
imageicon(array($im16, $im32, $im48), 'output/rgb.ico');
