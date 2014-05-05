--TEST--
MP4 test
--SKIPIF--
<?php 
	if(!function_exists('imagecreatetruecolor')) print 'skip GD not available';
?>
--FILE--
<?php

require("helpers.php");

$folder = dirname(__FILE__);
$width = 640;
$height = 480;
$frame_rate = 24;
$length = 10;

// open file for writing
$file_out = av_file_open("$folder/test.mp4", "w");
$v_strm_out = av_stream_open($file_out, "video", array( "width" => $width, "height" => $height, "frame_rate" => $frame_rate, "bit_rate" => 1024 * 1000, "gop" => 0));
$a_strm_out = av_stream_open($file_out, "audio");

if(!$file_out || !$v_strm_out || !$a_strm_out) die();

$a_time = 0;
$v_time = 0;

$animation = new BouncingBallAnimation($width, $height, $frame_rate);
$tone = new SineWave;

while($v_time < $length && $a_time < $length) {
	if($v_time < $a_time) {
		// write to video stream
		$animation->write($v_strm_out, $v_time);
		$animation->advance();
	} else {
		// write to audio stream
		$tone->write($a_strm_out, $a_time);
	}
}

// close file
av_stream_close($v_strm_out);
av_stream_close($a_strm_out);
av_file_close($file_out);

// open file for reading
$file_in = av_file_open("$folder/test.mp4", "w");
$v_strm_in = av_stream_open($file_in, "video");
$a_strm_in = av_stream_open($file_in, "audio");

$image1 = $animation->reset();
$image2 = imagecreatetruecolor($width, $height);
$pcm1 = "";
$len1 = 0;

$a_time = 0;
$v_time = 0;

while($v_time < $length && $a_time < $length) {
	if($v_time < $a_time) {
		// read from video stream
		if(!av_stream_read_image($v_strm_in, $image2, $v_time)) {
			die("Cannot read from video stream\n");
		}
		$diff = compareImage($image1, $image2);
		$image1 = $animation->advance();
		echo "Picture diff: $diff\n";
	} else {
		// read from audio stream
		if(!av_stream_read_pcm($a_strm_in, $pcm2, $a_time)) {
			die("Cannot read from audio stream\n");
		}
		$len2 = strlen($pcm2);
		while($len1 < $len2) {
			$pcm1 .= $tone->generate();
		}
		$diff = comparePCM($pcm1, $pcm2);
		echo "Audio diff: $diff\n";
		$pcm1 = substr($pcm1, $len2);
		$len1 -= $len2;
	}
}

// close file
av_stream_close($v_strm_in);
av_stream_close($a_strm_in);
av_file_close($file_in);

?>
--EXPECT--
