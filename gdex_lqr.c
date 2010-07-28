/*
 * Extra image functions: liquid rescale functions
 *
 * Copyright (c) 2007-2009 Ryusuke SEKIYAMA. All rights reserved.
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
 * @package     php_gdextra
 * @author      Ryusuke SEKIYAMA <rsky0711@gmail.com>
 * @copyright   2007-2009 Ryusuke SEKIYAMA
 * @license     http://www.opensource.org/licenses/mit-license.php  MIT License
 * @version     SVN: $Id$
 */

#include "php_gdextra.h"
#include <lqr.h>

/* {{{ private function prototypes */

static const char *
_lqrerrstr(LqrRetVal ret);

static int
_get_lqr_options(HashTable *options, gint *max_step, gfloat *rigidity,
                 int *vertical_seam, int *save_alpha TSRMLS_DC);

static LqrCarver *
_gdimage_to_lqrcaver(const gdImagePtr im, int save_alpha);

static gdImagePtr
_lqrcarver_to_gdimage(LqrCarver *carver);

/* }}} */

/* {{{ _lqrerrstr()
 * Lookup error codes
 */
static const char *
_lqrerrstr(LqrRetVal ret)
{
	switch (ret) {
		case LQR_ERROR:
			return "generic error";
		case LQR_OK:
			return "ok";
		case LQR_NOMEM:
			return "not enough memory";
		case LQR_USRCANCEL:
			return "action cancelled by user";
		default:
			return "unknown error";
	}
}
/* }}} */

/* {{{ _get_lqr_options
 * Get liquid rescale options
 */
static int
_get_lqr_options(HashTable *options, gint *max_step, gfloat *rigidity,
                 int *vertical_seam, int *save_alpha TSRMLS_DC)
{
	zval **entry = NULL;

	if (options == NULL) {
		return SUCCESS;
	}

	if (hash_find(options, "max_step", &entry) == SUCCESS) {
		long l = gdex_get_lval(*entry);
		if (l < 0L) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"'max_step' must not be a negative number");
			return FAILURE;
		}
		*max_step = (gint)l;
	}

	if (hash_find(options, "rigidity", &entry) == SUCCESS) {
		double d = gdex_get_dval(*entry);
		if (!zend_finite(d)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"'rigidity' must be a finite number");
			return FAILURE;
		} else if (d < 0.0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"'rigidity' must not be a negative number");
			return FAILURE;
		}
		*rigidity = (gfloat)d;
	}

	if (hash_find(options, "vertical_seam", &entry) == SUCCESS) {
		*vertical_seam = zval_is_true(*entry);
	}

	if (hash_find(options, "save_alpha", &entry) == SUCCESS) {
		*save_alpha = zval_is_true(*entry);
	}

	return SUCCESS;
}
/* }}} */

/* {{{ _gdimage_to_lqrcaver()
 * Convert gdImage to LqrCarver
 */
static LqrCarver *
_gdimage_to_lqrcaver(const gdImagePtr im, int save_alpha)
{
	LqrCarver *carver;
	guchar *buffer, *ptr;
	int x, y, width, height, channels;

	width = gdImageSX(im);
	height = gdImageSY(im);
	if (save_alpha) {
		channels = 4;
	} else {
		channels = 3;
	}
	buffer = g_try_new(guchar, width * height * channels);
	if (!buffer) {
		return NULL;
	}
	ptr = buffer;

	if (save_alpha) {
		if (gdImageTrueColor(im)) {
			int c;

			for (y = 0; y < height; y++) {
				for (x = 0; x < width; x++) {
					c = unsafeGetTrueColorPixel(im, x, y);
					*(ptr++) = (guchar)getR(c);
					*(ptr++) = (guchar)getG(c);
					*(ptr++) = (guchar)getB(c);
					*(ptr++) = (guchar)_alpha2gray(getA(c));
				}
			}
		} else {
			unsigned char i;

			for (y = 0; y < height; y++) {
				for (x = 0; x < width; x++) {
					i = unsafeGetPalettePixel(im, x, y);
					*(ptr++) = (guchar)paletteR(im, i);
					*(ptr++) = (guchar)paletteG(im, i);
					*(ptr++) = (guchar)paletteB(im, i);
					*(ptr++) = (guchar)_alpha2gray(paletteA(im, i));
				}
			}
		}
	} else {
		if (gdImageTrueColor(im)) {
			int c;

			for (y = 0; y < height; y++) {
				for (x = 0; x < width; x++) {
					c = unsafeGetTrueColorPixel(im, x, y);
					*(ptr++) = (guchar)getR(c);
					*(ptr++) = (guchar)getG(c);
					*(ptr++) = (guchar)getB(c);
				}
			}
		} else {
			unsigned char i;

			for (y = 0; y < height; y++) {
				for (x = 0; x < width; x++) {
					i = unsafeGetPalettePixel(im, x, y);
					*(ptr++) = (guchar)paletteR(im, i);
					*(ptr++) = (guchar)paletteG(im, i);
					*(ptr++) = (guchar)paletteB(im, i);
				}
			}
		}
	}

	carver = lqr_carver_new(buffer, (gint)width, (gint)height, (gint)channels);
	if (!carver) {
		g_free(buffer);
		return NULL;
	}
	return carver;
}
/* }}} */

