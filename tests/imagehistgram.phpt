--TEST--
imagehistgram() function
--SKIPIF--
--FILE--
<?php
chdir(dirname(__FILE__));
$im = imagecreatefromjpeg('../examples/images/mosaic.jpg');
$histgram = imagehistgram($im);
if (is_array($histgram) && count($histgram) === 3 &&
    is_array($histgram[0]) && count($histgram[0]) == 256) {
    echo 'OK';
} else {
    echo 'NG';
}
?>
--EXPECT--
OK
