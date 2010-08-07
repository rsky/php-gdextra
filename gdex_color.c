/*
 * Extra image functions: color allocator functions
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
#define GDEXTRA_SVG_COLORS_DECLARE_ONLY 1
#include "svg_color.h"

ZEND_EXTERN_MODULE_GLOBALS(gdextra);

/* {{{ private function prototypes */

static void
_hsl2rgb(double h, double s, double l, double *r, double *g, double *b),
_hsv2rgb(double h, double s, double v, double *r, double *g, double *b),
_rgb2hsl(double r, double g, double b, double *h, double *s, double *l),
_rgb2hsv(double r, double g, double b, double *h, double *s, double *v),
_rgb2cmyk(double r, double g, double b, double *c, double *m, double *y, double *k),
_cmyk2rgb(double c, double m, double y, double k, double *r, double *g, double *b);

static int
_parse_css_color(const char *color, int length, int *r, int *g, int *b, double *a);

static void
_color_allocate(INTERNAL_FUNCTION_PARAMETERS, int colorspace);

static void
_color_convert(INTERNAL_FUNCTION_PARAMETERS, int c_from, int c_to);

/* }}} */
/* {{{ gdex_fetch_color() */

/*
 * Fetch a color from zval.
 */
GDEXTRA_LOCAL int
gdex_fetch_color(zval *zv, const gdImagePtr im)
{
	int c = -1, r = 0, g = 0, b = 0, a = gdAlphaOpaque;

	if (Z_TYPE_P(zv) == IS_LONG || Z_TYPE_P(zv) == IS_DOUBLE) {
		/* verify the color value */
		if (Z_TYPE_P(zv) == IS_DOUBLE) {
			double d = Z_DVAL_P(zv);

			if (!zend_finite(d) || d < 0.0 || d > (double)INT_MAX) {
				return -1;
			}
			c = (int)d;
		} else {
			c = (int)Z_LVAL_P(zv);
		}

		if (im == NULL || gdImageTrueColor(im)) {
			if (!isValidTrueColor(c)) {
				c = -1;
			}
		} else {
			if (!paletteExists(im, c)) {
				c = -1;
			} else if (paletteOpened(im, c)) {
				c = -1;
			} else {
				r = paletteR(im, c);
				g = paletteG(im, c);
				b = paletteB(im, c);
				if (c == gdImageGetTransparent(im)) {
					a = gdAlphaTransparent;
				} else {
					a = paletteA(im, c);
				}
				c = gdTrueColorAlpha(r, g, b, a);
			}
		}
	} else {
		/* parse the color string */
		char *color;
		int color_len = 0;
		zend_bool is_copy = 0;

		color = gdex_get_strval(zv, &color_len, &is_copy);
		if (gdex_parse_css_color(color, color_len, &r, &g, &b, &a) == SUCCESS) {
			c = gdTrueColorAlpha(r, g, b, a);
		}
		if (is_copy) {
			efree(color);
		}
	}

	return c;
}

/* }}} */
/* {{{ gdex_parse_css_color() */

/*
 * Parse CSS3-style color strings.
 *
 * @see http://www.w3.org/TR/css3-color/
 */
GDEXTRA_LOCAL int
gdex_parse_css_color(const char *color, int length, int *r, int *g, int *b, int *a)
{
	double d = 1.0;

	if (_parse_css_color(color, length, r, g, b, &d) == SUCCESS) {
		*a = _float2alpha((float)d);
		return SUCCESS;
	} else {
		*r = 0;
		*g = 0;
		*b = 0;
		*a = gdAlphaOpaque;
		return FAILURE;
	}
}

/* }}} */
/* {{{ _parse_css_color() */

/*
 * Parse CSS3-style color strings (and so on).
 *
 * @see http://www.w3.org/TR/css3-color/
 */
