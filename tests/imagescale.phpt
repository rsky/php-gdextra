--TEST--
imagescale() function
--SKIPIF--
--FILE--
<?php
chdir(dirname(__FILE__));
$im = imagecreatefromjpeg('../examples/images/mutzig.jpg');
$resized = imagescale($im, 200, 200, IMAGE_EX_SCALE_STRETCH);
if (200 === imagesx($resized) && 200 === imagesy($resized)) {
    echo 'OK';
} else {
    echo 'NG';
}
?>
--EXPECT--
OK
