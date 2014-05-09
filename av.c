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

#ifdef _MSC_VER
int isnan(double x) {
	return x != x;
}
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_av_stream_read_subtitle, 0, 0, 3)
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_av_stream_write_subtitle, 0, 0, 3)
    ZEND_ARG_INFO(0, stream)
    ZEND_ARG_INFO(0, buffer)
    ZEND_ARG_INFO(0, time)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_av_get_encoders, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_av_get_decoders, 0, 0, 0)
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
	PHP_FE(av_stream_read_subtitle,		arginfo_av_stream_read_subtitle)
	PHP_FE(av_stream_write_image,		arginfo_av_stream_write_image)
	PHP_FE(av_stream_write_pcm,			arginfo_av_stream_write_pcm)
	PHP_FE(av_stream_write_subtitle,	arginfo_av_stream_write_subtitle)

	PHP_FE(av_get_encoders,				arginfo_av_get_encoders)
	PHP_FE(av_get_decoders,				arginfo_av_get_decoders)

#ifdef PHP_FE_END
	PHP_FE_END	/* Must be the last line in av_functions[] */
#else
	{NULL, NULL, NULL}
#endif
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
    if (!frame || !*frame)
        return;

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
						efree(strm->frame->data[0]);
						strm->frame->data[0] = NULL;
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
				if(strm->scaler_cxt) {	// buffer for planar to interleave conversion
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
#endif
					if(strm->resampler_queue) {
						efree(strm->resampler_queue);
					}
				}
				if(strm->subtitle) {
					avsubtitle_free(strm->subtitle);
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
					efree(strm->samples);
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

static void av_flush_remaining_frames(av_stream *strm);

static void av_free_stream(av_stream *strm) {
	if(!(strm->flags & AV_STREAM_FREED)) {
		av_file *file = strm->file;
		file->open_stream_count--;
		if(file->flags & AV_FILE_WRITE) {
			if(file->flags & AV_FILE_HEADER_WRITTEN) {
				av_flush_remaining_frames(strm);
			}
		}
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

static int av_get_stream_type(const char *type TSRMLS_DC) {
	if(strcmp(type, "video") == 0) {
		return AVMEDIA_TYPE_VIDEO;
	} else if(strcmp(type, "audio") == 0) {
		return AVMEDIA_TYPE_AUDIO;
	} else if(strcmp(type, "subtitle") == 0) {
		return AVMEDIA_TYPE_SUBTITLE;
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "'%s' is not a recognized stream type", type);
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

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(av)
{
	if(le_gd == -1) {
		le_gd = zend_fetch_list_dtor_id("gd");
	}
#if !defined(HAVE_AVCODEC_ENCODE_VIDEO2) || !defined(HAVE_AVCODEC_ENCODE_AUDIO2) || !defined(HAVE_AVCODEC_ENCODE_SUBTITLE2)
	AV_G(encoding_buffer) = NULL;
	AV_G(encoding_buffer_size) = 0;
#endif
#ifdef USE_CUSTOM_MALLOC
	av_install_custom_malloc();
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(av)
{
#if !defined(HAVE_AVCODEC_ENCODE_VIDEO2) || !defined(HAVE_AVCODEC_ENCODE_AUDIO2) || !defined(HAVE_AVCODEC_ENCODE_SUBTITLE2)
	if(AV_G(encoding_buffer)) {
		efree(AV_G(encoding_buffer));
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
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Metadata should be an array");
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
	char *new_filename = NULL;

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

	if(strncmp(filename, "mms://", 6) == 0) {
		new_filename = emalloc(filename_len + 2);
		memcpy(new_filename, "mmsh://", 7);
		memcpy(new_filename + 7, filename + 6, filename_len - 6 + 1);
		filename = new_filename;
	}

	if(flags & AV_FILE_READ) {
		if (avformat_open_input(&format_cxt, filename, NULL, NULL) < 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Error opening file for reading: %s", filename);
			return;
		}
		if (avformat_find_stream_info(format_cxt, NULL) < 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Error finding stream info: %s", filename);
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
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot find output format: %s", short_name);
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
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot deduce output format from filename: %s", filename);
				return;
			}
		}

		if(!(output_format->flags & AVFMT_NOFILE)) {
			if(avio_open(&pb, filename, AVIO_FLAG_READ_WRITE) < 0) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Error opening file for writing: %s", filename);
				return;
			}
		}
		format_cxt = avformat_alloc_context();
		format_cxt->pb = pb;
		format_cxt->oformat = output_format;

		// copy metadata
		av_copy_metadata(&format_cxt->metadata, z_options TSRMLS_CC);
	}

	if(new_filename) {
		efree(new_filename);
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
	double overall_duration;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &z_file) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(file, av_file *, &z_file, -1, "av file", le_av_file);
	f = file->format_cxt;

	av_set_log_level(TSRMLS_C);

	array_init(return_value);

	format = (f->iformat) ? f->iformat->name : f->oformat->name;
	format_name = (f->iformat) ? f->iformat->long_name : f->oformat->long_name;
	if(f->duration != AV_NOPTS_VALUE) {
		overall_duration = (double) f->duration / AV_TIME_BASE;
	} else {
		overall_duration = INFINITY;
	}
	av_set_element_stringl(return_value, "format", format, av_get_name_length(format));
	av_set_element_string(return_value, "format_name", format_name);
	av_set_element_long(return_value, "bit_rate", f->bit_rate);
	av_set_element_double(return_value, "duration", overall_duration);

	// add metadata of file
	MAKE_STD_ZVAL(metadata);
	array_init(metadata);
	zend_hash_update(HASH_OF(return_value), "metadata", sizeof("metadata"), (void *) &metadata, sizeof(zval *), NULL);
	for(e = NULL; (e = av_dict_get(f->metadata, "", e, AV_DICT_IGNORE_SUFFIX)); ) {
		av_set_element_string(metadata, e->key, e->value);
	}

	MAKE_STD_ZVAL(streams);
	array_init(streams);
	zend_hash_update(HASH_OF(return_value), "streams", (uint32_t) strlen("streams") + 1, (void *) &streams, sizeof(zval *), NULL);
	for(i = 0; i < f->nb_streams; i++) {
		zval *stream;
		AVStream *s = f->streams[i];
		AVCodecContext *c = s->codec;
		AVCodec *d = avcodec_find_decoder(c->codec_id);
		const char *stream_type, *codec, *codec_name;
		double duration;

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
		if(s->duration != AV_NOPTS_VALUE) {
			duration = (double) s->duration * av_q2d(s->time_base);
		} else if(s->nb_frames != 0) {
			// not sure why duration isn't set if it could be calculated this way
			duration = (double) s->nb_frames * av_q2d(s->time_base);
		} else {
			// use the overall duration
			duration = overall_duration;
		}

		av_set_element_string(stream, "type", stream_type);
		av_set_element_stringl(stream, "codec", codec, av_get_name_length(codec));
		av_set_element_string(stream, "codec_name", codec_name);
		av_set_element_long(stream, "bit_rate", c->bit_rate);
		av_set_element_double(stream, "duration", duration);

		switch(c->codec_type) {
			case AVMEDIA_TYPE_VIDEO: {
				double frame_rate = 0;
				if(c->codec_type == AVMEDIA_TYPE_VIDEO) {
					if(s->avg_frame_rate.den != 0) {
						frame_rate = av_q2d(s->avg_frame_rate);
					} else if(s->r_frame_rate.den != 0) {
						frame_rate = av_q2d(s->r_frame_rate);
					}
				}
				av_set_element_double(stream, "frame_rate", frame_rate);
				av_set_element_long(stream, "height",  c->height);
				av_set_element_long(stream, "width", c->width);
			}	break;
			case AVMEDIA_TYPE_AUDIO: {
				av_set_element_long(stream, "channel_layout", (long) c->channel_layout);
				av_set_element_long(stream, "channels", c->channels);
				av_set_element_long(stream, "sample_rate", c->sample_rate);
			}	break;
			case AVMEDIA_TYPE_SUBTITLE: {
				if(c->subtitle_header_size > 0) {
					av_set_element_stringl(stream, "subtitle_header", (char *) c->subtitle_header, (long) c->subtitle_header_size);
				}
			}	break;
			default:
				break;
		}

		// add metadata of stream
		MAKE_STD_ZVAL(metadata);
		array_init(metadata);
		zend_hash_update(HASH_OF(stream), "metadata", sizeof("metadata"), (void *) &metadata, sizeof(zval *), NULL);
		for(e = NULL; (e = av_dict_get(s->metadata, "", e, AV_DICT_IGNORE_SUFFIX)); ) {
			av_set_element_string(metadata, e->key, e->value);
		}

		// refer to stream using string key if it happens to be the best stream of a given type
		if(i == av_find_best_stream(f, c->codec_type, -1, -1, NULL, 0)) {
			Z_ADDREF_P(stream);
			zend_hash_update(HASH_OF(streams), stream_type, (uint32_t) strlen(stream_type) + 1, (void *) &stream, sizeof(zval *), NULL);
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
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Error opening file for writing: %s", filename);
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
	if(strm->frame_pts == AV_NOPTS_VALUE) {
		strm->frame_pts = strm->packet->pts;
	}
	return ret;
}
#else
static int av_stream_get_buffer(AVCodecContext *c, AVFrame *pic) {
	av_stream *strm = c->opaque;
	int ret = avcodec_default_get_buffer(c, pic);
	if(strm->frame_pts == AV_NOPTS_VALUE) {
		strm->frame_pts = strm->packet->pts;
	}
	return ret;
}
#endif

int av_strcasecmp(const char *a, const char *b);

#ifndef HAVE_AV_CODEC_IS_ENCODER
int av_codec_is_encoder(const AVCodec *codec)
{
#if HAVE_AVCODEC_ENCODE_AUDIO2
    return codec && (codec->encode2 || codec->encode);
#else
    return codec && codec->encode;
#endif
}

int av_codec_is_decoder(const AVCodec *codec)
{
    return codec && codec->decode;
}
#endif

static int av_find_codec(const char *short_name, AVCodec **p_enc, AVCodec **p_dec) {
	int found = FALSE;
	AVCodec *codec = NULL, *encoder = NULL, *decoder = NULL;
	while((codec = av_codec_next(codec))) {
		int match = FALSE;
		if(av_strcasecmp(short_name, codec->name) == 0) {
			match = TRUE;
		} else if(codec->name[0] == 'l' && codec->name[1] == 'i' && codec->name[2] == 'b') {
			if(av_strcasecmp(short_name, codec->name + 3) == 0) {
				match = TRUE;
			}
		}
		if(match) {
			if(av_codec_is_encoder(codec)) {
				encoder = codec;
			} else if(av_codec_is_decoder(codec)) {
				decoder = codec;
			}
			found = TRUE;
			if((!p_enc || encoder) && decoder) {
				break;
			}
		}
	}
	if(decoder && !encoder && p_enc) {
		// sometimes the encoder doesn't have the same name
		codec = NULL;
		while ((codec = av_codec_next(codec))) {
			if(codec->id == decoder->id) {
				if(av_codec_is_encoder(codec)) {
					encoder = codec;
					break;
				}
			}
		}
	}
 	if(p_enc) {
		*p_enc = encoder;
 	}
	if(p_dec) {
		*p_dec = decoder;
	}
	return found;
}

/* {{{ proto string av_stream_open(resource file, mixed id, [, array options])
   Create an encoder */
PHP_FUNCTION(av_stream_open)
{
	zval *z_strm, *z_id, *z_options = NULL;
	AVCodec *codec = NULL;
	AVCodecContext *codec_cxt = NULL;
	AVStream *stream = NULL;
	av_file *file;
	av_stream *strm;
	int32_t stream_index;
	double frame_duration = 0;
	long thread_count = 0;
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
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Parameter 2 should be \"video\", \"audio\", \"subtitle\", or an stream index");
				return;
			}
			stream_index = av_find_best_stream(file->format_cxt, media_type, -1, -1, &codec, 0);
			if(stream_index < 0) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot find a stream of type '%s'", Z_STRVAL_P(z_id));
				return;
			}
		} else if(Z_TYPE_P(z_id) == IS_LONG || Z_TYPE_P(z_id) == IS_DOUBLE) {
			stream_index = (Z_TYPE_P(z_id) == IS_DOUBLE) ? (long) Z_DVAL_P(z_id) : Z_LVAL_P(z_id);
			
			if(stream_index >= 0 && (uint32_t) stream_index < file->stream_count) {
				stream = file->format_cxt->streams[stream_index];
				media_type = stream->codec->codec_type;
				codec = avcodec_find_decoder(stream->codec->codec_id);
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Stream index must be between 0 and %d", file->stream_count);
				return;
			}
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Parameter 2 should be \"video\", \"audio\", \"subtitle\", or an stream index");
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
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Stream #%d is already open", stream_index);
				}
				return;
			}
		}
	} else if(file->flags & AV_FILE_WRITE) {
		if(Z_TYPE_P(z_id) == IS_STRING) {
			media_type = av_get_stream_type(Z_STRVAL_P(z_id) TSRMLS_CC);
			if(media_type < 0) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Parameter 2 should be \"video\", \"audio\", \"subtitle\"");
				return;
			}
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Parameter 2 should be \"video\", \"audio\", \"subtitle\"");
			return;
		}
		if(file->flags & AV_FILE_HEADER_WRITTEN) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot add additional streams as encoding has already begun");
			return;
		}
		stream_index = file->stream_count;
	}

	// set the thread count
	if(AV_G(max_threads_per_stream) != 0) {
		switch(media_type) {
			case AVMEDIA_TYPE_VIDEO:
				thread_count = AV_G(threads_per_video_stream);
				av_get_element_long(z_options, "threads", &thread_count);
				break;
			case AVMEDIA_TYPE_AUDIO:
				thread_count = AV_G(threads_per_audio_stream);
				av_get_element_long(z_options, "threads", &thread_count);
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
		stream = file->format_cxt->streams[stream_index];
		codec_cxt = stream->codec;
		codec_cxt->thread_count = thread_count;
		if(avcodec_open2(codec_cxt, codec, NULL) < 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to open codec '%s'", (codec) ? codec->name : "???");
			return;
		}
		
		if (codec_cxt->codec->capabilities & CODEC_CAP_TRUNCATED) {
			codec_cxt->flags |= CODEC_FLAG_TRUNCATED;
		}
#ifdef HAVE_AVCODEC_DEFAULT_GET_BUFFER2
		codec_cxt->get_buffer2 = av_stream_get_buffer2;
#else
		codec_cxt->get_buffer = av_stream_get_buffer;
#endif
	} else if(file->flags & AV_FILE_WRITE) {
		double frame_rate = 24.0;
		long sample_rate = 44100;
		long bit_rate = (media_type == AVMEDIA_TYPE_VIDEO) ? 256000 : 64000;
		long width = 320, height = 240;
		long gop_size = 600;
		long channels = 2;
		long channel_layout = 0;
		enum AVPixelFormat pix_fmt = AV_PIX_FMT_NONE;
		char *codec_name = NULL;
		char *pixel_format_name = NULL;
		char *subtitle_header = NULL;
		long subtitle_header_size;

		if(av_get_element_string(z_options, "codec", &codec_name)) {
			if(!av_find_codec(codec_name, &codec, NULL)) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find codec '%s'", codec_name);
				return;
			} else if(!codec) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "No encoding capability for codec '%s'", codec_name);
				return;
			}
		} else {
			enum AVCodecID codec_id = av_guess_codec((AVOutputFormat *) file->output_format, NULL, NULL, NULL, media_type);
			codec = avcodec_find_encoder(codec_id);
			if(!codec) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to find codec");
				return;
			}
		}

		// add the stream
		stream = avformat_new_stream(file->format_cxt, codec);
		if(!stream) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to open stream");
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
				av_get_element_double(z_options, "frame_rate", &frame_rate);
				frame_duration = 1 / frame_rate;
				av_get_element_long(z_options, "bit_rate", &bit_rate);
				av_get_element_long(z_options, "width", &width);
				av_get_element_long(z_options, "height", &height);
				av_get_element_long(z_options, "gop", &gop_size);
				av_get_element_string(z_options, "pix_fmt", &pixel_format_name);
				if(pixel_format_name) {
					pix_fmt = av_get_pix_fmt(pixel_format_name);
					if(pix_fmt == AV_PIX_FMT_NONE) {
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid pixel format '%s'", pixel_format_name);
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
				av_get_element_long(z_options, "sampling_rate", &sample_rate);
				av_get_element_long(z_options, "bit_rate", &bit_rate);
				av_get_element_long(z_options, "channels", &channels);
				if(!av_get_element_long(z_options, "channel_layout", &channel_layout)) {
					switch(channels) {
						case 1: channel_layout = AV_CH_LAYOUT_MONO; break;
						case 2: channel_layout = AV_CH_LAYOUT_STEREO; break;
						default: channel_layout = 0; break;
					}
				}

				file->format_cxt->audio_codec_id = codec->id;
				codec_cxt->sample_fmt = codec->sample_fmts[0];
				codec_cxt->time_base = av_d2q(1 / sample_rate, 1024);
				codec_cxt->sample_rate = sample_rate;
				codec_cxt->channel_layout = channel_layout;
				codec_cxt->channels = channels;
				codec_cxt->global_quality = 3540;
				codec_cxt->bit_rate = bit_rate;
				codec_cxt->flags |= CODEC_FLAG_QSCALE;
				break;
			case AVMEDIA_TYPE_SUBTITLE:
				if(av_get_element_stringl(z_options, "subtitle_header", &subtitle_header, &subtitle_header_size)) {
					codec_cxt->subtitle_header = av_malloc(subtitle_header_size + 1);
					memcpy(codec_cxt->subtitle_header, subtitle_header, subtitle_header_size + 1);
					codec_cxt->subtitle_header_size = subtitle_header_size;
				}
				file->format_cxt->subtitle_codec_id = codec->id;
				codec_cxt->time_base.num = 1;
				codec_cxt->time_base.den = 90000;
				break;
			default:
				break;
		}

		if (avcodec_open2(codec_cxt, codec, NULL) < 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to open codec '%s'", (codec) ? codec->name : "???");
			return;
		}

		// copy metadata
		av_copy_metadata(&stream->metadata, z_options TSRMLS_CC);
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
	strm->index = stream_index;
	strm->frame_duration = frame_duration;
	codec_cxt->opaque = strm;

	switch(media_type) {
		case AVMEDIA_TYPE_VIDEO:
		case AVMEDIA_TYPE_AUDIO:
			strm->frame = avcodec_alloc_frame();
			strm->frame->pts = 0;
			break;
		case AVMEDIA_TYPE_SUBTITLE:
			strm->subtitle = av_malloc(sizeof(AVSubtitle));
			memset(strm->subtitle, 0, sizeof(AVSubtitle));
			break;
		default:
			break;
	}

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
			if(!pending_stream || av_compare_ts(strm->packet->pts, strm->stream->time_base, pending_stream->packet->pts, pending_stream->stream->time_base) < 0) {
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

	if(!AV_G(encoding_buffer)) {
		AV_G(encoding_buffer_size) = 100000;
		AV_G(encoding_buffer) = emalloc(AV_G(encoding_buffer_size));
	}
	ret = avcodec_encode_video(avctx, AV_G(encoding_buffer), AV_G(encoding_buffer_size), frame);

	if(ret > 0) {
		avpkt->size = ret;
		avpkt->data = av_realloc(avpkt->data, avpkt->size + FF_INPUT_BUFFER_PADDING_SIZE);
		avpkt->destruct = av_destruct_packet;
		avpkt->pts = avctx->coded_frame->pts;
		if(avctx->coded_frame && avctx->coded_frame->key_frame) {
			avpkt->flags |= AV_PKT_FLAG_KEY;
		}
		memcpy(avpkt->data, AV_G(encoding_buffer), ret);
		*got_packet_ptr = TRUE;
	} else {
		*got_packet_ptr = FALSE;
	}
	return ret;
}
#endif

#ifndef HAVE_AVCODEC_ENCODE_AUDIO2
int avcodec_encode_audio2(AVCodecContext *avctx, AVPacket *avpkt, const AVFrame *frame, int *got_packet_ptr) {
	int fs_tmp = 0;
	int ret;
	TSRMLS_FETCH();

	if(!frame) {
		*got_packet_ptr = FALSE;
		return 0;
	}
	if(!AV_G(encoding_buffer)) {
		AV_G(encoding_buffer_size) = 100000;
		AV_G(encoding_buffer) = emalloc(AV_G(encoding_buffer_size));
	}
    if ((avctx->codec->capabilities & CODEC_CAP_SMALL_LAST_FRAME) && frame->nb_samples < avctx->frame_size) {
         fs_tmp = avctx->frame_size;
         avctx->frame_size = frame->nb_samples;
     }
	ret = avcodec_encode_audio(avctx, AV_G(encoding_buffer), AV_G(encoding_buffer_size), (frame) ? (const short *) frame->data[0] : NULL);

	if(ret > 0) {
		avpkt->size = ret;
		avpkt->data = av_realloc(avpkt->data, avpkt->size + FF_INPUT_BUFFER_PADDING_SIZE);
		avpkt->destruct = av_destruct_packet;
		if(avctx->coded_frame) {
			avpkt->pts = avctx->coded_frame->pts;
		} else {
			avpkt->pts = frame->pts;
		}
        if(fs_tmp) {
			AVRational duration = { 1, avctx->sample_rate };
			avpkt->duration = (int) av_rescale_q(avctx->frame_size, duration, avctx->time_base);
        }
		avpkt->flags |= AV_PKT_FLAG_KEY;
		avctx->frame_number++;
		memcpy(avpkt->data, AV_G(encoding_buffer), ret);
		*got_packet_ptr = TRUE;
	} else {
		*got_packet_ptr = FALSE;
	}
	return ret;
}
#endif

#ifndef HAVE_AVCODEC_ENCODE_SUBTITLE2
int avcodec_encode_subtitle2(AVCodecContext *avctx, AVPacket *avpkt, const AVSubtitle *subtitle, int *got_packet_ptr) {
	int ret;
	TSRMLS_FETCH();

	if(!subtitle) {
		*got_packet_ptr = FALSE;
		return 0;
	}
	if(!AV_G(encoding_buffer)) {
		AV_G(encoding_buffer_size) = 100000;
		AV_G(encoding_buffer) = emalloc(AV_G(encoding_buffer_size));
	}
	ret = avcodec_encode_subtitle(avctx, AV_G(encoding_buffer), AV_G(encoding_buffer_size), subtitle);

	if(ret > 0) {
		AVRational time_base = { 1, AV_TIME_BASE };
		avpkt->size = ret;
		avpkt->data = av_realloc(avpkt->data, avpkt->size + FF_INPUT_BUFFER_PADDING_SIZE);
		avpkt->destruct = av_destruct_packet;
		avpkt->pts = av_rescale_q(subtitle->pts, time_base, avctx->time_base);
		avpkt->flags |= AV_PKT_FLAG_KEY;
		memcpy(avpkt->data, AV_G(encoding_buffer), ret);
		*got_packet_ptr = TRUE;
	} else {
		*got_packet_ptr = FALSE;
	}
	return ret;
}
#endif

static void av_transfer_pcm_to_frame(av_stream *strm);
static int av_encode_next_frame(av_stream *strm, double time);

static void av_flush_remaining_frames(av_stream *strm) {
	if(!(strm->flags & AV_STREAM_FLUSHED)) {
		int packet_finished;
		int result;
		AVPacket *packet;

		if(strm->codec->type == AVMEDIA_TYPE_AUDIO) {
			if(strm->sample_count > 0) {
				av_transfer_pcm_to_frame(strm);
				av_encode_next_frame(strm, strm->sample_start_time);
				strm->sample_count = 0;
			}
		}

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
		strm->flags |= AV_STREAM_FLUSHED;
	}
}

static int av_flush_pending_packets(av_file *file) {
	av_stream *pending_stream;

	if(file->flags & AV_FILE_HEADER_WRITTEN) {
		uint32_t i;
		for(i = 0; i < file->stream_count; i++) {
			av_stream *strm = file->streams[i];
			if(strm) {
				av_flush_remaining_frames(strm);
			}
		}
	}

	file->flags |= AV_FILE_EOF_REACHED;
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

#if !defined(HAVE_SWRESAMPLE) && !defined(HAVE_AVRESAMPLE)
#	define RESAMPLER_REQUIRES_EXTRA_SAMPLES		16
#endif

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
		int sample_size;

		if(strm->codec_cxt->sample_fmt >= AV_SAMPLE_FMT_U8P && strm->codec_cxt->sample_fmt <= AV_SAMPLE_FMT_DBLP) {
			codec_format = strm->codec_cxt->sample_fmt - AV_SAMPLE_FMT_U8P;
			strm->deinterleave = TRUE;
		} else {
			codec_format = strm->codec_cxt->sample_fmt;
			strm->deinterleave = FALSE;
		}
		switch(codec_format) {
			case AV_SAMPLE_FMT_U8: sample_size = sizeof(uint8_t) * strm->codec_cxt->channels; break;
			case AV_SAMPLE_FMT_S16: sample_size = sizeof(int16_t) * strm->codec_cxt->channels; break;
			case AV_SAMPLE_FMT_S32: sample_size = sizeof(int32_t) * strm->codec_cxt->channels; break;
			case AV_SAMPLE_FMT_FLT: sample_size = sizeof(float) * strm->codec_cxt->channels; break;
			case AV_SAMPLE_FMT_DBL: sample_size = sizeof(double) * strm->codec_cxt->channels; break;
			default: sample_size = 0;
		}
		if(purpose == FOR_ENCODING) {
			strm->resampler_cxt = av_audio_resample_init(strm->codec_cxt->channels, 2, strm->codec_cxt->sample_rate, 44100, codec_format, AV_SAMPLE_FMT_FLT, 16, 10, 0, 0.8);
			strm->resampler_extra_sample_count = RESAMPLER_REQUIRES_EXTRA_SAMPLES;
			strm->target_sample_size = sample_size;
			strm->source_sample_size = sizeof(float) * 2;
		} else {
			strm->resampler_cxt = av_audio_resample_init(2, strm->codec_cxt->channels, 44100, strm->codec_cxt->sample_rate, AV_SAMPLE_FMT_FLT, codec_format, 16, 10, 0, 0.8);
			strm->target_sample_size = sizeof(float) * 2;
			strm->source_sample_size = sample_size;
		}
		if(purpose == FOR_ENCODING || strm->deinterleave) {
			int resampler_queue_size = av_samples_get_buffer_size(NULL, strm->codec_cxt->channels, strm->codec_cxt->frame_size + RESAMPLER_REQUIRES_EXTRA_SAMPLES, codec_format, 1);
			strm->resampler_queue = emalloc(resampler_queue_size + FF_INPUT_BUFFER_PADDING_SIZE);
		}
#endif
		if(frame_duration > 0) {
			if(purpose == FOR_ENCODING) {
				strm->sample_buffer_size = (uint32_t) floor(frame_duration * 44100);
			} else {
				strm->sample_buffer_size = (uint32_t) ceil(frame_duration * 44100);
			}
#ifdef RESAMPLER_REQUIRES_EXTRA_SAMPLES
			strm->samples = emalloc(sizeof(float) * (strm->sample_buffer_size + RESAMPLER_REQUIRES_EXTRA_SAMPLES) * 2 + FF_INPUT_BUFFER_PADDING_SIZE);
#else
			strm->samples = emalloc(sizeof(float) * strm->sample_buffer_size * 2 + FF_INPUT_BUFFER_PADDING_SIZE);
#endif
		}
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

#ifndef HAVE_AVCODEC_FILL_AUDIO_FRAME
int avcodec_fill_audio_frame(AVFrame *frame, int nb_channels,
                             enum AVSampleFormat sample_fmt, const uint8_t *buf,
                             int buf_size, int align)
{
    int ch, planar, needed_size, ret = 0;

    needed_size = av_samples_get_buffer_size(NULL, nb_channels,
                                             frame->nb_samples, sample_fmt,
                                             align);
    if (buf_size < needed_size)
        return AVERROR(EINVAL);

    planar = av_sample_fmt_is_planar(sample_fmt);
    if (planar && nb_channels > AV_NUM_DATA_POINTERS) {
        if (!(frame->extended_data = av_mallocz(nb_channels *
                                                sizeof(*frame->extended_data))))
            return AVERROR(ENOMEM);
    } else {
        frame->extended_data = frame->data;
    }

    if ((ret = av_samples_fill_arrays(frame->extended_data, &frame->linesize[0],
                                      buf, nb_channels, frame->nb_samples,
                                      sample_fmt, align)) < 0) {
        if (frame->extended_data != frame->data)
            av_freep(&frame->extended_data);
        return ret;
    }
    if (frame->extended_data != frame->data) {
        for (ch = 0; ch < AV_NUM_DATA_POINTERS; ch++)
            frame->data[ch] = frame->extended_data[ch];
    }

    return ret;
}
#endif

static void av_transfer_pcm_to_frame(av_stream *strm) {
	// allocate the audio frame if it's not there
	if(!(strm->flags & AV_STREAM_AUDIO_BUFFER_ALLOCATED)) {
		uint32_t buffer_size = av_samples_get_buffer_size(NULL, strm->codec_cxt->channels, strm->codec_cxt->frame_size, strm->codec_cxt->sample_fmt, 1);
		uint8_t *buffer = emalloc(buffer_size + FF_INPUT_BUFFER_PADDING_SIZE);

	    strm->frame->format = strm->codec_cxt->sample_fmt;
	    strm->frame->nb_samples = strm->codec_cxt->frame_size;
		avcodec_fill_audio_frame(strm->frame, strm->codec_cxt->channels, strm->codec_cxt->sample_fmt, buffer, buffer_size, 1);
		strm->flags |= AV_STREAM_AUDIO_BUFFER_ALLOCATED;
	}
#if defined(HAVE_SWRESAMPLE)
	strm->frame->nb_samples = swr_convert(strm->resampler_cxt, (uint8_t **) strm->frame->data, strm->frame->nb_samples, (const uint8_t **) &strm->samples, strm->sample_count);
#elif defined(HAVE_AVRESAMPLE)
	strm->frame->nb_samples = avresample_convert(strm->resampler_cxt, (uint8_t **) strm->frame->data, 0, strm->frame->nb_samples, (uint8_t **) &strm->samples, 0, strm->sample_count);
#else
	if(strm->sample_count > 0) {
		uint8_t *cursor = strm->resampler_queue + strm->resampler_queue_length * strm->target_sample_size;
		strm->resampler_queue_length += audio_resample(strm->resampler_cxt, (short *) cursor, (short *) strm->samples, strm->sample_count);
	}
	if(strm->resampler_queue_length <= strm->codec_cxt->frame_size) {
		strm->frame->nb_samples = strm->resampler_queue_length;
		strm->resampler_queue_length = 0;
	} else {
		strm->frame->nb_samples = strm->codec_cxt->frame_size;
		strm->resampler_queue_length -= strm->codec_cxt->frame_size;
	}

	if(strm->codec_cxt->sample_fmt >= AV_SAMPLE_FMT_U8P && strm->codec_cxt->sample_fmt <= AV_SAMPLE_FMT_DBLP) {
		int32_t i, j;
		switch(strm->codec_cxt->sample_fmt) {
			case AV_SAMPLE_FMT_U8P: {
				int8_t *src_pointer = (int8_t *) strm->resampler_queue;
				int8_t **dest_pointers = (int8_t **) strm->frame->data;
				for(i = 0; i < strm->frame->nb_samples; i++) {
					for(j = 0; j < strm->codec_cxt->channels; j++) {
						int8_t *dest_pointer = dest_pointers[j] + i;
						*dest_pointer = *src_pointer;
						src_pointer++;
					}
				}
			}	break;
			case AV_SAMPLE_FMT_S16P: {
				int16_t *src_pointer = (int16_t *) strm->resampler_queue;
				int16_t **dest_pointers = (int16_t **) strm->frame->data;
				for(i = 0; i < strm->frame->nb_samples; i++) {
					for(j = 0; j < strm->codec_cxt->channels; j++) {
						int16_t *dest_pointer = dest_pointers[j] + i;
						*dest_pointer = *src_pointer;
						src_pointer++;
					}
				}
			}	break;
			case AV_SAMPLE_FMT_S32P: {
				int32_t *src_pointer = (int32_t *) strm->resampler_queue;
				int32_t **dest_pointers = (int32_t **) strm->frame->data;
				for(i = 0; i < strm->frame->nb_samples; i++) {
					for(j = 0; j < strm->codec_cxt->channels; j++) {
						int32_t *dest_pointer = dest_pointers[j] + i;
						*dest_pointer = *src_pointer;
						src_pointer++;
					}
				}
			}	break;
			case AV_SAMPLE_FMT_FLTP: {
				float *src_pointer = (float *) strm->resampler_queue;
				float **dest_pointers = (float **) strm->frame->data;
				for(i = 0; i < strm->frame->nb_samples; i++) {
					for(j = 0; j < strm->codec_cxt->channels; j++) {
						float *dest_pointer = dest_pointers[j] + i;
						*dest_pointer = *src_pointer;
						src_pointer++;
					}
				}
			}	break;
			case AV_SAMPLE_FMT_DBLP: {
				double *src_pointer = (double *) strm->resampler_queue;
				double **dest_pointers = (double **) strm->frame->data;
				for(i = 0; i < strm->frame->nb_samples; i++) {
					for(j = 0; j < strm->codec_cxt->channels; j++) {
						double *dest_pointer = dest_pointers[j] + i;
						*dest_pointer = *src_pointer;
						src_pointer++;
					}
				}
			}	break;
			default: break;
		}
	} else {
		memcpy(strm->frame->data[0], strm->resampler_queue, strm->frame->nb_samples * strm->target_sample_size);
	}
	// move remaining output samples to the front
	if(strm->resampler_queue_length > 0) {
		memcpy(strm->resampler_queue, strm->resampler_queue + strm->frame->nb_samples * strm->target_sample_size, strm->resampler_queue_length * strm->target_sample_size);
	}
#endif
}

static void av_transfer_pcm_from_frame(av_stream *strm) {
	uint32_t sample_count = (uint32_t) ceil(strm->frame_duration * 44100);
	if(strm->sample_buffer_size < sample_count) {
		strm->sample_buffer_size = sample_count;
		strm->samples = erealloc(strm->samples, sizeof(float) * strm->sample_buffer_size * 2 + FF_INPUT_BUFFER_PADDING_SIZE);
	}
#if defined(HAVE_SWRESAMPLE)
	strm->sample_count = swr_convert(strm->resampler_cxt, (uint8_t **) &strm->samples, strm->sample_buffer_size, (const uint8_t **) strm->frame->data, strm->frame->nb_samples);
#elif defined(HAVE_AVRESAMPLE)
	strm->sample_count = avresample_convert(strm->resampler_cxt, (uint8_t **) &strm->samples, 0, strm->sample_buffer_size, (uint8_t **) strm->frame->data, 0, strm->frame->nb_samples);
#else
	uint8_t *src_buffer;
	if(strm->deinterleave) {
		int32_t i, j;
		switch(strm->codec_cxt->sample_fmt) {
			case AV_SAMPLE_FMT_U8P: {
				int8_t *dest_pointer = (int8_t *) strm->resampler_queue;
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
				int16_t *dest_pointer = (int16_t *) strm->resampler_queue;
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
				int32_t *dest_pointer = (int32_t *) strm->resampler_queue;
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
				float *dest_pointer = (float *) strm->resampler_queue;
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
				double *dest_pointer = (double *) strm->resampler_queue;
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
		src_buffer = strm->resampler_queue;
	} else {
		src_buffer = strm->frame->data[0];
	}
	strm->sample_count = audio_resample(strm->resampler_cxt, (short *) strm->samples, (short *) src_buffer, strm->frame->nb_samples);
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

#pragma pack(push, 1)
struct rgba {
	char r;
	char g;
	char b;
	char a;
};
#pragma pack(pop)

static void av_copy_subtitle_to_gd(AVPicture *picture, uint32_t color_count, gdImagePtr image) {
	uint8_t *gd_pixel;
	uint8_t *av_pixel;
	struct rgba *av_palette;
	uint32_t i;

	// copy pixels
	for(i = 0; i < (uint32_t) image->sy; i++) {
		gd_pixel = image->pixels[i];
		av_pixel = picture->data[0] + picture->linesize[0] * i;
		memcpy(gd_pixel, av_pixel, image->sx);
	}

	// copy palette
	if(color_count > gdMaxColors) {
		color_count = gdMaxColors;
	}
	av_palette = (struct rgba *) picture->data[1];
	for(i = 0; i < color_count; i++) {
		image->open[i] = FALSE;
		image->red[i] = av_palette[i].r;
		image->green[i] = av_palette[i].g;
		image->blue[i] = av_palette[i].b;
		image->alpha[i] = gdAlphaTransparent - (av_palette[i].a >> 1);
		if(av_palette[i].a == 0 && image->transparent == -1) {
			image->transparent = i;
		}
	}
	image->colorsTotal = color_count;
	image->saveAlphaFlag = TRUE;
}

static void av_copy_subtitle_from_gd(AVPicture *picture, int *p_color_count, gdImagePtr image) {
	uint8_t *gd_pixel;
	uint8_t *av_pixel;
	struct rgba *av_palette;
	uint32_t i;

	// copy pixels
	picture->data[0] = av_realloc(picture->data[0], (uint32_t) image->sy * image->sx);
	picture->linesize[0] = image->sx;
	for(i = 0; i < (uint32_t) image->sy; i++) {
		gd_pixel = image->pixels[i];
		av_pixel = picture->data[0] + picture->linesize[0] * i;
		memcpy(av_pixel, gd_pixel, image->sx);
	}

	// copy palette
	picture->data[1] = av_realloc(picture->data[1], sizeof(struct rgba) * image->colorsTotal);
	av_palette = (struct rgba *) picture->data[1];
	for(i = 0; i < (uint32_t) image->colorsTotal; i++) {
		av_palette[i].r = image->red[i];
		av_palette[i].g = image->green[i];
		av_palette[i].b = image->blue[i];
		if(image->alpha[i] == 255) {
			av_palette[i].a = gdAlphaOpaque;
		} else {
			av_palette[i].a = (gdAlphaTransparent - image->alpha[i]) << 1;
		}
	}
	*p_color_count = image->colorsTotal;
}

static int av_write_file_header(av_file *file) {
	if(!(file->flags & AV_FILE_HEADER_WRITTEN)) {
		if(avformat_write_header(file->format_cxt, NULL) < 0) {
			if(!(file->flags & AV_FILE_HEADER_ERROR_ENCOUNTERED)) {
				TSRMLS_FETCH();
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Error encountered writing file header");
				file->flags |= AV_FILE_HEADER_ERROR_ENCOUNTERED;
			}
			return FALSE;
		}
		file->flags |= AV_FILE_HEADER_WRITTEN;
	}
	return TRUE;
}

static int av_encode_next_frame(av_stream *strm, double time) {
	int packet_finished = FALSE;
	int result;
	AVPacket *packet;

	strm->frame->pts = (int64_t) (time / av_q2d(strm->codec_cxt->time_base));

	if(!av_write_file_header(strm->file)) {
		return FALSE;
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
	strm->frame_pts = AV_NOPTS_VALUE;

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
				// use the pts of the packet that triggered the creation
				// of the frame; if none, use the packet's dts  
				// the timestamp is the number of
				// time-united employed in the stream
				if(strm->frame_pts != AV_NOPTS_VALUE) {
					time_stamp = strm->frame_pts;
				} else if(strm->packet->dts != AV_NOPTS_VALUE) {
					time_stamp = strm->packet->dts;
				} else {
					time_stamp = AV_NOPTS_VALUE;
				}
			}
			if(time_stamp != AV_NOPTS_VALUE) {
				*p_time = strm->next_frame_time = time_stamp * av_q2d(strm->stream->time_base);
			} else {
				*p_time = strm->next_frame_time;
			}

			if(strm->codec->type == AVMEDIA_TYPE_AUDIO) {
				strm->frame_duration = (double) dest_frame->nb_samples / strm->codec_cxt->sample_rate;
			}
			strm->next_frame_time += strm->frame_duration;
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
		} while(!(current_frame_time <= strm->time_sought && strm->time_sought < next_frame_time) && next_frame_time < strm->time_sought);

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

static int av_encode_next_subtitle(av_stream *strm, double time) {
	int packet_finished = FALSE;
	int result;
	AVPacket *packet;

	strm->subtitle->pts = (int64_t) (time * AV_TIME_BASE);

	if(!av_write_file_header(strm->file)) {
		return FALSE;
	}

	packet = emalloc(sizeof(AVPacket));
	av_init_packet(packet);
	packet->data = NULL;
	packet->size = 0;

	result = avcodec_encode_subtitle2(strm->codec_cxt, packet, strm->subtitle, &packet_finished);

	if(packet_finished) {
		av_write_next_packet(strm, packet);
	} else {
		av_free_packet(packet);
		efree(packet);
	}
	return !(result < 0);
}

static int av_decode_subtitle_at_cursor(av_stream *strm, AVSubtitle *dest_subtitle, double *p_time TSRMLS_DC) {
	int frame_finished = FALSE;
	for(;;) {
		int bytes_decoded;
		if(av_read_next_packet(strm TSRMLS_CC)) {
			bytes_decoded = avcodec_decode_subtitle2(strm->codec_cxt, dest_subtitle, &frame_finished, strm->packet);
		} else {
			return FALSE;
		}
		if(bytes_decoded < 0) {
			return FALSE;
		}
		strm->packet_bytes_remaining -= bytes_decoded;

		if(frame_finished) {
			int64_t time_stamp = dest_subtitle->pts;
			*p_time = time_stamp * (1.0 / AV_TIME_BASE);
			break;
		}
	}
	if(!strm->packet_bytes_remaining) {
		av_shift_packet(strm);
	}
	return TRUE;
}

static int av_decode_next_subtitle(av_stream *strm, double *p_time TSRMLS_DC) {
	if(strm->flags & AV_STREAM_SOUGHT) {
		AVSubtitle *current_subtitle = strm->subtitle;
		double current_subtitle_time;
		double start_time, end_time;
		do {
			// decode the next subtitle
			if(!av_decode_subtitle_at_cursor(strm, current_subtitle, &current_subtitle_time TSRMLS_CC)) {
				return FALSE;
			}
			start_time = current_subtitle_time + current_subtitle->start_display_time * (1.0 / 1000);
			end_time = current_subtitle_time + current_subtitle->end_display_time * (1.0 / 1000);
		} while(!(start_time <= strm->time_sought && strm->time_sought < end_time) && end_time < strm->time_sought);

		strm->flags &= ~AV_STREAM_SOUGHT;
		strm->subtitle = current_subtitle;
		*p_time = current_subtitle_time;
	} else {
		return av_decode_subtitle_at_cursor(strm, strm->subtitle, p_time TSRMLS_CC);
	}
	return TRUE;
}

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
	int result;

	if(Z_TYPE_P(buffer) != IS_STRING) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Audio data must be contained in a string");
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
				result = av_encode_next_frame(strm, strm->sample_start_time);
				strm->sample_count = 0;
				if(!result) {
					return FALSE;
				}
			}
			strm->sample_start_time = time;
			dst_samples = strm->samples;
		}
	}

	// keep copying into audio frame until are samples are used up
	while(src_samples_remaining) {
		uint32_t samples_needed = strm->sample_buffer_size - strm->sample_count;
		uint32_t samples_to_copy;
		uint32_t i;

#ifdef RESAMPLER_REQUIRES_EXTRA_SAMPLES
		if(strm->resampler_extra_sample_count > 0) {
			samples_needed += strm->resampler_extra_sample_count;
		}
#endif

		if(samples_needed < src_samples_remaining) {
			samples_to_copy = samples_needed;
		} else {
			samples_to_copy = src_samples_remaining;
		}

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

		if(strm->sample_count >= strm->sample_buffer_size) {
			// transfer the data to the frame then compress it
			av_transfer_pcm_to_frame(strm);
			result = av_encode_next_frame(strm, strm->sample_start_time);

			// adjust the time
			strm->sample_start_time += strm->sample_count * (1 / 44100.0);
			strm->sample_count = 0;
#ifdef RESAMPLER_REQUIRES_EXTRA_SAMPLES
			strm->resampler_extra_sample_count = 0;
#endif
			dst_samples = strm->samples;

			if(!result) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to encode audio packet");
				return FALSE;
			}
		}
	}
	return TRUE;
}


