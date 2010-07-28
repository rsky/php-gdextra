<?php
extension_loaded('gdextra') || dl('gdextra.so') || exit(1);

// Read 32-bit PNG images.
$im16 = imagecreatefrompng('rgba-16x16.png');
$im32 = imagecreatefrompng('rgba-32x32.png');
$im48 = imagecreatefrompng('rgba-48x48.png');

// Create a 32-bit Icon.
imageiconarray_ex(array($im16, $im32, $im48), 'rgba.ico');

// Convert to 8-bit image.
imagesavealpha($im16, false);
imagesavealpha($im32, false);
imagesavealpha($im48, false);
imagetruecolortopalette($im16, true, 256);
imagetruecolortopalette($im32, true, 256);
imagetruecolortopalette($im48, true, 256);

// Create a 8-bit Icon.
imageiconarray_ex(array($im16, $im32, $im48), 'rgb.ico');
