--TEST--
Subtitle test
--FILE--
<?php

$folder = dirname(__FILE__);
$file_in = av_file_open("$folder/source-code.mkv", "r");
$stat_in = av_file_stat($file_in);
$s_strm_in = av_stream_open($file_in, "subtitle");

if(!$file_in || !$s_strm_in) {
	die();
}

while(!av_file_eof($file_in)) {
	if(av_stream_read_subtitle($s_strm_in, $subtitle, $s_time)) {			
		$rects = $subtitle['rects'];
		foreach($rects as $rect) {
			$ass = $rect['ass'];
			echo $ass;
		}
	} else {
		break;
	}
}

?>
--EXPECT--
Dialogue: 0,0:00:00.36,0:00:04.14,Default,,0,0,0,,What would you do if you know if have less than eight minutes to live?
Dialogue: 0,0:00:04.78,0:00:05.90,Default,,0,0,0,,I don't know
Dialogue: 0,0:00:06.12,0:00:08.14,Default,,0,0,0,,I would make those seconds {\i1}count{\i0}
Dialogue: 0,0:00:09.02,0:00:10.70,Default,,0,0,0,,I would call my dad
Dialogue: 0,0:00:12.38,0:00:14.00,Default,,0,0,0,,I would hear his voice
Dialogue: 0,0:00:14.44,0:00:18.14,Default,,0,0,0,,and I will tell him that I'm sorry
Dialogue: 0,0:00:21.04,0:00:23.04,Default,,0,0,0,,
Dialogue: 0,0:00:22.46,0:00:24.48,Default,,0,0,0,,Tell me everything is going to be okay
Dialogue: 0,0:00:25.56,0:00:28.60,Default,,0,0,0,,Everything is going to be okay
