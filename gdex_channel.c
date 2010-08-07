/*
 * Extra image functions: channel handling functions
 *
 * Copyright (c) 2007-2010 Ryusuke SEKIYAMA. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * @package     php-gdextra
 * @author      Ryusuke SEKIYAMA <rsky0711@gmail.com>
 * @copyright   2007-2010 Ryusuke SEKIYAMA
 * @license     http://www.opensource.org/licenses/mit-license.php  MIT License
 */

#include "php_gdextra.h"

ZEND_EXTERN_MODULE_GLOBALS(gdextra);

/* {{{ macros */

#define MAX_CHANNELS 5

#define GET_INTENSITY_PARAMETERS const channel_t *ch, int x, int y, int oob
#define GET_INTENSITY_PARAMS_ADJUST() \
	x -= ch->xOffset; \
	y -= ch->yOffset;

#define MASK_ALPHA_PARAMETERS gdImagePtr im, const channel_t *ach, int x, int y, int z

/* }}} */
/* {{{ private type definitions */

typedef enum _grayscale_type_t {
	GRAYSCALE_B2W4 =  2,
	GRAYSCALE_B2W8 =  1,
	GRAYSCALE_NONE =  0,
	GRAYSCALE_W2B8 = -1,
	GRAYSCALE_W2B4 = -2
} grayscale_type_t;

typedef struct _channel_t channel_t;

typedef int (*get_intensity_func_t)(GET_INTENSITY_PARAMETERS);

struct _channel_t {
	gdImagePtr im;
	int width;
	int height;
	int xOffset;
	int yOffset;
	get_intensity_func_t get;
};

typedef void (*mask_alpha_func_t)(MASK_ALPHA_PARAMETERS);

/* }}} */
/* {{{ private function prototypes */

static int
_get_intensity_truecolor(GET_INTENSITY_PARAMETERS),
_get_intensity_palette(GET_INTENSITY_PARAMETERS),
_get_intensity_grayscale(GET_INTENSITY_PARAMETERS),
_get_intensity_grayindex(GET_INTENSITY_PARAMETERS),
_get_alpha_truecolor(GET_INTENSITY_PARAMETERS),
_get_alpha_palette(GET_INTENSITY_PARAMETERS),
_get_alpha_grayscale(GET_INTENSITY_PARAMETERS),
_get_alpha_grayindex(GET_INTENSITY_PARAMETERS),
_get_raw_alpha_truecolor(GET_INTENSITY_PARAMETERS),
_get_raw_alpha_palette(GET_INTENSITY_PARAMETERS),
_get_raw_alpha_grayscale(GET_INTENSITY_PARAMETERS),
_get_raw_alpha_grayindex(GET_INTENSITY_PARAMETERS),
_get_intensity_zero(GET_INTENSITY_PARAMETERS),
_get_alpha_opaque(GET_INTENSITY_PARAMETERS);

static get_intensity_func_t
_get_intensity_converter(const gdImagePtr im),
_get_alpha_converter(const gdImagePtr im, int raw_alpha);

static void
_mask_alpha_set(MASK_ALPHA_PARAMETERS),
_mask_alpha_set_not(MASK_ALPHA_PARAMETERS),
_mask_alpha_merge(MASK_ALPHA_PARAMETERS),
_mask_alpha_merge_not(MASK_ALPHA_PARAMETERS),
_mask_alpha_screen(MASK_ALPHA_PARAMETERS),
_mask_alpha_screen_not(MASK_ALPHA_PARAMETERS),
_mask_alpha_and(MASK_ALPHA_PARAMETERS),
_mask_alpha_and_not(MASK_ALPHA_PARAMETERS),
_mask_alpha_or(MASK_ALPHA_PARAMETERS),
_mask_alpha_or_not(MASK_ALPHA_PARAMETERS),
_mask_alpha_xor(MASK_ALPHA_PARAMETERS),
_mask_alpha_xor_not(MASK_ALPHA_PARAMETERS);

static grayscale_type_t
_is_grayscale(const gdImagePtr im);

static gdImagePtr
_create_grayscale_image(int width, int height);

static void
_channel_merge_rgb(gdImagePtr im,
                   const channel_t *rch,
                   const channel_t *gch,
                   const channel_t *bch,
                   const channel_t *ach);

static void
_channel_merge_3ch(gdImagePtr im,
                   const channel_t *ch1,
                   const channel_t *ch2,
                   const channel_t *ch3,
                   const channel_t *ach,
                   gdex_3ch_to_rgb_func_t cs_conv);

static void
_channel_merge_4ch(gdImagePtr im,
                   const channel_t *ch1,
                   const channel_t *ch2,
                   const channel_t *ch3,
                   const channel_t *ch4,
                   const channel_t *ach,
                   gdex_4ch_to_rgb_func_t cs_conv);

static void
_channel_extract_rgb(const gdImagePtr im,
                     gdImagePtr rch,
                     gdImagePtr gch,
                     gdImagePtr bch,
                     gdImagePtr ach,
                     int raw_alpha);

static void
_channel_extract_3ch(const gdImagePtr im,
                     gdImagePtr ch1,
                     gdImagePtr ch2,
                     gdImagePtr ch3,
                     gdImagePtr ach,
                     gdex_rgb_to_3ch_func_t cs_conv,
                     int raw_alpha);

static void
_channel_extract_4ch(const gdImagePtr im,
                     gdImagePtr ch1,
                     gdImagePtr ch2,
                     gdImagePtr ch3,
                     gdImagePtr ch4,
                     gdImagePtr ach,
                     gdex_rgb_to_4ch_func_t cs_conv,
                     int raw_alpha);

/* }}} */
/* {{{ functions to get intensity */
/* {{{ _get_intensity_truecolor() */

/*
 * Get intensity from a true color image.
 */
