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

ZEND_DECLARE_MODULE_GLOBALS(av)

/* True global resources - no need for thread safety here */
static int le_av_file;
static int le_av_strm;
static int le_gd = -1;

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_av_file_open, 0, 0, 2)
    ZEND_ARG_INFO(0, path)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_av_file_close, 0, 0, 1)
    ZEND_ARG_INFO(0, file)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_av_file_eof, 0, 0, 1)
    ZEND_ARG_INFO(0, file)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_av_file_stat, 0, 0, 1)
	ZEND_ARG_INFO(0, stream)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_av_stream_open, 0, 0, 2)
    ZEND_ARG_INFO(0, file)
    ZEND_ARG_INFO(0, id)
    ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_av_stream_close, 0, 0, 1)
    ZEND_ARG_INFO(0, file)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_av_stream_read_image, 0, 0, 2)
    ZEND_ARG_INFO(0, stream)
    ZEND_ARG_INFO(0, image)
    ZEND_ARG_INFO(0, time)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_av_stream_read_pcm, 0, 0, 2)
    ZEND_ARG_INFO(0, stream)
    ZEND_ARG_INFO(1, buffer)
    ZEND_ARG_INFO(0, time)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_av_stream_write_image, 0, 0, 2)
    ZEND_ARG_INFO(0, stream)
    ZEND_ARG_INFO(0, image)
    ZEND_ARG_INFO(0, time)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_av_stream_write_pcm, 0, 0, 2)
    ZEND_ARG_INFO(0, stream)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_INFO(0, time)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_av_stream_get_duration, 0, 0, 1)
    ZEND_ARG_INFO(0, stream)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ av_functions[]
 *
 * Every user visible function must have an entry in av_functions[].
 */
const zend_function_entry av_functions[] = {
	PHP_FE(av_file_open,				arginfo_av_file_open)
	PHP_FE(av_file_close,				arginfo_av_file_close)
	PHP_FE(av_file_eof,					arginfo_av_file_eof)
	PHP_FE(av_file_stat,				arginfo_av_file_stat)

	PHP_FE(av_stream_open,				arginfo_av_stream_open)
	PHP_FE(av_stream_close,				arginfo_av_stream_close)
	PHP_FE(av_stream_read_image,		arginfo_av_stream_read_image)
	PHP_FE(av_stream_read_pcm,			arginfo_av_stream_read_pcm)
	PHP_FE(av_stream_write_image,		arginfo_av_stream_write_image)
	PHP_FE(av_stream_write_pcm,			arginfo_av_stream_write_pcm)
	PHP_FE(av_stream_get_duration,		arginfo_av_stream_get_duration)

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

static void av_file_free(av_file *file) {
	file->flags |= AV_FILE_FREED;
	if(file->open_stream_count == 0) {
		// don't free anything until all streams are closed
		if(file->flags & AV_FILE_READ) {
			avformat_close_input(&file->format_cxt);
		}
		efree(file->streams);
		efree(file);
	}
}

static void av_stream_free(av_stream *strm) {
	av_file *file = strm->file;
	if(file->open_stream_count) {
		uint32_t i = 0;
		for(i = 0; i < strm->packet_count; i++) {
			av_free_packet(strm->packet_queue[i]);
			efree(strm->packet_queue[i]);
		}
		efree(strm->packet_queue);
		if(strm->packet) {
			av_free_packet(strm->packet);
			efree(strm->packet);
		}
		avcodec_close(strm->codec_cxt);
		if(strm->samples) {
			efree(strm->samples);
		}
		if(strm->resampler_cxt) {
			swr_free(&strm->resampler_cxt);
		}

		file->streams[strm->index] = NULL;
		file->open_stream_count--;
		if(file->open_stream_count == 0) {
			if(file->flags & AV_FILE_WRITE) {
				if(file->flags & AV_FILE_HEADER_WRITTEN) {
					av_write_trailer(file->format_cxt);
				}
			}
			if(file->flags & AV_FILE_FREED) {
				// free the file if no zval is referencing it
				av_file_free(file);
			}
		}
		efree(strm);
	}
}

static void php_free_av_file(zend_rsrc_list_entry *rsrc TSRMLS_DC) {
	av_file_free(rsrc->ptr);
}

static void php_free_av_stream(zend_rsrc_list_entry *rsrc TSRMLS_DC) {
	av_stream_free(rsrc->ptr);
}

static double av_get_option_double(zval *options, const char *key, double default_value) {
	if(options) {
		if(Z_TYPE_P(options) == IS_ARRAY) {
			zval **p_data;
			if(zend_hash_find(Z_ARRVAL_P(options), key, strlen(key) + 1, (void **) &p_data) == SUCCESS) {
				convert_to_double(*p_data);
				return Z_DVAL_PP(p_data);
			}
		}
	}
	return default_value;
}

static long av_get_option_long(zval *options, const char *key, long default_value) {
	if(options) {
		if(Z_TYPE_P(options) == IS_ARRAY) {
			zval **p_data;
			if(zend_hash_find(Z_ARRVAL_P(options), key, strlen(key) + 1, (void **) &p_data) == SUCCESS) {
				convert_to_long(*p_data);
				return Z_LVAL_PP(p_data);
			}
		}
	}
	return default_value;
}

static const char * av_get_option_string(zval *options, const char *key, const char *default_value) {
	if(options) {
		if(Z_TYPE_P(options) == IS_ARRAY) {
			zval **p_data;
			if(zend_hash_find(Z_ARRVAL_P(options), key, strlen(key) + 1, (void **) &p_data) == SUCCESS) {
				convert_to_string(*p_data);
				return Z_STRVAL_PP(p_data);
			}
		}
	}
	return default_value;
}

static int av_get_stream_type(const char *type TSRMLS_DC) {
	if(strcmp(type, "video") == 0) {
		return AVMEDIA_TYPE_VIDEO;
	} else if(strcmp(type, "audio") == 0) {
		return AVMEDIA_TYPE_AUDIO;
	} else if(strcmp(type, "subtitle") == 0) {
		return AVMEDIA_TYPE_SUBTITLE;
	} else {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "'%s' is not a recognized stream type", type);
		return -1;
	}
}

