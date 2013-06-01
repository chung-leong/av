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

ZEND_BEGIN_ARG_INFO_EX(arginfo_av_file_seek, 0, 0, 2)
	ZEND_ARG_INFO(0, file)
	ZEND_ARG_INFO(0, time)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_av_file_eof, 0, 0, 1)
    ZEND_ARG_INFO(0, file)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_av_file_stat, 0, 0, 1)
	ZEND_ARG_INFO(0, stream)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_av_file_optimize, 0, 0, 1)
	ZEND_ARG_INFO(0, path)
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
    ZEND_ARG_INFO(1, time)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_av_stream_read_pcm, 0, 0, 2)
    ZEND_ARG_INFO(0, stream)
    ZEND_ARG_INFO(1, buffer)
    ZEND_ARG_INFO(1, time)
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

/* }}} */

/* {{{ av_functions[]
 *
 * Every user visible function must have an entry in av_functions[].
 */
const zend_function_entry av_functions[] = {
	PHP_FE(av_file_open,				arginfo_av_file_open)
	PHP_FE(av_file_close,				arginfo_av_file_close)
	PHP_FE(av_file_seek,				arginfo_av_file_seek)
	PHP_FE(av_file_eof,					arginfo_av_file_eof)
	PHP_FE(av_file_stat,				arginfo_av_file_stat)
	PHP_FE(av_file_optimize,			arginfo_av_file_optimize)

	PHP_FE(av_stream_open,				arginfo_av_stream_open)
	PHP_FE(av_stream_close,				arginfo_av_stream_close)
	PHP_FE(av_stream_read_image,		arginfo_av_stream_read_image)
	PHP_FE(av_stream_read_pcm,			arginfo_av_stream_read_pcm)
	PHP_FE(av_stream_write_image,		arginfo_av_stream_write_image)
	PHP_FE(av_stream_write_pcm,			arginfo_av_stream_write_pcm)

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
	STRING(AV_MAJOR_VERSION) "." STRING(AV_MINOR_VERSION), /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_AV
ZEND_GET_MODULE(av)
#endif

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("av.optimize_output", "1", PHP_INI_ALL, OnUpdateBool, optimize_output, zend_av_globals, av_globals)
    STD_PHP_INI_ENTRY("av.verbose_reporting", "0", PHP_INI_ALL, OnUpdateBool, verbose_reporting, zend_av_globals, av_globals)

	STD_PHP_INI_ENTRY("av.max_threads_per_stream", "2", PHP_INI_SYSTEM, OnUpdateLong, max_threads_per_stream, zend_av_globals, av_globals)
    STD_PHP_INI_ENTRY("av.threads_per_video_stream", "2", PHP_INI_ALL, OnUpdateLong, threads_per_video_stream, zend_av_globals, av_globals)
    STD_PHP_INI_ENTRY("av.threads_per_audio_stream", "2", PHP_INI_ALL, OnUpdateLong, threads_per_audio_stream, zend_av_globals, av_globals)
PHP_INI_END()
/* }}} */

/* {{{ php_av_init_globals
 */
static void php_av_init_globals(zend_av_globals *av_globals)
{
	av_globals->optimize_output = TRUE;
	av_globals->verbose_reporting = FALSE;
	av_globals->max_threads_per_stream = 2;
	av_globals->threads_per_video_stream = 2;
	av_globals->threads_per_audio_stream = 2;
}
/* }}} */

static int av_is_qt_format(AVOutputFormat *of) {
	AVOutputFormat *qt = av_guess_format("mov", NULL, NULL);
	if(qt && qt->write_header == of->write_header && qt->write_packet == of->write_packet && qt->write_trailer == of->write_trailer) {
		return TRUE;
	}
	return FALSE;
}

static int av_flush_pending_packets(av_file *file);

#ifndef HAVE_AVCODEC_FREE_FRAME
void avcodec_free_frame(AVFrame **frame)
{
    AVFrame *f;

    if (!frame || !*frame)
        return;

    f = *frame;

    if (f->extended_data != f->data) {
    	if (f->extended_data) {
    		//av_freep(&f->extended_data);
    	}
    }

    av_freep(frame);
}
#endif

static void av_free_file(av_file *file) {
	file->flags |= AV_FILE_FREED;
	// don't free anything until all streams are closed
	if(file->open_stream_count == 0) {
		uint32_t i = 0, j = 0;
		if(file->flags & AV_FILE_WRITE) {
			av_flush_pending_packets(file);
			if(file->flags & AV_FILE_HEADER_WRITTEN) {
				av_write_trailer(file->format_cxt);
			}
		}

		// free the streams
		for(i = 0; i < file->stream_count; i++) {
			av_stream *strm = file->streams[i];
			if(strm) {
				avcodec_close(strm->codec_cxt);
				if(strm->frame) {
					if(strm->flags & AV_STREAM_FRAME_BUFFER_ALLOCATED) {
						avpicture_free((AVPicture *) strm->frame);
					} else if(strm->flags & AV_STREAM_AUDIO_BUFFER_ALLOCATED) {
						av_free(strm->frame->extended_data[0]);
					}
					avcodec_free_frame(&strm->frame);
				}
				if(strm->next_frame) {
					avcodec_free_frame(&strm->next_frame);
				}
				if(strm->picture) {
					avpicture_free((AVPicture *) strm->picture);
					avcodec_free_frame(&strm->picture);
				}
				if(strm->scaler_cxt) {
					sws_freeContext(strm->scaler_cxt);
				}
				if(strm->resampler_cxt) {
#if defined(HAVE_SWRESAMPLE)
					swr_free(&strm->resampler_cxt);
#elif defined(HAVE_AVRESAMPLE)
					avresample_close(strm->resampler_cxt);
					avresample_free(&strm->resampler_cxt);
#else
					audio_resample_close(strm->resampler_cxt);
					if(strm->deplanarized_samples) {
						av_free(strm->deplanarized_samples);
					}
#endif
				}
				if(strm->packet) {
					av_free_packet(strm->packet);
					efree(strm->packet);
				}
				if(strm->packet_queue) {
					for(j = 0; j < strm->packet_count; j++) {
						av_free_packet(strm->packet_queue[j]);
						efree(strm->packet_queue[j]);
					}
					efree(strm->packet_queue);
				}
				if(strm->samples) {
					av_free(strm->samples);
				}
				efree(strm);
			}
		}
		if(file->streams) {
			efree(file->streams);
		}
		if(file->flags & AV_FILE_WRITE) {
			TSRMLS_FETCH();
			if(AV_G(optimize_output)) {
				if(av_is_qt_format(file->format_cxt->oformat)) {
					avio_flush(file->format_cxt->pb);
					av_optimize_mov_file(file->format_cxt->pb);
				}
			}
	        avio_close(file->format_cxt->pb);
			avformat_free_context(file->format_cxt);
		} else {
			avformat_close_input(&file->format_cxt);
		}
		efree(file);
	}
}

static void av_free_stream(av_stream *strm) {
	if(!(strm->flags & AV_STREAM_FREED)) {
		av_file *file = strm->file;
		file->open_stream_count--;
		strm->flags |= AV_STREAM_FREED;
		if(file->open_stream_count == 0) {
			if(file->flags & AV_FILE_FREED) {
				// free the file if no zval is referencing it
				av_free_file(file);
			}
		}
	}
}

static void php_free_av_file(zend_rsrc_list_entry *rsrc TSRMLS_DC) {
	av_free_file(rsrc->ptr);
}

static void php_free_av_stream(zend_rsrc_list_entry *rsrc TSRMLS_DC) {
	av_free_stream(rsrc->ptr);
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

static void av_set_log_level(TSRMLS_D) {
	if(AV_G(verbose_reporting)) {
		av_log_set_level(AV_LOG_VERBOSE);
	} else {
		av_log_set_level(AV_LOG_FATAL);
	}
}

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(av)
{
	ZEND_INIT_MODULE_GLOBALS(av, php_av_init_globals, NULL);

	REGISTER_INI_ENTRIES();

	av_set_log_level(TSRMLS_C);
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
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

#ifdef USE_CUSTOM_MALLOC

void *custom_malloc(size_t size) {
	void *p = emalloc(size);
	return p;
}

void *custom_realloc(void *ptr, size_t size) {
	void *p = erealloc(ptr, size);
	return p;
}

void custom_free(void *ptr) {
	if(ptr) {
		efree(ptr);
	}
}

#endif

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(av)
{
	if(le_gd == -1) {
		le_gd = zend_fetch_list_dtor_id("gd");
	}
#ifndef HAVE_AVCODEC_ENCODE_VIDEO2
	AV_G(video_buffer) = NULL;
	AV_G(video_buffer_size) = 0;
#endif
#ifdef USE_CUSTOM_MALLOC
	av_set_custom_malloc(custom_malloc, custom_realloc, custom_free);
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(av)
{
#ifndef HAVE_AVCODEC_ENCODE_VIDEO2
	if(AV_G(video_buffer)) {
		efree(AV_G(video_buffer));
	}
#endif
	return SUCCESS;
}
/* }}} */

static char *av_insert_spaces(const char *list, char *buffer, size_t buffer_size) {
	if(list && list[0]) {
		// add space after comma so the list doesn't blow out the table
		uint32_t i, j;
		for(i = 0, j = 0; list[i] != '\0'; i++) {
			if(list[i] == ',') {
				if(list[i + 1] != ' ' && list[i + 1] != '\0') {
					buffer[j++] = ',';
					buffer[j++] = ' ';
				} else {
					buffer[j++] = ',';
				}
			} else {
				buffer[j++] = list[i];
			}
			if(j + 2 > buffer_size) {
				break;
			}
		}
		buffer[j] = '\0';
		return buffer;
	} else {
		return NULL;
	}
}

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(av)
{
    AVOutputFormat *ofmt = NULL;
    AVInputFormat *ifmt = NULL;
 	char name_buffer[1024], ext_buffer[1024];

	php_info_print_table_start();
	php_info_print_table_header(2, "av support", "enabled");
	php_info_print_table_row(2, "Version", STRING(AV_MAJOR_VERSION) "." STRING(AV_MINOR_VERSION));
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();

	php_info_print_table_start();
	php_info_print_table_colspan_header(4, "Output formats");
	php_info_print_table_header(4, "Name", "Short name", "MIME type", "Extensions");
    while((ofmt = av_oformat_next(ofmt))) {
    	php_info_print_table_row(4, ofmt->long_name, av_insert_spaces(ofmt->name, name_buffer, sizeof(name_buffer)), ofmt->mime_type, av_insert_spaces(ofmt->extensions, ext_buffer, sizeof(ext_buffer)));
    }
	php_info_print_table_end();

	php_info_print_table_start();
	php_info_print_table_colspan_header(3, "Input formats");
	php_info_print_table_header(3, "Name", "Short name", "Extensions");
    while((ifmt = av_iformat_next(ifmt))) {
    	php_info_print_table_row(3, ifmt->long_name, av_insert_spaces(ifmt->name, name_buffer, sizeof(name_buffer)), av_insert_spaces(ifmt->extensions, ext_buffer, sizeof(ext_buffer)));
    }
	php_info_print_table_end();
}
/* }}} */

int av_copy_metadata(AVDictionary **dict, zval *options TSRMLS_DC) {
	if(options) {
		zval **p_metadata;
		if(zend_hash_find(Z_ARRVAL_P(options), "metadata", 9, (void **) &p_metadata) == SUCCESS) {
			zval *metadata = *p_metadata;
			if(Z_TYPE_P(metadata) == IS_ARRAY) {
				Bucket *p;
				HashTable *ht = Z_ARRVAL_P(metadata);
				for(p = ht->pListHead; p; p = p->pListNext) {
					if(p->nKeyLength > 0) {
						zval **p_element = p->pData, *element = *p_element;
						const char *key, *value;
						convert_to_string(element);
						key = p->arKey;
						value = Z_STRVAL_P(element);
						av_dict_set(dict, key, value, 0);
					}
				}
			} else {
				php_error_docref(NULL TSRMLS_CC, E_ERROR, "metadata should be an array");
				return FALSE;
			}

		}
	}
	return TRUE;
}

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

	av_set_log_level(TSRMLS_C);

	for(code = mode; *code; code++) {
		if(*code == 'r') {
			flags = AV_FILE_READ;
		} else if(*code == 'w') {
			flags = AV_FILE_WRITE;
		}
	}

	if(flags & AV_FILE_READ) {
		if (avformat_open_input(&format_cxt, filename, NULL, NULL) < 0) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error opening file for reading: %s", filename);
			return;
		}
		if (avformat_find_stream_info(format_cxt, NULL) < 0) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error finding stream info: %s", filename);
			return;
		}
		input_format = format_cxt->iformat;
	} else if(flags & AV_FILE_WRITE) {
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
			if(avio_open(&pb, filename, AVIO_FLAG_READ_WRITE) < 0) {
				php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error opening file for writing: %s", filename);
				return;
			}
		}
		format_cxt = avformat_alloc_context();
		format_cxt->pb = pb;
		format_cxt->oformat = output_format;

		// copy metadata
		av_copy_metadata(&format_cxt->metadata, z_options TSRMLS_CC);
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
	zval *z_file;
	av_file *file;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_file) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(file, av_file *, &z_file, -1, "av file", le_av_file);

	av_set_log_level(TSRMLS_C);

	if(file->open_stream_count == 0) {
		RETVAL_TRUE;
	} else {
		RETVAL_FALSE;
	}
	zend_list_delete(Z_LVAL_P(z_file));
}
/* }}} */

