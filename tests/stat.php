<?php

$folder = dirname(__FILE__);
$file_in = av_file_open("$folder/source-code.mov", "rb");
$info = av_file_stat($file_in);
print_r($info);
av_file_close($file_in);

?>