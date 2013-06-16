require 'formula'

class PhpAv < Formula
  homepage 'http://www.php-qb.net/'
  url 'https://php-qb.googlecode.com/files/av-1.1-source-code.tar.bz2'
  sha1 'e0445a1d2f8213bc90e70979e4372dce854d696d'

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
    system "cp", "modules/qb.so", "#{prefix}/qb.so"
    system "cp", "av.ini", "#{prefix}/av.ini"
  end

  test do
    system "echo \"<?php exit(extension_loaded('av') || dl('av.so') ? 0 : -1); ?>\" | php"
  end
end