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
$filename = "test.mp4";
$width = 640;
$height = 480;
$frame_rate = 24;
$length = 5;

// open file for writing
$file_out = av_file_open("$folder/$filename", "w");
$v_strm_out = av_stream_open($file_out, "video", array( "width" => $width, "height" => $height, "frame_rate" => $frame_rate, "bit_rate" => 1024 * 1000, "gop" => 0));
$a_strm_out = av_stream_open($file_out, "audio");

if(!$file_out || !$v_strm_out || !$a_strm_out) die();

$a_time = 0;
$v_time = 0.5 / $frame_rate;

$animation = new BouncingBallAnimation($width, $height, $frame_rate);
$tone = new SineWave;

while($v_time < $length && $a_time < $length) {
	if($v_time < $a_time) {
		// write to video stream
		if(isset($image1)) {
			imagepng($image1, "$folder/$filename.$v_time.correct.png");
		}
		$animation->write($v_strm_out, $v_time);
		$image1 = $animation->advance();
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
$file_in = av_file_open("$folder/$filename", "r");
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
		if(av_stream_read_image($v_strm_in, $image2, $v_time)) {
			$diff = compareImage($image1, $image2);
			if($diff > 1) {
				if(abs($length - $v_time) > 0.1) {
					imagepng($image1, "$folder/$filename.$v_time.correct.png");
					imagepng($image2, "$folder/$filename.$v_time.incorrect.png");
					die("Frame at $v_time is different\n");
				}
			}
			$image1 = $animation->advance();
		} else {
			if(abs($length - $v_time) > 0.1) {
				die("Cannot read from video stream\n");
			} else {
				$v_time = INF;
			}
		}
	} else {
		// read from audio stream
		if(av_stream_read_pcm($a_strm_in, $pcm2, $a_time)) {
			$len2 = strlen($pcm2);
			if($a_time < 0) {
				$offset = 0;
				while($len2 > 0 && $a_time < 0) {
					$len2 -= 8;
					$a_time += 1 / 44100;
					$offset += 8;
				}
				if($len2 == 0) {
					continue;
				}
				$pcm2 = substr($pcm2, $offset);
				$array2 = unpack("f*", $pcm2);
			}
			while($len1 < $len2) {
				$pcm1 .= $tone->generate();
				$len1 = strlen($pcm1);
			}
			$diff = comparePCM($pcm1, $pcm2);
			if($diff > 1) {
				if(abs($length - $a_time) > 0.1) {
					die("Audio at $a_time is different\n");
				}
			}
			$pcm1 = substr($pcm1, $len2);
			$len1 -= $len2;
		} else {
			if(abs($length - $a_time) > 0.1) {
				die("Cannot read from audio stream\n");
			} else {
				$a_time = INF;
			}
		}
	}
}

// close file
av_stream_close($v_strm_in);
av_stream_close($a_strm_in);
av_file_close($file_in);

echo "OK\n";

?>
--EXPECT--
OK