static void av_push_packet(av_stream *strm, AVPacket *packet) {
	if(!strm->packet) {
		strm->packet = packet;
		strm->packet_bytes_remaining = packet->size;
		return;
	}
	if(strm->packet_count >= strm->packet_queue_size) {
		strm->packet_queue_size += 16;
		strm->packet_queue = erealloc(strm->packet_queue, sizeof(AVPacket *) * strm->packet_queue_size);
	}
	strm->packet_queue[strm->packet_count] = packet;
	strm->packet_count++;
}

static int av_shift_packet(av_stream *strm) {
	if(strm->packet) {
		av_free_packet(strm->packet);
		efree(strm->packet);
	}
	if(strm->packet_count > 0) {
		strm->packet = strm->packet_queue[0];
		strm->packet_bytes_remaining = strm->packet->size;
		strm->packet_count--;
		memmove(&strm->packet_queue[0], &strm->packet_queue[1], sizeof(AVPacket *) * strm->packet_count);
		return TRUE;
	} else {
		strm->packet = NULL;
		return FALSE;
	}
}

/* {{{ php_qb_init_globals
 */
static void php_av_init_globals(zend_av_globals *av_globals)
{
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(av)
{
	ZEND_INIT_MODULE_GLOBALS(av, php_av_init_globals, NULL);

	av_register_all();
	avcodec_register_all();

	le_av_file = zend_register_list_destructors_ex(php_free_av_file, NULL, "av file", module_number);
	le_av_strm = zend_register_list_destructors_ex(php_free_av_stream, NULL, "av stream", module_number);

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
	AV_G(video_buffer) = NULL;
	AV_G(video_buffer_size) = 0;
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(av)
{
	if(AV_G(video_buffer)) {
		efree(AV_G(video_buffer));
	}
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

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string av_file_open(string arg, string mode [, array options])
   Create an encoder */
PHP_FUNCTION(av_file_open)
{
	char *filename, *mode;
	int filename_len, mode_len;
	zval *z_options = NULL;
	char *code;
	int32_t flags = 0;
	av_file *file;
	AVInputFormat *input_format = NULL;
	AVOutputFormat *output_format = NULL;
	AVFormatContext *format_cxt = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|a", &filename, &filename_len, &mode, &mode_len, &z_options) == FAILURE) {
		return;
	}

	for(code = mode; *code; code++) {
		if(*code == 'r') {
			flags |= AV_FILE_READ;
		} else if(*code == 'w') {
			flags |= AV_FILE_WRITE;
		} else if(*code == 'a') {
			flags |= AV_FILE_READ | AV_FILE_WRITE | AV_FILE_APPEND;
		}
	}

	if(!(flags & AV_FILE_READ) && (flags & AV_FILE_WRITE)) {
		AVIOContext *pb = NULL;
		if(z_options) {
			zval **p_value = NULL;
			if(zend_hash_find(Z_ARRVAL_P(z_options), "format", 7, (void **) &p_value) != FAILURE) {
				if(Z_TYPE_PP(p_value) == IS_STRING) {
					char *short_name = Z_STRVAL_PP(p_value);
					output_format = av_guess_format(short_name, NULL, NULL);
					if(!output_format) {
						php_error_docref(NULL TSRMLS_CC, E_ERROR, "Cannot find output format: %s", short_name);
						return;
					}
				}
			}
			if(!output_format) {
				if(zend_hash_find(Z_ARRVAL_P(z_options), "mime", 5, (void **) &p_value) != FAILURE) {
					if(Z_TYPE_PP(p_value) == IS_STRING) {
						char *mime_type = Z_STRVAL_PP(p_value);
						output_format = av_guess_format(NULL, NULL, mime_type);
					}
				}
			}
		}
		if(!output_format) {
			output_format = av_guess_format(NULL, filename, NULL);
			if(!output_format) {
				php_error_docref(NULL TSRMLS_CC, E_ERROR, "Cannot deduce output format from filename: %s", filename);
				return;
			}
		}

		if(!(output_format->flags & AVFMT_NOFILE)) {
			if(avio_open(&pb, filename, AVIO_FLAG_WRITE) < 0) {
				php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error opening file for writing: %s", filename);
				return;
			}
		}
		format_cxt = avformat_alloc_context();
		format_cxt->pb = pb;
		format_cxt->oformat = output_format;
	} else {
		if (avformat_open_input(&format_cxt, filename, NULL, NULL) < 0) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error opening file for reading: %s", filename);
			return;
		}
		if (avformat_find_stream_info(format_cxt, NULL) < 0) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error finding stream info: %s", filename);
			return;
		}
		input_format = format_cxt->iformat;
	}

	file = emalloc(sizeof(av_file));
	memset(file, 0, sizeof(av_file));
	file->format_cxt = format_cxt;
	file->input_format = input_format;
	file->output_format = output_format;
	file->flags = flags;

	if(format_cxt->nb_streams) {
		file->streams = emalloc(sizeof(av_stream) * format_cxt->nb_streams);
		file->stream_count = format_cxt->nb_streams;
		memset(file->streams, 0, sizeof(av_stream) * format_cxt->nb_streams);
	}

	ZEND_REGISTER_RESOURCE(return_value, file, le_av_file);
}
/* }}} */

