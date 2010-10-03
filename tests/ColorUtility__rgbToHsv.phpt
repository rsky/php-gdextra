--TEST--
ColorUtility::rgbToHsv() member function
--SKIPIF--
--FILE--
<?php
$hsv = ColorUtility::rgbToHsv(255, 0, 0);
printf('hsv(%d,%d,%d)', $hsv[0], $hsv[1], $hsv[2]);
?>
--EXPECT--
hsv(0,100,100)
