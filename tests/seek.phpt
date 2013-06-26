--TEST--
Video seek test
--SKIPIF--
<?php 
	if(!function_exists('imagecreatetruecolor')) print 'skip GD not available';
?>
--FILE--
<?php

$folder = dirname(__FILE__);
$image = imagecreatetruecolor(852, 480);
$file_in = av_file_open("$folder/source-code.mkv", "rb");

$a_strm_in = av_stream_open($file_in, "audio");
$v_strm_in = av_stream_open($file_in, "video");

av_file_seek($file_in, 10);

$v_time = 0;
$a_time = 0;
if(av_stream_read_image($v_strm_in, $image, $v_time)) {
	printf("V: %.4f\n", $v_time);
}
if(av_stream_read_pcm($a_strm_in, $pcm, $a_time)) {
	printf("A: %.4f\n", $a_time);
}
if(av_stream_read_image($v_strm_in, $image, $v_time)) {
	printf("V: %.4f\n", $v_time);
}
if(av_stream_read_pcm($a_strm_in, $pcm, $a_time)) {
	printf("A: %.4f\n", $a_time);
}

// 30 seconds => beyond the actual duration of video
av_file_seek($file_in, 30);

$v_time = 0;
$a_time = 0;
if(av_stream_read_image($v_strm_in, $image, $v_time)) {
	printf("V: %.4f\n", $v_time);
}
if(av_stream_read_pcm($a_strm_in, $pcm, $a_time)) {
	printf("A: %.4f\n", $a_time);
}

// imprecise seek (stop at key frame)
av_file_seek($file_in, 10, false);

$v_time = 0;
$a_time = 0;
if(av_stream_read_image($v_strm_in, $image, $v_time)) {
	printf("V: %.4f\n", $v_time);
}
if(av_stream_read_pcm($a_strm_in, $pcm, $a_time)) {
	printf("A: %.4f\n", $a_time);
}

?>
--EXPECT--
V: 9.9690
A: 9.9830
V: 10.0100
A: 10.0050
V: 29.8220
A: 29.8870
V: 9.1760
A: 9.1510
