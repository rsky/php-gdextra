--TEST--
ColorUtility::hslToRgb() member function
--SKIPIF--
--FILE--
<?php
$rgb = ColorUtility::hslToRgb(0, 100, 50);
printf('rgb(%d,%d,%d)', $rgb[0], $rgb[1], $rgb[2]);
?>
--EXPECT--
rgb(255,0,0)
