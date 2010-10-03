--TEST--
ColorUtility::rgbToCmyk() member function
--SKIPIF--
--FILE--
<?php
$cmyk = ColorUtility::rgbToCmyk(0, 0, 255);
printf('cmyk(%d,%d,%d,%d)', $cmyk[0], $cmyk[1], $cmyk[2], $cmyk[3]);
?>
--EXPECT--
cmyk(100,100,0,0)