/* {{{ proto string av_file_close(resource file)
   Close an av file */
PHP_FUNCTION(av_file_close)
{
	zval *z_strm;
	av_file *file;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_strm) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(file, av_file *, &z_strm, -1, "av file", le_av_file);
	if(file->open_stream_count == 0) {
		RETVAL_TRUE;
	} else {
		RETVAL_FALSE;
	}
	zend_list_delete(Z_LVAL_P(z_strm));
}
/* }}} */

/* {{{ proto string av_file_eof(resource file)
   Indicate whether there's more contents */
PHP_FUNCTION(av_file_eof)
{
	zval *z_strm;
	av_file *file;
	int32_t eof = FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_strm) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(file, av_file *, &z_strm, -1, "av file", le_av_file);

	if(file->flags & AV_FILE_EOF_REACHED) {
		uint32_t i;
		eof = TRUE;
		for(i = 0; i < file->stream_count; i++) {
			av_stream *strm = file->streams[i];
			if(strm) {
				if((strm->packet && strm->packet_bytes_remaining > 0) || strm->packet_count > 0) {
					eof = FALSE;
				}
			}
		}
	}
	RETVAL_BOOL(eof);
}
/* }}} */

#define ADD_LONG(zv, name, value)	\
{\
	zval *element;\
	MAKE_STD_ZVAL(element);\
	ZVAL_LONG(element, value);\
	zend_hash_update(HASH_OF(zv), name, sizeof(name), (void *) &element, sizeof(zval *), NULL);\
}\

#define ADD_DOUBLE(zv, name, value)	\
{\
	zval *element;\
	MAKE_STD_ZVAL(element);\
	ZVAL_DOUBLE(element, value);\
	zend_hash_update(HASH_OF(zv), name, sizeof(name), (void *) &element, sizeof(zval *), NULL);\
}\

#define ADD_STRING(zv, name, value)	\
{\
	zval *element;\
	MAKE_STD_ZVAL(element);\
	ZVAL_STRING(element, (value) ? value : "", TRUE);\
	zend_hash_update(HASH_OF(zv), name, strlen(name) + 1, (void *) &element, sizeof(zval *), NULL);\
}\

/* {{{ proto string av_file_stat(resource file)
   Return information about media file */
PHP_FUNCTION(av_file_stat)
{
	zval *z_strm;
	av_file *file;
	AVFormatContext *f;
	zval *streams, *metadata;
	uint32_t i;
	AVDictionaryEntry *e;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_strm) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(file, av_file *, &z_strm, -1, "av file", le_av_file);
	f = file->format_cxt;

	array_init(return_value);

	if(f->iformat) {
		ADD_STRING(return_value, "format", f->iformat->long_name);
	} else if(f->oformat) {
		ADD_STRING(return_value, "format", f->oformat->long_name);
	}
	ADD_LONG(return_value, "bit rate", f->bit_rate);
	ADD_DOUBLE(return_value, "duration", (double) f->duration / AV_TIME_BASE);

	// add metadata of file
	MAKE_STD_ZVAL(metadata);
	array_init(metadata);
	zend_hash_update(HASH_OF(return_value), "metadata", sizeof("metadata"), (void *) &metadata, sizeof(zval *), NULL);
	for(e = NULL; (e = av_dict_get(f->metadata, "", e, AV_DICT_IGNORE_SUFFIX)); ) {
		ADD_STRING(metadata, e->key, e->value);
	}

	MAKE_STD_ZVAL(streams);
	array_init(streams);
	zend_hash_update(HASH_OF(return_value), "streams", strlen("streams") + 1, (void *) &streams, sizeof(zval *), NULL);
	for(i = 0; i < f->nb_streams; i++) {
		zval *stream;
		AVStream *s = f->streams[i];
		AVCodecContext *c = s->codec;
		const AVCodecDescriptor *d = avcodec_descriptor_get(c->codec_id);
		const char *stream_type = av_get_media_type_string(c->codec_type);

		MAKE_STD_ZVAL(stream);
		array_init(stream);
		zend_hash_next_index_insert(HASH_OF(streams), (void *) &stream, sizeof(zval *), NULL);

		ADD_STRING(stream, "type", stream_type);
		ADD_STRING(stream, "codec", (d) ? d->long_name : "unknown");
		ADD_LONG(stream, "bit rate", c->bit_rate);
		ADD_DOUBLE(stream, "duration", (double) s->duration * av_q2d(s->time_base));

		ADD_LONG(stream, "height", c->height);
		ADD_LONG(stream, "width", c->width);

		// add metadata of stream
		MAKE_STD_ZVAL(metadata);
		array_init(metadata);
		zend_hash_update(HASH_OF(stream), "metadata", sizeof("metadata"), (void *) &metadata, sizeof(zval *), NULL);
		for(e = NULL; (e = av_dict_get(s->metadata, "", e, AV_DICT_IGNORE_SUFFIX)); ) {
			ADD_STRING(metadata, e->key, e->value);
		}

		// refer to stream using string key if it happens to be the best stream of a given type
		if(i == av_find_best_stream(f, c->codec_type, -1, -1, NULL, 0)) {
			Z_ADDREF_P(stream);
			zend_hash_update(HASH_OF(streams), stream_type, strlen(stream_type) + 1, (void *) &stream, sizeof(zval *), NULL);
		}
	}
}
/* }}} */

