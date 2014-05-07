--TEST--
Issue #12: Read from write stream test
--SKIPIF--
<?php 
	if(!in_array('mpeg1video', av_get_encoders())) print 'skip MPEG encoder not avilable';
?>
--FILE--
<?php

$folder = dirname(__FILE__);
$filename = "test.avi";
$path = "$folder/$filename";

$file = av_file_open($path, "w");
$videoStream = av_stream_open($file, "video", array( "width" => 100, "height" => 100, "frame_rate" => 12, "bit_rate" => 1024 * 1000, "gop" => 0, "encoder" => "mpeg1video"));

$image = imagecreatetruecolor(50, 50);
av_stream_read_image($videoStream, $image, $videoTime);

?>
--EXPECTREGEX--
.*Warning.*Not a readable stream.*
