--TEST--
imagechannelextract() function
--SKIPIF--
--FILE--
<?php
chdir(dirname(__FILE__));
$im = imagecreatefromjpeg('../examples/images/mosaic.jpg');
$channels = imagechannelextract($im);
if (is_array($channels)) {
    echo 'OK';
} else {
    echo 'NG';
}
?>
--EXPECT--
OK
