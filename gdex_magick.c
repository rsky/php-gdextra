/*
 * Extra image functions: ImageMagick image loader functions
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
#include <wand/MagickWand.h>

/* {{{ private function prototypes */

static int
_pixelwand_to_gdtruecolor(const PixelWand *pixel);

static void
_magickwand_error(MagickWand *wand, int errcode, const char *errmsg TSRMLS_DC);

static void
_magickwand_to_gdimage(INTERNAL_FUNCTION_PARAMETERS, zend_bool is_file);

/* }}} */

/* {{{ _pixelwand_to_gdtruecolor()
 * Convert from PixelWand to gdTrueColorAlpha value
 */
static int
_pixelwand_to_gdtruecolor(const PixelWand *pixel)
{
	int r, g, b, a;

	/*if (pixel == NULL || IsPixelWand(pixel) == MagickFalse) {
		return gdTrueColorAlpha(gdRedMax, gdGreenMax, gdBlueMax, gdAlphaOpaque);
	}*/

	r = (int)(255.5 * PixelGetRed(pixel));
	g = (int)(255.5 * PixelGetGreen(pixel));
	b = (int)(255.5 * PixelGetBlue(pixel));
	a = (int)(127.5 * (1.0 - PixelGetAlpha(pixel)));

	r = MINMAX(r, 0, gdRedMax);
	g = MINMAX(g, 0, gdGreenMax);
	b = MINMAX(b, 0, gdBlueMax);
	a = MINMAX(a, 0, gdAlphaMax);

	return gdTrueColorAlpha(r, g, b, a);
}
/* }}} */

/* {{{ _magickwand_error()
 * Raise an error
 */
static void
_magickwand_error(MagickWand *wand, int errcode, const char *errmsg TSRMLS_DC)
{
	ExceptionType severity;
	char *description = NULL;

	if (wand != NULL && IsMagickWand(wand) == MagickTrue) {
		description = MagickGetException(wand, &severity);
	}

	if (description == NULL || *description == '\0') {
		if (errmsg == NULL || *errmsg == '\0') {
			php_error_docref(NULL TSRMLS_CC, errcode, "Unknown error");
		} else {
			php_error_docref(NULL TSRMLS_CC, errcode, "%s", errmsg);
		}
	} else {
		php_error_docref(NULL TSRMLS_CC, errcode, "%s", description);
		MagickRelinquishMemory(description);
		MagickClearException(wand);
	}
}
/* }}} */

/* {{{ _magickwand_to_gdimage()
 * Create a new image from file, URL or the image stream in the string
 */
static void
_magickwand_to_gdimage(INTERNAL_FUNCTION_PARAMETERS, zend_bool is_file)
{
	char *input = NULL;
	int input_len = 0;
	const char *errmsg = NULL;
	php_stream *stream = NULL;
	gdImagePtr im = NULL;
	MagickWand *wand = NULL;
	PixelWand *pixel = NULL;
	MagickBooleanType status;
	unsigned long width, height, x, y;

	/* parse the arguments */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s" FS_CONV_M,
			&input, &input_len FS_CONV_C) == FAILURE)
	{
		return;
	}

	/* initialize MagickWand */
	wand = NewMagickWand();

	/* read the image */
	if (is_file) {
		FILE *fp;
		stream = php_stream_open_wrapper(input, "rb",
				ENFORCE_SAFE_MODE | REPORT_ERRORS | STREAM_WILL_CAST, NULL);
		if (stream == NULL) {
			(void)DestroyMagickWand(wand);
			RETURN_FALSE;
		}
		if (FAILURE == php_stream_cast(stream, PHP_STREAM_AS_STDIO, (void **)&fp, REPORT_ERRORS)) {
			php_stream_close(stream);
			(void)DestroyMagickWand(wand);
			RETURN_FALSE;
		}
		status = MagickReadImageFile(wand, fp);
	} else {
		status = MagickReadImageBlob(wand, (const void *)input, (const size_t)input_len);
	}
	if (status == MagickFalse) {
		goto error_return_false;
	}

	/* get the image size */
	width = MagickGetImageWidth(wand);
	height = MagickGetImageHeight(wand);
	if (width == 0UL || width >= (unsigned long)INT_MAX ||
		height == 0UL || height >= (unsigned long)INT_MAX)
	{
		errmsg = "Invalid image dimensions";
		goto error_return_false;
	}

	/* initialize gdImage */
	im = gdImageCreateTrueColor((int)width, (int)height);
	if (im == NULL) {
		errmsg = "Cannot create a new image";
		goto error_return_false;
	}

	/* set each pixels */
	pixel = NewPixelWand();
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			(void)MagickGetImagePixelColor(wand, x, y, pixel);
			unsafeSetTrueColorPixel(im, x, y, _pixelwand_to_gdtruecolor(pixel));
		}
	}
	(void)DestroyPixelWand(pixel);

	/* cleanup */
	if (stream != NULL) {
		php_stream_close(stream);
	}
	(void)DestroyMagickWand(wand);

	/* return a new image resource */
	ZEND_REGISTER_RESOURCE(return_value, im, phpi_get_le_gd());
	return;

	/* on failure... */
  error_return_false:
	_magickwand_error(wand, E_WARNING, errmsg TSRMLS_CC);
	if (im != NULL) {
		gdImageDestroy(im);
	}
	if (stream != NULL) {
		php_stream_close(stream);
	}
	(void)DestroyMagickWand(wand);
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto resource imagecreatebymagick_ex(string filename)
 * Create a new image from file or URL.
 */
GDEXTRA_LOCAL PHP_FUNCTION(imagecreatebymagick_ex)
{
	_magickwand_to_gdimage(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto resource imagecreatefromstring_ex(string data)
 * Create a new image from the image stream in the string.
 */
GDEXTRA_LOCAL PHP_FUNCTION(imagecreatefromstring_ex)
{
	_magickwand_to_gdimage(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
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