static int av_seek_file(av_file *file, double time, int precise) {
	double time_unit;
	int64_t min_time_stamp, time_stamp, max_time_stamp;
	int32_t stream_index = -1;
	uint32_t i;

	// get rid of packets that were placed in each stream's queue
	for(i = 0; i < file->stream_count; i++) {
		av_stream *strm = file->streams[i];
		if(strm) {
			while(strm->packet) {
				av_shift_packet(strm);
			}
			if(strm->next_frame) {
				// remove the next frame as well(retrieved by a previous precise seek) 
				avcodec_free_frame(&strm->next_frame);
			    strm->next_frame = NULL;
			    strm->next_frame_time = 0;
			}
		}
	}

	// use the video stream if there's one opened
	for(i = 0; i < file->stream_count; i++) {
		av_stream *strm = file->streams[i];
		if(strm && !(strm->flags & AV_STREAM_FREED)) {
			if(strm->codec->type == AVMEDIA_TYPE_VIDEO) {
				stream_index = i;
				break;
			}
		}
	}

	// use the first opened stream
	if(stream_index == -1) {
		for(i = 0; i < file->stream_count; i++) {
			av_stream *strm = file->streams[i];
			if(strm && !(strm->flags & AV_STREAM_FREED)) {
				stream_index = i;
				break;
			}
		}
	}

	// use the best video stream if no stream was opened
	if(stream_index == -1) {
		int32_t best_stream_index = av_find_best_stream(file->format_cxt, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
		if(best_stream_index >= 0) {
			stream_index = best_stream_index;
		}
	}

	// let ffmpeg choose the stream otherwise
	if(stream_index == -1) {
		time_unit = 1.0 / AV_TIME_BASE;
	} else {
		AVStream *s = file->format_cxt->streams[stream_index];
		time_unit = av_q2d(s->time_base);
	}

	// not setting a minimum--decode to the correct position if we have to
	// never seek beyond the time given
	min_time_stamp = (int64_t) (-1 / time_unit);
	time_stamp = (int64_t) (time / time_unit);
	max_time_stamp = time_stamp;

	if(avformat_seek_file(file->format_cxt, stream_index, min_time_stamp, time_stamp, max_time_stamp, 0) < 0) {
		return FALSE;
	}

	// note that a seek has occured
	for(i = 0; i < file->stream_count; i++) {
		av_stream *strm = file->streams[i];
		if(strm) {
			if(precise) {
				// note the time so we can decode to it
				strm->flags |= AV_STREAM_SOUGHT;
				strm->time_sought = time;
			} else {
				// just in case a seek had occurred before
				strm->flags &= ~AV_STREAM_SOUGHT;
				strm->time_sought = 0;
			}
		}
	}

	// clear eof flag
	file->flags &= ~AV_FILE_EOF_REACHED;
	return TRUE;
}

/* {{{ proto string av_file_seek(resource file, double time)
   Seek to a specific time */
PHP_FUNCTION(av_file_seek)
{
	zval *z_file;
	av_file *file;
	double time;
	zend_bool precise = TRUE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rd|b", &z_file, &time, &precise) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(file, av_file *, &z_file, -1, "av file", le_av_file);

	av_set_log_level(TSRMLS_C);

	if(av_seek_file(file, time, precise)) {
		RETVAL_TRUE;
	} else {
		RETVAL_FALSE;
	}
}
/* }}} */

/* {{{ proto string av_file_eof(resource file)
   Indicate whether there's more contents */
PHP_FUNCTION(av_file_eof)
{
	zval *z_file;
	av_file *file;
	int32_t eof = FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_file) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(file, av_file *, &z_file, -1, "av file", le_av_file);

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

#define ADD_STRINGL(zv, name, value, value_length)	\
{\
	zval *element;\
	MAKE_STD_ZVAL(element);\
	ZVAL_STRINGL(element, (value) ? value : "", value_length, TRUE);\
	zend_hash_update(HASH_OF(zv), name, strlen(name) + 1, (void *) &element, sizeof(zval *), NULL);\
}\

static uint32_t av_get_name_length(const char *s) {
	uint32_t i = 0;
	while(s && s[i] != ',' && s[i] != '\0') {
		i++;
	}
	return i;
}

/* {{{ proto string av_file_stat(resource file)
   Return information about media file */
PHP_FUNCTION(av_file_stat)
{
	zval *z_file;
	av_file *file;
	AVFormatContext *f;
	zval *streams, *metadata;
	uint32_t i;
	AVDictionaryEntry *e;
	const char *format, *format_name;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_file) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(file, av_file *, &z_file, -1, "av file", le_av_file);
	f = file->format_cxt;

	av_set_log_level(TSRMLS_C);

	array_init(return_value);

	format = (f->iformat) ? f->iformat->name : f->oformat->name;
	format_name = (f->iformat) ? f->iformat->long_name : f->oformat->long_name;
	ADD_STRINGL(return_value, "format", format, av_get_name_length(format));
	ADD_STRING(return_value, "format_name", format_name);
	ADD_LONG(return_value, "bit_rate", f->bit_rate);
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
		AVCodec *d = avcodec_find_decoder(c->codec_id);
		const char *stream_type, *codec, *codec_name;

		MAKE_STD_ZVAL(stream);
		array_init(stream);
		zend_hash_next_index_insert(HASH_OF(streams), (void *) &stream, sizeof(zval *), NULL);

	    switch (c->codec_type) {
	    	case AVMEDIA_TYPE_VIDEO: stream_type = "video"; break;
	    	case AVMEDIA_TYPE_AUDIO: stream_type = "audio"; break;
	    	case AVMEDIA_TYPE_DATA: stream_type = "data"; break;
	    	case AVMEDIA_TYPE_SUBTITLE: stream_type = "subtitle"; break;
	    	case AVMEDIA_TYPE_ATTACHMENT: stream_type = "attachment"; break;
	    	default: stream_type = NULL;
	    }
		codec = (d) ? d->name : "unknown";
		codec_name = (d) ? d->long_name : "unknown";

		ADD_STRING(stream, "type", stream_type);
		ADD_STRINGL(stream, "codec", codec, av_get_name_length(codec));
		ADD_STRING(stream, "codec_name", codec_name);
		ADD_LONG(stream, "bit_rate", c->bit_rate);
		ADD_DOUBLE(stream, "duration", (double) s->duration * av_q2d(s->time_base));
		ADD_DOUBLE(stream, "frame_rate", av_q2d(s->avg_frame_rate));

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

static int av_optimize_file(AVIOContext *pb) {
	return av_optimize_mov_file(pb);
}

/* {{{ proto string av_file_optimize(string arg)
   Optimize a file */
PHP_FUNCTION(av_file_optimize)
{
	char *filename;
	int filename_len;
	int result = FALSE;
	AVIOContext *pb = NULL;
	AVDictionary *options = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &filename, &filename_len) == FAILURE) {
		return;
	}

	av_set_log_level(TSRMLS_C);

	av_dict_set(&options, "truncate", "0", 0);
	if(avio_open2(&pb, filename, AVIO_FLAG_READ_WRITE, NULL, &options) < 0) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Error opening file for writing: %s", filename);
		av_dict_free(&options);
		return;
	}

	result = av_optimize_file(pb);

	avio_close(pb);
	av_dict_free(&options);
	RETURN_BOOL(result);
}
/* }}} */

