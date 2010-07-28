--TEST--
imageiconarray_ex() function (deprecated, use imageicon_ex() instead of this function)
--SKIPIF--
<?php if(!extension_loaded('gdextra')) die('skip extension gdextra is not loaded'); ?>
--FILE--
<?php
if (defined('E_STRICT')) {
    if (defined('E_DEPRECATED')) {
        error_reporting(E_ALL & ~(E_STRICT | E_DEPRECATED));
    } else {
        error_reporting(E_ALL & ~E_STRICT);
    }
} else {
    error_reporting(E_ALL);
}
chdir(dirname(__FILE__));
$images = array(imagecreatefrompng('../examples/rgba-16x16.png'),
                imagecreatefrompng('../examples/rgba-32x32.png'),
                imagecreatefrompng('../examples/rgba-48x48.png'));
if (!imageiconarray_ex($images, 'icons.ico')) {
    exit;
}
ob_start();
$result = imageiconarray_ex($images);
$output = ob_get_clean();
echo ($result == true && md5($output) == md5_file('icons.ico')) ? 'OK' : 'NG';
@unlink('icons.ico');
?>
--EXPECT--
OK
