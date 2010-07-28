--TEST--
imagebmp_ex() function
--SKIPIF--
<?php if(!extension_loaded('gdextra')) die('skip extension gdextra is not loaded'); ?>
--FILE--
<?php
chdir(dirname(__FILE__));
$im = imagecreatebymagick_ex('../examples/rgb-24bit.tif');
if (!imagebmp_ex($im, 'sample.bmp')) {
    exit;
}
ob_start();
$result = imagebmp_ex($im);
$output = ob_get_clean();
echo ($result == true && md5($output) == md5_file('sample.bmp')) ? 'OK' : 'NG';
@unlink('sample.bmp');
?>
--EXPECT--
OK
