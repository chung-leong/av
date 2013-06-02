--TEST--
MP3 as source test
--SKIPIF--
<?php 
	if(!function_exists('imagecreatetruecolor')) { 
		print 'skip GD not available'; 
	} else if(!function_exists('imagecreatefromjpeg')) { 
		print 'skip JPEG function not available';
	}
?>
--FILE--
<?php

$folder = dirname(__FILE__);
$image = imagecreatefromjpeg("$folder/upior-w-operze.jpg");
$file_in = av_file_open("$folder/o-tyle-prosze-cie.mp3", "r");
$stat_in = av_file_stat($file_in);
$duration_in = $stat_in['duration'];
$file_out1 = av_file_open("$folder/o-tyle-prosze-cie.mp4", "w");
$file_out2 = av_file_open("$folder/o-tyle-prosze-cie.webm", "w");

$frame_rate = 12;

$a_strm_in = av_stream_open($file_in, "audio");

$a_strm_out1 = av_stream_open($file_out1, "audio");
$v_strm_out1 = av_stream_open($file_out1, "video", array( "width" => imagesx($image), "height" => imagesy($image), "frame_rate" => $frame_rate ));

$a_strm_out2 = av_stream_open($file_out2, "audio");
$v_strm_out2 = av_stream_open($file_out2, "video", array( "width" => imagesx($image), "height" => imagesy($image), "frame_rate" => $frame_rate ));

$v_time = 0;
$a_time = 0;
while(!av_file_eof($file_in)) {
	if($v_time < $a_time || !isset($a_strm_in)) {
		av_stream_write_image($v_strm_out1, $image, $v_time);
		av_stream_write_image($v_strm_out2, $image, $v_time);
		$v_time += (1 / $frame_rate);
	} else {
		if(av_stream_read_pcm($a_strm_in, $pcm, $a_time)) {
			av_stream_write_pcm($a_strm_out1, $pcm, $a_time);
			av_stream_write_pcm($a_strm_out2, $pcm, $a_time);
		} else {
			break;
		}
	}
}

av_file_close($file_out1);
av_stream_close($a_strm_out1);
av_stream_close($v_strm_out1);

av_file_close($file_out2);
av_stream_close($a_strm_out2);
av_stream_close($v_strm_out2);

av_file_close($file_in);
av_stream_close($a_strm_in);

printf("%.4f %.4f\n", $duration_in, $a_time);

?>
--EXPECT--
257.5732 257.5151
