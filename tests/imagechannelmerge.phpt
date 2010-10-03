--TEST--
imagechannelmerge() function
--SKIPIF--
--FILE--
<?php
chdir(dirname(__FILE__));
$im = imagecreatefrompng('../examples/images/alpha-star.png');
$merged = imagechannelmerge(array($im, $im, $im));
if (imagesx($im) === imagesx($merged) && imagesy($im) === imagesy($merged)) {
    echo 'OK';
} else {
    echo 'NG';
}
?>
--EXPECT--
OK