static int
_get_intensity_truecolor(GET_INTENSITY_PARAMETERS)
{
	GET_INTENSITY_PARAMS_ADJUST();
	if (gdImageBoundsSafe(ch->im, x, y)) {
		int c = unsafeGetTrueColorPixel(ch->im, x, y);
		return _rgb2gray(getR(c), getG(c), getB(c));
	} else {
		return oob;
	}
}

/* }}} */
/* {{{ _get_intensity_palette() */

/*
 * Get intensity from an index color image.
 */
static int
_get_intensity_palette(GET_INTENSITY_PARAMETERS)
{
	GET_INTENSITY_PARAMS_ADJUST();
	if (gdImageBoundsSafe(ch->im, x, y)) {
		unsigned char i = unsafeGetPalettePixel(ch->im, x, y);
		return _rgb2gray(paletteR(ch->im, i), paletteG(ch->im, i), paletteB(ch->im, i));
	} else {
		return oob;
	}
}

/* }}} */
/* {{{ _get_intensity_grayscale() */

/*
 * Get intensity from a gray scale image.
 */
static int
_get_intensity_grayscale(GET_INTENSITY_PARAMETERS)
{
	GET_INTENSITY_PARAMS_ADJUST();
	if (gdImageBoundsSafe(ch->im, x, y)) {
		return paletteG(ch->im, unsafeGetPalettePixel(ch->im, x, y));
	} else {
		return oob;
	}
}

/* }}} */
/* {{{ _get_intensity_grayindex() */

/*
 * Get intensity from a gray scale image (intensity == index).
 */
static int
_get_intensity_grayindex(GET_INTENSITY_PARAMETERS)
{
	GET_INTENSITY_PARAMS_ADJUST();
	if (gdImageBoundsSafe(ch->im, x, y)) {
		return unsafeGetPalettePixel(ch->im, x, y);
	} else {
		return oob;
	}
}

/* }}} */
/* {{{ _get_alpha_truecolor() */

/*
 * Get alpha channel value from a true color image.
 */
static int
_get_alpha_truecolor(GET_INTENSITY_PARAMETERS)
{
	GET_INTENSITY_PARAMS_ADJUST();
	if (gdImageBoundsSafe(ch->im, x, y)) {
		int c = unsafeGetTrueColorPixel(ch->im, x, y);
		return _rgb2alpha(getR(c), getG(c), getB(c));
	} else {
		return oob;
	}
}

/* }}} */
/* {{{ _get_alpha_palette() */

/*
 * Get alpha channel value from an index color image.
 */
static int
_get_alpha_palette(GET_INTENSITY_PARAMETERS)
{
	GET_INTENSITY_PARAMS_ADJUST();
	if (gdImageBoundsSafe(ch->im, x, y)) {
		unsigned char i = unsafeGetPalettePixel(ch->im, x, y);
		return _rgb2alpha(paletteR(ch->im, i), paletteG(ch->im, i), paletteB(ch->im, i));
	} else {
		return oob;
	}
}

/* }}} */
/* {{{ _get_alpha_grayscale() */

/*
 * Get alpha channel value from a gray scale image.
 */
static int
_get_alpha_grayscale(GET_INTENSITY_PARAMETERS)
{
	GET_INTENSITY_PARAMS_ADJUST();
	if (gdImageBoundsSafe(ch->im, x, y)) {
		return _gray2alpha(paletteG(ch->im, unsafeGetPalettePixel(ch->im, x, y)));
	} else {
		return oob;
	}
}

/* }}} */
/* {{{ _get_alpha_grayindex() */

/*
 * Get alpha channel value from a gray scale image (value == index).
 */
static int
_get_alpha_grayindex(GET_INTENSITY_PARAMETERS)
{
	GET_INTENSITY_PARAMS_ADJUST();
	if (gdImageBoundsSafe(ch->im, x, y)) {
		return _gray2alpha(unsafeGetPalettePixel(ch->im, x, y));
	} else {
		return oob;
	}
}

/* }}} */
/* {{{ _get_raw_alpha_truecolor() */

/*
 * Get raw alpha channel value from a true color image.
 */
static int
_get_raw_alpha_truecolor(GET_INTENSITY_PARAMETERS)
{
	int a = _get_intensity_truecolor(ch, x, y, oob);
	return MINMAX(a, 0, gdAlphaMax);
}

/* }}} */
/* {{{ _get_raw_alpha_palette() */

/*
 * Get raw alpha channel value from an index color image.
 */
static int
_get_raw_alpha_palette(GET_INTENSITY_PARAMETERS)
{
	int a = _get_intensity_palette(ch, x, y, oob);
	return MINMAX(a, 0, gdAlphaMax);
}

/* }}} */
/* {{{ _get_raw_alpha_grayscale() */

/*
 * Get raw alpha channel value from a gray scale image.
 */
static int
_get_raw_alpha_grayscale(GET_INTENSITY_PARAMETERS)
{
	int a = _get_intensity_grayscale(ch, x, y, oob);
	return MINMAX(a, 0, gdAlphaMax);
}

/* }}} */
/* {{{ _get_raw_alpha_grayindex() */

/*
 * Get raw alpha channel value from a gray scale image (value == index).
 */
static int
_get_raw_alpha_grayindex(GET_INTENSITY_PARAMETERS)
{
	int a = _get_intensity_grayindex(ch, x, y, oob);
	return MINMAX(a, 0, gdAlphaMax);
}

/* }}} */
/* {{{ _get_intensity_zero() */

/*
 * Always return 0.
 */
static int
_get_intensity_zero(GET_INTENSITY_PARAMETERS)
{
	return 0;
}

/* }}} */
/* {{{ _get_alpha_opaque() */

/*
 * Always return gdAlphaOpaque.
 */
static int
_get_alpha_opaque(GET_INTENSITY_PARAMETERS)
{
	return gdAlphaOpaque;
}

/* }}} */

#undef GET_INTENSITY_PARAMETERS

/* }}} */
/* {{{ _get_intensity_converter() */

/*
 * Get a pixel to intensity conversion function.
 */
