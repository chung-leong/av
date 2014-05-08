<?php

class Ball {
	private $boundLeft = 0;
	private $boundTop = 0;
	private $boundRight;
	private $boundBottom;
	
	private $x;
	private $y;
	private $width;
	private $height;
	private $speedX;
	private $speedY;
	
	private $color;

	public function __construct($initialX, $initialY, $width, $height, $speedX, $speedY, $boundRight, $boundBottom, $color) {
		$this->x = $initialX;
		$this->y = $initialY;
		$this->width = $width;
		$this->height = $height;
		$this->speedX = $speedX;
		$this->speedY = $speedY;
		$this->boundRight = $boundRight;
		$this->boundBottom = $boundBottom;
		$this->color = $color;
	}
	
	public function tick($timeElapsed) {
		$this->x += $this->speedX * $timeElapsed;
		$this->y += $this->speedY * $timeElapsed;
		
		if($this->x - ($this->width / 2) < $this->boundLeft) {
			$this->speedX = -$this->speedX;
			$this->x = $this->boundLeft + ($this->width / 2);
		} else if($this->x + ($this->width / 2) > $this->boundRight) {
			$this->speedX = -$this->speedX;
			$this->x = $this->boundRight - ($this->width / 2);
		}
		
		if($this->y - ($this->height / 2) < $this->boundTop) {
			$this->speedY = -$this->speedY;
			$this->y = $this->boundTop + ($this->height / 2);
		} else if($this->y + ($this->height / 2) > $this->boundBottom) {
			$this->speedY = -$this->speedY;
			$this->y = $this->boundBottom - ($this->height / 2);
		}
	}
	
	public function render($image) {
		imagefilledellipse($image, $this->x, $this->y, $this->width, $this->height, $this->color);
	}
}

class BouncingBallAnimation {
	private $width;
	private $height;
	private $image;
	private $balls;
	private $backgroundColor;
	private $duration;
	
	public function __construct($width, $height, $frameRate) {
		$this->width = $width;
		$this->height = $height;
		$this->image = imagecreatetruecolor($this->width, $this->height);
		
		$this->balls = array();
		$this->balls[] = new Ball(50, 50, 200, 200, 10, 20, $this->width, $this->height, imagecolorallocate($this->image, 255, 0, 0));
		$this->balls[] = new Ball(150, 50, 90, 90, -10, 20, $this->width, $this->height, imagecolorallocate($this->image, 255, 255, 0));
		$this->balls[] = new Ball(450, 150, 150, 150, -20, 1, $this->width, $this->height, imagecolorallocate($this->image, 0, 255, 0));
		$this->balls[] = new Ball(0, 350, 150, 150, 2, 10, $this->width, $this->height, imagecolorallocate($this->image, 0, 0, 255));
		$this->balls[] = new Ball(250, 0, 120, 120, 40, 5, $this->width, $this->height, imagecolorallocate($this->image, 0, 255, 255));
		$this->balls[] = new Ball(350, 350, 70, 70, 10, -40, $this->width, $this->height, imagecolorallocate($this->image, 0, 0, 0));
		
		$this->backgroundColor = imagecolorallocate($this->image, 255, 255, 255);
		$this->duration = 1 / $frameRate;
	
		foreach($this->balls as $ball) {
			$ball->tick(0);
		}
	}

	public function advance() {
		imagefilledrectangle($this->image, 0, 0, $this->width, $this->height, $this->backgroundColor);
		foreach($this->balls as $ball) {
			$ball->render($this->image);
			$ball->tick(1);
		}
	}

	public function get() {
		return $this->image;
	}
	
	public function write($output, &$time) {
		$list = is_array($output) ? $output : array($output);
		foreach($list as $strm) {
			av_stream_write_image($strm, $this->image, $time);
		}
		$time += $this->duration;
	}
}

class SineWave {
	private $pcm;
	private $duration;

