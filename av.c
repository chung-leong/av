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
static int le_gd = -1;

/* {{{ av_functions[]
 *
 * Every user visible function must have an entry in av_functions[].
 */
const zend_function_entry av_functions[] = {
	PHP_FE(av_encoder_create,			NULL)
	PHP_FE(av_encoder_destroy,			NULL)
	PHP_FE(av_decoder_create,			NULL)
	PHP_FE(av_decoder_destroy,			NULL)
	PHP_FE(av_decoder_extract_frame,	NULL)
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
	if(dec->packet.data) {
		av_free_packet(&dec->packet.data);
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
	if(le_gd == -1) {
		le_gd = zend_fetch_list_dtor_id("gd");
	}
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
	int stream_index;
	AVDictionaryEntry *entry = NULL;;

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
	stream_index = av_find_best_stream(dec->format_cxt, AVMEDIA_TYPE_VIDEO, -1, -1, &dec->video_codec, 0);
	if (stream_index < 0) {
		RETURN_FALSE
	}
	dec->format = dec->format_cxt->iformat;
	dec->video_stream = dec->format_cxt->streams[stream_index];
	dec->video_codec_cxt = dec->video_stream->codec;
	if (avcodec_open2(dec->video_codec_cxt, dec->video_codec, NULL) < 0) {
		RETURN_FALSE
	}
	if (dec->video_codec->capabilities & CODEC_CAP_TRUNCATED) {
		dec->video_codec_cxt->flags |= CODEC_FLAG_TRUNCATED;
	}
	ZEND_REGISTER_RESOURCE(return_value, dec, le_av_dec);
}
/* }}} */

static int av_decode_next_frame(av_decoder *dec, int required_width, int required_height) {
	int frame_finished = FALSE;

	if(!dec->frame) {
		dec->frame = avcodec_alloc_frame();
	}

	while(!frame_finished) {
		if(dec->packet_bytes_remaining == 0) {
			// read the next packet
			if(dec->packet.data) {
                av_free_packet(&dec->packet);
			}
			av_read_packet(dec->format_cxt, &dec->packet);
			dec->packet_bytes_remaining = dec->packet.size;
		}

		if(dec->video_stream && dec->packet.stream_index == dec->video_stream->index) {
			// decode the video data
			int bytes_decoded = avcodec_decode_video2(dec->video_codec_cxt, dec->frame, &frame_finished, &dec->packet);
			if(bytes_decoded < 0) {
				return FALSE;
			}
			dec->packet_bytes_remaining -= bytes_decoded;
		} else if(dec->audio_stream && dec->packet.stream_index == dec->audio_stream->index) {

		} else {
			// skip
			dec->packet_bytes_remaining = 0;
		}
	}

	if(!dec->rgb_frame || dec->rgb_frame->width != required_width || dec->rgb_frame->height != required_height) {
		if(dec->rgb_frame) {
			avpicture_free((AVPicture *) dec->rgb_frame);
		} else {
			dec->rgb_frame = avcodec_alloc_frame();
		}
		avpicture_alloc((AVPicture *) dec->rgb_frame, PIX_FMT_RGBA, required_width, required_height);
		dec->scalar_cxt = sws_getCachedContext(dec->scalar_cxt, dec->video_codec_cxt->width, dec->video_codec_cxt->height, dec->video_codec_cxt->pix_fmt, required_width, required_height, PIX_FMT_RGBA, SWS_FAST_BILINEAR, NULL, NULL, NULL);
	}
	sws_scale(dec->scalar_cxt, dec->frame->data, dec->frame->linesize, 0, dec->frame->height, dec->rgb_frame->data, dec->rgb_frame->linesize);
	return TRUE;
}

/* {{{ proto string av_encoder_create(string arg)
   Create a decoder */
PHP_FUNCTION(av_decoder_extract_frame)
{
	zval *res1, *res2;
	av_decoder *dec;
	gdImagePtr image;
	uint32_t i, j;
	int *gd_pixel;
	uint8_t *av_pixel;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr", &res1, &res2) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(dec, av_decoder *, &res1, -1, "Decoder", le_av_dec);
	ZEND_FETCH_RESOURCE(image, gdImagePtr, &res2, -1, "Image", le_gd);

	av_decode_next_frame(dec, image->sx, image->sy);

	for(i = 0; i < image->sy; i++) {
		gd_pixel = image->tpixels[i];
		av_pixel = dec->rgb_frame->data[0] + dec->rgb_frame->linesize[0] * i;
		for(j = 0; j < image->sx; j++) {
			int r = av_pixel[0];
			int g = av_pixel[1];
			int b = av_pixel[2];
			int a = gdAlphaMax - (av_pixel[3] >> 1);
			*gd_pixel = gdTrueColorAlpha(r, g, b, a);
			gd_pixel += 1;
			av_pixel += 4;
		}
	}
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
