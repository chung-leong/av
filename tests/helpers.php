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
	private $initialBalls;
	private $backgroundColor;
	private $duration;
	
	public function __construct($width, $height, $frame_rate) {
		$this->width = $width;
		$this->height = $height;
		$this->image = imagecreatetruecolor($this->width, $this->height);
		
		$this->initialBalls = array();
		$this->initialBalls[] = new Ball(50, 50, 200, 200, 10, 20, $this->width, $this->height, imagecolorallocate($this->image, 255, 0, 0));
		$this->initialBalls[] = new Ball(150, 50, 90, 90, -10, 20, $this->width, $this->height, imagecolorallocate($this->image, 255, 255, 0));
		$this->initialBalls[] = new Ball(450, 150, 150, 150, -20, 1, $this->width, $this->height, imagecolorallocate($this->image, 0, 255, 0));
		$this->initialBalls[] = new Ball(0, 350, 150, 150, 2, 10, $this->width, $this->height, imagecolorallocate($this->image, 0, 0, 255));
		$this->initialBalls[] = new Ball(250, 0, 120, 120, 40, 5, $this->width, $this->height, imagecolorallocate($this->image, 0, 255, 255));
		$this->initialBalls[] = new Ball(350, 350, 70, 70, 10, -40, $this->width, $this->height, imagecolorallocate($this->image, 0, 0, 0));
		
		$this->backgroundColor = imagecolorallocate($this->image, 255, 255, 255);
		$this->duration = 1 / $frame_rate;
	
		foreach($this->initialBalls as $ball) {
			$ball->tick(0);
		}
		$this->reset();
	}

	public function reset() {
		$this->balls = array();
		foreach($this->initialBalls as $ball) {
			$this->balls[] = clone $ball;
		}
		return $this->advance();
	}
	
	public function advance() {
		imagefilledrectangle($this->image, 0, 0, $this->width, $this->height, $this->backgroundColor);
		foreach($this->balls as $ball) {
			$ball->render($this->image);
			$ball->tick(1);
		}
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

	public function __construct() {
		// 100 stero samples comprising a sin wave at 441 hertz
		$samples = "";
		for($i = 0; $i < 100; $i++) {
			$sin = sin(($i / 100) * 2 * M_PI);
			$samples .= pack("ff", $sin, $sin);
		}
		
		// duplicate them ten times => 1000 samples
		$this->pcm = str_repeat($samples, 10);
		$this->duration = 1000 / 44100;
	}
	
	public function generate() {
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

function compareImage($img1, $img2) {
	$sample1 = imagecreatetruecolor(16, 16);
	$sample2 = imagecreatetruecolor(16, 16);
	imagecopyresample($sample1, $img1, 0, 0, 0, 0, 16, 16, imagesx($img1), imagesy($img1));
	imagecopyresample($sample2, $img2, 0, 0, 0, 0, 16, 16, imagesx($img2), imagesy($img2));

	$diff = 0;
	for($y = 0; $y < 16; $y++) {
		for($x = 0; $x < 16; $x++) {
			$argb1 = imagecolorat($img1, $x, $y);
			$a1 = ($argb >> 24) & 0xFF;
			$r1 = ($argb >> 16) & 0xFF;
			$g1 = ($argb >>  8) & 0xFF;
			$b1 = ($argb >>  0) & 0xFF;

			$argb2 = imagecolorat($img2, $x, $y);
			$a2 = ($argb >> 24) & 0xFF;
			$r2 = ($argb >> 16) & 0xFF;
			$g2 = ($argb >>  8) & 0xFF;
			$b2 = ($argb >>  0) & 0xFF;

			$diff += abs($a1 - $a2) / 127;
			$diff += abs($r1 - $r2) / 255;
			$diff += abs($g1 - $g2) / 255;
			$diff += abs($b1 - $b2) / 255;
		}
	}
	return $diff;
}

?>
