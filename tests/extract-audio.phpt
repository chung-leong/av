--TEST--
Audio extraction test
--FILE--
<?php

$folder = dirname(__FILE__);
$file_in = av_file_open("$folder/source-code.mkv", "r");
$stat_in = av_file_stat($file_in);
$file_out = av_file_open("$folder/source-code.ogg", "w");

$a_strm_in = av_stream_open($file_in, "audio");
$a_strm_out = av_stream_open($file_out, "audio");
$a_time = 0;

if($file_in) {
	while(!av_file_eof($file_in)) {
		if(av_stream_read_pcm($a_strm_in, $pcm, $a_time)) {
			av_stream_write_pcm($a_strm_out, $pcm);
		} else {
			break;
		}
	}
}

av_stream_close($a_strm_out);
av_file_close($file_out);

printf("%.4f\n", $a_time);

?>
--EXPECT--
29.8870