static int
_parse_css_color(const char *color, int length, int *r, int *g, int *b, double *a)
{
	char *ltcolor;
	int ltlength = 0;
	int result = FAILURE;
	gdex_rgba_t *rgb = NULL;
	int i1 = 0, i2 = 0, i3 = 0;
	double p1 = 0.0, p2 = 0.0, p3 = 0.0, p4 = 1.0;
	double d1 = 0.0, d2 = 0.0, d3 = 0.0, d4 = 0.0;

	ltcolor = gdex_str_tolower_trim(color, length, &ltlength);

	/*
	 * RGB color values in six or three hexadecimal characters
	 *
	 * @see http://www.w3.org/TR/css3-color/#rgb-color
	 */
	if (*ltcolor == '#') {
		if (ltlength == 7 && sscanf(color, "#%2x%2x%2x", &i1, &i2, &i3) == 3) {
			*r = i1;
			*g = i2;
			*b = i3;
			*a = 1.0;
			result = SUCCESS;
		} else if (ltlength == 4 && sscanf(color, "#%1x%1x%1x", &i1, &i2, &i3) == 3) {
			*r = (i1 << 4) | i1;
			*g = (i2 << 4) | i2;
			*b = (i3 << 4) | i3;
			*a = 1.0;
			result = SUCCESS;
		}
		goto finish;
	}

	/* invalid leading character */
	if (*ltcolor < 'a' || 'z' < *ltcolor) {
		goto finish;
	}

	/*
	 * transparent
	 *
	 * @see http://www.w3.org/TR/css3-color/#transparent
	 */
	if (ltlength == sizeof("transparent") - 1 &&
		!memcmp(ltcolor, "transparent", ltlength))
	{
		*r = 0;
		*g = 0;
		*b = 0;
		*a = 0.0;
		result = SUCCESS;
		goto finish;
	}

	/*
	 * SVG color keywords (contains HTML4 color keywords)
	 *
	 * @see http://www.w3.org/TR/css3-color/#svg-color
	 * @see http://www.w3.org/TR/css3-color/#html4
	 */
	if (zend_hash_find(gdex_get_svg_color_table(),
			ltcolor, ltlength + 1, (void **)&rgb) == SUCCESS)
	{
		*r = rgb->r;
		*g = rgb->g;
		*b = rgb->b;
		*a = 1.0;
		result = SUCCESS;
		goto finish;
	}

	/*
	 * RGB/RGBA color values in percentage
	 *
	 * @see http://www.w3.org/TR/css3-color/#rgb-color
	 * @see http://www.w3.org/TR/css3-color/#rgba-color
	 */
	if (*ltcolor == 'r' && (
		sscanf(ltcolor, "rgba(%lf%%,%lf%%,%lf%%,%lf)", &p1, &p2, &p3, &p4) == 4 ||
		sscanf(ltcolor, "rgb(%lf%%,%lf%%,%lf%%)", &p1, &p2, &p3) == 3))
	{
		if (zend_finite(p1) && zend_finite(p2) && zend_finite(p3) && zend_finite(p4)) {
			*r = (int)(MINMAX(p1, 0.0, 100.0) * 2.555);
			*g = (int)(MINMAX(p2, 0.0, 100.0) * 2.555);
			*b = (int)(MINMAX(p3, 0.0, 100.0) * 2.555);
			*a = MINMAX(p4, 0.0, 1.0);
			result = SUCCESS;
		}
		goto finish;
	}

	/*
	 * RGB/RGBA color values in integer
	 *
	 * @see http://www.w3.org/TR/css3-color/#rgb-color
	 * @see http://www.w3.org/TR/css3-color/#rgba-color
	 */
	if (*ltcolor == 'r' && (
		sscanf(ltcolor, "rgba(%d,%d,%d,%lf)", &i1, &i2, &i3, &p4) == 4 ||
		sscanf(ltcolor, "rgb(%d,%d,%d)", &i1, &i2, &i3) == 3))
	{
		if (zend_finite(p4)) {
			*r = MINMAX(i1, 0, 255);
			*g = MINMAX(i2, 0, 255);
			*b = MINMAX(i3, 0, 255);
			*a = MINMAX(p4, 0.0, 1.0);
			result = SUCCESS;
		}
		goto finish;
	}

	/*
	 * HSL/HSLA color values
	 *
	 * @see http://www.w3.org/TR/css3-color/#hsl-color
	 * @see http://www.w3.org/TR/css3-color/#hsla-color
	 */
	if (*ltcolor == 'h' && (
		sscanf(ltcolor, "hsla(%lf,%lf%%,%lf%%,%lf)", &p1, &p2, &p3, &p4) == 4 ||
		sscanf(ltcolor, "hsl(%lf,%lf%%,%lf%%)", &p1, &p2, &p3) == 3))
	{
		if (zend_finite(p1) && zend_finite(p2) && zend_finite(p3) && zend_finite(p4)) {
			p1 = modf(p1 / 360.0, &d4);
			if (p1 < 0.0) {
				p1 += 1.0;
			}
			p2 = MINMAX(p2, 0.0, 100.0) / 100.0;
			p3 = MINMAX(p3, 0.0, 100.0) / 100.0;
			_hsl2rgb(p1, p2, p3, &d1, &d2, &d3);
			*r = (int)(d1 * 255.5);
			*g = (int)(d2 * 255.5);
			*b = (int)(d3 * 255.5);
			*a = MINMAX(p4, 0.0, 1.0);
			result = SUCCESS;
		}
		goto finish;
	}

#if PHP_GDEXTRA_EXPERIMENTAL
	/*
	 * HSV/HSVA color values (not CSS3 color)
	 */
	if (*ltcolor == 'h' && (
		sscanf(ltcolor, "hsva(%lf,%lf%%,%lf%%,%lf)", &p1, &p2, &p3, &p4) == 4 ||
		sscanf(ltcolor, "hsv(%lf,%lf%%,%lf%%)", &p1, &p2, &p3) == 3))
	{
		if (zend_finite(p1) && zend_finite(p2) && zend_finite(p3) && zend_finite(p4)) {
			p1 = modf(p1 / 360.0, &d4);
			if (p1 < 0.0) {
				p1 += 1.0;
			}
			p2 = MINMAX(p2, 0.0, 100.0) / 100.0;
			p3 = MINMAX(p3, 0.0, 100.0) / 100.0;
			_hsv2rgb(p1, p2, p3, &d1, &d2, &d3);
			*r = (int)(d1 * 255.5);
			*g = (int)(d2 * 255.5);
			*b = (int)(d3 * 255.5);
			*a = MINMAX(p4, 0.0, 1.0);
			result = SUCCESS;
		}
		goto finish;
	}
#endif /* EXPERIMENTAL - HSV */

#if PHP_GDEXTRA_EXPERIMENTAL
	/*
	 * CMYK color values in percentage (not CSS3 color)
	 */
	if (*ltcolor == 'c' &&
		sscanf(ltcolor, "cmyk(%lf%%,%lf%%,%lf%%,%lf%%)", &p1, &p2, &p3, &p4) == 4)
	{
		if (zend_finite(p1) && zend_finite(p2) && zend_finite(p3) && zend_finite(p4)) {
			p1 = MINMAX(p1, 0.0, 100.0) / 100.0;
			p2 = MINMAX(p2, 0.0, 100.0) / 100.0;
			p3 = MINMAX(p3, 0.0, 100.0) / 100.0;
			p4 = MINMAX(p4, 0.0, 100.0) / 100.0;
			_cmyk2rgb(p1, p2, p3, p4, &d1, &d2, &d3);
			*r = (int)(d1 * 255.5);
			*g = (int)(d2 * 255.5);
			*b = (int)(d3 * 255.5);
			*a = 1.0;
			result = SUCCESS;
		}
		goto finish;
	}
#endif /* EXPERIMENTAL - CMYK */

  finish:
	efree(ltcolor);
	return result;
}

