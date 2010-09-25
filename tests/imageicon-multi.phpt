--TEST--
imageicon() function with multiple images
--SKIPIF--

--FILE--
<?php
chdir(dirname(__FILE__));
$images = array(imagecreatefrompng('../examples/rgba-16x16.png'),
                imagecreatefrompng('../examples/rgba-32x32.png'),
                imagecreatefrompng('../examples/rgba-48x48.png'));
if (!imageicon($images, 'icons.ico')) {
    exit;
}
ob_start();
$result = imageicon($images);
$output = ob_get_clean();
echo ($result == true && md5($output) == md5_file('icons.ico')) ? 'OK' : 'NG';
@unlink('icons.ico');
?>
--EXPECT--
OK