static int av_stream_get_buffer(AVCodecContext *c, AVFrame *pic) {
	av_stream *strm = c->opaque;
	int ret = avcodec_default_get_buffer(c, pic);
	strm->frame_pts = strm->packet->pts;
	return ret;
}

/* {{{ proto string av_stream_open(resource file, mixed id, [, array options])
   Create an encoder */
PHP_FUNCTION(av_stream_open)
{
	zval *z_strm, *z_id, *z_options = NULL;
	AVCodec *codec = NULL;
	AVCodecContext *codec_cxt = NULL;
	AVStream *stream = NULL;
	AVFrame *frame = NULL;
	av_file *file;
	av_stream *strm;
	int32_t stream_index;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rz|a", &z_strm, &z_id, &z_options) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(file, av_file *, &z_strm, -1, "av file", le_av_file);

	if(file->flags & AV_FILE_READ) {
		if(Z_TYPE_P(z_id) == IS_STRING) {
			int32_t type = av_get_stream_type(Z_STRVAL_P(z_id) TSRMLS_CC);
			if(type < 0) {
				return;
			}
			stream_index = av_find_best_stream(file->format_cxt, type, -1, -1, &codec, 0);
			if(stream_index < 0) {
				php_error_docref(NULL TSRMLS_CC, E_ERROR, "Cannot find a stream of type '%s'", Z_STRVAL_P(z_id));
				return;
			}
		} else if(Z_TYPE_P(z_id) == IS_LONG || Z_TYPE_P(z_id) == IS_DOUBLE) {
			stream_index = (Z_TYPE_P(z_id) == IS_DOUBLE) ? (long) Z_DVAL_P(z_id) : Z_LVAL_P(z_id);
			if(!(stream_index >= 0 && (uint32_t) stream_index < file->stream_count)) {
				php_error_docref(NULL TSRMLS_CC, E_ERROR, "Stream index must be between 0 and %d", file->stream_count);
				return;
			}
		} else {
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "parameter 2 should be \"video\", \"audio\", \"subtitle\", or an stream index");
			return;
		}

		stream = file->format_cxt->streams[stream_index];
		codec_cxt = stream->codec;
		codec_cxt->thread_count = 1;
		if (avcodec_open2(codec_cxt, codec, NULL) < 0) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "Unable to open codec '%s'", codec_cxt->codec->name);
			return;
		}
		frame = avcodec_alloc_frame();
		if (codec_cxt->codec->capabilities & CODEC_CAP_TRUNCATED) {
			codec_cxt->flags |= CODEC_FLAG_TRUNCATED;
		}
		codec_cxt->get_buffer = av_stream_get_buffer;
	} else if(file->flags & AV_FILE_WRITE){
		double frame_rate = av_get_option_double(z_options, "frame rate", 25.0);
		uint32_t sample_rate = av_get_option_long(z_options, "sampling rate", 44100);
		uint32_t bit_rate = av_get_option_long(z_options, "bit rate", 256000);
		uint32_t width = av_get_option_long(z_options, "width", 320);
		uint32_t height = av_get_option_long(z_options, "height", 240);
		enum AVMediaType media_type;
		enum AVCodecID codec_id;

		if(Z_TYPE_P(z_id) == IS_STRING) {
			media_type = av_get_stream_type(Z_STRVAL_P(z_id) TSRMLS_CC);
			if(media_type < 0) {
				return;
			}
		}

		switch(media_type) {
			case AVMEDIA_TYPE_VIDEO:
				codec_id = file->output_format->video_codec;
				break;
			case AVMEDIA_TYPE_AUDIO:
				codec_id = file->output_format->audio_codec;
				break;
			case AVMEDIA_TYPE_SUBTITLE:
				codec_id = file->output_format->subtitle_codec;
				break;
			default:
				break;
		}
		codec = avcodec_find_encoder(codec_id);
		if(!codec) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "Unable to find codec");
			return;
		}

		stream = avformat_new_stream(file->format_cxt, codec);
		stream_index = stream->index;

		codec_cxt = stream->codec;
		if(codec->capabilities & CODEC_CAP_EXPERIMENTAL) {
			codec_cxt->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
		}
		codec_cxt->thread_count = 1;
		switch(media_type) {
			case AVMEDIA_TYPE_VIDEO:
				file->format_cxt->video_codec_id = codec->id;
				codec_cxt->width = width;
				codec_cxt->height = height;
				codec_cxt->time_base = av_d2q(1 / frame_rate, 255);
				codec_cxt->pix_fmt = codec->pix_fmts[0];
				codec_cxt->gop_size = 600;
				codec_cxt->bit_rate = bit_rate;
				break;
			case AVMEDIA_TYPE_AUDIO:
				file->format_cxt->audio_codec_id = codec->id;
				codec_cxt->sample_fmt = codec->sample_fmts[0];
				codec_cxt->time_base = av_d2q(1 / sample_rate, 255);
				codec_cxt->sample_rate = sample_rate;
				codec_cxt->channel_layout = AV_CH_LAYOUT_STEREO;
				codec_cxt->channels = 2;
				codec_cxt->global_quality = 3540;
				codec_cxt->flags |= CODEC_FLAG_QSCALE;
				break;
			case AVMEDIA_TYPE_SUBTITLE:
				file->format_cxt->subtitle_codec_id = codec->id;
				break;
			default:
				break;
		}

		if (avcodec_open2(codec_cxt, codec, NULL) < 0) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "Unable to open codec '%s'", codec->name);
			return;
		}

		frame = avcodec_alloc_frame();
		frame->pts = 0;
	    if(file->output_format->flags & AVFMT_GLOBALHEADER) {
	    	codec_cxt->flags |= CODEC_FLAG_GLOBAL_HEADER;
	    }
	}

	strm = emalloc(sizeof(av_stream));
	memset(strm, 0, sizeof(av_stream));
	strm->stream = stream;
	strm->codec_cxt = codec_cxt;
	strm->codec = codec_cxt->codec;
	strm->packet_queue_size = 32;
	strm->packet_queue = emalloc(sizeof(AVPacket) * strm->packet_queue_size);
	memset(strm->packet_queue, 0, sizeof(AVPacket) * strm->packet_queue_size);
	strm->file = file;
	strm->frame = frame;
	strm->index = stream_index;
	codec_cxt->opaque = strm;

	if((uint32_t) stream_index >= file->stream_count) {
		file->stream_count = stream_index + 1;
		file->streams = erealloc(file->streams, sizeof(av_stream *) * file->stream_count);
	}
	file->streams[stream_index] = strm;
	file->open_stream_count++;
	ZEND_REGISTER_RESOURCE(return_value, strm, le_av_strm);
}
/* }}} */

