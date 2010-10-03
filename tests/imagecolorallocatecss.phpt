--TEST--
imagecolorallocatecss() function
--SKIPIF--
--FILE--
<?php
$im = imagecreatetruecolor(1, 1);
$c = imagecolorallocatecss($im, 'green');
if (((0 << 16) | (128 << 8) | 0) === $c) {
    echo 'OK';
} else {
    printf('NG (%x)', $c);
}
?>
--EXPECT--
OK
