--TEST--
OGG test
--SKIPIF--
<?php 
	if(!in_array('vorbis', av_get_encoders())) print 'skip OGG encoder not avilable';
?>
--FILE--
<?php

require("helpers.php");

$folder = dirname(__FILE__);
$filename = "test.ogg";

$testVideo = new TestVideo("$folder/$filename", 640, 480, 24, 5.0);
$testVideo->setVideoCodec(false);
$testVideo->create();
$testVideo->verify();
$testVideo->delete();

echo "OK\n";

?>
--EXPECT--
OK