/* }}} */
/* {{{ gdex_palette_to_truecolor() */

/*
 * Convert a palette image to a true color image.
 */
GDEXTRA_LOCAL int
gdex_palette_to_truecolor(gdImagePtr im TSRMLS_DC)
{
	gdImagePtr tc;
	gdImage tmp;
	int x, y, width, height;
	int i, a, transparent;
	int argb[gdMaxColors];

	/* verify */
	if (gdImageTrueColor(im)) {
		return SUCCESS;
	}

	/* create a new image */
	width = gdImageSX(im);
	height = gdImageSY(im);
	tc = gdImageCreateTrueColor(width, height);
	if (tc == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot create a truecolor image");
		return FAILURE;
	}

	/* get the transparent color index */
	transparent = gdImageGetTransparent(im);

	/* make ARGB color palette */
	for (i = 0; i < gdImageColorsTotal(im); i++) {
		a = (i == transparent) ? gdAlphaTransparent : paletteA(im, i);
		argb[i] = gdTrueColorAlpha(paletteR(im, i), paletteG(im, i), paletteB(im, i), a);
	}
	while (i < gdMaxColors) {
		argb[i++] = 0;
	}

	/* copy pixels */
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			unsafeSetTrueColorPixel(tc, x, y, argb[unsafeGetPalettePixel(im, x, y)]);
		}
	}

	/* swap and cleanup */
	memcpy(&tmp, tc, sizeof(gdImage));
	memcpy(tc,   im, sizeof(gdImage));
	memcpy(im, &tmp, sizeof(gdImage));
	gdImageDestroy(tc);

	return SUCCESS;
}

/* }}} */
/* {{{ _color_allocate() */

/*
 * Allocate a color from HSV/HSL/CMYK components.
 */
static void
_color_allocate(INTERNAL_FUNCTION_PARAMETERS, int colorspace)
{
	zval *zim = NULL;
	gdImagePtr im = NULL;
	double p1 = 0.0, p2 = 0.0, p3 = 0.0, p4 = 0.0;
	long alpha = gdAlphaOpaque;
	double r = 0.0, g = 0.0, b = 0.0;

	/* parse the arguments */
	if (colorspace == COLORSPACE_CMYK) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rdddd|l",
				&zim, &p1, &p2, &p3, &p4, &alpha) == FAILURE)
		{
			return;
		}
	} else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rddd|l",
			&zim, &p1, &p2, &p3, &alpha) == FAILURE)
		{
			return;
		}
	}
	ZEND_FETCH_RESOURCE(im, gdImagePtr, &zim, -1, "Image", GDEXG(le_gd));

	if (!(zend_finite(p1) && zend_finite(p2) && zend_finite(p3) && zend_finite(p4))) {
		RETURN_FALSE;
	}

	/* normalize parameters and convert color */
	if (colorspace == COLORSPACE_CMYK) {
		p1 = MINMAX(p1, 0.0, 100.0) / 100.0;
		p2 = MINMAX(p2, 0.0, 100.0) / 100.0;
		p3 = MINMAX(p3, 0.0, 100.0) / 100.0;
		p4 = MINMAX(p4, 0.0, 100.0) / 100.0;
		_cmyk2rgb(p1, p2, p3, p4, &r, &g, &b);
	} else {
		p1 = modf(p1 / 360.0, &p4);
		if (p1 < 0.0) {
			p1 += 1.0;
		}
		p2 = MINMAX(p2, 0.0, 100.0) / 100.0;
		p3 = MINMAX(p3, 0.0, 100.0) / 100.0;
		if (colorspace == COLORSPACE_HSL) {
			_hsl2rgb(p1, p2, p3, &r, &g, &b);
		} else {
			_hsv2rgb(p1, p2, p3, &r, &g, &b);
		}
	}

	RETURN_LONG(gdImageColorResolveAlpha(im,
			(int)(r * 255.5), (int)(g * 255.5), (int)(b * 255.5),
			(int)MINMAX(alpha, 0, gdAlphaMax)));
}

/* }}} */
/* {{{ bool imagepalettetotruecolor_ex(resource im) */

