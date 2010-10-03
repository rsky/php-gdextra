--TEST--
imagehistgram216() function
--SKIPIF--
--FILE--
<?php
chdir(dirname(__FILE__));
$im = imagecreatefromjpeg('../examples/images/mosaic.jpg');
$histgram = imagehistgram216($im);
if (is_array($histgram) && count($histgram) === 216) {
    echo 'OK';
} else {
    echo 'NG';
}
?>
--EXPECT--
OK