static get_intensity_func_t
_get_intensity_converter(const gdImagePtr im)
{
	if (im == NULL) {
		return _get_intensity_zero;
	}
	if (gdImageTrueColor(im)) {
		return _get_intensity_truecolor;
	}
	switch (_is_grayscale(im)) {
		case GRAYSCALE_NONE:
			return _get_intensity_palette;
		case GRAYSCALE_B2W8:
			return _get_intensity_grayindex;
		default:
			return _get_intensity_grayscale;
	}
}

/* }}} */
/* {{{ _get_alpha_converter() */

/*
 * Get a pixel to alpha channel value conversion function.
 */
static get_intensity_func_t
_get_alpha_converter(const gdImagePtr im, int raw_alpha)
{
	if (im == NULL) {
		return _get_alpha_opaque;
	}
	if (raw_alpha) {
		if (gdImageTrueColor(im)) {
			return _get_raw_alpha_truecolor;
		}
		switch (_is_grayscale(im)) {
			case GRAYSCALE_NONE:
				return _get_raw_alpha_palette;
			case GRAYSCALE_B2W8:
				return _get_raw_alpha_grayindex;
			default:
				return _get_raw_alpha_grayscale;
		}
	} else {
		if (gdImageTrueColor(im)) {
			return _get_alpha_truecolor;
		}
		switch (_is_grayscale(im)) {
			case GRAYSCALE_NONE:
				return _get_alpha_palette;
			case GRAYSCALE_B2W8:
				return _get_alpha_grayindex;
			default:
				return _get_alpha_grayscale;
		}
	}
}

/* }}} */
/* {{{ alpha mask functions for each line */

#define SET_ALPHA(a) \
	unsafeSetTrueColorPixel(im, x, y, gdTrueColorAlpha(getR(c), getG(c), getB(c), (a)))

static inline int
_alpha_merge(int a1, int a2)
{
	int a0 = gdAlphaMax - a1;
	return gdAlphaMax - (a0 * a0 + (gdAlphaMax - a2) * a1) / gdAlphaMax;
}

static inline int
_alpha_screen(int a1, int a2)
{
	return gdAlphaMax - ((gdAlphaMax - a1) + (gdAlphaMax - a2) * a1 / gdAlphaMax);
}

/* {{{ _mask_alpha_set() */

/*
 * Set alpha channel.
 */
static void
_mask_alpha_set(MASK_ALPHA_PARAMETERS)
{
	int c;

	while (x < z) {
		c = unsafeGetTrueColorPixel(im, x, y);
		SET_ALPHA(ach->get(ach, x, y, gdAlphaTransparent));
		x++;
	}
}

/* }}} */
/* {{{ _mask_alpha_set_not() */

/*
 * Set and negate alpha channel.
 */
static void
_mask_alpha_set_not(MASK_ALPHA_PARAMETERS)
{
	int c;

	while (x < z) {
		c = unsafeGetTrueColorPixel(im, x, y);
		SET_ALPHA(gdAlphaMax & ~ach->get(ach, x, y, gdAlphaTransparent));
		x++;
	}
}

/* }}} */
/* {{{ _mask_alpha_merge() */

/*
 * Apply merge blending for alpha channel.
 */
static void
_mask_alpha_merge(MASK_ALPHA_PARAMETERS)
{
	int c;

	while (x < z) {
		c = unsafeGetTrueColorPixel(im, x, y);
		SET_ALPHA(_alpha_merge(ach->get(ach, x, y, gdAlphaTransparent), getA(c)));
		x++;
	}
}

/* }}} */
/* {{{ _mask_alpha_merge_not() */

/*
 * Apply merge blending and negate alpha channel.
 */
static void
_mask_alpha_merge_not(MASK_ALPHA_PARAMETERS)
{
	int c;

	while (x < z) {
		c = unsafeGetTrueColorPixel(im, x, y);
		SET_ALPHA(gdAlphaMax & ~_alpha_merge(ach->get(ach, x, y, gdAlphaTransparent), getA(c)));
		x++;
	}
}

/* }}} */
/* {{{ _mask_alpha_screen() */

/*
 * Apply screen blending for alpha channel.
 */
static void
_mask_alpha_screen(MASK_ALPHA_PARAMETERS)
{
	int c;

	while (x < z) {
		c = unsafeGetTrueColorPixel(im, x, y);
		SET_ALPHA(_alpha_screen(ach->get(ach, x, y, gdAlphaTransparent), getA(c)));
		x++;
	}
}

/* }}} */
/* {{{ _mask_alpha_screen_not() */

/*
 * Apply screen blending and negate alpha channel.
 */
static void
_mask_alpha_screen_not(MASK_ALPHA_PARAMETERS)
{
	int c;

	while (x < z) {
		c = unsafeGetTrueColorPixel(im, x, y);
		SET_ALPHA(gdAlphaMax & ~_alpha_screen(ach->get(ach, x, y, gdAlphaTransparent), getA(c)));
		x++;
	}
}

/* }}} */
/* {{{ _mask_alpha_and() */

/*
 * AND operation for alpha channel.
 * GD's alpha channel value is in range [0..0x7f],
 * '0' means opaque and '0x7f' means transparent.
 * This is a reason for using OR operator.
 */
static void
_mask_alpha_and(MASK_ALPHA_PARAMETERS)
{
	int c;

	while (x < z) {
		c = unsafeGetTrueColorPixel(im, x, y);
		SET_ALPHA(getA(c) | ach->get(ach, x, y, gdAlphaTransparent));
		x++;
	}
}

/* }}} */
/* {{{ _mask_alpha_and_not() */

/*
 * AND + NOT operation for alpha channel.
 */
static void
_mask_alpha_and_not(MASK_ALPHA_PARAMETERS)
{
	int c;

	while (x < z) {
		c = unsafeGetTrueColorPixel(im, x, y);
		SET_ALPHA(gdAlphaMax & ~(getA(c) | ach->get(ach, x, y, gdAlphaTransparent)));
		x++;
	}
}

