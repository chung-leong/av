--TEST--
Video file details test
--FILE--
<?php

$folder = dirname(__FILE__);
$file_in = av_file_open("$folder/source-code.mkv", "r");
$info = av_file_stat($file_in);
av_file_close($file_in);

extract($info);
echo "$format\n";
foreach($metadata as $name => $field) {
	echo "$name: $field\n";
}
foreach($streams as $id => $stream) {
	if(is_int($id)) {
		echo "STREAM #$id:\n";
		foreach($stream as $key => $value) {
			if(is_scalar($value) && $key != 'subtitle_header') {
				echo "$key = $value\n";
			}
		}
	}
}

?>
--EXPECT--
matroska
title: "Source Code" Clip
MAJOR_BRAND: qt  
MINOR_VERSION: 537199360
COMPATIBLE_BRANDS: qt  
COPYRIGHT-eng: © 2011, Summit Entertainment, LLC.
TITLE-eng: "Source Code" Clip
ARTIST: Yahoo! Movies
ARTIST-eng: Yahoo! Movies
COPYRIGHT: © 2011, Summit Entertainment, LLC.
ENCODER: Lavf55.3.100
STREAM #0:
type = video
codec = h264
codec_name = H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10
bit_rate = 0
duration = 29.908
frame_rate = 23.976
height = 480
width = 852
STREAM #1:
type = audio
codec = aac
codec_name = AAC (Advanced Audio Coding)
bit_rate = 0
duration = 29.908
STREAM #2:
type = subtitle
codec = ass
codec_name = SSA (SubStation Alpha) subtitle
bit_rate = 0
duration = 29.908