/*
 * Convert a palette image to a true color image
 */
GDEXTRA_LOCAL PHP_FUNCTION(imagepalettetotruecolor_ex)
{
	zval *zim = NULL;
	gdImagePtr im = NULL;

	/* parse the arguments */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zim) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(im, gdImagePtr, &zim, -1, "Image", GDEXG(le_gd));

	if (gdImageTrueColor(im)) {
		RETURN_TRUE;
	} else if (gdex_palette_to_truecolor(im TSRMLS_CC) == SUCCESS) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}

/* }}} */
/* {{{ int imagecolorallocatecss_ex(resource im, string color) */

/*
 * Allocate a color from CSS3-style color strings.
 */
GDEXTRA_LOCAL PHP_FUNCTION(imagecolorallocatecss_ex)
{
	zval *zim = NULL;
	gdImagePtr im = NULL;
	char *color = NULL;
	int color_len = 0;
	int r = 0, g = 0, b = 0, a = gdAlphaOpaque;

	/* parse the arguments */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs",
			&zim, &color, &color_len) == FAILURE)
	{
		return;
	}
	ZEND_FETCH_RESOURCE(im, gdImagePtr, &zim, -1, "Image", GDEXG(le_gd));

	/* parse the color */
	if (gdex_parse_css_color(color, color_len, &r, &g, &b, &a) == FAILURE) {
		RETURN_FALSE;
	}

	RETURN_LONG(gdImageColorResolveAlpha(im, r, g, b, a));
}

/* }}} */
/* {{{ int imagecolorallocatecmyk_ex(resource im,
                                     float cyan, float magenta, float yellow,
                                     float black[, int alpha])*/

/*
 * Allocate a color from CMYK components.
 */
GDEXTRA_LOCAL PHP_FUNCTION(imagecolorallocatecmyk_ex)
{
	_color_allocate(INTERNAL_FUNCTION_PARAM_PASSTHRU, COLORSPACE_CMYK);
}

/* }}} */
/* {{{ int imagecolorallocatehsl_ex(resource im,
                                    float hue, float saturation, float lightness
                                    [, int alpha])*/

/*
 * Allocate a color from HSL/HSLA components.
 */
GDEXTRA_LOCAL PHP_FUNCTION(imagecolorallocatehsl_ex)
{
	_color_allocate(INTERNAL_FUNCTION_PARAM_PASSTHRU, COLORSPACE_HSL);
}

/* }}} */
/* {{{ int imagecolorallocatehsv_ex(resource im,
                                    float hue, float saturation, float value
                                    [, int alpha])*/

/*
 * Allocate a color from HSV/HSVA components.
 */
GDEXTRA_LOCAL PHP_FUNCTION(imagecolorallocatehsv_ex)
{
	_color_allocate(INTERNAL_FUNCTION_PARAM_PASSTHRU, COLORSPACE_HSV);
}

/* }}} */
/* {{{ _color_convert() */

/*
 * Convert color components from one to another.
 */
static void
_color_convert(INTERNAL_FUNCTION_PARAMETERS, int c_from, int c_to)
{
	double p1 = 0.0, p2 = 0.0, p3 = 0.0, p4 = 0.0;
	double d1 = 0.0, d2 = 0.0, d3 = 0.0, d4 = 0.0;

	/* parse the arguments and convert the components */
	if (c_from == COLORSPACE_CMYK) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "dddd",
				&p1, &p2, &p3, &p4) == FAILURE)
		{
			return;
		}

		if (!(zend_finite(p1) && zend_finite(p2) && zend_finite(p3) && zend_finite(p4))) {
			RETURN_FALSE;
		}

		p1 = MINMAX(p1, 0.0, 100.0) / 100.0;
		p2 = MINMAX(p2, 0.0, 100.0) / 100.0;
		p3 = MINMAX(p3, 0.0, 100.0) / 100.0;
		p4 = MINMAX(p4, 0.0, 100.0) / 100.0;
		_cmyk2rgb(p1, p2, p3, p4, &d1, &d2, &d3);
		d1 *= 255.0;
		d2 *= 255.0;
		d3 *= 255.0;
	} else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ddd",
				&p1, &p2, &p3) == FAILURE)
		{
			return;
		}

		if (!(zend_finite(p1) && zend_finite(p2) && zend_finite(p3))) {
			RETURN_FALSE;
		}

		if (c_from == COLORSPACE_RGB) {
			p1 = MINMAX(p1, 0.0, 255.0) / 255.0;
			p2 = MINMAX(p2, 0.0, 255.0) / 255.0;
			p3 = MINMAX(p3, 0.0, 255.0) / 255.0;

			switch (c_to) {
				case COLORSPACE_CMYK:
					_rgb2cmyk(p1, p2, p3, &d1, &d2, &d3, &d4);
					d1 *= 100.0;
					d2 *= 100.0;
					d3 *= 100.0;
					d4 *= 100.0;
					break;
				case COLORSPACE_HSL:
				case COLORSPACE_HSV:
					if (c_to == COLORSPACE_HSL) {
						_rgb2hsl(p1, p2, p3, &d1, &d2, &d3);
					} else {
						_rgb2hsv(p1, p2, p3, &d1, &d2, &d3);
					}
					d1 *= 360.0;
					d2 *= 100.0;
					d3 *= 100.0;
					break;
			}
		} else {
			p1 = modf(p1 / 360.0, &p4);
			if (p1 < 0.0) {
				p1 += 1.0;
			}
			p2 = MINMAX(p2, 0.0, 100.0) / 100.0;
			p3 = MINMAX(p3, 0.0, 100.0) / 100.0;

			if (c_from == COLORSPACE_HSL) {
				_hsl2rgb(p1, p2, p3, &d1, &d2, &d3);
			} else {
				_hsv2rgb(p1, p2, p3, &d1, &d2, &d3);
			}
			d1 *= 255.0;
			d2 *= 255.0;
			d3 *= 255.0;
		}
	}

	/* set return value */
	array_init_size(return_value, 8);
	add_index_double(return_value, 0, d1);
	add_index_double(return_value, 1, d2);
	add_index_double(return_value, 2, d3);
	switch (c_to) {
		case COLORSPACE_RGB:
			gdex_add_assoc_double(return_value, "red",   d1);
			gdex_add_assoc_double(return_value, "green", d2);
			gdex_add_assoc_double(return_value, "blue",  d3);
			break;
		case COLORSPACE_HSL:
		case COLORSPACE_HSV:
			gdex_add_assoc_double(return_value, "hue", d1);
			gdex_add_assoc_double(return_value, "saturation", d2);
			if (c_to == COLORSPACE_HSL) {
				gdex_add_assoc_double(return_value, "lightness", d3);
			} else {
				gdex_add_assoc_double(return_value, "value", d3);
			}
			break;
		case COLORSPACE_CMYK:
			add_index_double(return_value, 3, d4); /* key */
			gdex_add_assoc_double(return_value, "cyan",    d1);
			gdex_add_assoc_double(return_value, "magenta", d2);
			gdex_add_assoc_double(return_value, "yellow",  d3);
			gdex_add_assoc_double(return_value, "black",   d4); /* key */
			break;
	}
}