/* }}} */
/* {{{ _mask_alpha_or() */

/*
 * OR operation for alpha channel.
 * GD's alpha channel value is in range [0..0x7f],
 * '0' means opaque and '0x7f' means transparent.
 * This is a reason for using AND operator.
 */
static void
_mask_alpha_or(MASK_ALPHA_PARAMETERS)
{
	int c;

	while (x < z) {
		c = unsafeGetTrueColorPixel(im, x, y);
		SET_ALPHA(getA(c) & ach->get(ach, x, y, gdAlphaTransparent));
		x++;
	}
}

/* }}} */
/* {{{ _mask_alpha_or_not() */

/*
 * OR + NOT operation for alpha channel.
 */
static void
_mask_alpha_or_not(MASK_ALPHA_PARAMETERS)
{
	int c;

	while (x < z) {
		c = unsafeGetTrueColorPixel(im, x, y);
		SET_ALPHA(gdAlphaMax & ~(getA(c) & ach->get(ach, x, y, gdAlphaTransparent)));
		x++;
	}
}

/* }}} */
/* {{{ _mask_alpha_xor() */

/*
 * XOR operation for alpha channel.
 */
static void
_mask_alpha_xor(MASK_ALPHA_PARAMETERS)
{
	int c;

	while (x < z) {
		c = unsafeGetTrueColorPixel(im, x, y);
		SET_ALPHA(getA(c) ^ ach->get(ach, x, y, gdAlphaTransparent));
		x++;
	}
}

/* }}} */
/* {{{ _mask_alpha_xor_not() */

/*
 * XOR + NOT operation for alpha channel.
 */
static void
_mask_alpha_xor_not(MASK_ALPHA_PARAMETERS)
{
	int c;

	while (x < z) {
		c = unsafeGetTrueColorPixel(im, x, y);
		SET_ALPHA(gdAlphaMax & ~(getA(c) ^ ach->get(ach, x, y, gdAlphaTransparent)));
		x++;
	}
}

/* }}} */

#undef MASK_ALPHA_PARAMETERS
#undef SET_ALPHA

/* }}} */
/* {{{ _is_grayscale() */

/*
 * Check whether the image is an 8-bit gray scale image.
 */
static grayscale_type_t
_is_grayscale(const gdImagePtr im)
{
	/* index, jump, key, length, red, green, blue */
	int i, j, k, l, r, g, b;

	if (gdImageTrueColor(im)) {
		return GRAYSCALE_NONE;
	}

	l = gdImageColorsTotal(im);
	if (l == 256) {
		j = 1;
	} else if (l == 16) {
		j = 17;
	} else {
		return GRAYSCALE_NONE;
	}

	k = paletteR(im, 0);
	if (k == 0) {
		for (i = 0; i < l; i++) {
			r = paletteR(im, i);
			g = paletteG(im, i);
			b = paletteB(im, i);
			if (r != k || g != k || b != k) {
				return GRAYSCALE_NONE;
			}
			k += j;
		}
		return (l == 16) ? GRAYSCALE_B2W4 : GRAYSCALE_B2W8;
	} else if (k == 255) {
		for (i = 0; i < l; i++) {
			r = paletteR(im, i);
			g = paletteG(im, i);
			b = paletteB(im, i);
			if (r != k || g != k || b != k) {
				return GRAYSCALE_NONE;
			}
			k -= j;
		}
		return (l == 16) ? GRAYSCALE_W2B4 : GRAYSCALE_W2B8;
	} else {
		return GRAYSCALE_NONE;
	}
}

/* }}} */
/* {{{ _create_grayscale_image() */

/*
 * Create a blank gray scale image.
 */
static gdImagePtr
_create_grayscale_image(int width, int height)
{
	gdImagePtr im;
	int idx;

	im = gdImageCreate(width, height);
	if (im == NULL) {
		return NULL;
	}

	for (idx = 0; idx < 256; idx++) {
		im->red[idx]   = idx;
		im->green[idx] = idx;
		im->blue[idx]  = idx;
		im->alpha[idx] = gdAlphaOpaque;
		im->open[idx]  = 0;
	}
	im->colorsTotal = 256;

	return im;
}

/* }}} */
/* {{{ _channel_merge_rgb() */

/*
 * Merge RGB/RGBA channels.
 */
static void
_channel_merge_rgb(gdImagePtr im,
                   const channel_t *rch,
                   const channel_t *gch,
                   const channel_t *bch,
                   const channel_t *ach)
{
	int x, y, width, height;

	width = gdImageSX(im);
	height = gdImageSY(im);

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			unsafeSetTrueColorPixel(im, x, y, gdTrueColorAlpha(
					rch->get(rch, x, y, 0),
					gch->get(gch, x, y, 0),
					bch->get(bch, x, y, 0),
					ach->get(ach, x, y, gdAlphaTransparent)));
		}
	}
}

/* }}} */
/* {{{ _channel_merge_3ch() */

/*
 * Merge 3+alpha channels.
 */
static void
_channel_merge_3ch(gdImagePtr im,
                   const channel_t *ch1,
                   const channel_t *ch2,
                   const channel_t *ch3,
                   const channel_t *ach,
                   gdex_3ch_to_rgb_func_t cs_conv)
{
	int x, y, width, height;
	int r, g, b;

	width = gdImageSX(im);
	height = gdImageSY(im);

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			cs_conv((float)ch1->get(ch1, x, y, 0) / 255.0f,
					(float)ch2->get(ch2, x, y, 0) / 255.0f,
					(float)ch3->get(ch3, x, y, 0) / 255.0f,
					&r, &g, &b);
			unsafeSetTrueColorPixel(im, x, y, gdTrueColorAlpha(
					r, g, b, ach->get(ach, x, y, gdAlphaTransparent)));
		}
	}
}

