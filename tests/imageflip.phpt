--TEST--
imageflip() function
--SKIPIF--
--FILE--
<?php
chdir(dirname(__FILE__));
$im = imagecreatefromjpeg('../examples/images/mosaic.jpg');
$width = imagesx($im);
$height = imagesy($im);
$x = floor($width * 2 / 3);
$y = floor($height * 2 / 3);
$c1b = imagecolorat($im, 0, 0);
$c2b = imagecolorat($im, $x, $y);

imageflip($im, IMAGE_EX_FLIP_BOTH);
$c1a = imagecolorat($im, $width - 1, $height - 1);
$c2a = imagecolorat($im, $width - 1 - $x, $height - $y - 1);
if ($c1b === $c1a && $c2b === $c2a) {
    echo 'OK';
} else {
    echo 'NG';
}
?>
--EXPECT--
OK