/* }}} */
/* {{{ array ImageExUtil::cmykToRgb(float cyan, float magenta, float yellow, float black) */

/*
 * Convert CMYK color components to RGB color components.
 */
GDEXTRA_LOCAL PHP_METHOD(ImageExUtil, cmykToRgb)
{
	_color_convert(INTERNAL_FUNCTION_PARAM_PASSTHRU, COLORSPACE_CMYK, COLORSPACE_RGB);
}

/* }}} */
/* {{{ array ImageExUtil::hslToRgb(float hue, float saturation, float lightness) */

/*
 * Convert HSL color components to RGB color components.
 */
GDEXTRA_LOCAL PHP_METHOD(ImageExUtil, hslToRgb)
{
	_color_convert(INTERNAL_FUNCTION_PARAM_PASSTHRU, COLORSPACE_HSL, COLORSPACE_RGB);
}

/* }}} */
/* {{{ array ImageExUtil::hslToRgb(float hue, float saturation, float value) */

/*
 * Convert HSV color components to RGB color components.
 */
GDEXTRA_LOCAL PHP_METHOD(ImageExUtil, hsvToRgb)
{
	_color_convert(INTERNAL_FUNCTION_PARAM_PASSTHRU, COLORSPACE_HSV, COLORSPACE_RGB);
}

/* }}} */
/* {{{ array ImageExUtil::rgbToCmyk(float red, float green, float blue) */

/*
 * Convert RGB color components to CMYK color components.
 */
GDEXTRA_LOCAL PHP_METHOD(ImageExUtil, rgbToCmyk)
{
	_color_convert(INTERNAL_FUNCTION_PARAM_PASSTHRU, COLORSPACE_RGB, COLORSPACE_CMYK);
}

/* }}} */
/* {{{ array ImageExUtil::rgbToHsl(float red, float green, float blue) */

/*
 * Convert RGB color components to HSL color components.
 */
GDEXTRA_LOCAL PHP_METHOD(ImageExUtil, rgbToHsl)
{
	_color_convert(INTERNAL_FUNCTION_PARAM_PASSTHRU, COLORSPACE_RGB, COLORSPACE_HSL);
}

/* }}} */
/* {{{ array ImageExUtil::rgbToHsv(float red, float green, float blue) */

/*
 * Convert RGB color components to HSV color components.
 */
GDEXTRA_LOCAL PHP_METHOD(ImageExUtil, rgbToHsv)
{
	_color_convert(INTERNAL_FUNCTION_PARAM_PASSTHRU, COLORSPACE_RGB, COLORSPACE_HSV);
}

/* }}} */
/* {{{ array ImageExUtil::getSvgColorTable(string color) */

/*
 * Get RGBA values of CSS3-style color strings.
 */
GDEXTRA_LOCAL PHP_METHOD(ImageExUtil, parseCssColor)
{
	char *color = NULL;
	int color_len = 0;
	int r = 0, g = 0, b = 0;
	double a = 1.0;

	/* parse the arguments */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
			&color, &color_len) == FAILURE)
	{
		return;
	}

	/* parse the color */
	if (_parse_css_color(color, color_len, &r, &g, &b, &a) == FAILURE) {
		RETURN_FALSE;
	}

	/* set return value */
	array_init_size(return_value, 8);
	add_index_long(return_value,   0, r);
	add_index_long(return_value,   1, g);
	add_index_long(return_value,   2, b);
	add_index_double(return_value, 3, a);
	gdex_add_assoc_long(return_value,   "red",   r);
	gdex_add_assoc_long(return_value,   "green", g);
	gdex_add_assoc_long(return_value,   "blue",  b);
	gdex_add_assoc_double(return_value, "alpha", a);
}