/* }}} */
/* {{{ _channel_merge_4ch() */

/*
 * Merge 4+alpha channels.
 */
static void
_channel_merge_4ch(gdImagePtr im,
                    const channel_t *ch1,
                    const channel_t *ch2,
                    const channel_t *ch3,
                    const channel_t *ch4,
                    const channel_t *ach,
                    gdex_4ch_to_rgb_func_t cs_conv)
{
	int x, y, width, height;
	int r, g, b;

	width = gdImageSX(im);
	height = gdImageSY(im);

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			cs_conv((float)ch1->get(ch1, x, y, 0) / 255.0f,
					(float)ch2->get(ch2, x, y, 0) / 255.0f,
					(float)ch3->get(ch3, x, y, 0) / 255.0f,
					(float)ch4->get(ch4, x, y, 0) / 255.0f,
					&r, &g, &b);
			unsafeSetTrueColorPixel(im, x, y, gdTrueColorAlpha(
					r, g, b, ach->get(ach, x, y, gdAlphaTransparent)));
		}
	}
}

/* }}} */
/* {{{ _channel_extract_rgb() */

/*
 * Extract RGB/RGBA channels.
 */
static void
_channel_extract_rgb(const gdImagePtr im,
                     gdImagePtr rch,
                     gdImagePtr gch,
                     gdImagePtr bch,
                     gdImagePtr ach,
                     int raw_alpha)
{
	int x, y, width, height;
	int a;

	width = gdImageSX(im);
	height = gdImageSY(im);

	if (gdImageTrueColor(im)) {
		int c;

		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				c = unsafeGetTrueColorPixel(im, x, y);
				unsafeSetPalettePixel(rch, x, y, getR(c));
				unsafeSetPalettePixel(gch, x, y, getG(c));
				unsafeSetPalettePixel(bch, x, y, getB(c));
				if (ach != NULL) {
					if (raw_alpha) {
						a = getA(c);
					} else {
						a = _alpha2gray(getA(c));
					}
					unsafeSetPalettePixel(ach, x, y, a);
				}
			}
		}
	} else {
		unsigned char i;
		int transparent;

		transparent = gdImageGetTransparent(im);

		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				i = unsafeGetPalettePixel(im, x, y);
				unsafeSetPalettePixel(rch, x, y, paletteR(im, i));
				unsafeSetPalettePixel(gch, x, y, paletteG(im, i));
				unsafeSetPalettePixel(bch, x, y, paletteB(im, i));
				if (ach != NULL) {
					if (i == transparent) {
						a = (raw_alpha) ? gdAlphaTransparent : 0;
					} else if (raw_alpha) {
						a = paletteA(im, i);
					} else {
						a = _alpha2gray(paletteA(im, i));
					}
					unsafeSetPalettePixel(ach, x, y, a);
				}
			}
		}
	}
}

/* }}} */
/* {{{ _channel_extract_3ch() */

/*
 * Extract 3+alpha channels.
 */
static void
_channel_extract_3ch(const gdImagePtr im,
                     gdImagePtr ch1,
                     gdImagePtr ch2,
                     gdImagePtr ch3,
                     gdImagePtr ach,
                     gdex_rgb_to_3ch_func_t cs_conv,
                     int raw_alpha)
{
	int x, y, width, height;
	int a;
	float f1, f2, f3;

	width = gdImageSX(im);
	height = gdImageSY(im);

	if (gdImageTrueColor(im)) {
		int c;

		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				c = unsafeGetTrueColorPixel(im, x, y);
				cs_conv(getR(c), getG(c), getB(c), &f1, &f2, &f3);
				unsafeSetPalettePixel(ch1, x, y, _float2byte(f1));
				unsafeSetPalettePixel(ch2, x, y, _float2byte(f2));
				unsafeSetPalettePixel(ch3, x, y, _float2byte(f3));
				if (ach != NULL) {
					if (raw_alpha) {
						a = getA(c);
					} else {
						a = _alpha2gray(getA(c));
					}
					unsafeSetPalettePixel(ach, x, y, a);
				}
			}
		}
	} else {
		unsigned char i;
		int transparent;

		transparent = gdImageGetTransparent(im);

		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				i = unsafeGetPalettePixel(im, x, y);
				cs_conv(paletteR(im, i), paletteG(im, i), paletteB(im, i), &f1, &f2, &f3);
				unsafeSetPalettePixel(ch1, x, y, _float2byte(f1));
				unsafeSetPalettePixel(ch2, x, y, _float2byte(f2));
				unsafeSetPalettePixel(ch3, x, y, _float2byte(f3));
				if (ach != NULL) {
					if (i == transparent) {
						a = (raw_alpha) ? gdAlphaTransparent : 0;
					} else if (raw_alpha) {
						a = paletteA(im, i);
					} else {
						a = _alpha2gray(paletteA(im, i));
					}
					unsafeSetPalettePixel(ach, x, y, a);
				}
			}
		}
	}
}

/* }}} */
/* {{{ _channel_extract_4ch() */

/*
 * Extract 4+alpha channels.
 */
