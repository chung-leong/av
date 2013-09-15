--TEST--
Subtitle transfer test
--SKIPIF--
<?php 
	if(!function_exists('imagecreatetruecolor')) print 'skip GD not available';
?>
--FILE--
<?php

$folder = dirname(__FILE__);
$image = imagecreatetruecolor(852, 480);
$file_in = av_file_open("$folder/source-code.mkv", "r");
$stat_in = av_file_stat($file_in);
$file_out = av_file_open("$folder/source-code-sub.mkv", "w");

$s_strm_in = av_stream_open($file_in, "subtitle");
$a_strm_in = av_stream_open($file_in, "audio");
$v_strm_in = av_stream_open($file_in, "video");

$subtitle_stat = $stat_in['streams']['subtitle'];
$subtitle_codec = $subtitle_stat['codec'];

$s_strm_out = av_stream_open($file_out, "subtitle", $subtitle_stat );
$a_strm_out = av_stream_open($file_out, "audio");
$v_strm_out = av_stream_open($file_out, "video", array( "width" => imagesx($image), "height" => imagesy($image) ));

if(!$file_in || !$file_out || !$a_strm_in || !$a_strm_out || !$v_strm_in || !$v_strm_out) {
	die();
}

$v_time = 0;
$a_time = 0;
$s_time = 0;

while(!av_file_eof($file_in)) {
	$min_time = min($v_time, $a_time, $s_time);
	if($min_time == $v_time) {
		if(av_stream_read_image($v_strm_in, $image, $v_time)) {
			av_stream_write_image($v_strm_out, $image, $v_time);
		} else {
			$v_time = INF;
		}
	} else if($min_time == $a_time) {
		if(av_stream_read_pcm($a_strm_in, $pcm, $a_time)) {
			av_stream_write_pcm($a_strm_out, $pcm, $a_time);
		} else {
			$a_time = INF;
		}
	} else {
		if(av_stream_read_subtitle($s_strm_in, $subtitle, $s_time)) {
			av_stream_write_subtitle($s_strm_out, $subtitle, $s_time);
			echo "[SUBTITLE]\n";
		} else {
			$s_time = INF;
		}
	}
	if($v_time > 60) {
		break;
	}
}

?>
--EXPECT--
[SUBTITLE]
[SUBTITLE]
[SUBTITLE]
[SUBTITLE]
[SUBTITLE]
[SUBTITLE]
[SUBTITLE]
[SUBTITLE]
[SUBTITLE]
