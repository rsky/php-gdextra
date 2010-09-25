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
$file = '../examples/rgb-24bit.tif';
$im = imagecreatefromstring(file_get_contents($file));
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
