/*
 * Extra image functions: common inline functions
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

#ifndef _PHP_GDEXTRA_INLINE_H_
#define _PHP_GDEXTRA_INLINE_H_

#include "php_gdextra.h"

BEGIN_EXTERN_C()

/* {{{ pixel reader/writer inline functions */

/*
 * Almost equivalent to `gdImagePalettePixel(im, x, y)`.
 */
static inline unsigned char
unsafeGetPalettePixel(const gdImagePtr im, int x, int y)
{
	return im->pixels[y][x];
}

/*
 * Almost equivalent to `gdImagePalettePixel(im, x, y) = index`.
 */
static inline void
unsafeSetPalettePixel(gdImagePtr im, int x, int y, unsigned char index)
{
	im->pixels[y][x] = index;
}

/*
 * Almost equivalent to `gdImageTrueColorPixel(im, x, y)`.
 */
static inline int
unsafeGetTrueColorPixel(const gdImagePtr im, int x, int y)
{
	return im->tpixels[y][x];
}

/*
 * Almost equivalent to `gdImageTrueColorPixel(im, x, y) = color`.
 */
static inline void
unsafeSetTrueColorPixel(gdImagePtr im, int x, int y, int color)
{
	im->tpixels[y][x] = color;
}

/* }}} */

/* {{{ color component conversion inline functions */

/*
 * Correct gamma based on ITU-R BT. 709 recommendation.
 */
static inline double
_rec709gamma(double value)
{
	if (value < 0.018) {
		return value * 4.5;
	} else {
		return 1.099 * pow(value, 0.45) - 0.099;
	}
}

/*
 * Single precision version of _rec709gamma().
 */
static inline float
_rec709gammaf(float value)
{
	if (value < 0.018f) {
		return value * 4.5f;
	} else {
		return 1.099f * powf(value, 0.45f) - 0.099f;
	}
}

/*
 * Inverse function of _rec709gamma().
 */
static inline double
_irec709gamma(double value)
{
	if (value < 0.0812) {
		return value / 4.5;
	} else {
		return pow((value + 0.099) / 1.099, 1.0 / 0.45);
	}
}

/*
 * Single precision version of _irec709gamma().
 */
static inline float
_irec709gammaf(float value)
{
	if (value < 0.0812f) {
		return value / 4.5f;
	} else {
		return powf((value + 0.099f) / 1.099f, 1.0f / 0.45f);
	}
}

/*
 * Convert float [0..1] to integer [0..255].
 */
static inline int
_float2byte(float value)
{
	int v = (int)(value * 255.5f);
	return MINMAX(v, 0, 255);
}
/*
#define _FLOAT2BYTE(_val) (int)(MINMAX((_val), 0.0f, 1.0f) * 255.5f)
*/

/*
 * Convert float [0..1] to integer [gdAlphaTransparent..gdAlphaOpaque].
 * 'gdAlphaTransparent' is 127 and 'gdAlphaOpaque' is 0.
 */
static inline int
_float2alpha(float value)
{
	int v = (int)((1.0f - value) * 127.5f);
	return MINMAX(v, 0, gdAlphaMax);
}

/*
 * Convert GD's alpha channel value to 8-bit gray scale.
 */
static inline int
_alpha2gray(int a)
{
	if (a <= 0) {
		return 255;
	} else if (a >= 127) {
		return 0;
	} else {
		return 255 - a * 2;
	}
}

/*
 * Convert 8-bit gray scale to GD's alpha channel value.
 */
static inline int
_gray2alpha(int a)
{
	return 127 - a / 2;
}

/*
 * Convert 8-bit RGB to gray scale (calculate intensity).
 */
static inline int
_rgb2gray(int r, int g, int b)
{
	return (int)((float)r * 0.299f + (float)g * 0.587f + (float)b * 0.114f);
}

/*
 * Convert 8-bit RGB to GD's alpha channel value.
 */
static inline int
_rgb2alpha(int r, int g, int b)
{
	return _gray2alpha(_rgb2gray(r, g, b));
}

/* }}} */

END_EXTERN_C()

#endif /* _PHP_GDEXTRA_INLINE_H_ */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
