--TEST--
imagecreatebymagick() function
--SKIPIF--
<?php
if (!function_exists('imagecreatebymagick')) {
    die('skip function imagecreatebymagick() is not enabled');
}
?>
--FILE--
<?php
chdir(dirname(__FILE__));
$file = '../examples/images/rgb-24bit.tif';
$im = imagecreatebymagick($file);
$info = getimagesize($file);
if ($im && $info) {
    if ($info[0] == imagesx($im) && $info[1] == imagesy($im) && $info[2] = IMAGETYPE_TIFF_MM) {
        echo 'OK';
    } else {
        echo 'NG';
    }
}
?>
--EXPECT--
OK
