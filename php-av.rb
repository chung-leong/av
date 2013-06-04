require 'formula'

class PhpAv < Formula
  homepage 'http://www.php-qb.net/'
  url 'https://php-qb.googlecode.com/files/av-1.1-source-code.tar.bz2'
  sha1 '06ddef3a26cf101ad848bbc374a28a0d9625875c'

  depends_on 'ffmpeg'

  def install
    system "phpize"
    system "./configure"
    system "make"
    system "sudo", "make", "install"
  end

  test do
    system "echo \"<?php exit(extension_loaded('av') || dl('av.so') ? 0 : -1); ?>\" | php"
  end
end