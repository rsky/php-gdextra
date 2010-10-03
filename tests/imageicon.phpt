--TEST--
imageicon() function
--SKIPIF--
--FILE--
<?php
chdir(dirname(__FILE__));
$im = imagecreatefrompng('../examples/images/rgba-32x32.png');
if (!imageicon($im, 'icon.ico')) {
    exit;
}
ob_start();
$result = imageicon($im);
$output = ob_get_clean();
echo ($result == true && md5($output) == md5_file('icon.ico')) ? 'OK' : 'NG';
@unlink('icon.ico');
?>
--EXPECT--
OK