/* }}} */
/* {{{ array ImageExUtil::getSvgColorTable(void) */

/*
 * Get list of SVG color keywords and their RGB values.
 */
GDEXTRA_LOCAL PHP_METHOD(ImageExUtil, getSvgColorTable)
{
	HashTable *svg_colors;
	HashPosition pos;
	zval *color;
	gdex_rgba_t *rgb;
	char *key;
	uint len;
	ulong idx;

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	array_init_size(return_value, SVG_COLOR_NUM);

	svg_colors = gdex_get_svg_color_table();
	zend_hash_internal_pointer_reset_ex(svg_colors, &pos);
	while (zend_hash_get_current_data_ex(svg_colors, (void **)&rgb, &pos) == SUCCESS) {
		if (zend_hash_get_current_key_ex(svg_colors, &key, &len, &idx, 0, &pos)
		 		== HASH_KEY_IS_STRING)
		{
			MAKE_STD_ZVAL(color);
			array_init_size(color, 6);
			add_index_long(color, 0, rgb->r);
			add_index_long(color, 1, rgb->g);
			add_index_long(color, 2, rgb->b);
			gdex_add_assoc_long(color, "red",   rgb->r);
			gdex_add_assoc_long(color, "green", rgb->g);
			gdex_add_assoc_long(color, "blue",  rgb->b);
			add_assoc_zval_ex(return_value, key, len, color);
		}
		zend_hash_move_forward_ex(svg_colors, &pos);
	}
}

/* }}} */
/* {{{ single precision versions of the color converters */
/* {{{ gdex_hsl_to_rgb() */

/*
 * Convert HSL color to RGB color. (float to integer)
 */
GDEXTRA_LOCAL void
gdex_hsl_to_rgb(float h, float s, float l, int *r, int *g, int *b)
{
	if (s == 0.0f) {
		int i;

		i = _float2byte(l);
		*r = i;
		*g = i;
		*b = i;
	} else {
		float mn, mx;

		if (l <= 0.5f) {
			mx = l * (s + 1.0f);
		} else {
			mx = l + s - l * s;
		}
		mn = l * 2.0f - mx;
		h *= 6.0f;

		switch ((int)floorf(h)) {
			case 0:
				*r = _float2byte(mx);
				*g = _float2byte(mn + (mx - mn) * h);
				*b = _float2byte(mn);
				break;
			case 1:
				*r = _float2byte(mn + (mx - mn) * (2.0f - h));
				*g = _float2byte(mx);
				*b = _float2byte(mn);
				break;
			case 2:
				*r = _float2byte(mn);
				*g = _float2byte(mx);
				*b = _float2byte(mn + (mx - mn) * (h - 2.0f));
				break;
			case 3:
				*r = _float2byte(mn);
				*g = _float2byte(mn + (mx - mn) * (4.0f - h));
				*b = _float2byte(mx);
				break;
			case 4:
				*r = _float2byte(mn + (mx - mn) * (h - 4.0f));
				*g = _float2byte(mn);
				*b = _float2byte(mx);
				break;
			case 5:
				*r = _float2byte(mx);
				*g = _float2byte(mn);
				*b = _float2byte(mn + (mx - mn) * (6.0f - h));
				break;
			default:
				/* fallback */
				*r = 0;
				*g = 0;
				*b = 0;
		}
	}
}

/* }}} */
/* {{{ gdex_hsv_to_rgb() */

/*
 * Convert HSV color to RGB color. (float to integer)
 */
GDEXTRA_LOCAL void
gdex_hsv_to_rgb(float h, float s, float v, int *r, int *g, int *b)
{
	if (s == 0.0f) {
		int i;

		i = _float2byte(v);
		*r = i;
		*g = i;
		*b = i;
	} else {
		int j;
		float e, f;

		j = _float2byte(v);
		f = modff(h * 6.0f, &e);
#define _x (1.0f - s)
#define _y (1.0f - s * f)
#define _z (1.0f - s * (1.0f - f))
		switch((int)e) {
			case 0:
				*r = j;
				*g = _float2byte(v * _z);
				*b = _float2byte(v * _x);
				break;
			case 1:
				*r = _float2byte(v * _y);
				*g = j;
				*b = _float2byte(v * _x);
				break;
			case 2:
				*r = _float2byte(v * _x);
				*g = j;
				*b = _float2byte(v * _z);
				break;
			case 3:
				*r = _float2byte(v * _x);
				*g = _float2byte(v * _y);
				*b = j;
				break;
			case 4:
				*r = _float2byte(v * _z);
				*g = _float2byte(v * _x);
				*b = j;
				break;
			case 5:
				*r = j;
				*g = _float2byte(v * _x);
				*b = _float2byte(v * _y);
				break;
			default:
				/* fallback */
				*r = 0;
				*g = 0;
				*b = 0;
		}
#undef _x
#undef _y
#undef _z
	}
}

/* }}} */
/* {{{ gdex_rgb_to_hsl() */