static int av_read_next_packet(av_stream *strm TSRMLS_DC) {
	if(strm->packet) {
		if(strm->packet_bytes_remaining > 0) {
			return TRUE;
		} else {
			av_shift_packet(strm);
		}
	}
	if(!strm->packet) {
		av_file *file = strm->file;
		av_stream *dst_strm = NULL;
		do {
			AVPacket *packet = emalloc(sizeof(AVPacket));
			memset(packet, 0, sizeof(AVPacket));
			if(av_read_frame(file->format_cxt, packet) < 0) {
				file->flags |= AV_FILE_EOF_REACHED;
				efree(packet);
				break;
			}
			if(packet->stream_index >= 0 && (uint32_t) packet->stream_index < file->stream_count) {
				dst_strm = file->streams[packet->stream_index];
			} else {
				php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Invalid stream index: %d", packet->stream_index);
				dst_strm = NULL;
			}
			if(dst_strm) {
				av_push_packet(dst_strm, packet);
			} else {
				av_free_packet(packet);
				efree(packet);
			}
		} while(dst_strm != strm);
	}
	return (strm->packet_bytes_remaining);
}

static int av_write_next_packet(av_stream *strm, AVPacket *packet) {
	av_file *file = strm->file;
	AVPacket *next_packet = NULL;
	av_stream *next_stream = NULL;
	uint32_t i;

	av_push_packet(strm, packet);

	for(i = 0; i < file->stream_count; i++) {
		strm = file->streams[i];
		if(strm && strm->packet) {
			if(!next_packet || strm->packet->pts < next_packet->pts) {
				next_packet = strm->packet;
				next_stream = strm;
			}
		}
	}

	printf("Stream %d\n", next_packet->stream_index);
	if(av_interleaved_write_frame(file->format_cxt, next_packet) < 0) {
		return FALSE;
	}
	av_shift_packet(next_stream);
	return TRUE;
}

#define FOR_ENCODING		0
#define FOR_DECODING		1

static void av_create_picture_and_scalar(av_stream *strm, uint32_t width, uint32_t height, int purpose) {
	if(!strm->picture || strm->picture->width != width || strm->picture->height != height) {
		if(strm->picture) {
			avpicture_free((AVPicture *) strm->picture);
		} else {
			strm->picture = avcodec_alloc_frame();
		}
		avpicture_alloc((AVPicture *) strm->picture, PIX_FMT_RGBA, width, height);
		strm->picture->width = width;
		strm->picture->height = height;
		if(purpose == FOR_ENCODING) {
			strm->scalar_cxt = sws_getCachedContext(strm->scalar_cxt, width, height, PIX_FMT_RGBA, strm->codec_cxt->width, strm->codec_cxt->height, strm->codec_cxt->pix_fmt, SWS_FAST_BILINEAR, NULL, NULL, NULL);
		} else {
			strm->scalar_cxt = sws_getCachedContext(strm->scalar_cxt, strm->codec_cxt->width, strm->codec_cxt->height, strm->codec_cxt->pix_fmt, width, height, PIX_FMT_RGBA, SWS_FAST_BILINEAR, NULL, NULL, NULL);
		}
	}
}

