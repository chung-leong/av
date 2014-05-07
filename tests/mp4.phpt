--TEST--
MP4 test
--SKIPIF--
<?php 
	if(!function_exists('imagecreatetruecolor')) print 'skip GD not available';
	if(!in_array('mpeg4', av_get_encoders())) print 'skip MP4 encoder not avilable';
?>
--FILE--
<?php

require("helpers.php");

$folder = dirname(__FILE__);
$filename = "test.mp4";

$testVideo = new TestVideo("$folder/$filename", 640, 480, 24, 5.0);
$testVideo->create();
$testVideo->verify();
$testVideo->delete();

echo "OK\n";

?>
--EXPECT--
OK