/*
 * Convert RGB color to HSL color. (integer to float)
 */
GDEXTRA_LOCAL void
gdex_rgb_to_hsl(int r, int g, int b, float *h, float *s, float *l)
{
	int mx, mn;

	if (r > g) {
		if (r > b) {
			mx = r;
			mn = (b > g) ? g : b;
		} else {
			mx = b;
			mn = g;
		}
	} else {
		if (b > g) {
			mx = b;
			mn = r;
		} else {
			mx = g;
			mn = (r > b) ? b : r;
		}
	}

	if (mx == 0) {
		*l = 0.0f;
	} else {
		*l = (float)(mx + mn) / 510.0f;
	}

	if (mx == mn) {
		*s = 0.0f;
		*h = 0.0f;
	} else {
		float d, f;

		d = (float)(mx - mn);

		if (*l <= 0.5f) {
			*s = d / (float)(mx + mn);
		} else {
			*s = d / (float)(510 - mx - mn);
		}

		if (mx == r) {
			f = (float)(g - b) / d;
		} else if (mx == g) {
			f = 2.0f + (float)(b - r) / d;
		} else {
			f = 4.0f + (float)(r - g) / d;
		}
		f /= 6.0f;
		*h = (f < 0.0f) ? f + 1.0f : f;
	}
}

/* }}} */
/* {{{ gdex_rgb_to_hsv() */

/*
 * Convert RGB color to HSV color. (integer to float)
 */
GDEXTRA_LOCAL void
gdex_rgb_to_hsv(int r, int g, int b, float *h, float *s, float *v)
{
	int mx, mn;

	if (r > g) {
		if (r > b) {
			mx = r;
			mn = (b > g) ? g : b;
		} else {
			mx = b;
			mn = g;
		}
	} else {
		if (b > g) {
			mx = b;
			mn = r;
		} else {
			mx = g;
			mn = (r > b) ? b : r;
		}
	}

	if (mx == 0) {
		*v = 0.0f;
	} else {
		*v = (float)mx / 255.0f;
	}

	if (mx == mn) {
		*s = 0.0f;
		*h = 0.0f;
	} else {
		float d, f;

		d = (float)(mx - mn);

		*s = d / (float)mx;

		if (mx == r) {
			f = (float)(g - b) / d;
		} else if (mx == g) {
			f = 2.0f + (float)(b - r) / d;
		} else {
			f = 4.0f + (float)(r - g) / d;
		}
		f /= 6.0f;
		*h = (f < 0.0f) ? f + 1.0f : f;
	}
}

/* }}} */
/* {{{ gdex_rgb_to_cmyk() */

/*
 * Convert RGB color to CMYK color. (integer to float)
 */
GDEXTRA_LOCAL void
gdex_rgb_to_cmyk(int r, int g, int b, float *c, float *m, float *y, float *k)
{
	if (r == g && g == b) {
		*c = 0.0f;
		*y = 0.0f;
		*m = 0.0f;
		*k = (float)(255 - r) / 255.0f;
	} else {
		int mx;
		float w;

		if (r > g) {
			if (r > b) {
				mx = r;
			} else {
				mx = b;
			}
		} else {
			if (g > b) {
				mx = g;
			} else {
				mx = b;
			}
		}
		w = (float)mx;
		*c = (float)(mx - r) / w;
		*m = (float)(mx - g) / w;
		*y = (float)(mx - b) / w;
		*k = (float)(255 - mx) / 255.0f;
	}
}

/* }}} */
/* {{{ gdex_cmyk_to_rgb() */

/*
 * Convert CMYK color to RGB color. (float to integer)
 */
GDEXTRA_LOCAL void
gdex_cmyk_to_rgb(float c, float m, float y, float k, int *r, int *g, int *b)
{
	float w = 1.0f - k;
	*r = _float2byte(w - c * w);
	*g = _float2byte(w - m * w);
	*b = _float2byte(w - y * w);
}

/* }}} */
/* }}} */
/* {{{ double precision versions of the color converters */
/* {{{ _hsl2rgb() */

/*
 * Convert HSL color to RGB color. (in double precision)
 */
static void
_hsl2rgb(double h, double s, double l, double *r, double *g, double *b)
{
	if (s == 0.0) {
		*r = l;
		*g = l;
		*b = l;
	} else {
		double mn, mx;

		if (l <= 0.5) {
			mx = l * (s + 1.0);
		} else {
			mx = l + s - l * s;
		}
		mn = l * 2.0 - mx;
		h *= 6.0;

		switch ((int)floor(h)) {
			case 0:
				*r = mx;
				*g = mn + (mx - mn) * h;
				*b = mn;
				break;
			case 1:
				*r = mn + (mx - mn) * (2.0 - h);
				*g = mx;
				*b = mn;
				break;
			case 2:
				*r = mn;
				*g = mx;
				*b = mn + (mx - mn) * (h - 2.0);
				break;
			case 3:
				*r = mn;
				*g = mn + (mx - mn) * (4.0 - h);
				*b = mx;
				break;
			case 4:
				*r = mn + (mx - mn) * (h - 4.0);
				*g = mn;
				*b = mx;
				break;
			case 5:
				*r = mx;
				*g = mn;
				*b = mn + (mx - mn) * (6.0 - h);
				break;
			default:
				/* fallback */
				*r = 0.0;
				*g = 0.0;
				*b = 0.0;
		}
	}
}