/* {{{ _lqrcarver_to_gdimage()
 * Convert LqrCarver to gdImage
 */
static gdImagePtr
_lqrcarver_to_gdimage(LqrCarver *carver)
{
	gdImagePtr im;
	gint x, y;
	guchar *ch;

	im = gdImageCreateTrueColor(lqr_carver_get_width(carver),
	                            lqr_carver_get_height(carver));
	if (!im) {
		return NULL;
	}

	lqr_carver_scan_reset(carver);
	if (lqr_carver_get_channels(carver) == 4) {
		while (lqr_carver_scan(carver, &x, &y, &ch)) {
			unsafeSetTrueColorPixel(im, x, y, gdTrueColorAlpha(
					ch[0], ch[1], ch[2], _gray2alpha(ch[3])));
		}
	} else {
		while (lqr_carver_scan(carver, &x, &y, &ch)) {
			unsafeSetTrueColorPixel(im, x, y, gdTrueColor(ch[0], ch[1], ch[2]));
		}
	}

	return im;
}
/* }}} */

/* {{{ gdex_liquid_rescale()
 *  Do liquid rescaling.
 */
GDEXTRA_LOCAL gdImagePtr
gdex_liquid_rescale(const gdImagePtr src, int width, int height,
                    HashTable *options TSRMLS_DC)
{
	LqrCarver *carver;
	LqrRetVal ret;
	gdImagePtr dst;
	gint max_step = 1;
	gfloat rigidity = 0.0;
	int vertical_seam = 0;
	int save_alpha = 0;

	if (_get_lqr_options(options, &max_step, &rigidity,
			&vertical_seam, &save_alpha TSRMLS_CC) == FAILURE)
	{
		return NULL;
	}

	carver = _gdimage_to_lqrcaver(src, save_alpha);
	if (carver == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Cannot create the caver");
		return NULL;
	}

	ret = lqr_carver_init(carver, max_step, rigidity);
	if (ret != LQR_OK) {
		lqr_carver_destroy(carver);
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Failed to initialize the caver (%s)", _lqrerrstr(ret));
		return NULL;
	}

	if (vertical_seam) {
		lqr_carver_set_resize_order(carver, LQR_RES_ORDER_VERT);
	}

	ret = lqr_carver_resize(carver, width, height);
	if (ret != LQR_OK) {
		lqr_carver_destroy(carver);
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Failed to resize (%s)", _lqrerrstr(ret));
		return NULL;
	}

	dst = _lqrcarver_to_gdimage(carver);
	lqr_carver_destroy(carver);
	if (dst == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Cannot create the post-carving image");
		return NULL;
	}

	return dst;
}
/* }}} */

/* {{{ proto resource imagecarve_ex(resource src, int width, int height[, array options])
 *
 */
GDEXTRA_LOCAL PHP_FUNCTION(imagecarve_ex)
{
	zval *zsrc;
	gdImagePtr src, dst;
	long width, height;
	zval *zoptions = NULL;
	HashTable *options = NULL;
	int le_gd = phpi_get_le_gd();

	/* parse the arguments */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rll|a!",
			&zsrc, &width, &height, &zoptions) == FAILURE)
	{
		return;
	}
	ZEND_FETCH_RESOURCE(src, gdImagePtr, &zsrc, -1, "Image", le_gd);

	if (width < 1L || height < 1L || width > INT_MAX || height > INT_MAX) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid image dimensions");
		RETURN_FALSE;
	}

	if (zoptions != NULL && Z_TYPE_P(zoptions) == IS_ARRAY) {
		options = Z_ARRVAL_P(zoptions);
	}

	/* resize the image */
	dst = gdex_liquid_rescale(src, (int)width, (int)height, options TSRMLS_CC);
	if (dst == NULL) {
		RETURN_FALSE;
	}
	ZEND_REGISTER_RESOURCE(return_value, dst, le_gd);
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