	public function __construct($amplitude = 1) {
		// 100 stero samples comprising a sin wave at 441 hertz
		$samples = "";
		for($i = 0; $i < 100; $i++) {
			$sin = sin(($i / 100) * 2 * M_PI) * $amplitude;
			$samples .= pack("ff", $sin, $sin);
		}
		
		// duplicate them ten times => 1000 samples
		$this->pcm = str_repeat($samples, 10);
		$this->duration = 1000 / 44100;
	}
	
	public function get() {
		return $this->pcm;
	}

	public function write($output, &$time) {
		$list = is_array($output) ? $output : array($output);
		foreach($list as $strm) {
			av_stream_write_pcm($strm, $this->pcm, $time);
		}
		$time += $this->duration;
	}	
}

class TestVideo {
	private $path;
	private $width;
	private $height;
	private $frameRate;
	private $duration;
	private $videoCodec;
	private $audioCodec;
	private $metadata;
	
	public function __construct($path, $width, $height, $frameRate, $duration) {
		$this->path = $path;
		$this->width = $width;
		$this->height = $height;
		$this->frameRate = $frameRate;
		$this->duration = $duration;
	}
	
	public function setVideoCodec($codec) {
		$this->videoCodec = $codec;
	}

	public function setAudioCodec($codec) {
		$this->audioCodec = $codec;
	}
	
	public function attachMetadata($metadata) {
		$this->metadata = $metadata;
	}

	public function create() {
		// open file for writing
		$file = av_file_open($this->path, "w");
		if($this->videoCodec !== false) {
			$params = array();
			$params["width"] = $this->width;
			$params["height"] = $this->height;
			$params["frame_rate"] = $this->frameRate;
			$params["bit_rate"] = 1024000;
			$params["gop"] = 0;
			if($this->videoCodec) {
				$params["codec"] = $this->videoCodec;
			}
			$videoStream = av_stream_open($file, "video", $params);
		} else {
			$videoStream = false;
		}		
		if($this->audioCodec !== false) {
			$params = array();
			if($this->audioCodec) {
				$params["codec"] = $this->audioCodec;
			}
			$audioStream = av_stream_open($file, "audio", $params);
		} else {
			$audioStream = false;
		}

		if($file === null|| $videoStream === null || $audioStream === null) {
			throw new Exception("Error opening {$this->path} for writing");
		}

		if($videoStream) {
			$animation = new BouncingBallAnimation($this->width, $this->height, $this->frameRate);
			$videoTime = 0.5 / $this->frameRate;
		} else {
			$videoTime = INF;
		}
		if($audioStream) {
			$tone = new SineWave;
			$audioTime = 0;
		} else {
			$audioTime = INF;
		}

		while($videoTime < $this->duration || $audioTime < $this->duration) {
			if($videoTime < $audioTime) {
				// write to video stream
				$animation->write($videoStream, $videoTime);
				$animation->advance();
			} else {
				// write to audio stream
				$tone->write($audioStream, $audioTime);
			}
		}

		// close file
		av_file_close($file);
	}
	
