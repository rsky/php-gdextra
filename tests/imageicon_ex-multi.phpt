--TEST--
imageicon_ex() function with multiple images
--SKIPIF--
<?php if(!extension_loaded('gdextra')) die('skip extension gdextra is not loaded'); ?>
--FILE--
<?php
chdir(dirname(__FILE__));
$images = array(imagecreatefrompng('../examples/rgba-16x16.png'),
                imagecreatefrompng('../examples/rgba-32x32.png'),
                imagecreatefrompng('../examples/rgba-48x48.png'));
if (!imageicon_ex($images, 'icons.ico')) {
    exit;
}
ob_start();
$result = imageicon_ex($images);
$output = ob_get_clean();
echo ($result == true && md5($output) == md5_file('icons.ico')) ? 'OK' : 'NG';
@unlink('icons.ico');
?>
--EXPECT--
OK
