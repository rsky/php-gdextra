--TEST--
imagecreatebymagick() function with a blob
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
$im = imagecreatebymagick(file_get_contents($file), true);
$info = getimagesize($file);
if ($im && $info) {
    if ($info[0] == imagesx($im) && $info[1] == imagesy($im)) {
        echo 'OK';
    } else {
        echo 'NG';
    }
}
?>
--EXPECT--
OK
