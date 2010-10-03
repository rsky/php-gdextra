--TEST--
imageclone() function
--SKIPIF--
--FILE--
<?php
chdir(dirname(__FILE__));
$im = imagecreatefrompng('../examples/images/alpha-star.png');
$cloned = imageclone($im);
if (imagesx($im) === imagesx($cloned) && imagesy($im) === imagesy($cloned)) {
    echo 'OK';
} else {
    echo 'NG';
}
?>
--EXPECT--
OK
