dnl $Id$
dnl config.m4 for extension av

PHP_ARG_WITH(av, whether to include libav support,
[  --with-av             Include libav support])

if test -z "$PHP_AV_DIR"; then
  PHP_ARG_WITH(av-dir, for the location of libav,
  [  --with-av-dir[=DIR]     AV: Set the path to libav install prefix], no, no)
fi

if test "$PHP_AV" != "no"; then
  # --with-av -> check with-path
  SEARCH_PATH="/usr/local /usr"     
  SEARCH_FOR="include/libavcodec/avcodec.h" 
  if test -r $PHP_AV_DIR/$SEARCH_FOR; then
    AV_DIR=$PHP_AV_DIR
  else # search default path list
    AC_MSG_CHECKING([for libavcodec files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        AV_DIR=$i
        AC_MSG_RESULT(found in $i)
        break
      fi
    done
  fi
  
  PHP_ADD_INCLUDE($AV_DIR/include/libavcodec/)
  PHP_ADD_INCLUDE($AV_DIR/include/libavformat/)
  PHP_ADD_INCLUDE($AV_DIR/include/libavfilter/)
  PHP_ADD_INCLUDE($AV_DIR/include/libswscale/)      

  if test -z "$AV_DIR"; then
     AC_MSG_RESULT([not found])
     AC_MSG_ERROR([Please reinstall libavcodec distribution])
  fi

  PHP_ADD_INCLUDE($AV_DIR/include)

  PHP_CHECK_LIBRARY(avcodec,avcodec_open2,
  [
    PHP_ADD_LIBRARY_WITH_PATH(avcodec, $AV_DIR/lib, AV_SHARED_LIBADD)
    AC_DEFINE(HAVE_AVLIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong libavcodec lib version or lib not found])
  ],[
    -L$AV_DIR/lib -lm
  ])

  PHP_CHECK_LIBRARY(avcodec,avcodec_encode_audio2,
  [
    AC_DEFINE(HAVE_AVCODEC_ENCODE_AUDIO2,1,[ ])
  ],[
  ],[
    -L$AV_DIR/lib -lm
  ]) 

  PHP_CHECK_LIBRARY(avcodec,avcodec_encode_video2,
  [
    AC_DEFINE(HAVE_AVCODEC_ENCODE_VIDEO2,1,[ ])
  ],[
  ],[
    -L$AV_DIR/lib -lm
  ]) 

  PHP_CHECK_LIBRARY(avcodec,avcodec_default_get_buffer2,
  [
    AC_DEFINE(HAVE_AVCODEC_DEFAULT_GET_BUFFER2,1,[ ])
  ],[
  ],[
    -L$AV_DIR/lib -lm
  ]) 

  PHP_CHECK_LIBRARY(avformat,ffurl_read_complete,
  [
    AC_DEFINE(HAVE_FFURL_READ_COMPLETE,1,[ ])
  ],[
  ],[
    -L$AV_DIR/lib -lm
  ]) 

  PHP_CHECK_LIBRARY(avformat,ffurl_write,
  [
    AC_DEFINE(HAVE_FFURL_WRITE,1,[ ])
  ],[
  ],[
    -L$AV_DIR/lib -lm
  ]) 
  
  PHP_CHECK_LIBRARY(avformat,ffurl_seek,
  [
    AC_DEFINE(HAVE_FFURL_SEEK,1,[ ])
  ],[
  ],[
    -L$AV_DIR/lib -lm
  ]) 
  
  PHP_CHECK_LIBRARY(avformat,avio_open,
  [
    PHP_ADD_LIBRARY_WITH_PATH(avformat, $AV_DIR/lib, AV_SHARED_LIBADD)
    AC_DEFINE(HAVE_AVFORMAT,1,[ ])
  ],[
    AC_MSG_ERROR([wrong libavformat lib version or lib not found])
  ],[
    -L$AV_DIR/lib -lm
  ])
  
  PHP_CHECK_LIBRARY(avutil,av_dict_get,
  [
    PHP_ADD_LIBRARY_WITH_PATH(avutil, $AV_DIR/lib, AV_SHARED_LIBADD)
    AC_DEFINE(HAVE_AVUTIL,1,[ ])
  ],[
    AC_MSG_ERROR([wrong libavutil lib version or lib not found])
  ],[
    -L$AV_DIR/lib -lm
  ]) 

  PHP_CHECK_LIBRARY(swscale,sws_scale,
  [
    PHP_ADD_LIBRARY_WITH_PATH(swscale, $AV_DIR/lib, AV_SHARED_LIBADD)
    AC_DEFINE(HAVE_SWSCALE,1,[ ])
  ],[
    AC_MSG_ERROR([wrong libswscale lib version or lib not found])
  ],[
    -L$AV_DIR/lib -lm
  ]) 

  if test -r "$AV_DIR/include/libswresample/swresample.h"; then
    PHP_ADD_INCLUDE($AV_DIR/include/libswresample/)
    PHP_CHECK_LIBRARY(swresample,swr_convert,
    [
      PHP_ADD_LIBRARY_WITH_PATH(swresample, $AV_DIR/lib, AV_SHARED_LIBADD)
      AC_DEFINE(HAVE_SWRESAMPLE,1,[ ])
    ],[
    ],[
      -L$AV_DIR/lib -lm
    ])
  fi 

  if test -r "$AV_DIR/include/libavresample/avresample.h"; then
    PHP_ADD_INCLUDE($AV_DIR/include/libavresample/)
    PHP_CHECK_LIBRARY(avresample,avresample_convert,
    [
      PHP_ADD_LIBRARY_WITH_PATH(avresample, $AV_DIR/lib, AV_SHARED_LIBADD)
      AC_DEFINE(HAVE_AVRESAMPLE,1,[ ])
    ],[
    ],[
      -L$AV_DIR/lib -lm
    ])
  fi 
  
  PHP_SUBST(AV_SHARED_LIBADD)

  PHP_NEW_EXTENSION(av, av.c faststart.c, $ext_shared)
fi