/* }}} */
/* {{{ _hsv2rgb() */

/*
 * Convert HSV color to RGB color. (in double precision)
 */
static void
_hsv2rgb(double h, double s, double v, double *r, double *g, double *b)
{
	if (s == 0.0) {
		*r = v;
		*g = v;
		*b = v;
	} else {
		double e, f;

		f = modf(h * 6.0, &e);
#define _x (1.0 - s)
#define _y (1.0 - s * f)
#define _z (1.0 - s * (1.0 - f))
		switch((int)e) {
			case 0:
				*r = v;
				*g = v * _z;
				*b = v * _x;
				break;
			case 1:
				*r = v * _y;
				*g = v;
				*b = v * _x;
				break;
			case 2:
				*r = v * _x;
				*g = v;
				*b = v * _z;
				break;
			case 3:
				*r = v * _x;
				*g = v * _y;
				*b = v;
				break;
			case 4:
				*r = v * _z;
				*g = v * _x;
				*b = v;
				break;
			case 5:
				*r = v;
				*g = v * _x;
				*b = v * _y;
				break;
			default:
				/* fallback */
				*r = 0.0;
				*g = 0.0;
				*b = 0.0;
		}
#undef _x
#undef _y
#undef _z
	}
}

/* }}} */
/* {{{ _rgb2hsl() */

/*
 * Convert RGB color to HSL color. (in double precision)
 */
static void
_rgb2hsl(double r, double g, double b, double *h, double *s, double *l)
{
	double mx, mn;

	if (r > g) {
		if (r > b) {
			mx = r;
			mn = (b > g) ? g : b;
		} else {
			mx = b;
			mn = g;
		}
	} else {
		if (b > g) {
			mx = b;
			mn = r;
		} else {
			mx = g;
			mn = (r > b) ? b : r;
		}
	}

	if (mx == 0) {
		*l = 0.0;
	} else {
		*l = (mx + mn) / 2.0;
	}

	if (mx == mn) {
		*s = 0.0;
		*h = 0.0;
	} else {
		double d, f;

		d = mx - mn;

		if (*l <= 0.5) {
			*s = d / (mx + mn);
		} else {
			*s = d / (2.0 - mx - mn);
		}

		if (mx == r) {
			f = (g - b) / d;
		} else if (mx == g) {
			f = 2.0 + (b - r) / d;
		} else {
			f = 4.0 + (r - g) / d;
		}
		f /= 6.0;
		*h = (f < 0.0) ? f + 1.0 : f;
	}
}

/* }}} */
/* {{{ _rgb2hsv() */

/*
 * Convert RGB color to HSV color. (in double precision)
 */
static void
_rgb2hsv(double r, double g, double b, double *h, double *s, double *v)
{
	double mx, mn;

	if (r > g) {
		if (r > b) {
			mx = r;
			mn = (b > g) ? g : b;
		} else {
			mx = b;
			mn = g;
		}
	} else {
		if (b > g) {
			mx = b;
			mn = r;
		} else {
			mx = g;
			mn = (r > b) ? b : r;
		}
	}

	if (mx == 0) {
		*v = 0.0;
	} else {
		*v = mx;
	}

	if (mx == mn) {
		*s = 0.0;
		*h = 0.0;
	} else {
		double d, f;

		d = (mx - mn);

		*s = d / mx;

		if (mx == r) {
			f = (g - b) / d;
		} else if (mx == g) {
			f = 2.0 + (b - r) / d;
		} else {
			f = 4.0 + (r - g) / d;
		}
		f /= 6.0;
		*h = (f < 0.0) ? f + 1.0 : f;
	}
}

/* }}} */
/* {{{ _rgb2cmyk() */

/*
 * Convert RGB color to CMYK color. (in double precision)
 */
static void
_rgb2cmyk(double r, double g, double b, double *c, double *m, double *y, double *k)
{
	if (r == g && g == b) {
		*c = 0.0;
		*y = 0.0;
		*m = 0.0;
		*k = 1.0 - r;
	} else {
		double mx;

		if (r > g) {
			if (r > b) {
				mx = r;
			} else {
				mx = b;
			}
		} else {
			if (g > b) {
				mx = g;
			} else {
				mx = b;
			}
		}
		*c = (mx - r) / mx;
		*m = (mx - g) / mx;
		*y = (mx - b) / mx;
		*k = 1.0 - mx;
	}
}

/* }}} */
/* {{{ _cmyk2rgb() */

/*
 * Convert CMYK color to RGB color. (in double precision)
 */
static void
_cmyk2rgb(double c, double m, double y, double k, double *r, double *g, double *b)
{
	double kc, km, ky;

	kc = c * (1.0 - k) + k;
	if (kc > 1.0) {
		*r = 0.0;
	} else {
		*r = 1.0 - kc;
	}
	km = m * (1.0 - k) + k;
	if (km > 1.0) {
		*g = 0.0;
	} else {
		*g = 1.0 - km;
	}
	ky = y * (1.0 - k) + k;
	if (ky > 1.0) {
		*b = 0.0;
	} else {
		*b = 1.0 - ky;
	}
}

/* }}} */
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
