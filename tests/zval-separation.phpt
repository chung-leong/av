--TEST--
Variable separation test
--FILE--
<?php

$folder = dirname(__FILE__);
$file_in = av_file_open("$folder/o-tyle-prosze-cie.mp3", "rb");

$a_strm_in = av_stream_open($file_in, "audio");

$var1 = "This is a test";
$var2 = $var1;

av_stream_read_pcm($a_strm_in, $var1, $a_time);

echo $var2;

?>
--EXPECT--
This is a test
