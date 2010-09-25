--TEST--
imagebmp() function
--SKIPIF--
--FILE--
<?php
chdir(dirname(__FILE__));
$im = imagecreatefromgif('../examples/rgb-4bit.gif');
if (!imagebmp($im, 'sample.bmp')) {
    exit;
}
ob_start();
$result = imagebmp($im);
$output = ob_get_clean();
echo ($result == true && md5($output) == md5_file('sample.bmp')) ? 'OK' : 'NG';
@unlink('sample.bmp');
?>
--EXPECT--
OK