#ifdef HAVE_AVCODEC_DEFAULT_GET_BUFFER2
static int av_stream_get_buffer2(AVCodecContext *c, AVFrame *pic, int flags) {
	av_stream *strm = c->opaque;
	int ret = avcodec_default_get_buffer2(c, pic, flags);
	strm->frame_pts = strm->packet->pts;
	return ret;
}
#else
static int av_stream_get_buffer(AVCodecContext *c, AVFrame *pic) {
	av_stream *strm = c->opaque;
	int ret = avcodec_default_get_buffer(c, pic);
	strm->frame_pts = strm->packet->pts;
	return ret;
}
#endif

int av_strcasecmp(const char *a, const char *b);

#ifndef HAVE_AV_CODEC_IS_ENCODER
int av_codec_is_encoder(const AVCodec *codec)
{
    return codec && codec->encode2;
}

int av_codec_is_decoder(const AVCodec *codec)
{
    return codec && codec->decode;
}
#endif

static int av_find_codec(const char *short_name, AVCodec **p_enc, AVCodec **p_dec)
{
	AVCodec *codec = NULL;
	if(p_enc) {
		*p_enc = NULL;
	}
	if(p_dec) {
		*p_dec = NULL;
	}
    while ((codec = av_codec_next(codec))) {
        if (av_strcasecmp(short_name, codec->name) == 0) {
        	enum AVCodecID id = codec->id;
        	do {
        		if(av_codec_is_encoder(codec)) {
        			if(p_enc) {
        				*p_enc = codec;
        			}
        		} else if(av_codec_is_decoder(codec)) {
        			if(p_dec) {
        				*p_dec = codec;
        			}
        		}
        		codec = av_codec_next(codec);
        	} while(codec && codec->id == id);
            return TRUE;
        }
    }
    return FALSE;
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
	double frame_duration = 0;
	int thread_count = 0;
	enum AVMediaType media_type;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rz|a", &z_strm, &z_id, &z_options) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(file, av_file *, &z_strm, -1, "av file", le_av_file);

	av_set_log_level(TSRMLS_C);

	// figure out the stream index first
	if(file->flags & AV_FILE_READ) {
		if(Z_TYPE_P(z_id) == IS_STRING) {
			media_type = av_get_stream_type(Z_STRVAL_P(z_id) TSRMLS_CC);
			if(media_type < 0) {
				php_error_docref(NULL TSRMLS_CC, E_ERROR, "parameter 2 should be \"video\", \"audio\", \"subtitle\", or an stream index");
				return;
			}
			stream_index = av_find_best_stream(file->format_cxt, media_type, -1, -1, &codec, 0);
			if(stream_index < 0) {
				php_error_docref(NULL TSRMLS_CC, E_ERROR, "cannot find a stream of type '%s'", Z_STRVAL_P(z_id));
				return;
			}
		} else if(Z_TYPE_P(z_id) == IS_LONG || Z_TYPE_P(z_id) == IS_DOUBLE) {
			stream_index = (Z_TYPE_P(z_id) == IS_DOUBLE) ? (long) Z_DVAL_P(z_id) : Z_LVAL_P(z_id);
			if(!(stream_index >= 0 && (uint32_t) stream_index < file->stream_count)) {
				php_error_docref(NULL TSRMLS_CC, E_ERROR, "stream index must be between 0 and %d", file->stream_count);
				return;
			}
		} else {
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "parameter 2 should be \"video\", \"audio\", \"subtitle\", or an stream index");
			return;
		}
		if(stream_index < (int32_t) file->stream_count) {
			strm = file->streams[stream_index];
			if(strm) {
				if(strm->flags & AV_STREAM_FREED) {
					// return it again
					strm->flags &= ~AV_STREAM_FREED;
					file->open_stream_count++;
					ZEND_REGISTER_RESOURCE(return_value, strm, le_av_strm);
				} else {
					php_error_docref(NULL TSRMLS_CC, E_ERROR, "stream #%d is already open", stream_index);
				}
				return;
			}
		}
	} else if(file->flags & AV_FILE_WRITE) {
		if(Z_TYPE_P(z_id) == IS_STRING) {
			media_type = av_get_stream_type(Z_STRVAL_P(z_id) TSRMLS_CC);
			if(media_type < 0) {
				php_error_docref(NULL TSRMLS_CC, E_ERROR, "parameter 2 should be \"video\", \"audio\", \"subtitle\"");
				return;
			}
		}
		if(file->flags & AV_FILE_HEADER_WRITTEN) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "cannot add additional streams as encoding has already begun");
			return;
		}
		stream_index = file->stream_count;
	}

	// set the thread count
	if(AV_G(max_threads_per_stream) != 0) {
		switch(media_type) {
			case AVMEDIA_TYPE_VIDEO:
				thread_count = av_get_option_long(z_options, "threads", AV_G(threads_per_video_stream));
				break;
			case AVMEDIA_TYPE_AUDIO:
				thread_count = av_get_option_long(z_options, "threads", AV_G(threads_per_audio_stream));
				break;
			case AVMEDIA_TYPE_SUBTITLE:
				break;
			default:
				break;
		}
		if(thread_count > 0) {
			if(thread_count > AV_G(max_threads_per_stream)) {
				thread_count = AV_G(max_threads_per_stream);
			}
		}
	}

	if(file->flags & AV_FILE_READ) {
		if(Z_TYPE_P(z_id) == IS_STRING) {
			int32_t type = av_get_stream_type(Z_STRVAL_P(z_id) TSRMLS_CC);
			if(type < 0) {
				return;
			}
			stream_index = av_find_best_stream(file->format_cxt, type, -1, -1, &codec, 0);
			if(stream_index < 0) {
				php_error_docref(NULL TSRMLS_CC, E_ERROR, "cannot find a stream of type '%s'", Z_STRVAL_P(z_id));
				return;
			}
		} else if(Z_TYPE_P(z_id) == IS_LONG || Z_TYPE_P(z_id) == IS_DOUBLE) {
			stream_index = (Z_TYPE_P(z_id) == IS_DOUBLE) ? (long) Z_DVAL_P(z_id) : Z_LVAL_P(z_id);
			if(!(stream_index >= 0 && (uint32_t) stream_index < file->stream_count)) {
				php_error_docref(NULL TSRMLS_CC, E_ERROR, "stream index must be between 0 and %d", file->stream_count);
				return;
			}
		} else {
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "parameter 2 should be \"video\", \"audio\", \"subtitle\", or an stream index");
			return;
		}

		stream = file->format_cxt->streams[stream_index];
		codec_cxt = stream->codec;
		codec_cxt->thread_count = thread_count;
		if (avcodec_open2(codec_cxt, codec, NULL) < 0) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "Unable to open codec '%s'", codec_cxt->codec->name);
			return;
		}
		frame = avcodec_alloc_frame();
		if (codec_cxt->codec->capabilities & CODEC_CAP_TRUNCATED) {
			codec_cxt->flags |= CODEC_FLAG_TRUNCATED;
		}
