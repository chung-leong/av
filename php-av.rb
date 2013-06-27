require 'formula'

class PhpAv < Formula
  homepage 'http://www.php-qb.net/'
  url 'https://php-qb.googlecode.com/files/av-1.2-source-code.tar.bz2'
  sha1 '9674f5fc826f6f39ecb385d2bee64b65f031d273'

  depends_on 'autoconf' => :build
  depends_on 'ffmpeg' => %w{
  	with-x264
  	with-faac 
  	with-lame 
  	with-xvid 
  	with-theora 
  	with-libvorbis 
  	with-libvpx
  	with-libass
  	with-speex
  	with-schroedinger
  	with-opus
  	with-openjpeg 
  	}

  def install
    ENV['PHP_AUTOCONF'] = '/usr/local/bin/autoconf'
    ENV['PHP_AUTOHEADER'] = '/usr/local/bin/autoheader'
    system "phpize"
    system "./configure"
    system "make"
    system "cp", "modules/av.so", "#{prefix}/av.so"
    system "cp", "av.ini", "#{prefix}/av.ini"
  end

  test do
    system "echo \"<?php exit(extension_loaded('av') || dl('av.so') ? 0 : -1); ?>\" | php"
  end
end