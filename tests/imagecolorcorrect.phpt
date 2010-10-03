--TEST--
imagecolorcorrect() function
--SKIPIF--
--FILE--
<?php
chdir(dirname(__FILE__));
$im = imagecreatefromjpeg('../examples/images/mutzig.jpg');
if (imagecolorcorrect($im, array('gamma' => 1.8))) {
    echo 'OK';
} else {
    echo 'NG';
}
?>
--EXPECT--
OK
