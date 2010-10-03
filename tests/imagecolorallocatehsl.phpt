--TEST--
imagecolorallocatehsl() function
--SKIPIF--
--FILE--
<?php
$im = imagecreatetruecolor(1, 1);
$c = imagecolorallocatehsl($im, 0, 100, 50);
if (((255 << 16) | (0 << 8) | 0) === $c) {
    echo 'OK';
} else {
    printf('NG (%x)', $c);
}
?>
--EXPECT--
OK
