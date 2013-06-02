--TEST--
Video file details test
--FILE--
<?php

$folder = dirname(__FILE__);
$file_in = av_file_open("$folder/source-code.mov", "r");
$info = av_file_stat($file_in);
av_file_close($file_in);

extract($info);
echo "$format\n";
echo "$bit_rate\n";
foreach($metadata as $name => $field) {
	echo "$name: $field\n";
}
foreach($streams as $key => $stream) {
	echo "stream $key\n";
	extract($stream);
	echo "$type\n";
	echo "$codec\n";
	echo "$bit_rate\n";
	echo "$duration\n";
	echo "$frame_rate\n";
	echo "$height\n";
	echo "$width\n";
}

?>
--EXPECT--
mov
2681791
major_brand: qt  
minor_version: 537199360
compatible_brands: qt  
creation_time: 2011-04-01 03:36:02
title: "Source Code" Clip
title-eng: "Source Code" Clip
artist: Yahoo! Movies
artist-eng: Yahoo! Movies
copyright: © 2011, Summit Entertainment, LLC.
copyright-eng: © 2011, Summit Entertainment, LLC.
stream 0
audio
aac
142077
29.866666666667
0
0
0
stream audio
audio
aac
142077
29.866666666667
0
0
0
stream 1
video
h264
2533818
29.863196529863
23.976
480
852
stream video
video
h264
2533818
29.863196529863
23.976
480
852