static void av_create_audio_buffer_and_resampler(av_stream *strm, int purpose) {
	if(!strm->samples) {
		double frame_duration = (double) strm->codec_cxt->frame_size / strm->codec_cxt->sample_rate;
		strm->sample_buffer_size = (uint32_t) (frame_duration * 44100);

		if(purpose == FOR_ENCODING) {
			strm->resampler_cxt = swr_alloc_set_opts(NULL, strm->codec_cxt->channel_layout, strm->codec_cxt->sample_fmt, strm->codec_cxt->sample_rate, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_FLT, 44100, 0, NULL);
		} else {
			strm->resampler_cxt = swr_alloc_set_opts(NULL, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_FLT, 44100, strm->codec_cxt->channel_layout, strm->codec_cxt->sample_fmt, strm->codec_cxt->sample_rate, 0, NULL);
		}
		swr_init(strm->resampler_cxt);
		strm->samples = emalloc(sizeof(float) * strm->sample_buffer_size * 2);
	}
}

static void av_transfer_picture_to_frame(av_stream *strm) {
	// allocate the frame buffer if it's not there
	if(!strm->frame->data[0]) {
		avpicture_alloc((AVPicture *) strm->frame, strm->codec_cxt->pix_fmt, strm->codec_cxt->width, strm->codec_cxt->height);
		strm->frame->width = strm->codec_cxt->width;
		strm->frame->height = strm->codec_cxt->height;
	}
	// rescale the picture to the proper dimension and transform pixels to format used by codec
	sws_scale(strm->scalar_cxt, (const uint8_t * const *) strm->picture->data, strm->picture->linesize, 0, strm->picture->height, strm->frame->data, strm->frame->linesize);
}

static void av_transfer_picture_from_frame(av_stream *strm) {
	// rescale the picture and transform pixels to RGBA
	sws_scale(strm->scalar_cxt, (const uint8_t * const *) strm->frame->data, strm->frame->linesize, 0, strm->frame->height, strm->picture->data, strm->picture->linesize);
}

static void av_transfer_pcm_to_frame(av_stream *strm) {
	int result;

	// allocate the audio frame if it's not there
	if(!strm->frame->data[0]) {
		uint32_t buffer_size = av_samples_get_buffer_size(NULL, strm->codec_cxt->channels, strm->codec_cxt->frame_size, strm->codec_cxt->sample_fmt, 1);
	    float *samples = av_malloc(buffer_size);

	    strm->frame->format = strm->codec_cxt->sample_fmt;
	    strm->frame->nb_samples = strm->codec_cxt->frame_size;
		avcodec_fill_audio_frame(strm->frame, strm->codec_cxt->channels, strm->codec_cxt->sample_fmt, (uint8_t *) samples, buffer_size, 1);
	}

	result = swr_convert(strm->resampler_cxt, (uint8_t **) strm->frame->data, strm->frame->nb_samples, (const uint8_t **) &strm->samples, strm->sample_buffer_size);
}

static void av_transfer_pcm_from_frame(av_stream *strm) {
	int result;

	result = swr_convert(strm->resampler_cxt, (uint8_t **) &strm->samples, strm->sample_buffer_size, (const uint8_t **) strm->frame->data, strm->frame->nb_samples);
}

static void av_copy_image_from_gd(AVFrame *picture, gdImagePtr image) {
	int *gd_pixel;
	uint8_t *av_pixel;
	uint32_t i, j;
	for(i = 0; i < (uint32_t) image->sy; i++) {
		gd_pixel = image->tpixels[i];
		av_pixel = picture->data[0] + picture->linesize[0] * i;
		for(j = 0; j < (uint32_t) image->sx; j++) {
			av_pixel[0] = gdTrueColorGetRed(*gd_pixel);
			av_pixel[1] = gdTrueColorGetGreen(*gd_pixel);
			av_pixel[2] = gdTrueColorGetBlue(*gd_pixel);
			av_pixel[3] = (gdAlphaMax - gdTrueColorGetAlpha(*gd_pixel)) << 1;
			gd_pixel += 1;
			av_pixel += 4;
		}
	}
}

