--TEST--
Live stream test
--SKIPIF--
<?php 
	if(!function_exists('imagecreatetruecolor')) print 'skip GD not available'; 
	if(!in_array('wmv3', av_get_decoders())) print 'skip WMV decoder not avilable';
?>
--FILE--
<?php

$url = "mmsh://a1709.l1856953708.c18569.g.lm.akamaistream.net/D/1709/18569/v0001/reflector:53708";
$file = av_file_open($url, "r");
$stat = av_file_stat($file);
$image = imagecreatetruecolor(640, 360);
$strm = av_stream_open($file, "video");
av_stream_read_image($strm, $image, $time);

echo $stat['format'], "\n";
echo $stat['duration'], "\n";
echo $stat['metadata']['title'], "\n";
echo $stat['metadata']['artist'], "\n";

?>
--EXPECT--
asf
INF
NASA-ISS-push Streaming ISS Video
Johnson Space Center
