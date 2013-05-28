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
#include <swscale.h>
#include <swresample.h>

typedef struct av_file av_file;
typedef struct av_stream av_stream;

struct av_stream {
	AVCodecContext *codec_cxt;
	const AVCodec *codec;
	AVStream *stream;

	AVFrame *frame;						// the current frame
	int64_t frame_pts;					// the PTS of the packet that triggered the creation of the frame (used during decoding only)
	AVFrame *next_frame;
	double next_frame_time;

	AVFrame *picture;					// RGBA picture
	struct SwsContext *scalar_cxt;		// scalar context

	float *samples;						// PCM data after resampling
	uint32_t sample_count;				// the number of samples currently buffered
	uint32_t sample_buffer_size;		// the number of samples in an audio frame
	SwrContext *resampler_cxt;

	AVPacket *packet;					// the current packet
	AVPacket **packet_queue;			// packets for this stream waiting to be decoded or written to disk
	uint32_t packet_queue_size;			// length of the queue
	uint32_t packet_count;				// the number of packets in the queue
	uint32_t packet_bytes_remaining;	// the number of bytes remaining in current packet (used during decoding only)

	av_file *file;						// AV file containing this stream
	uint32_t index;						// index of this stream

	double time_sought;					// time passed to av_file_seek()

	int32_t flags;
};

enum {
	AV_FILE_READ 						= 0x0001,
	AV_FILE_WRITE 						= 0x0002,
	AV_FILE_APPEND						= 0x0004,

	AV_FILE_EOF_REACHED					= 0x1000,
	AV_FILE_HEADER_WRITTEN				= 0x2000,
	AV_FILE_LOCKED						= 0x4000,
	AV_FILE_FREED						= 0x8000,
};

enum {
	AV_STREAM_AUDIO_BUFFER_ALLOCATED	= 0x1000,
	AV_STREAM_FRAME_BUFFER_ALLOCATED			= 0x2000,
	AV_STREAM_SOUGHT					= 0x4000,
	AV_STREAM_FREED						= 0x8000,
};

struct av_file {
	AVFormatContext *format_cxt;
	const AVInputFormat *input_format;
	const AVOutputFormat *output_format;

	av_stream **streams;
	uint32_t stream_count;
	uint32_t open_stream_count;
	int32_t flags;
};

int av_optimize_mov_file(AVIOContext *pb);

PHP_MINIT_FUNCTION(av);
PHP_MSHUTDOWN_FUNCTION(av);
PHP_RINIT_FUNCTION(av);
PHP_RSHUTDOWN_FUNCTION(av);
PHP_MINFO_FUNCTION(av);

PHP_FUNCTION(av_file_open);
PHP_FUNCTION(av_file_close);
PHP_FUNCTION(av_file_seek);
PHP_FUNCTION(av_file_eof);
PHP_FUNCTION(av_file_stat);
PHP_FUNCTION(av_file_optimize);

PHP_FUNCTION(av_stream_open);
PHP_FUNCTION(av_stream_close);
PHP_FUNCTION(av_stream_read_image);
PHP_FUNCTION(av_stream_read_pcm);
PHP_FUNCTION(av_stream_write_image);
PHP_FUNCTION(av_stream_write_pcm);
PHP_FUNCTION(av_stream_get_duration);

ZEND_BEGIN_MODULE_GLOBALS(av)
#ifndef HAVE_AVCODEC_ENCODE_VIDEO2
	uint8_t *video_buffer;
	uint32_t video_buffer_size;
#endif

	zend_bool optimize_output;
ZEND_END_MODULE_GLOBALS(av)

#ifdef ZTS
#define AV_G(v) TSRMG(av_globals_id, zend_av_globals *, v)
#else
#define AV_G(v) (av_globals.v)
#endif

#ifndef TRUE
#define TRUE		1
#endif

#ifndef FALSE
#define FALSE		0
#endif

#define gdMaxColors 256