static int av_decode_pcm_to_zval(av_stream *strm, zval *buffer, double *p_time TSRMLS_DC) {
	if(av_decode_next_frame(strm, p_time TSRMLS_CC)) {
		int data_len;
		char *data;

		av_create_audio_buffer_and_resampler(strm, FOR_DECODING);
		av_transfer_pcm_from_frame(strm);

		data_len = sizeof(float) * 2 * strm->sample_count;
		if(Z_TYPE_P(buffer) != IS_STRING || Z_STRLEN_P(buffer) < data_len) {
			zval_dtor(buffer);
			data = emalloc(data_len + 1);
			Z_TYPE_P(buffer) = IS_STRING;
			Z_STRVAL_P(buffer) = data;
		} else {
			data = Z_STRVAL_P(buffer);
		}
		Z_STRLEN_P(buffer) = data_len;
		memcpy(data, strm->samples, data_len);
		data[data_len] = '\0';
		return TRUE;
	} else {
		zval_dtor(buffer);
		Z_TYPE_P(buffer) = IS_NULL;
		return FALSE;
	}
}

static int av_encode_subtitle_from_zval(av_stream *strm, zval *buffer, double time TSRMLS_DC) {
	HashTable *ht;
	Bucket *p;
	double start_time, end_time;
	long x, y, width, height;
	zval *z_image = NULL;
	gdImagePtr image = NULL;
	char *text = NULL, *ass = NULL;
	long len;
	uint32_t i;

	if(Z_TYPE_P(buffer) != IS_ARRAY) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Subtitle data must be contained in an array");
		return FALSE;
	}
	if(!av_get_element_double(buffer, "start", &start_time) || !av_get_element_double(buffer, "end", &end_time)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Subtitle data must contain start and end time");
		return FALSE;
	}
	if(!av_get_element_hash(buffer, "rects", &ht)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Subtitle data must contain rects");
		return FALSE;
	}
	for(p = ht->pListHead; p; p = p->pListNext) {
		zval **p_element = p->pData;
		if(!av_get_element_resource(*p_element, "image", &z_image)
		&& !av_get_element_string(*p_element, "text", &text)
		&& !av_get_element_string(*p_element, "ass", &ass)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Subtitle rectangle must contain image, text, or ass");
			return FALSE;
		}
		if(z_image) {
			image = (gdImagePtr) zend_fetch_resource(&z_image TSRMLS_CC, -1, NULL, NULL, 1, le_gd);
			if(!image || image->trueColor) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Subtitle image must use a palette");
				return FALSE;
			}
			if(!av_get_element_long(*p_element, "x", &x)
			|| !av_get_element_long(*p_element, "y", &y)
			|| !av_get_element_long(*p_element, "width", &width)
			|| !av_get_element_long(*p_element, "height", &height)) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Subtitle rectangle must contain x, y, width, and height");
				return FALSE;
			}
		}
	}

	if(!strm->subtitle) {
		strm->subtitle = av_malloc(sizeof(AVSubtitle));
		memset(strm->subtitle, 0, sizeof(AVSubtitle));
	}

	if(ht->nNumOfElements > strm->subtitle->num_rects) {
		strm->subtitle->rects = av_realloc(strm->subtitle->rects, sizeof(AVSubtitleRect *) * ht->nNumOfElements);
		for(i = strm->subtitle->num_rects; i < ht->nNumOfElements; i++) {
			strm->subtitle->rects[i] = av_malloc(sizeof(AVSubtitleRect));
			memset(strm->subtitle->rects[i], 0, sizeof(AVSubtitleRect));
		}
	} else if(ht->nNumOfElements < strm->subtitle->num_rects) {
		for(i = ht->nNumOfElements; i < strm->subtitle->num_rects; i++) {
			av_freep(&strm->subtitle->rects[i]->pict.data[0]);
			av_freep(&strm->subtitle->rects[i]->pict.data[1]);
			av_freep(&strm->subtitle->rects[i]->pict.data[2]);
			av_freep(&strm->subtitle->rects[i]->pict.data[3]);
			av_freep(&strm->subtitle->rects[i]->text);
			av_freep(&strm->subtitle->rects[i]->ass);
			av_freep(&strm->subtitle->rects[i]);
		}
	}
	strm->subtitle->num_rects = ht->nNumOfElements;
	strm->subtitle->start_display_time = (uint32_t) (start_time * 1000);
	strm->subtitle->end_display_time = (uint32_t) (end_time * 1000);

	for(p = ht->pListHead, i = 0; p; p = p->pListNext, i++) {
		zval **p_element = p->pData;
		AVSubtitleRect *rect = strm->subtitle->rects[i];

		if(av_get_element_resource(*p_element, "image", &z_image)) {
			av_get_element_long(*p_element, "x", &x);
			av_get_element_long(*p_element, "y", &y);
			av_get_element_long(*p_element, "width", &width);
			av_get_element_long(*p_element, "height", &height);
			rect->x = x;
			rect->y = y;
			rect->w = width;
			rect->h = height;

			image = (gdImagePtr) zend_fetch_resource(&z_image TSRMLS_CC, -1, NULL, NULL, 1, le_gd);
			av_copy_subtitle_from_gd(&rect->pict, &rect->nb_colors, image);
			rect->type = SUBTITLE_BITMAP;
		} else if(av_get_element_stringl(*p_element, "text", &text, &len)) {
			rect->text = av_malloc(len + 1);
			strcpy(rect->text, text);
			rect->type = SUBTITLE_TEXT;
		} else if(av_get_element_stringl(*p_element, "ass", &ass, &len)) {
			rect->ass = av_malloc(len + 1);
			strcpy(rect->ass, ass);
			rect->type = SUBTITLE_ASS;
		}
	}
	return av_encode_next_subtitle(strm, time);
}

