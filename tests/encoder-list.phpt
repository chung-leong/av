--TEST--
Encoder list test
--FILE--
<?php

foreach(av_get_encoders() as $name) {
	echo "$name\n";
}

?>
--EXPECTREGEX--
(?:\w+[\r\n]*)+