#define gdAlphaMax 127
#define gdAlphaOpaque 0
#define gdAlphaTransparent 127
#define gdRedMax 255
#define gdGreenMax 255
#define gdBlueMax 255
#define gdTrueColorGetAlpha(c) (((c) & 0x7F000000) >> 24)
#define gdTrueColorGetRed(c) (((c) & 0xFF0000) >> 16)
#define gdTrueColorGetGreen(c) (((c) & 0x00FF00) >> 8)
#define gdTrueColorGetBlue(c) ((c) & 0x0000FF)

#define gdImagePalettePixel(im, x, y) (im)->pixels[(y)][(x)]
#define gdImageTrueColorPixel(im, x, y) (im)->tpixels[(y)][(x)]

typedef struct gdImageStruct {
	/* Palette-based image pixels */
	unsigned char ** pixels;
	int sx;
	int sy;
	/* These are valid in palette images only. See also
		'alpha', which appears later in the structure to
		preserve binary backwards compatibility */
	int colorsTotal;
	int red[gdMaxColors];
	int green[gdMaxColors];
	int blue[gdMaxColors];
	int open[gdMaxColors];
	/* For backwards compatibility, this is set to the
		first palette entry with 100% transparency,
		and is also set and reset by the
		gdImageColorTransparent function. Newer
		applications can allocate palette entries
		with any desired level of transparency; however,
		bear in mind that many viewers, notably
		many web browsers, fail to implement
		full alpha channel for PNG and provide
		support for full opacity or transparency only. */
	int transparent;
	int *polyInts;
	int polyAllocated;
	struct gdImageStruct *brush;
	struct gdImageStruct *tile;
	int brushColorMap[gdMaxColors];
	int tileColorMap[gdMaxColors];
	int styleLength;
	int stylePos;
	int *style;
	int interlace;
	/* New in 2.0: thickness of line. Initialized to 1. */
	int thick;
	/* New in 2.0: alpha channel for palettes. Note that only
		Macintosh Internet Explorer and (possibly) Netscape 6
		really support multiple levels of transparency in
		palettes, to my knowledge, as of 2/15/01. Most
		common browsers will display 100% opaque and
		100% transparent correctly, and do something
		unpredictable and/or undesirable for levels
		in between. TBB */
	int alpha[gdMaxColors];
	/* Truecolor flag and pixels. New 2.0 fields appear here at the
		end to minimize breakage of existing object code. */
	int trueColor;
	int ** tpixels;
	/* Should alpha channel be copied, or applied, each time a
		pixel is drawn? This applies to truecolor images only.
		No attempt is made to alpha-blend in palette images,
		even if semitransparent palette entries exist.
		To do that, build your image as a truecolor image,
		then quantize down to 8 bits. */
	int alphaBlendingFlag;
	/* Should antialias functions be used */
	int antialias;
	/* Should the alpha channel of the image be saved? This affects
		PNG at the moment; other future formats may also
		have that capability. JPEG doesn't. */
	int saveAlphaFlag;


	/* 2.0.12: anti-aliased globals */
	int AA;
	int AA_color;
	int AA_dont_blend;
	unsigned char **AA_opacity;
	int AA_polygon;
	/* Stored and pre-computed variables for determining the perpendicular
	 * distance from a point to the anti-aliased line being drawn:
	 */
	int AAL_x1;
	int AAL_y1;
	int AAL_x2;
	int AAL_y2;
	int AAL_Bx_Ax;
	int AAL_By_Ay;
	int AAL_LAB_2;
	float AAL_LAB;

	/* 2.0.12: simple clipping rectangle. These values must be checked for safety when set; please use gdImageSetClip */
	int cx1;
	int cy1;
	int cx2;
	int cy2;
} gdImage;

typedef gdImage * gdImagePtr;

#define gdTrueColorAlpha(r, g, b, a) (((a) << 24) + \
	((r) << 16) + \
	((g) << 8) + \
	(b))

#endif	/* PHP_AV_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
