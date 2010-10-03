--TEST--
ColorUtility::cmykToRgb() member function
--SKIPIF--
--FILE--
<?php
$rgb = ColorUtility::cmykToRgb(100, 100, 0, 0);
printf('rgb(%d,%d,%d)', $rgb[0], $rgb[1], $rgb[2]);
?>
--EXPECT--
rgb(0,0,255)
