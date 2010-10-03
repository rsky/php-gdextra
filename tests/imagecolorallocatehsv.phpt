--TEST--
imagecolorallocatehsv() function
--SKIPIF--
--FILE--
<?php
$im = imagecreatetruecolor(1, 1);
$c = imagecolorallocatehsv($im, 0, 100, 100);
if (((255 << 16) | (0 << 8) | 0) === $c) {
    echo 'OK';
} else {
    printf('NG (%x)', $c);
}
?>
--EXPECT--
OK
