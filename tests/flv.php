<?php

$start = microtime(true);
$folder = dirname(__FILE__);
$image = imagecreatetruecolor(800, 600);
$file_in = av_file_open("$folder/socha-card-wipe.flv", "r");
$stat_in = av_file_stat($file_in);
$file_out1 = av_file_open("$folder/socha-card-wipe-output1.flv", "w" );
$file_out2 = av_file_open("$folder/socha-card-wipe-output2.flv", "w" );

$v_strm_in = av_stream_open($file_in, "video");
$v_strm_out1 = av_stream_open($file_out1, "video", array( "width" => imagesx($image), "height" => imagesy($image), "codec" => "vp6a" ));
$v_strm_out2 = av_stream_open($file_out2, "video", array( "width" => imagesx($image), "height" => imagesy($image), "codec" => "vp6f" ));

$v_time = 0;
$a_time = 0;
while(!av_file_eof($file_in)) {
	if(av_stream_read_image($v_strm_in, $image, $v_time)) {
		av_stream_write_image($v_strm_out1, $image, $v_time);
		av_stream_write_image($v_strm_out2, $image, $v_time);
		echo "V: $v_time\n";
	} else {
		break;
	}
}

av_file_close($file_out1);
av_stream_close($v_strm_out1);

av_file_close($file_out2);
av_stream_close($v_strm_out2);

av_file_close($file_in);
av_stream_close($v_strm_in);

$end = microtime(true);
$duration = $end - $start;

echo "$duration seconds\n";

?>