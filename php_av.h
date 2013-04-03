/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2013 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_AV_H
#define PHP_AV_H

extern zend_module_entry av_module_entry;
#define phpext_av_ptr &av_module_entry

#ifdef PHP_WIN32
#	define PHP_AV_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_AV_API __attribute__ ((visibility("default")))
#else
#	define PHP_AV_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#include <avcodec.h>
#include <avformat.h>

typedef struct av_encoder av_encoder;
typedef struct av_decoder av_decoder;

struct av_encoder {
	AVFormatContext *format_cxt;
	AVOutputFormat *format;
	AVCodecContext *video_codec_cxt;
	AVCodec *video_codec;
	AVCodecContext *audio_codec_cxt;
	AVCodec *audio_codec;
};

struct av_decoder {
	AVFormatContext *format_cxt;
	AVInputFormat *format;
	AVCodecContext *video_codec_cxt;
	AVCodec *video_codec;
	int video_stream_number;
	AVCodecContext *audio_codec_cxt;
	AVCodec *audio_codec;
	int audio_stream_number;
};

PHP_MINIT_FUNCTION(av);
PHP_MSHUTDOWN_FUNCTION(av);
PHP_RINIT_FUNCTION(av);
PHP_RSHUTDOWN_FUNCTION(av);
PHP_MINFO_FUNCTION(av);

PHP_FUNCTION(av_encoder_create);
PHP_FUNCTION(av_decoder_create);
PHP_FUNCTION(av_decoder_destroy);
PHP_FUNCTION(av_encoder_destroy);

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(av)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(av)
*/

/* In every utility function you add that needs to use variables 
   in php_av_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as AV_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define AV_G(v) TSRMG(av_globals_id, zend_av_globals *, v)
#else
#define AV_G(v) (av_globals.v)
#endif

#endif	/* PHP_AV_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
