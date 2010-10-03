--TEST--
imagecolorallocatecmyk() function
--SKIPIF--
--FILE--
<?php
$im = imagecreatetruecolor(1, 1);
$c = imagecolorallocatecmyk($im, 100, 100, 0, 0);
if (((0 << 16) | (0 << 8) | 255) === $c) {
    echo 'OK';
} else {
    printf('NG (%x)', $c);
}
?>
--EXPECT--
OK
