--TEST--
ColorUtility::parseCssColor() member function
--SKIPIF--
--FILE--
<?php
$rgba = ColorUtility::parseCssColor('rgba(10,20,30,0.4)');
printf('rgba(%d,%d,%d,%0.3f)', $rgba[0], $rgba[1], $rgba[2], $rgba[3]);
?>
--EXPECT--
rgba(10,20,30,0.400)
