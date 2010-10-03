--TEST--
ColorUtility::getSvgColorTable() member function
--SKIPIF--
--FILE--
<?php
$table = ColorUtility::getSvgColorTable();
$green = $table['green'];
printf('rgb(%d,%d,%d)', $green[0], $green[1], $green[2]);
?>
--EXPECT--
rgb(0,128,0)
