<?php

$folder = dirname(__FILE__);
$image = imagecreatetruecolor(852, 480);
$file_in = av_file_open("$folder/source-code.mov", "r");
$file_out = av_file_open("$folder/source-code-output.gif", "w");

$v_strm_in = av_stream_open($file_in, "video");
$v_strm_out = av_stream_open($file_out, "video", array( "width" => imagesx($image), "height" => imagesy($image)));

$v_time = 0;
while(!av_file_eof($file_in)) {
	if(av_stream_read_image($v_strm_in, $image, $v_time)) {
		av_stream_write_image($v_strm_out, $image, $v_time);
	}
	if($v_time > 5) {
		break;
	}
}

av_stream_close($v_strm_out);
av_file_close($file_out);

av_stream_close($v_strm_in);
av_file_close($file_in);

?>