static void
_channel_extract_4ch(const gdImagePtr im,
                     gdImagePtr ch1,
                     gdImagePtr ch2,
                     gdImagePtr ch3,
                     gdImagePtr ch4,
                     gdImagePtr ach,
                     gdex_rgb_to_4ch_func_t cs_conv,
                     int raw_alpha)
{
	int x, y, width, height;
	int a;
	float f1, f2, f3, f4;

	width = gdImageSX(im);
	height = gdImageSY(im);

	if (gdImageTrueColor(im)) {
		int c;

		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				c = unsafeGetTrueColorPixel(im, x, y);
				cs_conv(getR(c), getG(c), getB(c), &f1, &f2, &f3, &f4);
				unsafeSetPalettePixel(ch1, x, y, _float2byte(f1));
				unsafeSetPalettePixel(ch2, x, y, _float2byte(f2));
				unsafeSetPalettePixel(ch3, x, y, _float2byte(f3));
				unsafeSetPalettePixel(ch4, x, y, _float2byte(f4));
				if (ach != NULL) {
					if (raw_alpha) {
						a = getA(c);
					} else {
						a = _alpha2gray(getA(c));
					}
					unsafeSetPalettePixel(ach, x, y, a);
				}
			}
		}
	} else {
		unsigned char i;
		int transparent;

		transparent = gdImageGetTransparent(im);

		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				i = unsafeGetPalettePixel(im, x, y);
				cs_conv(paletteR(im, i), paletteG(im, i), paletteB(im, i), &f1, &f2, &f3, &f4);
				unsafeSetPalettePixel(ch1, x, y, _float2byte(f1));
				unsafeSetPalettePixel(ch2, x, y, _float2byte(f2));
				unsafeSetPalettePixel(ch3, x, y, _float2byte(f3));
				unsafeSetPalettePixel(ch4, x, y, _float2byte(f4));
				if (ach != NULL) {
					if (i == transparent) {
						a = (raw_alpha) ? gdAlphaTransparent : 0;
					} else if (raw_alpha) {
						a = paletteA(im, i);
					} else {
						a = _alpha2gray(paletteA(im, i));
					}
					unsafeSetPalettePixel(ach, x, y, a);
				}
			}
		}
	}
}

/* }}} */
/* {{{ resource imagechannelmerge_ex(array channels
                                     [, int colorspace[, int position]]) */

GDEXTRA_LOCAL PHP_FUNCTION(imagechannelmerge_ex)
{
	zval *zchannels = NULL;
	HashTable *channels_ht;
	HashPosition pos;
	zval **entry = NULL;
	channel_t ch[MAX_CHANNELS];
	int i, n;
	long orig_colorspace = COLORSPACE_RGB;
	int colorspace;
	long position = POSITION_DEFAULT;
	int crop = 0;
	int use_alpha = 0;
	int raw_alpha = 0;
	gdImagePtr im;
	int width, height;

	/* parse the arguments */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a|ll",
			&zchannels, &orig_colorspace, &position) == FAILURE)
	{
		return;
	}

	/* verify the color space */
	if (orig_colorspace & COLORSPACE_ALPHA) {
		use_alpha = 1;
		if (orig_colorspace & COLORSPACE_RAW) {
			raw_alpha = 1;
		}
	}
	colorspace = (int)(orig_colorspace ^ COLORSPACE_RAW_ALPHA);
	if (colorspace != COLORSPACE_RGB &&
		colorspace != COLORSPACE_HSV &&
		colorspace != COLORSPACE_HSL &&
		colorspace != COLORSPACE_CMYK)
	{
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Unsupported color space given (%ld)", orig_colorspace);
		RETURN_FALSE;
	}

	/* fetch the channels */
	memset(ch, 0, sizeof(ch));
	if (colorspace == COLORSPACE_CMYK) {
		n = (use_alpha) ? 5 : 4;
	} else {
		n = (use_alpha) ? 4 : 3;
	}
	channels_ht = Z_ARRVAL_P(zchannels);
	if (zend_hash_num_elements(channels_ht) < n) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Number of the channels is not enough for %s color space,"
					" %d channels required but %d channels given",
				gdex_get_colorspace_name((int)orig_colorspace),
				n, zend_hash_num_elements(channels_ht));
		RETURN_FALSE;
	}
	if (use_alpha) {
		zend_hash_internal_pointer_end_ex(channels_ht, &pos);
		zend_hash_get_current_data_ex(channels_ht, (void **)&entry, &pos);
		ZEND_FETCH_RESOURCE(ch[0].im, gdImagePtr, entry, -1, "Image", GDEXG(le_gd));
		n--;
	} else {
		ch[0].im = NULL;
	}
	zend_hash_internal_pointer_reset_ex(channels_ht, &pos);
	for (i = 0; i < n; i++) {
		zend_hash_get_current_data_ex(channels_ht, (void **)&entry, &pos);
		ZEND_FETCH_RESOURCE(ch[i + 1].im, gdImagePtr, entry, -1, "Image", GDEXG(le_gd));
		zend_hash_move_forward_ex(channels_ht, &pos);
	}

	/* setup parmeters */
	crop = (position & MERGE_CROP) ? 1 : 0;
	if (ch[0].im == NULL) {
		ch[0].get = _get_alpha_opaque;
		width = 0;
		height = 0;
	} else {
		ch[0].get = _get_alpha_converter(ch[0].im, raw_alpha);
		width = ch[0].width = gdImageSX(ch[0].im);
		height = ch[0].height = gdImageSY(ch[0].im);
	}
	for (i = 1; i <= n; i++) {
		ch[i].get = _get_intensity_converter(ch[i].im);
		ch[i].width = gdImageSX(ch[i].im);
		ch[i].height = gdImageSY(ch[i].im);
		if (crop) {
			if (width > ch[i].width || width == 0) {
				width = ch[i].width;
			}
			if (height > ch[i].height || height == 0) {
				height = ch[i].height;
			}
		} else {
			if (width < ch[i].width) {
				width = ch[i].width;
			}
			if (height < ch[i].height) {
				height = ch[i].height;
			}
		}
	}

	/* determine offsets */
	for (i = 0; i <= n; i++) {
		if (width != ch[i].width) {
			ch[i].xOffset = calc_x_offset(width, ch[i].width, position);
		}
		if (height != ch[i].height) {
			ch[i].yOffset = calc_y_offset(height, ch[i].height, position);
		}
	}

	/* create a new image */
	im = gdImageCreateTrueColor(width, height);
	if (im == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot create a new image");
		RETURN_FALSE;
	}

	/* merge channels */
	switch (colorspace) {
		case COLORSPACE_RGB:
			_channel_merge_rgb(im, &ch[1], &ch[2], &ch[3], &ch[0]);
			break;
		case COLORSPACE_HSV:
			_channel_merge_3ch(im, &ch[1], &ch[2], &ch[3], &ch[0], gdex_hsv_to_rgb);
			break;
		case COLORSPACE_HSL:
			_channel_merge_3ch(im, &ch[1], &ch[2], &ch[3], &ch[0], gdex_hsl_to_rgb);
			break;
		case COLORSPACE_CMYK:
			_channel_merge_4ch(im, &ch[1], &ch[2], &ch[3], &ch[4], &ch[0], gdex_cmyk_to_rgb);
			break;
	}
	if (use_alpha) {
		gdImageSaveAlpha(im, 1);
	}

	/* register the image to the return value */
	ZEND_REGISTER_RESOURCE(return_value, im, GDEXG(le_gd));
}

