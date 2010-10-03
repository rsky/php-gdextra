--TEST--
imagecarve() function
--SKIPIF--
<?php
if (!function_exists('imagecarve')) {
    die('skip function imagecarve() is not enabled');
}
?>
--FILE--
<?php
chdir(dirname(__FILE__));
$im = imagecreatefromjpeg('../examples/images/mutzig.jpg');
$resized = imagecarve($im, 200, 200);
if (200 === imagesx($resized) && 200 === imagesy($resized)) {
    echo 'OK';
} else {
    echo 'NG';
}
?>
--EXPECT--
OK
