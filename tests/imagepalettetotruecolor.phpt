--TEST--
imagepalettetotruecolor() function
--SKIPIF--
--FILE--
<?php
chdir(dirname(__FILE__));
$im = imagecreatefromgif('../examples/images/rgb-4bit.gif');
if (!imageistruecolor($im)) {
    if (imagepalettetotruecolor($im)) {
        if (imageistruecolor($im)) {
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