	public function verify() {
		// open file for reading
		$file = av_file_open($this->path, "r");
		if($this->videoCodec !== false) {
			$videoStream = av_stream_open($file, "video");
		} else {
			$videoStream = false;
		}
		if($this->audioCodec !== false) {
			$audioStream = av_stream_open($file, "audio");
		} else {
			$audioStream = false;
		}

		if($file === null|| $videoStream === null || $audioStream === null) {
			throw new Exception("Error opening {$this->path} for writing");
		}

		if($videoStream) {
			$animation = new BouncingBallAnimation($this->width, $this->height, $this->frameRate);
			$videoTime = 0.5 / $this->frameRate;
		} else {
			$videoTime = INF;
		}
		if($audioStream) {
			$tone = new SineWave;
			$audioTime = 0;
		} else {
			$audioTime = INF;
		}
		
		$image2 = imagecreatetruecolor($this->width, $this->height);
		$pcm1 = "";
		$len1 = 0;
		$beginning = true;

		while($videoTime < $this->duration || $audioTime < $this->duration) {
			if($videoTime < $audioTime) {
				// read from video stream
				if(av_stream_read_image($videoStream, $image2, $videoTime)) {
					$image1 = $animation->get();
					$diff = $this->compareImage($image1, $image2);
					if($diff > 1) {
						if(abs($this->duration - $videoTime) > 0.1) {
							imagepng($image1, "{$this->path}.correct.png");
							imagepng($image2, "{$this->path}.incorrect.png");
							throw new Exception("Frame at $videoTime is different");
						}
					}
					$animation->advance();
				} else {
					if(abs($this->duration - $videoTime) > 0.1) {
						throw new Exception("Cannot read from video stream");
					} else {
						$videoTime = INF;
					}
				}
			} else {
				// read from audio stream
				if(av_stream_read_pcm($audioStream, $pcm2, $audioTime)) {
					$len2 = strlen($pcm2);
					if($audioTime < 0) {
						$offset = 0;
						while($len2 > 0 && $audioTime < 0) {
							$len2 -= 8;
							$audioTime += 1 / 44100;
							$offset += 8;
						}
						if($len2 == 0) {
							continue;
						}
						$pcm2 = substr($pcm2, $offset);
					} 
					if($beginning) {
						if($audioTime > 0) {
							$timeOffset = fmod($audioTime, 1000 / 44100);
							$byteOffset = round($timeOffset * 44100) * 2 * 4;
							$pcm1 = substr($tone->get(), $byteOffset);
							$len1 = strlen($pcm1);
						}
						$beginning = false;
					}
					while($len1 < $len2) {
						// get another chunk of samples
						$pcm1 .= $tone->get();
						$len1 = strlen($pcm1);
					}
					$diff = $this->comparePCM($pcm1, $pcm2);
					if($diff > 1) {
						if(abs($this->duration - $audioTime) > 0.1) {
							throw new Exception("Audio at $audioTime is different");
						}
					}
					$pcm1 = substr($pcm1, $len2);
					$len1 -= $len2;
				} else {
					if(abs($this->duration - $audioTime) > 0.1) {
						throw new Exception("Cannot read from audio stream");
					} else {
						$audioTime = INF;
					}
				}
			}
		}
		
		// close file
		av_file_close($file);
	}
	
	public function delete() {
		unlink($this->path);
	}

	protected function compareImage($img1, $img2) {
		$w = 16;
		$h = 16;
		$sample1 = imagecreatetruecolor($w, $h);
		$sample2 = imagecreatetruecolor($w, $h);
		imagecopyresampled($sample1, $img1, 0, 0, 0, 0, $w, $h, imagesx($img1), imagesy($img1));
		imagecopyresampled($sample2, $img2, 0, 0, 0, 0, $w, $h, imagesx($img2), imagesy($img2));
	
		$diff = 0;
		for($y = 0; $y < $h; $y++) {
			for($x = 0; $x < $w; $x++) {
				$argb1 = imagecolorat($sample1, $x, $y);
				$a1 = ($argb1 >> 24) & 0xFF;
				$r1 = ($argb1 >> 16) & 0xFF;
				$g1 = ($argb1 >>  8) & 0xFF;
				$b1 = ($argb1 >>  0) & 0xFF;
	
				$argb2 = imagecolorat($sample2, $x, $y);
				$a2 = ($argb2 >> 24) & 0xFF;
				$r2 = ($argb2 >> 16) & 0xFF;
				$g2 = ($argb2 >>  8) & 0xFF;
				$b2 = ($argb2 >>  0) & 0xFF;
	
				$diff += abs($a1 - $a2) / 127;
				$diff += abs($r1 - $r2) / 255;
				$diff += abs($g1 - $g2) / 255;
				$diff += abs($b1 - $b2) / 255;
			}
		}
		return $diff / 50;
	}

	protected function comparePCM($pcm1, $pcm2) {
		$array1 = unpack("f*", $pcm1);
		$array2 = unpack("f*", $pcm2);
		$count = min(count($array1), count($array2));
		$diff = 0;
		for($i = 1; $i <= $count; $i++) {
			$float1 = $array1[$i];
			$float2 = $array2[$i];
			$diff += abs($float1 - $float2);
		}
		return $diff / $count * 10;
	}
}

?>