static int av_decode_subtitle_to_zval(av_stream *strm, zval *buffer, double *p_time TSRMLS_DC) {
	zval_dtor(buffer);

	if(av_decode_next_subtitle(strm, p_time TSRMLS_CC)) {
		uint32_t i;
		zval *z_rects;
		double start_time = (double) strm->subtitle->start_display_time * (1.0 / 1000);
		double end_time = (double) strm->subtitle->end_display_time * (1.0 / 1000);

		array_init(buffer);
		av_set_element_double(buffer, "start", start_time);
		av_set_element_double(buffer, "end", end_time);
		ALLOC_INIT_ZVAL(z_rects);
		array_init(z_rects);
		zend_hash_update(HASH_OF(buffer), "rects", (uint32_t) strlen("rects") + 1, (void *) &z_rects, sizeof(zval *), NULL);

		for(i = 0; i < strm->subtitle->num_rects; i++) {
			AVSubtitleRect *rect = strm->subtitle->rects[i];
			zval *z_rect, *z_image;
			ALLOC_INIT_ZVAL(z_rect);
			array_init(z_rect);

			av_set_element_long(z_rect, "x", rect->x);
			av_set_element_long(z_rect, "y", rect->y);
			av_set_element_long(z_rect, "width", rect->w);
			av_set_element_long(z_rect, "height", rect->h);

			switch(rect->type) {
				case SUBTITLE_BITMAP: {
					z_image = av_create_gd_image(rect->w, rect->h TSRMLS_CC);
					if(z_image) {
						gdImagePtr image = (gdImagePtr) zend_fetch_resource(&z_image TSRMLS_CC, -1, "image", NULL, 1, le_gd);
						av_copy_subtitle_to_gd(&rect->pict, rect->nb_colors, image);
						zend_hash_update(Z_ARRVAL_P(z_rect), "image", (uint32_t) strlen("image") + 1, (void *) &z_image, sizeof(zval *), NULL);
					}
				}	break;
				case SUBTITLE_TEXT: {
					av_set_element_string(z_rect, "text", rect->text);
				}	break;
				case SUBTITLE_ASS: {
					av_set_element_string(z_rect, "ass", rect->ass);
				}	break;
				default:
					break;
			}
			zend_hash_next_index_insert(Z_ARRVAL_P(z_rects), &z_rect, sizeof(zval *), NULL);
		}
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

	av_set_log_level(TSRMLS_C);

	if(strm->codec->type != AVMEDIA_TYPE_VIDEO) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Not a video stream");
		return;
	}
	if(!(strm->file->flags & AV_FILE_WRITE)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Not a writable stream");
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
	if(!(strm->file->flags & AV_FILE_WRITE)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Not a writable stream");
		return;
	}
	if(av_encode_pcm_from_zval(strm, z_buffer, time TSRMLS_CC)) {
		RETURN_TRUE;
	} else {
		RETVAL_FALSE;
	}
}
/* }}} */

/* {{{ proto bool av_stream_write_subtitle()
   Write subtitle */
PHP_FUNCTION(av_stream_write_subtitle)
{
	zval *z_strm, *z_buffer;
	av_stream *strm;
	double time = NAN;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rzd", &z_strm, &z_buffer, &time) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(strm, av_stream *, &z_strm, -1, "av stream", le_av_strm);

	av_set_log_level(TSRMLS_C);

	if(strm->codec->type != AVMEDIA_TYPE_SUBTITLE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Not a subtitle stream");
		return;
	}
	if(!(strm->file->flags & AV_FILE_WRITE)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Not a writable stream");
		return;
	}
	if(av_encode_subtitle_from_zval(strm, z_buffer, time TSRMLS_CC)) {
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
	if(!(strm->file->flags & AV_FILE_READ)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Not a readable stream");
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
	if(!(strm->file->flags & AV_FILE_READ)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Not a readable stream");
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

/* {{{ proto string av_stream_read_subtitle()
   Read an image */
PHP_FUNCTION(av_stream_read_subtitle)
{
	zval *z_strm, *z_subtitle, *z_time = NULL;
	av_stream *strm;
	double time;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rzz", &z_strm, &z_subtitle, &z_time) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(strm, av_stream *, &z_strm, -1, "av stream", le_av_strm);

	av_set_log_level(TSRMLS_C);

	if(strm->codec->type != AVMEDIA_TYPE_SUBTITLE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Not a subtitle stream");
		return;
	}
	if(!(strm->file->flags & AV_FILE_READ)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Not a readable stream");
		return;
	}
	if(av_decode_subtitle_to_zval(strm, z_subtitle, &time TSRMLS_CC)) {
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

/* {{{ proto string av_get_encoders(void)
   Get list of encoders available */
PHP_FUNCTION(av_get_encoders)
{
	AVCodec *codec = NULL, *decoder;

    if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	array_init(return_value);
	while((codec = av_codec_next(codec))) {
		if(av_codec_is_encoder(codec)) {
			add_next_index_string(return_value, codec->name, 1);

			if(strncmp(codec->name, "lib", 3) == 0) {
				// codec is available without the prefix
				add_next_index_string(return_value, codec->name + 3, 1);
			}

			decoder = NULL;
			while((decoder = av_codec_next(decoder))) {
				if(decoder->id == codec->id && decoder != codec) {
					if(av_codec_is_decoder(decoder)) {
						// see if the decoder has a different name
						if(strcmp(codec->name, decoder->name) != 0) {
							// make the encoder available under the name of the decoder as well
							add_next_index_string(return_value, decoder->name, 1);
						}
						break;
					}
				}
			}
		}
	}
}
/* }}} */

/* {{{ proto string av_get_decoders(void)
   Get list of decoders available */
PHP_FUNCTION(av_get_decoders)
{
	AVCodec *codec = NULL;

    if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	array_init(return_value);
	while((codec = av_codec_next(codec))) {
		if(av_codec_is_decoder(codec)) {
			add_next_index_string(return_value, codec->name, 1);
			if(strncmp(codec->name, "lib", 3) == 0) {
				add_next_index_string(return_value, codec->name + 3, 1);
			}
		}
	}
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
