--TEST--
imageicon_ex() function
--SKIPIF--
<?php if(!extension_loaded('gdextra')) die('skip extension gdextra is not loaded'); ?>
--FILE--
<?php
chdir(dirname(__FILE__));
$im = imagecreatefrompng('../examples/rgba-32x32.png');
if (!imageicon_ex($im, 'icon.ico')) {
    exit;
}
ob_start();
$result = imageicon_ex($im);
$output = ob_get_clean();
echo ($result == true && md5($output) == md5_file('icon.ico')) ? 'OK' : 'NG';
@unlink('icon.ico');
?>
--EXPECT--
OK
