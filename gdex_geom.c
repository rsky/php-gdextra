/*
 * Extra image functions: geometory functions
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

/* {{{ private function prototypes */

static void
_tile_copy(gdImagePtr dst, gdImagePtr src, int position);

/* }}} */

/* {{{ proto void imageflip_ex(resource im, int mode)
 *
 */
GDEXTRA_LOCAL PHP_FUNCTION(imageflip_ex)
{
	zval *zim = NULL;
	gdImagePtr im = NULL;
	long mode = 0L;
	int x, y, z, width, height, to;

	/* parse the arguments */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &zim, &mode) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(im, gdImagePtr, &zim, -1, "Image", phpi_get_le_gd());

	width = gdImageSX(im);
	height = gdImageSY(im);

	/* flip horizontally */
	if (mode & FLIP_HORIZONTAL) {
		to = height / 2;

		if (gdImageTrueColor(im)) {
			int c;

			for (y = 0; y < to; y++) {
				z = height - y - 1;
				for (x = 0; x < width; x++) {
					c = unsafeGetTrueColorPixel(im, x, y);
					unsafeSetTrueColorPixel(im, x, y, unsafeGetTrueColorPixel(im, x, z));
					unsafeSetTrueColorPixel(im, x, z, c);
				}
			}
		} else {
			unsigned char i;

			for (y = 0; y < to; y++) {
				z = height - y - 1;
				for (x = 0; x < width; x++) {
					i = unsafeGetPalettePixel(im, x, y);
					unsafeSetPalettePixel(im, x, y, unsafeGetPalettePixel(im, x, z));
					unsafeSetPalettePixel(im, x, z, i);
				}
			}
		}
	}

	/* flip vertically */
	if (mode & FLIP_VERTICAL) {
		to = width / 2;

		if (gdImageTrueColor(im)) {
			int c;

			for (x = 0; x < to; x++) {
				z = width - x - 1;
				for (y = 0; y < height; y++) {
					c = unsafeGetTrueColorPixel(im, x, y);
					unsafeSetTrueColorPixel(im, x, y, unsafeGetTrueColorPixel(im, z, y));
					unsafeSetTrueColorPixel(im, z, y, c);
				}
			}
		} else {
			unsigned char i;

			for (x = 0; x < to; x++) {
				z = width - x - 1;
				for (y = 0; y < height; y++) {
					i = unsafeGetPalettePixel(im, x, y);
					unsafeSetPalettePixel(im, x, y, unsafeGetPalettePixel(im, z, y));
					unsafeSetPalettePixel(im, z, y, i);
				}
			}
		}
	}
}
/* }}} */

/* {{{ proto resource imagescale_ex(resource im, int width, int height
 *                                  [, int mode[, array options]])
 *
 */
GDEXTRA_LOCAL PHP_FUNCTION(imagescale_ex)
{
	zval *zim = NULL;
	gdImagePtr src = NULL;
	gdImagePtr dst = NULL;
	long width = 0L, height = 0L;
	long mode = SCALE_FIT;
	zval *zoptions = NULL;
	HashTable *options = NULL;
	zval **entry;
	int position = POSITION_DEFAULT;
	int resample = 1;
	int new_w, new_h, dst_w, dst_h, src_w, src_h;
	int fit_w, fit_h, fill_w, fill_h;
	int dst_x = 0, dst_y = 0, src_x = 0, src_y = 0;
	double dst_r, src_r;
	int restoreAlphaBlending;
	int le_gd = phpi_get_le_gd();

	/* parse the arguments */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rll|la!",
			&zim, &width, &height, &mode, &zoptions) == FAILURE)
	{
		return;
	}
	ZEND_FETCH_RESOURCE(src, gdImagePtr, &zim, -1, "Image", le_gd);

	if (zoptions != NULL && Z_TYPE_P(zoptions) == IS_ARRAY) {
		options = Z_ARRVAL_P(zoptions);

		/* get the scaling position */
		if (hash_find(options, "position", &entry) == SUCCESS) {
			position = (int)(gdex_get_lval(*entry) & POSITION_MASK);
		}
	}

	/* get the image sizes */
	if (width < 1L || height < 1L || width > INT_MAX || height > INT_MAX) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid image dimensions");
		RETURN_FALSE;
	}
	dst_w = new_w = (int)width;
	dst_h = new_h = (int)height;
	dst_r = (double)dst_w / (double)dst_h;
	src_w = gdImageSX(src);
	src_h = gdImageSY(src);
	src_r = (double)src_w / (double)src_h;
	fit_w = (int)lround((double)dst_h * src_r);
	fit_h = (int)lround((double)dst_w / src_r);
	fill_w = (int)lround((double)src_h * dst_r);
	fill_h = (int)lround((double)src_w / dst_r);
#define _CHECK0(n) if (n < 1) { n = 1; }
	_CHECK0(fit_w) _CHECK0(fit_h) _CHECK0(fill_w) _CHECK0(fill_h)