static void av_copy_image_to_gd(AVFrame *picture, gdImagePtr image) {
	int *gd_pixel;
	uint8_t *av_pixel;
	uint32_t i, j;
	for(i = 0; i < (uint32_t) image->sy; i++) {
		gd_pixel = image->tpixels[i];
		av_pixel = picture->data[0] + picture->linesize[0] * i;
		for(j = 0; j < (uint32_t) image->sx; j++) {
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

#ifndef HAVE_AVCODEC_ENCODE_VIDEO2

int avcodec_encode_video2(AVCodecContext *avctx, AVPacket *avpkt, const AVFrame *frame, int *got_packet_ptr) {
	int ret;
	TSRMLS_FETCH();

	if(!AV_G(video_buffer)) {
		AV_G(video_buffer_size) = 100000;
		AV_G(video_buffer) = emalloc(AV_G(video_buffer_size));
	}
	ret = avcodec_encode_video(avctx, AV_G(video_buffer), AV_G(video_buffer_size), frame);

	if(ret > 0) {
		avpkt->size = ret;
		avpkt->data = av_realloc(avpkt->data, avpkt->size + FF_INPUT_BUFFER_PADDING_SIZE);
		memcpy(avpkt->data, AV_G(video_buffer), ret);
		*got_packet_ptr = TRUE;
	}
	return ret;
}

#endif

static int av_encode_next_frame(av_stream *strm, double time) {
	av_file *file = strm->file;
	int packet_finished;
	int result;
	AVPacket *packet = emalloc(sizeof(AVPacket));
	memset(packet, 0, sizeof(AVPacket));

	int64_t presentation_time = (int64_t) (time / av_q2d(strm->codec_cxt->time_base));

	if(!(file->flags & AV_FILE_HEADER_WRITTEN)) {
		avformat_write_header(file->format_cxt, NULL);
		file->flags |= AV_FILE_HEADER_WRITTEN;
	}

	av_init_packet(packet);
	switch(strm->codec->type) {
		case AVMEDIA_TYPE_VIDEO:
			strm->frame->pts = presentation_time;
			result = avcodec_encode_video2(strm->codec_cxt, packet, strm->frame, &packet_finished);
			break;
		case AVMEDIA_TYPE_AUDIO:
			result = avcodec_encode_audio2(strm->codec_cxt, packet, strm->frame, &packet_finished);
			break;
		case AVMEDIA_TYPE_SUBTITLE:
			break;
		default:
			break;
	}

	if(result < 0) {
		return FALSE;
	}
	if(packet_finished) {
		packet->pts = av_rescale_q(presentation_time, strm->codec_cxt->time_base, strm->stream->time_base);
		if(strm->codec_cxt->coded_frame && strm->codec_cxt->coded_frame->key_frame) {
			packet->flags |= AV_PKT_FLAG_KEY;
		}
		packet->stream_index = strm->stream->index;
		av_write_next_packet(strm, packet);
	} else {
		av_free_packet(packet);
		efree(packet);
	}
	return TRUE;
}

static int av_decode_next_frame(av_stream *strm, double *p_time TSRMLS_DC) {
	int frame_finished = FALSE;
	for(;;) {
		int bytes_decoded;
		if(av_read_next_packet(strm TSRMLS_CC)) {
			switch(strm->codec->type) {
				case AVMEDIA_TYPE_VIDEO:
					bytes_decoded = avcodec_decode_video2(strm->codec_cxt, strm->frame, &frame_finished, strm->packet);
					break;
				case AVMEDIA_TYPE_AUDIO:
					bytes_decoded = avcodec_decode_audio4(strm->codec_cxt, strm->frame, &frame_finished, strm->packet);
					break;
				case AVMEDIA_TYPE_SUBTITLE:
					//bytes_decoded = avcodec_decode_subtitle2(strm->codec_cxt, dec->frame, &audio_frame_finished, strm->packet);
					break;
				default:
					bytes_decoded = -1;
			}
		} else {
			return FALSE;
		}
		if(bytes_decoded < 0) {
			return FALSE;
		}
		strm->packet_bytes_remaining -= bytes_decoded;

		if(frame_finished) {
			int64_t time_stamp = 0;
			if(strm->packet) {
				// use the packet's dts if it's available; otherwise
				// use the pts of the packet that triggered the creation
				// of the frame; the timestamp is the number of
				// time-united employed in the stream
				if(strm->packet->dts != AV_NOPTS_VALUE) {
					time_stamp = strm->packet->dts;
				} else if(strm->frame_pts != AV_NOPTS_VALUE) {
					time_stamp = strm->frame_pts;
				}
			}
			*p_time = time_stamp * av_q2d(strm->stream->time_base);
			break;
		}
	}
	if(!strm->packet_bytes_remaining) {
		av_shift_packet(strm);
	}
	return TRUE;
}

static int av_encode_image_from_gd(av_stream *strm, gdImagePtr image, double time TSRMLS_DC) {
	av_create_picture_and_scalar(strm, image->sx, image->sy, FOR_ENCODING);
	av_copy_image_from_gd(strm->picture, image);
	av_transfer_picture_to_frame(strm);
	return av_encode_next_frame(strm, time);
}

static int av_decode_image_to_gd(av_stream *strm, gdImagePtr image, double *p_time TSRMLS_DC) {
	if(av_decode_next_frame(strm, p_time TSRMLS_CC)) {
		av_create_picture_and_scalar(strm, image->sx, image->sy, FOR_DECODING);
		av_transfer_picture_from_frame(strm);
		av_copy_image_to_gd(strm->picture, image);
		return TRUE;
	}
	return FALSE;
}

static int av_encode_pcm_from_zval(av_stream *strm, zval *buffer, double time TSRMLS_DC) {
	float *src_samples, *dst_samples;
	uint32_t src_samples_remaining;

	if(Z_TYPE_P(buffer) != IS_STRING) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Audio data must be contained in a string");
		return FALSE;
	}

	av_create_audio_buffer_and_resampler(strm, FOR_ENCODING);

	// source samples are assumed to be stereo--hence the x2
	src_samples = (float *) Z_STRVAL_P(buffer);
	src_samples_remaining = Z_STRLEN_P(buffer) / (sizeof(float) * 2);
	dst_samples = strm->samples + strm->sample_count * 2;

	// keep copying into audio frame until are samples are used up
	while(src_samples_remaining) {
		uint32_t samples_needed = strm->sample_buffer_size - strm->sample_count;
		uint32_t samples_to_copy = (samples_needed < src_samples_remaining) ? samples_needed : src_samples_remaining;

		memcpy(dst_samples, src_samples, (sizeof(float) * 2) * samples_to_copy);
		src_samples_remaining -= samples_to_copy;
		strm->sample_count += samples_to_copy;
		src_samples += samples_to_copy * 2;
		dst_samples += samples_to_copy * 2;

		if(strm->sample_count == strm->sample_buffer_size) {
			// transfer the data to the frame then compress it
			av_transfer_pcm_to_frame(strm);
			if(!av_encode_next_frame(strm, time)) {
				return FALSE;
			}
			strm->sample_count = 0;
			dst_samples = strm->samples;

			// adjust the time
			time += samples_to_copy * (1 / 44100.0 / 2);
		}
	}
	return TRUE;
}


static int av_decode_pcm_to_zval(av_stream *strm, zval *buffer, double *p_time TSRMLS_DC) {
	if(av_decode_next_frame(strm, p_time TSRMLS_CC)) {
		uint32_t data_len;

		av_create_audio_buffer_and_resampler(strm, FOR_DECODING);
		av_transfer_pcm_from_frame(strm);

		data_len = sizeof(float) * 2 * strm->sample_buffer_size;
		if(Z_TYPE_P(buffer) == IS_STRING) {
			Z_STRVAL_P(buffer) = erealloc(Z_STRVAL_P(buffer), data_len + 1);
		} else {
			zval_dtor(buffer);
			Z_TYPE_P(buffer) = IS_STRING;
			Z_STRVAL_P(buffer) = emalloc(data_len + 1);
		}
		Z_STRLEN_P(buffer) = data_len;
		memcpy(Z_STRVAL_P(buffer), strm->samples, data_len);
		Z_STRVAL_P(buffer)[data_len] = '\0';
		return TRUE;
	} else {
		Z_TYPE_P(buffer) = IS_NULL;
		return FALSE;
	}
}

/* {{{ proto bool av_stream_write_image()
   Write an image */
PHP_FUNCTION(av_stream_write_image)
{
	zval *z_strm, *z_img;
	av_stream *strm;
	gdImagePtr image;
	double time = NAN;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr|d", &z_strm, &z_img, &time) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(strm, av_stream *, &z_strm, -1, "av stream", le_av_strm);
	ZEND_FETCH_RESOURCE(image, gdImagePtr, &z_img, -1, "image", le_gd);
	if(av_encode_image_from_gd(strm, image, time TSRMLS_CC)) {
		RETURN_TRUE;
	} else {
		RETVAL_FALSE;
	}
}
/* }}} */

/* {{{ proto bool av_stream_write_pcm()
   Write audio data */
PHP_FUNCTION(av_stream_write_pcm)
{
	zval *z_strm, *z_buffer;
	av_stream *strm;
	double time = NAN;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rz|d", &z_strm, &z_buffer, &time) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(strm, av_stream *, &z_strm, -1, "av stream", le_av_strm);
	if(av_encode_pcm_from_zval(strm, z_buffer, time TSRMLS_CC)) {
		RETURN_TRUE;
	} else {
		RETVAL_FALSE;
	}
}
/* }}} */

/* {{{ proto string av_stream_read_image()
   Read an image */
PHP_FUNCTION(av_stream_read_image)
{
	zval *z_strm, *z_img, *z_time = NULL;
	av_stream *strm;
	gdImagePtr image;
	double time;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr|z", &z_strm, &z_img, &z_time) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(strm, av_stream *, &z_strm, -1, "av stream", le_av_strm);
	ZEND_FETCH_RESOURCE(image, gdImagePtr, &z_img, -1, "image", le_gd);

	if(av_decode_image_to_gd(strm, image, &time TSRMLS_CC)) {
		if(z_time) {
			zval_dtor(z_time);
			ZVAL_DOUBLE(z_time, time);
		}
		RETURN_TRUE;
	} else {
		RETVAL_FALSE;
	}
}
/* }}} */

/* {{{ proto string av_stream_read_pcm()
   Read audio data */
PHP_FUNCTION(av_stream_read_pcm)
{
	zval *z_strm, *z_buffer, *z_time = NULL;
	av_stream *strm;
	double time;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rz|z", &z_strm, &z_buffer, &z_time) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(strm, av_stream *, &z_strm, -1, "av stream", le_av_strm);

	if(av_decode_pcm_to_zval(strm, z_buffer, &time TSRMLS_CC)) {
		if(z_time) {
			zval_dtor(z_time);
			ZVAL_DOUBLE(z_time, time);
		}
		RETURN_TRUE;
	} else {
		RETVAL_FALSE;
	}
}
/* }}} */

/* {{{ proto string av_stream_get_duration(resource res)
   Get duration of av stream */
PHP_FUNCTION(av_stream_get_duration)
{
	zval *z_strm;
	av_stream *strm;
	double duration;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_strm) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(strm, av_stream *, &z_strm, -1, "av stream", le_av_strm);

	duration = strm->stream->duration * av_q2d(strm->stream->time_base);
	RETVAL_DOUBLE(duration);
}
/* }}} */

/* {{{ proto string av_stream_close(resource res)
   Close an av stream */
PHP_FUNCTION(av_stream_close)
{
	zval *z_strm;
	av_stream *strm;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_strm) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(strm, av_stream *, &z_strm, -1, "av stream", le_av_strm);
	RETVAL_TRUE;
	zend_list_delete(Z_LVAL_P(z_strm));
}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
