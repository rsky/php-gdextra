--TEST--
imagealphamask() function
--SKIPIF--
--FILE--
<?php
chdir(dirname(__FILE__));
$im = imagecreatefrompng('../examples/images/rgb-24bit.png');
$mask = imagecreatefrompng('../examples/images/alpha-star.png');
if (imagealphamask($im, $mask, IMAGE_EX_MASK_SET, IMAGE_EX_POSITION_TOP_LEFT)) {
    $c = imagecolorat($im, 0, 0);
    if (127 === ($c >> 24)) {
        echo 'OK';
    } else {
        printf('%x', $c);
    }
} else {
    echo'NG';
}
?>
--EXPECT--
OK
