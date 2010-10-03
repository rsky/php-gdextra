--TEST--
ColorUtility::hsvToRgb() member function
--SKIPIF--
--FILE--
<?php
$rgb = ColorUtility::hsvToRgb(0, 100, 100);
printf('rgb(%d,%d,%d)', $rgb[0], $rgb[1], $rgb[2]);
?>
--EXPECT--
rgb(255,0,0)
