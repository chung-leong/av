--TEST--
Live stream test
--SKIPIF--
<?php 
	if(!function_exists('imagecreatetruecolor')) { 
		print 'skip GD not available'; 
	}
?>
--FILE--
<?php

$url = "mmsh://a1709.l1856953708.c18569.g.lm.akamaistream.net/D/1709/18569/v0001/reflector:53708";
$file = av_file_open($url, "r");
$stat = av_file_stat($file);
print_r($stat);
$image = imagecreatetruecolor(640, 360);
$strm = av_stream_open($file, "video");
av_stream_read_image($strm, $image, $time);
$folder = dirname(__FILE__);
imagepng($image, "$folder/live.png");

?>
--EXPECT--
Array
(
    [format] => asf
    [format_name] => ASF (Advanced / Active Streaming Format)
    [bit_rate] => 265000
    [duration] => INF
    [metadata] => Array
        (
            [title] => NASA-ISS-push Streaming ISS Video
            [artist] => Johnson Space Center
            [comment] => VBrick Streaming Video
        )

    [streams] => Array
        (
            [0] => Array
                (
                    [type] => audio
                    [codec] => wmav2
                    [codec_name] => Windows Media Audio 2
                    [bit_rate] => 8000
                    [duration] => INF
                    [channel_layout] => 0
                    [channels] => 1
                    [sample_rate] => 8000
                    [metadata] => Array
                        (
                        )

                )

            [audio] => Array
                (
                    [type] => audio
                    [codec] => wmav2
                    [codec_name] => Windows Media Audio 2
                    [bit_rate] => 8000
                    [duration] => INF
                    [channel_layout] => 0
                    [channels] => 1
                    [sample_rate] => 8000
                    [metadata] => Array
                        (
                        )

                )

            [1] => Array
                (
                    [type] => video
                    [codec] => wmv3
                    [codec_name] => Windows Media Video 9
                    [bit_rate] => 257000
                    [duration] => INF
                    [frame_rate] => 14.985014985015
                    [height] => 360
                    [width] => 640
                    [metadata] => Array
                        (
                        )

                )

            [video] => Array
                (
                    [type] => video
                    [codec] => wmv3
                    [codec_name] => Windows Media Video 9
                    [bit_rate] => 257000
                    [duration] => INF
                    [frame_rate] => 14.985014985015
                    [height] => 360
                    [width] => 640
                    [metadata] => Array
                        (
                        )

                )

        )

)