/* }}} */
/* {{{ array imagechannelextract_ex(resource im[, int colorspace]) */

GDEXTRA_LOCAL PHP_FUNCTION(imagechannelextract_ex)
{
	zval *zim, *zch;
	gdImagePtr im, ch[MAX_CHANNELS];
	long orig_colorspace = COLORSPACE_RGB;
	int colorspace;
	int use_alpha = 0;
	int raw_alpha = 0;
	int i, width, height;
	int errid = -1;

	/* parse the arguments */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|l",
			&zim, &orig_colorspace) == FAILURE)
	{
		return;
	}
	ZEND_FETCH_RESOURCE(im, gdImagePtr, &zim, -1, "Image", GDEXG(le_gd));

	/* verify the color space */
	if (orig_colorspace & COLORSPACE_ALPHA) {
		use_alpha = 1;
		/*if (orig_colorspace & COLORSPACE_RAW) {
			raw_alpha = 1;
		}*/
	}
	colorspace = (int)(orig_colorspace ^ COLORSPACE_RAW_ALPHA);
	if (colorspace != COLORSPACE_RGB &&
		colorspace != COLORSPACE_HSV &&
		colorspace != COLORSPACE_HSL &&
		colorspace != COLORSPACE_CMYK)
	{
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Unsupported color space given (%ld)", orig_colorspace);
		RETURN_FALSE;
	}

	/* create each channel */
	memset(ch, 0, sizeof(ch));
	width = gdImageSX(im);
	height = gdImageSY(im);

	for (i = 1; i <= 3; i++) {
		ch[i] = _create_grayscale_image(width, height);
		if (ch[i] == NULL) {
			errid = i;
			goto failure;
		}
	}

	if (colorspace == COLORSPACE_CMYK) {
		ch[4] = _create_grayscale_image(width, height);
		if (ch[4] == NULL) {
			errid = 4;
			goto failure;
		}
	}

	if (use_alpha) {
		ch[0] = _create_grayscale_image(width, height);
		if (ch[0] == NULL) {
			errid = 0;
			goto failure;
		}
	}

	/* extract channels */
	switch (colorspace) {
		case COLORSPACE_RGB:
			_channel_extract_rgb(im, ch[1], ch[2], ch[3], ch[0], raw_alpha);
			break;
		case COLORSPACE_HSV:
			_channel_extract_3ch(im, ch[1], ch[2], ch[3], ch[0], gdex_rgb_to_hsv, raw_alpha);
			break;
		case COLORSPACE_HSL:
			_channel_extract_3ch(im, ch[1], ch[2], ch[3], ch[0], gdex_rgb_to_hsl, raw_alpha);
			break;
		case COLORSPACE_CMYK:
			_channel_extract_4ch(im, ch[1], ch[2], ch[3], ch[4], ch[0], gdex_rgb_to_cmyk, raw_alpha);
			break;
	}

	/* return new image resources */
	array_init_size(return_value, use_alpha ? 4 : 8);
	for (i = 1; i <= 3; i++) {
		MAKE_STD_ZVAL(zch);
		ZEND_REGISTER_RESOURCE(zch, ch[i], GDEXG(le_gd));
		add_next_index_zval(return_value, zch);
	}
	if (ch[4] != NULL) {
		MAKE_STD_ZVAL(zch);
		ZEND_REGISTER_RESOURCE(zch, ch[4], GDEXG(le_gd));
		add_next_index_zval(return_value, zch);
	}
	if (ch[0] != NULL) {
		MAKE_STD_ZVAL(zch);
		ZEND_REGISTER_RESOURCE(zch, ch[0], GDEXG(le_gd));
		add_next_index_zval(return_value, zch);
	}
	return;

  failure:
	/* failure */
	for (i = 0; i < MAX_CHANNELS; i++) {
		if (ch[i] != NULL) {
			gdImageDestroy(ch[i]);
		}
	}

	php_error_docref(NULL TSRMLS_CC, E_WARNING,
			"Cannot create a channel image #%d", errid);
	RETURN_FALSE;
}

/* }}} */
/* {{{ bool imagealphamask_ex(resource im, resource mask
                              [, int mode[, int position]]) */

/*
 * Apply the mask to the image's alpha channel.
 */
