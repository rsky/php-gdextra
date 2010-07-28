--TEST--
imagecreatefromstring_ex() function
--SKIPIF--
<?php if(!extension_loaded('gdextra')) die('skip extension gdextra is not loaded'); ?>
--FILE--
<?php
chdir(dirname(__FILE__));
$file = '../examples/rgb-24bit.tif';
$im = imagecreatefromstring_ex(file_get_contents($file));
$info = getimagesize($file);
if ($im && $info) {
    if ($info[0] == imagesx($im) && $info[1] == imagesy($im)) {
        echo 'OK';
    } else {
        echo 'NG';
    }
}
?>
--EXPECT--
OK
