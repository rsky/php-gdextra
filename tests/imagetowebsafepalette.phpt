--TEST--
imagetowebsafepalette() function
--SKIPIF--
--FILE--
<?php
chdir(dirname(__FILE__));
$im = imagecreatefrompng('../examples/images/rgb-24bit.png');
if (imageistruecolor($im)) {
    if (imagetowebsafepalette($im)) {
        if (!imageistruecolor($im)) {
            echo 'OK';
        } else {
            echo 'unexpected result';
        }
    } else {
        echo 'conversion failure';
    }
} else {
    echo 'unexpected image';
}
?>
--EXPECT--
OK
