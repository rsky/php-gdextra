--TEST--
ColorUtility::rgbToHsl() member function
--SKIPIF--
--FILE--
<?php
$hsl = ColorUtility::rgbToHsl(255, 0, 0);
printf('hsl(%d,%d,%d)', $hsl[0], $hsl[1], $hsl[2]);
?>
--EXPECT--
hsl(0,100,50)
