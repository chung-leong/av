<?php

$folder = dirname(__FILE__);
$image = imagecreatetruecolor(852, 480);
$file_in = av_file_open("$folder/source-code.mov", "rb");
$file_out = av_file_open("$folder/source-code-output.webm", "wb");

$a_strm_in = av_stream_open($file_in, "audio");
$v_strm_in = av_stream_open($file_in, "video");

$a_strm_out = av_stream_open($file_out, "audio");
$v_strm_out = av_stream_open($file_out, "video", array( "width" => imagesx($image), "height" => imagesy($image)));

$v_time = 0;
$a_time = 0;
while(!av_file_eof($file_in)) {
	if($v_time < $a_time || !$a_strm_in) {
		if(av_stream_read_image($v_strm_in, $image, $v_time)) {
			av_stream_write_image($v_strm_out, $image, $v_time);
			echo "V: $v_time\n";
		} else {
			$v_time = INF;
		}
	} else {
		if(av_stream_read_pcm($a_strm_in, $pcm, $a_time)) {
			av_stream_write_pcm($a_strm_out, $pcm, $a_time);
			echo "A: $a_time\n";
		} else {
			$a_time = INF;
		}
	}
	if($v_time > 1) {
		break;
	}
}

av_stream_close($a_strm_out);
av_stream_close($v_strm_out);
av_file_close($file_out);

av_stream_close($a_strm_in);
av_stream_close($v_strm_in);
av_file_close($file_in);

?>