#ifdef HAVE_AVCODEC_DEFAULT_GET_BUFFER2
		codec_cxt->get_buffer2 = av_stream_get_buffer2;
#else
		codec_cxt->get_buffer = av_stream_get_buffer;
#endif
	} else if(file->flags & AV_FILE_WRITE) {
		double frame_rate;
		uint32_t sample_rate, bit_rate, width, height, gop_size;
		enum AVPixelFormat pix_fmt = AV_PIX_FMT_NONE;
		const char *codec_name = av_get_option_string(z_options, "codec", NULL);
		const char *pixel_format_name;

		if(codec_name) {
			if(!av_find_codec(codec_name, &codec, NULL)) {
				php_error_docref(NULL TSRMLS_CC, E_ERROR, "unable to find codec '%s'", codec_name);
				return;
			} else if(!codec) {
				php_error_docref(NULL TSRMLS_CC, E_ERROR, "no encoding capability for codec '%s'", codec_name);
				return;
			}
		} else {
			enum AVCodecID codec_id = av_guess_codec((AVOutputFormat *) file->output_format, NULL, NULL, NULL, media_type);
			codec = avcodec_find_encoder(codec_id);
			if(!codec) {
				php_error_docref(NULL TSRMLS_CC, E_ERROR, "unable to find codec");
				return;
			}
		}

		// add the stream
		stream = avformat_new_stream(file->format_cxt, codec);
		if(!stream) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR, "unable to open stream");
			return;
		}

		codec_cxt = stream->codec;
	    if(file->output_format->flags & AVFMT_GLOBALHEADER) {
	    	codec_cxt->flags |= CODEC_FLAG_GLOBAL_HEADER;
	    }
		if(codec->capabilities & CODEC_CAP_EXPERIMENTAL) {
			codec_cxt->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
		}
		codec_cxt->thread_count = thread_count;

		switch(media_type) {
			case AVMEDIA_TYPE_VIDEO:
				frame_rate = av_get_option_double(z_options, "frame_rate", 24.0);
				frame_duration = 1 / frame_rate;
				bit_rate = av_get_option_long(z_options, "bit_rate", 256000);
				width = av_get_option_long(z_options, "width", 320);
				height = av_get_option_long(z_options, "height", 240);
				gop_size = av_get_option_long(z_options, "gop", 600);
				pixel_format_name = av_get_option_string(z_options, "pix_fmt", NULL);
				if(pixel_format_name) {
					pix_fmt = av_get_pix_fmt(pixel_format_name);
					if(pix_fmt == AV_PIX_FMT_NONE) {
						php_error_docref(NULL TSRMLS_CC, E_ERROR, "invalid pixel format '%s'", pixel_format_name);
						return;
					}
				} else {
					if(codec->pix_fmts) {
						pix_fmt = codec->pix_fmts[0];
					} else {
						// not sure why it is missing in earlier versions of libavcodec
						if(codec->id == AV_CODEC_ID_GIF) {
							pix_fmt = AV_PIX_FMT_RGB8;
						} else {
							pix_fmt = AV_PIX_FMT_RGB24;
						}
					}
				}

				file->format_cxt->video_codec_id = codec->id;
				codec_cxt->width = width;
				codec_cxt->height = height;
				codec_cxt->time_base = av_d2q(frame_duration, 1024);
				codec_cxt->pix_fmt = pix_fmt;
				codec_cxt->gop_size = gop_size;
				codec_cxt->bit_rate = bit_rate;
				break;
			case AVMEDIA_TYPE_AUDIO:
				sample_rate = av_get_option_long(z_options, "sampling_rate", 44100);
				bit_rate = av_get_option_long(z_options, "bit_rate", 64000);

				file->format_cxt->audio_codec_id = codec->id;
				codec_cxt->sample_fmt = codec->sample_fmts[0];
				codec_cxt->time_base = av_d2q(1 / sample_rate, 1024);
				codec_cxt->sample_rate = sample_rate;
				codec_cxt->channel_layout = AV_CH_LAYOUT_STEREO;
				codec_cxt->channels = 2;
				codec_cxt->global_quality = 3540;
				codec_cxt->bit_rate = bit_rate;
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

		// copy metadata
		av_copy_metadata(&stream->metadata, z_options TSRMLS_CC);

		frame = avcodec_alloc_frame();
		frame->pts = 0;
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
	strm->frame_duration = frame_duration;
	codec_cxt->opaque = strm;

	if((uint32_t) stream_index >= file->stream_count) {
		file->stream_count = stream_index + 1;
		if(!file->streams) {
			file->streams = ecalloc(file->stream_count, sizeof(av_stream *));
		} else {
			file->streams = erealloc(file->streams, sizeof(av_stream *) * file->stream_count);
		}
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

static av_stream *av_get_writable_stream(av_file *file) {
	av_stream *pending_stream = NULL;
	uint32_t i;

	if(!(file->flags & AV_FILE_EOF_REACHED)) {
		// wait until every stream has a packet
		for(i = 0; i < file->stream_count; i++) {
			av_stream *strm = file->streams[i];
			if(strm && !(strm->flags & AV_STREAM_FREED) && !strm->packet) {
				return NULL;
			}
		}
	}

	// see which stream has a packet with the smallest decode time
	for(i = 0; i < file->stream_count; i++) {
		av_stream *strm = file->streams[i];
		if(strm && strm->packet) {
			if(!pending_stream || strm->packet->pts < pending_stream->packet->pts) {
				pending_stream = strm;
			}
		}
	}
	return pending_stream;
}

static int av_write_next_packet(av_stream *strm, AVPacket *packet) {
	av_stream *pending_stream;
	av_file *file = strm->file;

	// rescale time values for container
	if(packet->pts != AV_NOPTS_VALUE) {
		packet->pts = av_rescale_q(packet->pts, strm->codec_cxt->time_base, strm->stream->time_base);
	}
	if(packet->dts != AV_NOPTS_VALUE) {
		packet->dts = av_rescale_q(packet->dts, strm->codec_cxt->time_base, strm->stream->time_base);
	}
	if(packet->duration != AV_NOPTS_VALUE) {
		packet->duration = (int) av_rescale_q(packet->duration, strm->codec_cxt->time_base, strm->stream->time_base);
	}
	if(strm->codec_cxt->coded_frame && strm->codec_cxt->coded_frame->key_frame) {
		packet->flags |= AV_PKT_FLAG_KEY;
	}
	packet->stream_index = strm->stream->index;
	// push the packet onto the queue
	av_push_packet(strm, packet);

	// write packets that are pending and ready
	while((pending_stream = av_get_writable_stream(file))) {
		AVPacket *next_packet = pending_stream->packet;
		int result = av_interleaved_write_frame(file->format_cxt, next_packet);
		av_shift_packet(pending_stream);
		if(result < 0) {
			return FALSE;
		}
	}
	return TRUE;
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
		avpkt->pts = avctx->coded_frame->pts;
		memcpy(avpkt->data, AV_G(video_buffer), ret);
		*got_packet_ptr = TRUE;
	} else {
		*got_packet_ptr = FALSE;
	}
	return ret;
}
#endif

static void av_flush_remaining_frames(av_stream *strm) {
	int packet_finished;
	int result;
	AVPacket *packet;

	if(strm->codec->capabilities & CODEC_CAP_DELAY) {
		for(;;) {
			packet = emalloc(sizeof(AVPacket));
			av_init_packet(packet);
			packet->data = NULL;
			packet->size = 0;

			switch(strm->codec->type) {
				case AVMEDIA_TYPE_VIDEO:
					result = avcodec_encode_video2(strm->codec_cxt, packet, NULL, &packet_finished);
					break;
				case AVMEDIA_TYPE_AUDIO:
					result = avcodec_encode_audio2(strm->codec_cxt, packet, NULL, &packet_finished);
					break;
				case AVMEDIA_TYPE_SUBTITLE:
					break;
				default:
					break;
			}

			if(result < 0) {
				return;
			}
			if(packet_finished) {
				av_write_next_packet(strm, packet);
			} else {
				av_free_packet(packet);
				efree(packet);
				break;
			}
		}
	}
}

static void av_transfer_pcm_to_frame(av_stream *strm);
static int av_encode_next_frame(av_stream *strm, double time);

static int av_flush_pending_packets(av_file *file) {
	av_stream *pending_stream;
	file->flags |= AV_FILE_EOF_REACHED;

	if(file->flags & AV_FILE_HEADER_WRITTEN) {
		uint32_t i;
		for(i = 0; i < file->stream_count; i++) {
			av_stream *strm = file->streams[i];
			if(strm) {
				if(strm->codec->type == AVMEDIA_TYPE_AUDIO) {
					if(strm->sample_count) {
						av_transfer_pcm_to_frame(strm);
						av_encode_next_frame(strm, strm->sample_start_time);
						strm->sample_count = 0;
					}
				}
				av_flush_remaining_frames(strm);
			}
		}
	}
	while((pending_stream = av_get_writable_stream(file))) {
		AVPacket *next_packet = pending_stream->packet;
		int result = av_interleaved_write_frame(file->format_cxt, next_packet);
		av_shift_packet(pending_stream);
		if(result < 0) {
			return FALSE;
		}
	}
	return TRUE;
}

#define FOR_ENCODING		0
#define FOR_DECODING		1

static void av_create_picture_and_scaler(av_stream *strm, uint32_t width, uint32_t height, int purpose) {
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
			strm->scaler_cxt = sws_getCachedContext(strm->scaler_cxt, width, height, PIX_FMT_RGBA, strm->codec_cxt->width, strm->codec_cxt->height, strm->codec_cxt->pix_fmt, SWS_FAST_BILINEAR, NULL, NULL, NULL);
		} else {
			strm->scaler_cxt = sws_getCachedContext(strm->scaler_cxt, strm->codec_cxt->width, strm->codec_cxt->height, strm->codec_cxt->pix_fmt, width, height, PIX_FMT_RGBA, SWS_FAST_BILINEAR, NULL, NULL, NULL);
		}
	}
}

