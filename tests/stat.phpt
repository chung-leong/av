--TEST--
Video file details test
--FILE--
<?php

$folder = dirname(__FILE__);
$file_in = av_file_open("$folder/source-code.mov", "r");
$info = av_file_stat($file_in);
print_r($info);
av_file_close($file_in);

?>
--EXPECT--
Array
(
    [format] => mov
    [format_name] => QuickTime / MOV
    [bit_rate] => 2682103
    [duration] => 29.863197
    [metadata] => Array
        (
            [major_brand] => qt  
            [minor_version] => 537199360
            [compatible_brands] => qt  
            [creation_time] => 2011-04-01 03:36:02
            [title] => "Source Code" Clip
            [title-eng] => "Source Code" Clip
            [artist] => Yahoo! Movies
            [artist-eng] => Yahoo! Movies
            [copyright] => © 2011, Summit Entertainment, LLC.
            [copyright-eng] => © 2011, Summit Entertainment, LLC.
        )

    [streams] => Array
        (
            [0] => Array
                (
                    [type] => audio
                    [codec] => aac
                    [codec_name] => AAC (Advanced Audio Coding)
                    [bit_rate] => 142077
                    [duration] => 29.866666666667
                    [frame_rate] => NAN
                    [height] => 0
                    [width] => 0
                    [metadata] => Array
                        (
                            [creation_time] => 2011-04-01 03:36:02
                            [language] => eng
                            [handler_name] => Apple Alias Data Handler
                        )

                )

            [audio] => Array
                (
                    [type] => audio
                    [codec] => aac
                    [codec_name] => AAC (Advanced Audio Coding)
                    [bit_rate] => 142077
                    [duration] => 29.866666666667
                    [frame_rate] => NAN
                    [height] => 0
                    [width] => 0
                    [metadata] => Array
                        (
                            [creation_time] => 2011-04-01 03:36:02
                            [language] => eng
                            [handler_name] => Apple Alias Data Handler
                        )

                )

            [1] => Array
                (
                    [type] => video
                    [codec] => h264
                    [codec_name] => H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10
                    [bit_rate] => 2533818
                    [duration] => 29.863196529863
                    [frame_rate] => 23.976
                    [height] => 480
                    [width] => 852
                    [metadata] => Array
                        (
                            [creation_time] => 2011-04-01 03:36:02
                            [language] => eng
                            [handler_name] => Apple Alias Data Handler
                        )

                )

            [video] => Array
                (
                    [type] => video
                    [codec] => h264
                    [codec_name] => H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10
                    [bit_rate] => 2533818
                    [duration] => 29.863196529863
                    [frame_rate] => 23.976
                    [height] => 480
                    [width] => 852
                    [metadata] => Array
                        (
                            [creation_time] => 2011-04-01 03:36:02
                            [language] => eng
                            [handler_name] => Apple Alias Data Handler
                        )

                )

        )

)
