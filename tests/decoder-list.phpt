--TEST--
Decoder list test
--FILE--
<?php

foreach(av_get_decoders() as $name) {
	echo "$name\n";
}

?>
--EXPECTREGEX--
(?:\w+[\r\n]*)+
