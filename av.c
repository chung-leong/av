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
  | Author: Chung Leong <cleong@cal.berkeley.edu>                        |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_av.h"

/* If you declare any globals in php_av.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(av)
*/

/* True global resources - no need for thread safety here */
static int le_av_enc;
static int le_av_dec;

/* {{{ av_functions[]
 *
 * Every user visible function must have an entry in av_functions[].
 */
const zend_function_entry av_functions[] = {
	PHP_FE(av_encoder_create,	NULL)
	PHP_FE(av_decoder_create,	NULL)
	PHP_FE(av_encoder_destroy,	NULL)
	PHP_FE(av_decoder_destroy,	NULL)
	PHP_FE_END	/* Must be the last line in av_functions[] */
};
/* }}} */

/* {{{ av_module_entry
 */
zend_module_entry av_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"av",
	av_functions,
	PHP_MINIT(av),
	PHP_MSHUTDOWN(av),
	PHP_RINIT(av),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(av),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(av),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_AV
ZEND_GET_MODULE(av)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("av.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_av_globals, av_globals)
    STD_PHP_INI_ENTRY("av.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_av_globals, av_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_av_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_av_init_globals(zend_av_globals *av_globals)
{
	av_globals->global_value = 0;
	av_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ php_free_av_encoder
 */
static void php_free_av_encoder(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	av_encoder *enc = rsrc->ptr;
	if(enc->video_codec_cxt) {
		avcodec_close(enc->video_codec_cxt);
	}
	if(enc->audio_codec_cxt) {
		avcodec_close(enc->audio_codec_cxt);
	}
	if(enc->format_cxt) {
		avformat_close_input(&enc->format_cxt);
	}
	efree(enc);
}
/* }}} */

/* {{{ php_free_av_decoder
 */
static void php_free_av_decoder(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	av_decoder *dec = rsrc->ptr;
	if(dec->video_codec_cxt) {
		avcodec_close(dec->video_codec_cxt);
	}
	if(dec->audio_codec_cxt) {
		avcodec_close(dec->audio_codec_cxt);
	}
	if(dec->format_cxt) {
		avformat_close_input(&dec->format_cxt);
	}
	efree(dec);
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(av)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	av_register_all();
	avcodec_register_all();

	le_av_enc = zend_register_list_destructors_ex(php_free_av_encoder, NULL, "av encoder", module_number);
	le_av_dec = zend_register_list_destructors_ex(php_free_av_decoder, NULL, "av decoder", module_number);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(av)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(av)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(av)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(av)
{
    AVOutputFormat *ofmt = NULL;
    AVInputFormat *ifmt = NULL;
    AVCodec *codec = NULL;

	php_info_print_table_start();
	php_info_print_table_header(2, "av support", "enabled");
	php_info_print_table_end();

	php_info_print_table_start();
	php_info_print_table_colspan_header(4, "Output formats");
	php_info_print_table_header(4, "Name", "Short name", "MIME type", "Extensions");
    while((ofmt = av_oformat_next(ofmt))) {
    	php_info_print_table_row(4, ofmt->long_name, ofmt->name, ofmt->mime_type, ofmt->extensions);
    }
	php_info_print_table_colspan_header(3, "Input formats");
	php_info_print_table_header(3, "Name", "Short name", "Extensions");
    while((ifmt = av_iformat_next(ifmt))) {
    	php_info_print_table_row(3, ifmt->long_name, ifmt->name, ifmt->extensions);
    }
	php_info_print_table_colspan_header(2, "Codecs");
	php_info_print_table_header(2, "Name", "Short name");
    while((codec = av_codec_next(codec))) {
    	php_info_print_table_row(2, codec->long_name, codec->name);
    }

	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* Remove the following function when you have succesfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string av_encoder_create(string arg)
   Create an encoder */
PHP_FUNCTION(av_encoder_create)
{
}
/* }}} */

/* {{{ proto string av_encoder_create(string arg)
   Create a decoder */
PHP_FUNCTION(av_decoder_create)
{
	char *filename;
	int filename_len;
	zval *params;
	av_decoder *dec;
	int stream;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &filename, &filename_len, &params) == FAILURE) {
		return;
	}

	dec = emalloc(sizeof(av_decoder));
	memset(dec, 0, sizeof(av_decoder));
	if (avformat_open_input(&dec->format_cxt, filename, NULL, NULL) < 0) {
		RETURN_FALSE
	}
	if (avformat_find_stream_info(dec->format_cxt, NULL) < 0) {
		RETURN_FALSE
	}
	stream = av_find_best_stream(dec->format_cxt, AVMEDIA_TYPE_VIDEO, -1, -1, &dec->video_codec, 0);
	if (stream < 0) {
		RETURN_FALSE
	}
	dec->video_stream_number = stream;
	dec->video_codec_cxt = avcodec_alloc_context3(dec->video_codec);
	if (avcodec_open2(dec->video_codec_cxt, dec->video_codec, NULL) < 0) {
		RETURN_FALSE
	}

	ZEND_REGISTER_RESOURCE(return_value, dec, le_av_dec);
}
/* }}} */

/* {{{ proto string av_encoder_create(string arg)
   Free an encoder */
PHP_FUNCTION(av_encoder_destroy)
{
	zval *params;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &params) == FAILURE) {
		return;
	}

}
/* }}} */

/* {{{ proto string av_encoder_create(string arg)
   Create a encoder */
PHP_FUNCTION(av_decoder_destroy)
{
	zval *res;
	av_decoder *dec;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(dec, av_decoder *, &res, -1, "Decoder", le_av_dec);

	zend_list_delete(Z_LVAL_P(res));

	RETURN_TRUE;
}
/* }}} */

/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