#undef _CHECK0

	/* calculate the image size */
	switch (mode) {
		case SCALE_NONE:
		case SCALE_CROP:
		case SCALE_TILE:
			if (mode == SCALE_TILE && src_w >= dst_w && src_h >= dst_h) {
				mode = SCALE_NONE;
			}
			if (src_w > dst_w) {
				src_x = calc_x_offset(src_w, dst_w, position);
				src_w = dst_w;
			} else if (src_w < dst_w) {
				if (mode == SCALE_CROP) {
					dst_w = src_w;
				} else {
					dst_x = calc_x_offset(dst_w, src_w, position);
				}
			}
			if (src_h > dst_h) {
				src_y = calc_y_offset(src_h, dst_h, position);
				src_h = dst_h;
			} else if (src_h < dst_h) {
				if (mode == SCALE_CROP) {
					dst_h = src_h;
				} else {
					dst_y = calc_y_offset(dst_h, src_h, position);
				}
			}
			resample = 0;
			break;

		case SCALE_FIT:
			if (src_r > dst_r) {
				dst_h = fit_h;
			} else if (src_r < dst_r) {
				dst_w = fit_w;
			}
			break;

		case SCALE_FILL:
			if (src_r > dst_r) {
				src_x = calc_x_offset(src_w, fill_w, position);
				src_w = fill_w;
			} else if (src_r < dst_r) {
				src_y = calc_y_offset(src_h, fill_h, position);
				src_h = fill_h;
			}
			break;

		case SCALE_STRETCH:
		 	/* pass */
			break;

		case SCALE_PAD:
			if (src_r > dst_r) {
				dst_y = calc_y_offset(dst_h, fit_h, position);
			} else if (src_r < dst_r) {
				dst_x = calc_x_offset(dst_w, fit_w, position);
			}
			break;

		case SCALE_CARVE:
#if PHP_GDEXTRA_WITH_LQR
			if (src_r > dst_r) {
				dst_w = fit_w;
			} else if (src_r < dst_r) {
				dst_h = fit_h;
			}
			break;
#else
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"'IMAGE_EX_SCALE_CARVE' mode is not available");
			RETURN_FALSE;
#endif

		default:
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unsupported mode given (%ld)", mode);
			RETURN_FALSE;
	}

	/* create a new image */
	dst = gdImageCreateTrueColor(dst_w, dst_h);
	if (dst == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot create a new image");
		RETURN_FALSE;
	}
	restoreAlphaBlending = dst->alphaBlendingFlag;
	dst->alphaBlendingFlag = gdEffectReplace;

	/* fill the background */
	if (options != NULL && hash_find(options, "bgcolor", &entry) == SUCCESS) {
		int c , x, y;

		c = gdex_fetch_color(*entry, src);
		if (c != -1) {
			for (y = 0; y < dst_h; y++) {
				for (x = 0; x < dst_w; x++) {
					unsafeSetTrueColorPixel(dst, x, y, c);
				}
			}
			dst->alphaBlendingFlag = gdEffectAlphaBlend;
		}
	}

	/* set the image scaling area size */
	if (mode == SCALE_PAD) {
		if (src_r > dst_r) {
			dst_h = fit_h;
		} else if (src_r < dst_r) {
			dst_w = fit_w;
		}
	}

	/* copy the image */
	if (resample) {
		gdImageCopyResampled(dst, src, dst_x, dst_y, src_x, src_y, dst_w, dst_h, src_w, src_h);
	} else if (mode == SCALE_TILE) {
		_tile_copy(dst, src, position);
	} else {
		gdImageCopy(dst, src, dst_x, dst_y, src_x, src_y, src_w, src_h);
	}
	dst->alphaBlendingFlag = restoreAlphaBlending;

	/* carve the overflown area */
#if PHP_GDEXTRA_WITH_LQR
	if (mode == SCALE_CARVE) {
		gdImagePtr tmp;
		HashTable *carve_options = NULL;

		if (options != NULL && hash_find(options, "carve", &entry) == SUCCESS) {
			carve_options = HASH_OF(*entry);
		}

		tmp = gdex_liquid_rescale(dst, new_w, new_h, carve_options TSRMLS_CC);
		gdImageDestroy(dst);
		if (tmp) {
			dst = tmp;
		} else {
			RETURN_FALSE;
		}
	}
#endif

	/* return a new image resource */
	ZEND_REGISTER_RESOURCE(return_value, dst, le_gd);
}
/* }}} */

/* {{{ _tile_copy()
 * Tile the destination image with the source image.
 */
static void
_tile_copy(gdImagePtr dst, gdImagePtr src, int position)
{
	int dst_w, dst_h, src_w, src_h;
	int x, y, z;

	dst_w = gdImageSX(dst);
	dst_h = gdImageSY(dst);
	src_w = gdImageSX(src);
	src_h = gdImageSY(src);

	if (dst_w > src_w) {
		if (position & POSITION_RIGHT) {
			x = (dst_w % src_w) - src_w;
		} else if (position & POSITION_CENTER) {
			x = (dst_w % src_w) / 2 - src_w;
		} else {
			x = 0;
		}
	} else {
		x = calc_x_offset(dst_w, src_w, position);
	}

	if (dst_h > src_h) {
		if (position & POSITION_BOTTOM) {
			y = (dst_h % src_h) - src_h;
		} else if (position & POSITION_MIDDLE) {
			y = (dst_h % src_h) / 2 - src_h;
		} else {
			y = 0;
		}
	} else {
		y = calc_y_offset(dst_h, src_h, position);
	}

	z = x;
	while (y < dst_h) {
		x = z;
		while (x < dst_w) {
			gdImageCopy(dst, src, x, y, 0, 0, src_w, src_h);
			x += src_w;
		}
		y += src_h;
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