GDEXTRA_LOCAL PHP_FUNCTION(imagealphamask_ex)
{
	zval *zim = NULL, *zmask = NULL;
	gdImagePtr im, mask;
	long orig_mode = MASK_SET;
	long position = POSITION_DEFAULT;
	int x, y, width, height;
	int mode, notm, tile, raw_alpha;
	mask_alpha_func_t mask_alpha;
	channel_t ach;

	/* parse the arguments */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rr|ll",
			&zim, &zmask, &orig_mode, &position) == FAILURE)
	{
		return;
	}
	ZEND_FETCH_RESOURCE(im, gdImagePtr, &zim, -1, "Image", GDEXG(le_gd));
	ZEND_FETCH_RESOURCE(mask, gdImagePtr, &zmask, -1, "Image", GDEXG(le_gd));

	/* verify the mask mode */
	notm = (orig_mode & MASK_NOT)  ? 1 : 0;
	tile = (orig_mode & MASK_TILE) ? 1 : 0;
	mode = (int)(orig_mode & ~(MASK_NOT | MASK_TILE | COLORSPACE_RAW_ALPHA));
	raw_alpha = (orig_mode & COLORSPACE_RAW_ALPHA) ? 1 : 0;
	switch (mode) {
		case MASK_SET:
			mask_alpha = (notm) ? _mask_alpha_set_not : _mask_alpha_set;
			break;
		case MASK_MERGE:
			mask_alpha = (notm) ? _mask_alpha_merge_not : _mask_alpha_merge;
			break;
		case MASK_SCREEN:
			mask_alpha = (notm) ? _mask_alpha_screen_not : _mask_alpha_screen;
			break;
		case MASK_AND:
			mask_alpha = (notm) ? _mask_alpha_and_not : _mask_alpha_and;
			break;
		case MASK_OR:
			mask_alpha = (notm) ? _mask_alpha_or_not : _mask_alpha_or;
			break;
		case MASK_XOR:
			mask_alpha = (notm) ? _mask_alpha_xor_not : _mask_alpha_xor;
			break;
		default:
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"Unsupported mask mode given (%ld)", orig_mode);
			RETURN_FALSE;
	}

	/* convert to true color */
	if (gdex_palette_to_truecolor(im TSRMLS_CC) == FAILURE) {
		RETURN_FALSE;
	}
	gdImageSaveAlpha(im, 1);

	/* setup parmeters */
	width = gdImageSX(im);
	height = gdImageSY(im);
	ach.im = mask;
	ach.get = _get_alpha_converter(mask, raw_alpha);
	ach.width = gdImageSX(mask);
	ach.height = gdImageSY(mask);
	ach.xOffset = calc_x_offset(width, ach.width, position);
	ach.yOffset = calc_y_offset(height, ach.height, position);

	/* apply the mask */
	if (tile) {
		int x0, x1, x2, y0, y1, y2;

		if (width > ach.width) {
			if (position & POSITION_RIGHT) {
				x0 = (width % ach.width) - ach.width;
			} else if (position & POSITION_CENTER) {
				x0 = (width % ach.width) / 2 - ach.width;
			} else {
				x0 = 0;
			}
			x1 = x0 + ach.width;
		} else {
			x0 = ach.xOffset;
			x1 = width;
		}

		if (height > ach.height) {
			if (position & POSITION_BOTTOM) {
				y0 = (height % ach.height) - ach.height;
			} else if (position & POSITION_MIDDLE) {
				y0 = (height % ach.height) / 2 - ach.height;
			} else {
				y0 = 0;
			}
			y1 = y0 + ach.width;
		} else {
			y0 = ach.yOffset;
			y1 = height;
		}

		ach.yOffset = y0;
		for (y = 0; y < y1; y++) {
			ach.xOffset = x0;
			mask_alpha(im, &ach, 0, y, x1);
			x = x1;
			while (x < width) {
				ach.xOffset += ach.width;
				x2 = x + ach.width;
				mask_alpha(im, &ach, x, y, MIN(x2, width));
				x = x2;
			}
		}

		y2 = 0;
		for (y = y1; y < height; y++) {
			if (y2 % ach.height == 0) {
				ach.yOffset += ach.height;
			}
			ach.xOffset = x0;
			mask_alpha(im, &ach, 0, y, x1);
			x = x1;
			while (x < width) {
				ach.xOffset += ach.width;
				x2 = x + ach.width;
				mask_alpha(im, &ach, x, y, MIN(x2, width));
				x = x2;
			}
			y2++;
		}
	} else {
		for (y = 0; y < height; y++) {
			mask_alpha(im, &ach, 0, y, width);
		}
	}

	RETURN_TRUE;
}

/* }}} */
/* {{{ array imagehistgram_ex(resource im[, int colorspace]) */

GDEXTRA_LOCAL PHP_FUNCTION(imagehistgram_ex)
{
	zval *extracted, *tmp, **entry;
	HashTable *channels;
	HashPosition pos;

	if (ZEND_NUM_ARGS() > 2) {
		WRONG_PARAM_COUNT;
	}

	MAKE_STD_ZVAL(extracted);
	tmp = return_value;
	return_value = extracted;

	PHP_FN(imagechannelextract_ex)(INTERNAL_FUNCTION_PARAM_PASSTHRU);

	extracted = return_value;
	return_value = tmp;

	if (Z_TYPE_P(extracted) != IS_ARRAY) {
		zval_ptr_dtor(&extracted);
		RETURN_FALSE;
	}

	channels = Z_ARRVAL_P(extracted);
	array_init_size(return_value, zend_hash_num_elements(channels));

	zend_hash_internal_pointer_reset(channels);
	while (zend_hash_get_current_data(channels, (void **)&entry) == SUCCESS) {
		gdImagePtr im;
		int x, y, width, height;
		double pixels;
		zval *ch;
		unsigned int i, counts[256];

		ZEND_FETCH_RESOURCE(im, gdImagePtr, entry, -1, "Image", GDEXG(le_gd));
		if (im == NULL || im->trueColor || im->colorsTotal != 256) {
			zval_ptr_dtor(&extracted);
			zval_dtor(return_value);
			php_error_docref(NULL TSRMLS_CC, E_ERROR,
					"Failed to extract channels");
			RETURN_FALSE;
		}

		width = gdImageSX(im);
		height = gdImageSY(im);
		pixels = (double)width * (double)height;

		memset(counts, 0, sizeof(counts));
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				counts[unsafeGetPalettePixel(im, x, y)]++;
			}
		}

		MAKE_STD_ZVAL(ch);
		array_init_size(ch, 256);

		for (i = 0; i < 256; i++) {
			add_index_double(ch, (ulong)i, (double)counts[i] / pixels);
		}

		add_next_index_zval(return_value, ch);

		zend_hash_move_forward(channels);
	}

	zval_ptr_dtor(&extracted);
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
