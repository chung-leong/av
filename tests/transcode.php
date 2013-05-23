<?php

$folder = dirname(__FILE__);
$image = imagecreatetruecolor(852, 480);
$file_in = av_file_open("$folder/source-code.mov", "rb");
$file_out = av_file_open("$folder/source-code-output.webm", "wb");

//$a_strm_in = av_stream_open($file_in, "audio");
$v_strm_in = av_stream_open($file_in, "video");

//$a_strm_out = av_stream_open($file_out, "audio");
$v_strm_out = av_stream_open($file_out, "video", array( "width" => imagesx($image), "height" => imagesy($image)));

$v_time = 0;
$a_time = 0;
while(!av_file_eof($file_in)) {
	if($v_time < $a_time) {
		av_stream_read_image($v_strm_in, $image, $v_time);
		if($v_time > 0) {
			av_stream_write_image($v_strm_out, $image, $v_time);
		}
	} else {
		av_stream_read_pcm($a_strm_in, $pcm, $a_time);
		//av_stream_write_pcm($a_strm_out, $pcm, $a_time);
	}
}

//av_stream_close($a_strm_out);
av_stream_close($v_strm_out);
av_file_close($file_out);

av_stream_close($a_strm_in);
av_stream_close($v_strm_in);
av_file_close($file_in);

?>