static void av_create_audio_buffer_and_resampler(av_stream *strm, int purpose) {
	if(!strm->samples) {
		double frame_duration = (double) strm->codec_cxt->frame_size / strm->codec_cxt->sample_rate;

#if defined(HAVE_SWRESAMPLE)
		if(purpose == FOR_ENCODING) {
			strm->resampler_cxt = swr_alloc_set_opts(NULL, strm->codec_cxt->channel_layout, strm->codec_cxt->sample_fmt, strm->codec_cxt->sample_rate, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_FLT, 44100, 0, NULL);
		} else {
			strm->resampler_cxt = swr_alloc_set_opts(NULL, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_FLT, 44100, strm->codec_cxt->channel_layout, strm->codec_cxt->sample_fmt, strm->codec_cxt->sample_rate, 0, NULL);
		}
		swr_init(strm->resampler_cxt);
#elif defined(HAVE_AVRESAMPLE)
		strm->resampler_cxt = avresample_alloc_context();
		if(purpose == FOR_ENCODING) {
			 av_opt_set_int(strm->resampler_cxt, "out_channel_layout", strm->codec_cxt->channel_layout, 0);
			 av_opt_set_int(strm->resampler_cxt, "out_sample_fmt",     strm->codec_cxt->sample_fmt, 0);
			 av_opt_set_int(strm->resampler_cxt, "out_sample_rate",    strm->codec_cxt->sample_rate, 0);
			 av_opt_set_int(strm->resampler_cxt, "in_channel_layout",  AV_CH_LAYOUT_STEREO, 0);
			 av_opt_set_int(strm->resampler_cxt, "in_sample_fmt",      AV_SAMPLE_FMT_FLT, 0);
			 av_opt_set_int(strm->resampler_cxt, "in_sample_rate",     44100, 0);
		} else {
			 av_opt_set_int(strm->resampler_cxt, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
			 av_opt_set_int(strm->resampler_cxt, "out_sample_fmt",     AV_SAMPLE_FMT_FLT, 0);
			 av_opt_set_int(strm->resampler_cxt, "out_sample_rate",    44100, 0);
			 av_opt_set_int(strm->resampler_cxt, "in_channel_layout",  strm->codec_cxt->channel_layout, 0);
			 av_opt_set_int(strm->resampler_cxt, "in_sample_fmt",      strm->codec_cxt->sample_fmt, 0);
			 av_opt_set_int(strm->resampler_cxt, "in_sample_rate",     strm->codec_cxt->sample_rate, 0);
		}
		avresample_open(strm->resampler_cxt);
#else
		enum AVSampleFormat codec_format;
		if(strm->codec_cxt->sample_fmt >= AV_SAMPLE_FMT_U8P && strm->codec_cxt->sample_fmt <= AV_SAMPLE_FMT_DBLP) {
			// need to deplanarized the samples before put them through audio_resample
			uint32_t deplanar_buffer_size;
			codec_format = strm->codec_cxt->sample_fmt - AV_SAMPLE_FMT_U8P;
			deplanar_buffer_size = av_samples_get_buffer_size(NULL, strm->codec_cxt->channels, strm->codec_cxt->frame_size, codec_format, 1);
			strm->deplanarized_samples = av_malloc(deplanar_buffer_size);
		} else {
			codec_format = strm->codec_cxt->sample_fmt;
		}
		if(purpose == FOR_ENCODING) {
			// cleong: I have no idea what the last four parameters do; using values that came up in Google
			strm->resampler_cxt = av_audio_resample_init(strm->codec_cxt->channels, 2, strm->codec_cxt->sample_rate, 44100, codec_format, AV_SAMPLE_FMT_FLT, 16, 10, 0, 0.8);
		} else {
			strm->resampler_cxt =  av_audio_resample_init(2, strm->codec_cxt->channels, 44100, strm->codec_cxt->sample_rate, AV_SAMPLE_FMT_FLT, codec_format, 16, 10, 0, 0.8);
		}
#endif
		strm->sample_buffer_size = (uint32_t) (frame_duration * 44100);
		strm->samples = av_malloc(sizeof(float) * strm->sample_buffer_size * 2);
	}
}

static void av_transfer_picture_to_frame(av_stream *strm) {
	// allocate the frame buffer if it's not there
	if(!(strm->flags & AV_STREAM_FRAME_BUFFER_ALLOCATED)) {
		avpicture_alloc((AVPicture *) strm->frame, strm->codec_cxt->pix_fmt, strm->codec_cxt->width, strm->codec_cxt->height);
		strm->frame->width = strm->codec_cxt->width;
		strm->frame->height = strm->codec_cxt->height;
		strm->frame->format = strm->codec_cxt->pix_fmt;
		strm->flags |= AV_STREAM_FRAME_BUFFER_ALLOCATED;
	}
	// rescale the picture to the proper dimension and transform pixels to format used by codec
	sws_scale(strm->scaler_cxt, (const uint8_t * const *) strm->picture->data, strm->picture->linesize, 0, strm->picture->height, strm->frame->data, strm->frame->linesize);
}

static void av_transfer_picture_from_frame(av_stream *strm) {
	// rescale the picture and transform pixels to RGBA
	sws_scale(strm->scaler_cxt, (const uint8_t * const *) strm->frame->data, strm->frame->linesize, 0, strm->frame->height, strm->picture->data, strm->picture->linesize);
}

static void av_transfer_pcm_to_frame(av_stream *strm) {
	// allocate the audio frame if it's not there
	if(!(strm->flags & AV_STREAM_AUDIO_BUFFER_ALLOCATED)) {
		uint32_t buffer_size = av_samples_get_buffer_size(NULL, strm->codec_cxt->channels, strm->codec_cxt->frame_size, strm->codec_cxt->sample_fmt, 1);
	    float *samples = av_malloc(buffer_size);

	    strm->frame->format = strm->codec_cxt->sample_fmt;
	    strm->frame->nb_samples = strm->codec_cxt->frame_size;
		avcodec_fill_audio_frame(strm->frame, strm->codec_cxt->channels, strm->codec_cxt->sample_fmt, (uint8_t *) samples, buffer_size, 1);
		strm->flags |= AV_STREAM_AUDIO_BUFFER_ALLOCATED;
	}
#if defined(HAVE_SWRESAMPLE)
	swr_convert(strm->resampler_cxt, (uint8_t **) strm->frame->data, strm->frame->nb_samples, (const uint8_t **) &strm->samples, strm->sample_count);
#elif defined(HAVE_AVRESAMPLE)
	avresample_convert(strm->resampler_cxt, (uint8_t **) strm->frame->data, 0, strm->frame->nb_samples, (uint8_t **) &strm->samples, 0, strm->sample_count);
#else
	if(strm->deplanarized_samples) {
		int32_t i, j;
		switch(strm->codec_cxt->sample_fmt) {
			case AV_SAMPLE_FMT_U8P: {
				int8_t *src_pointer = (int8_t *) strm->deplanarized_samples;
				int8_t **dest_pointers = (int8_t **) strm->frame->data;
				for(i = 0; i < (int32_t) strm->sample_count; i++) {
					for(j = 0; j < strm->codec_cxt->channels; j++) {
						int8_t *dest_pointer = dest_pointers[j] + i;
						*dest_pointer = *src_pointer;
						src_pointer++;
					}
				}
			}	break;
			case AV_SAMPLE_FMT_S16P: {
				int16_t *src_pointer = (int16_t *) strm->deplanarized_samples;
				int16_t **dest_pointers = (int16_t **) strm->frame->data;
				for(i = 0; i < (int32_t) strm->sample_count; i++) {
					for(j = 0; j < strm->codec_cxt->channels; j++) {
						int16_t *dest_pointer = dest_pointers[j] + i;
						*dest_pointer = *src_pointer;
						src_pointer++;
					}
				}
			}	break;
			case AV_SAMPLE_FMT_S32P: {
				int32_t *src_pointer = (int32_t *) strm->deplanarized_samples;
				int32_t **dest_pointers = (int32_t **) strm->frame->data;
				for(i = 0; i < (int32_t) strm->sample_count; i++) {
					for(j = 0; j < strm->codec_cxt->channels; j++) {
						int32_t *dest_pointer = dest_pointers[j] + i;
						*dest_pointer = *src_pointer;
						src_pointer++;
					}
				}
			}	break;
			case AV_SAMPLE_FMT_FLTP: {
				float *src_pointer = (float *) strm->deplanarized_samples;
				float **dest_pointers = (float **) strm->frame->data;
				for(i = 0; i < (int32_t) strm->sample_count; i++) {
					for(j = 0; j < strm->codec_cxt->channels; j++) {
						float *dest_pointer = dest_pointers[j] + i;
						*dest_pointer = *src_pointer;
						src_pointer++;
					}
				}
			}	break;
			case AV_SAMPLE_FMT_DBLP: {
				double *src_pointer = (double *) strm->deplanarized_samples;
				double **dest_pointers = (double **) strm->frame->data;
				for(i = 0; i < (int32_t) strm->sample_count; i++) {
					for(j = 0; j < strm->codec_cxt->channels; j++) {
						double *dest_pointer = dest_pointers[j] + i;
						*dest_pointer = *src_pointer;
						src_pointer++;
					}
				}
			}	break;
			default: break;
		}
		audio_resample(strm->resampler_cxt, strm->deplanarized_samples, (short *) strm->samples, strm->sample_count);
	} else {
		audio_resample(strm->resampler_cxt, (short *) strm->frame->data[0], (short *) strm->samples, strm->sample_count);
	}
#endif
}

static void av_transfer_pcm_from_frame(av_stream *strm) {
#if defined(HAVE_SWRESAMPLE)
	strm->sample_count = swr_convert(strm->resampler_cxt, (uint8_t **) &strm->samples, strm->sample_buffer_size, (const uint8_t **) strm->frame->data, strm->frame->nb_samples);
#elif defined(HAVE_AVRESAMPLE)
	strm->sample_count = avresample_convert(strm->resampler_cxt, (uint8_t **) &strm->samples, 0, strm->sample_buffer_size, (uint8_t **) strm->frame->data, 0, strm->frame->nb_samples);
#else
	short *src_buffer;
	if(strm->deplanarized_samples) {
		int32_t i, j;
		switch(strm->codec_cxt->sample_fmt) {
			case AV_SAMPLE_FMT_U8P: {
				int8_t *dest_pointer = (int8_t *) strm->deplanarized_samples;
				int8_t **src_pointers = (int8_t **) strm->frame->data;
				for(i = 0; i < strm->frame->nb_samples; i++) {
					for(j = 0; j < strm->codec_cxt->channels; j++) {
						int8_t *src_pointer = src_pointers[j] + i;
						*dest_pointer = *src_pointer;
						dest_pointer++;
					}
				}
			}	break;
			case AV_SAMPLE_FMT_S16P: {
				int16_t *dest_pointer = (int16_t *) strm->deplanarized_samples;
				int16_t **src_pointers = (int16_t **) strm->frame->data;
				for(i = 0; i < strm->frame->nb_samples; i++) {
					for(j = 0; j < strm->codec_cxt->channels; j++) {
						int16_t *src_pointer = src_pointers[j] + i;
						*dest_pointer = *src_pointer;
						dest_pointer++;
					}
				}
			}	break;
			case AV_SAMPLE_FMT_S32P: {
				int32_t *dest_pointer = (int32_t *) strm->deplanarized_samples;
				int32_t **src_pointers = (int32_t **) strm->frame->data;
				for(i = 0; i < strm->frame->nb_samples; i++) {
					for(j = 0; j < strm->codec_cxt->channels; j++) {
						int32_t *src_pointer = src_pointers[j] + i;
						*dest_pointer = *src_pointer;
						dest_pointer++;
					}
				}
			}	break;
			case AV_SAMPLE_FMT_FLTP: {
				float *dest_pointer = (float *) strm->deplanarized_samples;
				float **src_pointers = (float **) strm->frame->data;
				for(i = 0; i < strm->frame->nb_samples; i++) {
					for(j = 0; j < strm->codec_cxt->channels; j++) {
						float *src_pointer = src_pointers[j] + i;
						*dest_pointer = *src_pointer;
						dest_pointer++;
					}
				}
			}	break;
			case AV_SAMPLE_FMT_DBLP: {
				double *dest_pointer = (double *) strm->deplanarized_samples;
				double **src_pointers = (double **) strm->frame->data;
				for(i = 0; i < strm->frame->nb_samples; i++) {
					for(j = 0; j < strm->codec_cxt->channels; j++) {
						double *src_pointer = src_pointers[j] + i;
						*dest_pointer = *src_pointer;
						dest_pointer++;
					}
				}
			}	break;
			default: break;
		}
		src_buffer = strm->deplanarized_samples;
	} else {
		src_buffer = (short *) strm->frame->data[0];
	}
	strm->sample_count = audio_resample(strm->resampler_cxt, (short *) strm->samples, src_buffer, strm->frame->nb_samples);
#endif
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

static int av_encode_next_frame(av_stream *strm, double time) {
	av_file *file = strm->file;
	int packet_finished = FALSE;
	int result;
	AVPacket *packet;

	strm->frame->pts = (int64_t) (time / av_q2d(strm->codec_cxt->time_base));

	if(!(file->flags & AV_FILE_HEADER_WRITTEN)) {
		avformat_write_header(file->format_cxt, NULL);
		file->flags |= AV_FILE_HEADER_WRITTEN;
	}

	packet = emalloc(sizeof(AVPacket));
	av_init_packet(packet);
	packet->data = NULL;
	packet->size = 0;

	switch(strm->codec->type) {
		case AVMEDIA_TYPE_VIDEO:
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

	if(packet_finished) {
		av_write_next_packet(strm, packet);
	} else {
		av_free_packet(packet);
		efree(packet);
	}
	return !(result < 0);
}

static int av_decode_frame_at_cursor(av_stream *strm, AVFrame *dest_frame, double *p_time TSRMLS_DC) {
	int frame_finished = FALSE;
	for(;;) {
		int bytes_decoded;
		if(av_read_next_packet(strm TSRMLS_CC)) {
			switch(strm->codec->type) {
				case AVMEDIA_TYPE_VIDEO:
					bytes_decoded = avcodec_decode_video2(strm->codec_cxt, dest_frame, &frame_finished, strm->packet);
					break;
				case AVMEDIA_TYPE_AUDIO:
					bytes_decoded = avcodec_decode_audio4(strm->codec_cxt, dest_frame, &frame_finished, strm->packet);
					break;
				case AVMEDIA_TYPE_SUBTITLE:
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

static int av_decode_next_frame(av_stream *strm, double *p_time TSRMLS_DC) {
	if(strm->flags & AV_STREAM_SOUGHT) {
		// keep decoding frames until we have two frames straddling the time sought
		AVFrame *current_frame = strm->frame, *next_frame = NULL;
		double current_frame_time, next_frame_time = 0;
		do {
			if(next_frame) {
				// the one read earlier become the current frame
				AVFrame *current_frame_temp = current_frame;
				current_frame_time = next_frame_time;
				current_frame = next_frame;
				next_frame = current_frame_temp;
			} else {
				// decode the current frame
				if(!av_decode_frame_at_cursor(strm, current_frame, &current_frame_time TSRMLS_CC)) {
					return FALSE;
				}
				if(current_frame_time == strm->time_sought) {
					// we have an exact match--no need to do more
					break;
				}
				next_frame = avcodec_alloc_frame();
			}
			// read the next frame so we can check if the current frame is the closest
			// to the time sought without going over
			if(!av_decode_frame_at_cursor(strm, next_frame, &next_frame_time TSRMLS_CC)) {
			    avcodec_free_frame(&next_frame);
			    next_frame = NULL;
				break;
			}
		} while(!(current_frame_time <= strm->time_sought && strm->time_sought < next_frame_time));

		strm->flags &= ~AV_STREAM_SOUGHT;
		strm->frame = current_frame;
		strm->next_frame = next_frame;
		strm->next_frame_time = next_frame_time;
		*p_time = current_frame_time;
	} else {
		if(strm->next_frame) {
			// free the current frame and use the next frame
		    avcodec_free_frame(&strm->frame);
		    strm->frame = strm->next_frame;
		    *p_time = strm->next_frame_time;
		    strm->next_frame = NULL;
		    strm->next_frame_time = 0;
		} else {
			return av_decode_frame_at_cursor(strm, strm->frame, p_time TSRMLS_CC);
		}
	}
	return TRUE;
}

#ifdef _MSC_VER
	#if _MSC_VER < 1700
	int isnan(double x) {
		return x != x;
	}
	#endif
#endif

static int av_encode_image_from_gd(av_stream *strm, gdImagePtr image, double time TSRMLS_DC) {
	av_create_picture_and_scaler(strm, image->sx, image->sy, FOR_ENCODING);
	av_copy_image_from_gd(strm->picture, image);
	av_transfer_picture_to_frame(strm);
	if(isnan(time)) {
		time = strm->next_frame_time;
	}
	strm->next_frame_time = time + strm->frame_duration;
	return av_encode_next_frame(strm, time);
}

static int av_decode_image_to_gd(av_stream *strm, gdImagePtr image, double *p_time TSRMLS_DC) {
	if(av_decode_next_frame(strm, p_time TSRMLS_CC)) {
		av_create_picture_and_scaler(strm, image->sx, image->sy, FOR_DECODING);
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

	if(!isnan(time)) {
		double buffered_duration = strm->sample_count * (1 / 44100.0);
		double start_time = time - buffered_duration;
		double error = strm->sample_start_time - start_time;

		if(fabs(error) < 0.001) {
			// the error is small--ignore it
			strm->sample_start_time = start_time;
		} else {
			if(strm->sample_count > 0) {
				// start a new packet
				av_transfer_pcm_to_frame(strm);
				if(!av_encode_next_frame(strm, strm->sample_start_time)) {
					return FALSE;
				}
				strm->sample_count = 0;
			}
			strm->sample_start_time = time;
		}
	}

	// keep copying into audio frame until are samples are used up
	while(src_samples_remaining) {
		uint32_t samples_needed = strm->sample_buffer_size - strm->sample_count;
		uint32_t samples_to_copy = (samples_needed < src_samples_remaining) ? samples_needed : src_samples_remaining;
		uint32_t i;

		// make sure samples are between (-1.0, 1.0)
		for(i = 0; i < samples_to_copy * 2; i++) {
			float sample = src_samples[i];
			if(EXPECTED(-1.0f <= sample && sample <= 1.0f)) {
				dst_samples[i] = sample;
			} else {
				dst_samples[i] = (sample < -1) ? -1.0f : 1.0f;
			}
		}
		src_samples_remaining -= samples_to_copy;
		strm->sample_count += samples_to_copy;
		src_samples += samples_to_copy * 2;
		dst_samples += samples_to_copy * 2;

		if(strm->sample_count == strm->sample_buffer_size) {
			// transfer the data to the frame then compress it
			av_transfer_pcm_to_frame(strm);
			if(!av_encode_next_frame(strm, strm->sample_start_time)) {
				return FALSE;
			}
			// adjust the time
			strm->sample_start_time += strm->sample_count * (1 / 44100.0);

			strm->sample_count = 0;
			dst_samples = strm->samples;
		}
	}
	return TRUE;
}


static int av_decode_pcm_to_zval(av_stream *strm, zval *buffer, double *p_time TSRMLS_DC) {
	if(av_decode_next_frame(strm, p_time TSRMLS_CC)) {
		uint32_t data_len;

		av_create_audio_buffer_and_resampler(strm, FOR_DECODING);
		av_transfer_pcm_from_frame(strm);

		data_len = sizeof(float) * 2 * strm->sample_count;
		if(Z_TYPE_P(buffer) != IS_STRING || Z_STRLEN_P(buffer) < data_len) {
			zval_dtor(buffer);
			Z_TYPE_P(buffer) = IS_STRING;
			Z_STRVAL_P(buffer) = emalloc(data_len + 1);
		}
		Z_STRLEN_P(buffer) = data_len;
		memcpy(Z_STRVAL_P(buffer), strm->samples, data_len);
		Z_STRVAL_P(buffer)[data_len] = '\0';
		return TRUE;
	} else {
		zval_dtor(buffer);
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

	av_set_log_level(TSRMLS_C);

	if(strm->codec->type != AVMEDIA_TYPE_VIDEO) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Not a video stream");
		return;
	}
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

	av_set_log_level(TSRMLS_C);

	if(strm->codec->type != AVMEDIA_TYPE_AUDIO) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Not an audio stream");
		return;
	}
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

	av_set_log_level(TSRMLS_C);

	if(strm->codec->type != AVMEDIA_TYPE_VIDEO) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Not a video stream");
		return;
	}
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

	av_set_log_level(TSRMLS_C);

	if(strm->codec->type != AVMEDIA_TYPE_AUDIO) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Not an audio stream");
		return;
	}
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
