<?php

$folder = dirname(__FILE__);
$image = imagecreatetruecolor(852, 480);
$file_in = av_file_open("$folder/source-code.mov", "rb");

$a_strm_in = av_stream_open($file_in, "audio");
$v_strm_in = av_stream_open($file_in, "video");

av_file_seek($file_in, 10);

$v_time = 0;
$a_time = 0;
if(av_stream_read_image($v_strm_in, $image, $v_time)) {
	imagepng($image, "$folder/source-code-seek-point.png");
	echo "$v_time";
}

av_stream_close($a_strm_in);
av_stream_close($v_strm_in);
av_file_close($file_in);

?>