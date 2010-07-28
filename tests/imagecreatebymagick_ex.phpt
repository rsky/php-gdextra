--TEST--
imagecreatebymagick_ex() function
--SKIPIF--
<?php if(!extension_loaded('gdextra')) die('skip extension gdextra is not loaded'); ?>
--FILE--
<?php
chdir(dirname(__FILE__));
$file = '../examples/rgb-24bit.tif';
$im = imagecreatebymagick_ex($file);
$info = getimagesize($file);
if ($im && $info) {
    if ($info[0] == imagesx($im) && $info[1] == imagesy($im) && $info[2] = IMAGETYPE_TIFF_MM) {
        echo 'OK';
    } else {
        echo 'NG';
    }
}
?>
--EXPECT--
OK
