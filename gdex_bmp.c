/*
 * Extra image functions: BMP/ICON writer functions
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
#include <stdint.h>

ZEND_EXTERN_MODULE_GLOBALS(gdextra);

typedef unsigned char byte_t;

/* {{{ private function prototypes */

static int
_verify_icon_size(const gdImagePtr im);

static byte_t *
_gdimages_to_icon(gdImagePtr *images, size_t num, size_t *result_size TSRMLS_DC);

#define gdimages_to_icon(images, num, result_size) \
	_gdimages_to_icon((images), (num), (result_size) TSRMLS_CC)

static byte_t *
_write_bmp_header(byte_t *ptr,
                  size_t total_size, size_t image_offset, size_t image_size,
                  int width, int height, unsigned short bitcount);

static byte_t *
_write_bmp_palette(byte_t *ptr, const gdImagePtr im, int ncolors);

static byte_t
*_gdimage_to_bmp1(const gdImagePtr im, size_t *size TSRMLS_DC),
*_gdimage_to_bmp4(const gdImagePtr im, size_t *size, zend_bool fill_palette TSRMLS_DC),
*_gdimage_to_bmp8(const gdImagePtr im, size_t *size, zend_bool fill_palette TSRMLS_DC),
*_gdimage_to_bmp24(const gdImagePtr im, size_t *size TSRMLS_DC),
*_gdimage_to_bmp32(const gdImagePtr im, size_t *size, zend_bool v5header TSRMLS_DC);

#define gdimage_to_bmp1(im, size)       _gdimage_to_bmp1((im), (size) TSRMLS_CC)
#define gdimage_to_bmp4(im, size, fill) _gdimage_to_bmp4((im), (size), (fill) TSRMLS_CC)
#define gdimage_to_bmp8(im, size, fill) _gdimage_to_bmp8((im), (size), (fill) TSRMLS_CC)
#define gdimage_to_bmp24(im, size)     _gdimage_to_bmp24((im), (size) TSRMLS_CC)
#define gdimage_to_bmp32(im, size, v5) _gdimage_to_bmp32((im), (size), (v5) TSRMLS_CC)

static zend_bool
_output_image(const char *filename, const byte_t *buffer, size_t buffer_size TSRMLS_DC);

#define output_image(filename, buffer, buffer_size) \
	_output_image((filename), (buffer), (buffer_size) TSRMLS_CC)

/* }}} */
/* {{{ inline functions */

/*static inline byte_t *
_write_zero16(byte_t *ptr)
{
	*ptr++ = '\0';
	*ptr++ = '\0';
	return ptr;
}*/

/*static inline byte_t *
_write_zero32(byte_t *ptr)
{
	*ptr++ = '\0';
	*ptr++ = '\0';
	*ptr++ = '\0';
	*ptr++ = '\0';
	return ptr;
}*/

/*static inline byte_t *
_write_int16le(byte_t *ptr, int16_t n)
{
	*ptr++ = (byte_t)(0xffU & n);
	*ptr++ = (byte_t)(0xffU & (n >> 8));
	return ptr;
}*/

static inline byte_t *
_write_uint16le(byte_t *ptr, uint16_t n)
{
	*ptr++ = (byte_t)(0xffU & n);
	*ptr++ = (byte_t)(0xffU & (n >> 8));
	return ptr;
}

static inline byte_t *
_write_int32le(byte_t *ptr, int32_t n)
{
	*ptr++ = (byte_t)(0xffU & n);
	*ptr++ = (byte_t)(0xffU & (n >> 8));
	*ptr++ = (byte_t)(0xffU & (n >> 16));
	*ptr++ = (byte_t)(0xffU & (n >> 24));
	return ptr;
}

static inline byte_t *
_write_uint32le(byte_t *ptr, uint32_t n)
{
	*ptr++ = (byte_t)(0xffU & n);
	*ptr++ = (byte_t)(0xffU & (n >> 8));
	*ptr++ = (byte_t)(0xffU & (n >> 16));
	*ptr++ = (byte_t)(0xffU & (n >> 24));
	return ptr;
}

/* }}} */
/* {{{ _verify_icon_size() */

/*
 * Check whether the image size is within 256x256 pixels
 */
static int
_verify_icon_size(const gdImagePtr im)
{
	int width, height;

	width = gdImageSX(im);
	height = gdImageSY(im);

	if (width < 1 || width > 256 || height < 1 || height > 256) {
		TSRMLS_FETCH();
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid image dimensions as an icon");
		return FAILURE;
	}
	return SUCCESS;
}

/* }}} */
/* {{{ gdimages_to_icon() */

/*
 * Create a Windows Icon image from an image resources
 */
static byte_t *
_gdimages_to_icon(gdImagePtr *images, size_t num, size_t *result_size TSRMLS_DC)
{
	byte_t *icon, *ptr;
	size_t icon_size, pos;

	/* allocate for ICONDIR and ICONDIRENTRY */
	icon_size = 6 + 16 * num;

	/* write ICONDIR */
	icon = (byte_t *)emalloc(icon_size);
	ptr = icon;
	ptr = _write_uint16le(ptr, 0); /* idReserved */
	ptr = _write_uint16le(ptr, 1); /* idType: 1 for icons, 2 for cursors */
	ptr = _write_uint16le(ptr, (uint16_t)num); /* idCount */

	for (pos = 0; pos < num; pos++) {
		byte_t *temp, *buffer, *bitmap, *icondirentry;
		size_t buffer_size, bitmap_size, mask_size, mask_width, mask_pad;
		int x, y, width, height, transparent;
		gdImagePtr im;

		im = images[pos];
		width = gdImageSX(im);
		height = gdImageSY(im);
		transparent = gdImageGetTransparent(im);
		if (transparent < 0) {
			transparent = -1;
		}
		mask_width = ((size_t)width + 7) / 8;
		mask_pad = mask_width % 4;
		mask_size = (mask_width + mask_pad) * (size_t)height;

		/* write ICONDIRENTRY */
		icondirentry = icon + 6 + 16 * pos;
		*icondirentry++ = (byte_t)(0xffU & width);  /* bWidth:  0 if 256 pixels */
		*icondirentry++ = (byte_t)(0xffU & height); /* bHeight: 0 if 256 pixels */
		*icondirentry++ = '\0'; /* bColorCount: 0 if TrueColor or just (1 << wBitCount) colors */
		*icondirentry++ = '\0'; /* bReserved */
		icondirentry = _write_uint16le(icondirentry, 1); /* wPlanes */

		/* get a BMP image */
		if (gdImageTrueColor(im)) {
			buffer = gdimage_to_bmp32(im, &buffer_size, 0);
			icondirentry = _write_uint16le(icondirentry, 32); /* wBitCount */
		} else {
			int colors = gdImageColorsTotal(im);
			if (colors > 16) {
				buffer = gdimage_to_bmp8(im, &buffer_size, 1);
				icondirentry = _write_uint16le(icondirentry, 8); /* wBitCount */
			} else if (colors > 2) {
				buffer = gdimage_to_bmp4(im, &buffer_size, 1);
				icondirentry = _write_uint16le(icondirentry, 4); /* wBitCount */
			} else {
				buffer = gdimage_to_bmp1(im, &buffer_size);
				icondirentry = _write_uint16le(icondirentry, 1); /* wBitCount */
			}
		}
		if (buffer == NULL) {
			efree(icon);
			return NULL;
		}
		bitmap = buffer + 14;
		bitmap_size = buffer_size - 14;

		/* overwrite biHeight */
		(void)_write_int32le(bitmap + 8, (int32_t)height * 2);

		/* write ICONDIRENTRY (dwBytesInRes and dwImageOffset) */
		icondirentry = _write_uint32le(icondirentry, (uint32_t)(bitmap_size + mask_size));
		icondirentry = _write_uint32le(icondirentry, (uint32_t)icon_size);

		/* reallocate for ICONIMAGE */
		temp = erealloc(icon, icon_size + bitmap_size + mask_size);
		if (temp == NULL) {
			efree(buffer);
			efree(icon);
			php_error_docref(NULL TSRMLS_CC, E_ERROR,
					"Failed to reallocate memory for %zu bytes",
					icon_size + bitmap_size + mask_size);
			return NULL;
		}
		icon = temp;

		/* write ICONIMAGE (icHeader + icColors + icXOR = BMP without BITMAPFILEHEADER) */
		ptr = icon + icon_size;
		(void)memcpy(ptr, bitmap, bitmap_size);
		efree(buffer);

		/* write ICONIMAGE (icAND = 1-bit mask) */
		ptr = icon + icon_size + bitmap_size;
		y = height;
		while (y > 0) {
			byte_t shift = 7, value = 0;
			byte_t *eol = ptr + mask_width + mask_pad;

			--y;
			for (x = 0; x < width; x++) {
				if (gdImageTrueColor(im)) {
					if (gdAlphaTransparent == getA(unsafeGetTrueColorPixel(im, x, y))) {
						value |= (1 << shift);
					}
				} else if (transparent != -1) {
					if (transparent == unsafeGetPalettePixel(im, x, y)) {
						value |= (1 << shift);
					}
				}
				if (shift == 0) {
					*ptr++ = value;
					shift = 7;
					value = 0;
				} else {
					shift--;
				}
			}
			if (shift != 7) {
				*ptr++ = value;
			}
			while (ptr < eol) {
				*ptr++ = '\0';
			}
		}

		icon_size += bitmap_size + mask_size;
	}

	*result_size = icon_size;
	return icon;
}

/* }}} */
/* {{{ _write_bmp_header() */

/*
 * Write BITMAPFILEHEADER and BITMAPINFOHEADER
 */
static byte_t *
_write_bmp_header(byte_t *ptr,
                  size_t total_size, size_t image_offset, size_t image_size,
                  int width, int height, unsigned short bitcount)
{
	/* write BITMAPFILEHEADER */
	*ptr++ = 'B'; /* bfType */
	*ptr++ = 'M'; /* bfType */
	ptr = _write_uint32le(ptr, (uint32_t)total_size); /* bfSize */
	ptr = _write_uint16le(ptr, 0); /* bfReserved1 */
	ptr = _write_uint16le(ptr, 0); /* bfReserved2 */
	ptr = _write_uint32le(ptr, (uint32_t)image_offset); /* bfOffBits */

	/* write BITMAPINFOHEADER */
	ptr = _write_uint32le(ptr, 40); /* biSize */
	ptr = _write_int32le(ptr, (int32_t)width);  /* biWidth */
	ptr = _write_int32le(ptr, (int32_t)height); /* biHeight */
	ptr = _write_uint16le(ptr, 1); /* biPlanes */
	ptr = _write_uint16le(ptr, (uint16_t)bitcount); /* biBitCount */
	ptr = _write_uint32le(ptr, 0); /* biCopmression: BI_RGB */
	ptr = _write_uint32le(ptr, (uint32_t)image_size); /* biSizeImage */
	ptr = _write_int32le(ptr, 3780); /* biXPixPerMeter: 96ppi */
	ptr = _write_int32le(ptr, 3780); /* biYPixPerMeter: 96ppi */
	ptr = _write_uint32le(ptr, 0); /* biClrUsed */
	ptr = _write_uint32le(ptr, 0); /* biCirImportant */

	return ptr;
}

/* }}} */
/* {{{ _write_bmp_palette() */

/*
 * Write color pallete (RGBQUAD[])
 */
static byte_t *
_write_bmp_palette(byte_t *ptr, const gdImagePtr im, int ncolors)
{
	int idx = 0, tcolors = 0;

	if (!gdImageTrueColor(im)) {
		tcolors = gdImageColorsTotal(im);

		/* write RGBQUAD for each colors */
		for (idx = 0; idx < tcolors; idx++) {
			*ptr++ = paletteB(im, idx);
			*ptr++ = paletteG(im, idx);
			*ptr++ = paletteR(im, idx);
			*ptr++ = '\0';
		}
	}

	/* fill pallete */
	while (idx < ncolors) {
		*ptr++ = '\0';
		*ptr++ = '\0';
		*ptr++ = '\0';
		*ptr++ = '\0';
		idx++;
	}

	return ptr;
}

/* }}} */
/* {{{ _gdimage_to_bmp1() */

/*
 * Create an 1-bit Windows Bitmap image from an image resource
 */
static byte_t *
_gdimage_to_bmp1(const gdImagePtr im, size_t *size TSRMLS_DC)
{
	byte_t *buffer, *ptr;
	size_t buffer_size, image_offset, image_size, line_size;
	int x, y, width, height, colors;

	/* get image size and number of colors */
	width = gdImageSX(im);
	height = gdImageSY(im);
	colors = gdImageColorsTotal(im);
	if (colors != 1 && colors != 2) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Invalid number of colors (%d)", colors);
		return NULL;
	}

	/* calculate required memory size */
	image_offset = 14 + 40 + 4 * 2;
	line_size = ((size_t)width + 31) / 32 * 4;
	image_size = line_size * (size_t)height;
	buffer_size = image_offset + image_size;
	if (buffer_size >= (size_t)INT_MAX) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Required memory size too large");
		return NULL;
	}

	/* allocate */
	buffer = (byte_t *)emalloc(buffer_size);
	if (buffer == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR,
				"Failed to allocate memory for %zu bytes", buffer_size);
		return NULL;
	}
	ptr = buffer;

	/* write header */
	ptr = _write_bmp_header(ptr,
			buffer_size, image_offset, image_size, width, height, 1U);

	/* write color palette */
	ptr = _write_bmp_palette(ptr, im, 2);

	/* write image data */
	y = height;
	while (y > 0) {
		byte_t shift = 7, value = 0;
		byte_t *eol = ptr + line_size;

		--y;
		for (x = 0; x < width; x++) {
			if (unsafeGetPalettePixel(im, x, y)) {
				value |= (1 << shift);
			}
			if (shift == 0) {
				*ptr++ = value;
				shift = 7;
				value = 0;
			} else {
				shift--;
			}
		}
		if (shift != 7) {
			*ptr++ = value;
		}
		while (ptr < eol) {
			*ptr++ = '\0';
		}
	}

	if (size != NULL) {
		*size = buffer_size;
	}
	return buffer;
}

/* }}} */
/* {{{ _gdimage_to_bmp4() */

/*
 * Create an 4-bit Windows Bitmap image from an image resource
 */
static byte_t *
_gdimage_to_bmp4(const gdImagePtr im, size_t *size, zend_bool fill_palette TSRMLS_DC)
{
	byte_t *buffer, *ptr;
	size_t buffer_size, image_offset, image_size, line_size;
	int x, y, width, height, colors;

	/* get image size and number of colors */
	width = gdImageSX(im);
	height = gdImageSY(im);
	colors = gdImageColorsTotal(im);
	if (colors < 1 || colors > 16) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Invalid number of colors (%d)", colors);
		return NULL;
	}

	/* calculate required memory size */
	image_offset = 14 + 40 + 4 * ((fill_palette) ? 16 : (size_t)colors);
	line_size = ((size_t)width * 4 + 31) / 32 * 4;
	image_size = line_size * (size_t)height;
	buffer_size = image_offset + image_size;
	if (buffer_size >= (size_t)INT_MAX) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Required memory size too large");
		return NULL;
	}

	/* allocate */
	buffer = (byte_t *)emalloc(buffer_size);
	if (buffer == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR,
				"Failed to allocate memory for %zu bytes", buffer_size);
		return NULL;
	}
	ptr = buffer;

	/* write header */
	ptr = _write_bmp_header(ptr,
			buffer_size, image_offset, image_size, width, height, 4U);
	if (colors != 16 && !fill_palette) {
		/* overwrite biClrUsed */
		(void)_write_uint32le(buffer + 14 + 32, (uint32_t)colors);
	}

	/* write color palette */
	ptr = _write_bmp_palette(ptr, im, ((fill_palette) ? 16 : -1));

	/* write image data */
	y = height;
	while (y > 0) {
		byte_t *eol = ptr + line_size;

		--y;
		x = 0;
		while (x < width - 1) {
			*ptr = (byte_t)(0xf0U & (unsafeGetPalettePixel(im, x++, y) << 4));
			*ptr++ |= (byte_t)(0x0fU & unsafeGetPalettePixel(im, x++, y));
		}
		if (x == width - 1) {
			*ptr++ = (byte_t)(0xf0U & (unsafeGetPalettePixel(im, x, y) << 4));
		}
		while (ptr < eol) {
			*ptr++ = '\0';
		}
	}

	if (size != NULL) {
		*size = buffer_size;
	}
	return buffer;
}

/* }}} */
/* {{{ _gdimage_to_bmp8() */

/*
 * Create an 8-bit Windows Bitmap image from an image resource
 */
static byte_t *
_gdimage_to_bmp8(const gdImagePtr im, size_t *size, zend_bool fill_palette TSRMLS_DC)
{
	byte_t *buffer, *ptr;
	size_t buffer_size, image_offset, image_size, line_size;
	int x, y, width, height, colors;

	/* get image size and number of colors */
	width = gdImageSX(im);
	height = gdImageSY(im);
	colors = gdImageColorsTotal(im);
	if (colors < 1 || colors > 256) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Invalid number of colors (%d)", colors);
		return NULL;
	}

	/* calculate required memory size */
	image_offset = 14 + 40 + 4 * ((fill_palette) ? 256 : (size_t)colors);
	line_size = ((size_t)width * 8 + 31) / 32 * 4;
	image_size = line_size * (size_t)height;
	buffer_size = image_offset + image_size;
	if (buffer_size >= (size_t)INT_MAX) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Required memory size too large");
		return NULL;
	}

	/* allocate */
	buffer = (byte_t *)emalloc(buffer_size);
	if (buffer == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR,
				"Failed to allocate memory for %zu bytes", buffer_size);
		return NULL;
	}
	ptr = buffer;

	/* write header */
	ptr = _write_bmp_header(ptr,
			buffer_size, image_offset, image_size, width, height, 8U);
	if (colors != 256 && !fill_palette) {
		/* overwrite biClrUsed */
		(void)_write_uint32le(buffer + 14 + 32, (uint32_t)colors);
	}

	/* write color palette */
	ptr = _write_bmp_palette(ptr, im, ((fill_palette) ? 256 : -1));

	/* write image data */
	y = height;
	while (y > 0) {
		byte_t *eol = ptr + line_size;

		--y;
		for (x = 0; x < width; x++) {
			*ptr++ = unsafeGetPalettePixel(im, x, y);
		}
		while (ptr < eol) {
			*ptr++ = '\0';
		}
	}

	if (size != NULL) {
		*size = buffer_size;
	}
	return buffer;
}

/* }}} */
/* {{{ _gdimage_to_bmp24() */

/*
 * Create a 24-bit Windows Bitmap image from an image resource
 */
static byte_t *
_gdimage_to_bmp24(const gdImagePtr im, size_t *size TSRMLS_DC)
{
	byte_t *buffer, *ptr;
	size_t buffer_size, image_offset, image_size, line_size;
	int x, y, width, height;

	/* get image size */
	width = gdImageSX(im);
	height = gdImageSY(im);

	/* calculate required memory size */
	image_offset = 14 + 40;
	line_size = ((size_t)width * 24 + 31) / 32 * 4;
	image_size = line_size * (size_t)height;
	buffer_size = image_offset + image_size;
	if (buffer_size >= (size_t)INT_MAX) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Required memory size too large");
		return NULL;
	}

	/* allocate */
	buffer = (byte_t *)emalloc(buffer_size);
	if (buffer == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR,
				"Failed to allocate memory for %zu bytes", buffer_size);
		return NULL;
	}
	ptr = buffer;

	/* write header */
	ptr = _write_bmp_header(ptr,
			buffer_size, image_offset, image_size, width, height, 24U);

	/* write image data */
	y = height;
	while (y > 0) {
		byte_t *eol = ptr + line_size;

		--y;
		for (x = 0; x < width; x++) {
			int c = unsafeGetTrueColorPixel(im, x, y);
			*ptr++ = (byte_t)getB(c);
			*ptr++ = (byte_t)getG(c);
			*ptr++ = (byte_t)getR(c);
		}
		while (ptr < eol) {
			*ptr++ = '\0';
		}
	}

	if (size != NULL) {
		*size = buffer_size;
	}
	return buffer;
}

/* }}} */
/* {{{ _gdimage_to_bmp32() */

/*
 * Create a 32-bit Windows Bitmap image from an image resource
 */
static byte_t *
_gdimage_to_bmp32(const gdImagePtr im, size_t *size, zend_bool v5header TSRMLS_DC)
{
	byte_t *buffer, *ptr;
	size_t buffer_size, image_offset, image_size, line_size;
	int x, y, width, height;

	/* get image size */
	width = gdImageSX(im);
	height = gdImageSY(im);

	/* calculate required memory size */
	image_offset = 14 + ((v5header) ? 124 : 40);
	line_size = (size_t)width * 4;
	image_size = line_size * (size_t)height;
	buffer_size = image_offset + image_size;
	if (buffer_size >= (size_t)INT_MAX) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR, "Required memory size too large");
		return NULL;
	}

	/* allocate */
	buffer = (byte_t *)emalloc(buffer_size);
	if (buffer == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_ERROR,
				"Failed to allocate memory for %zu bytes", buffer_size);
		return NULL;
	}
	ptr = buffer;

	/* write header */
	ptr = _write_bmp_header(ptr,
			buffer_size, image_offset, image_size, width, height, 32U);
	if (v5header) {
		(void)_write_uint32le(buffer + 14, 124); /* bV5Size */
		(void)_write_uint32le(buffer + 14 + 16, 3); /* bV5Copmression: BI_BITFIELDS */
		ptr = _write_uint32le(ptr, 0x00ff0000); /* bV5RedMask */
		ptr = _write_uint32le(ptr, 0x0000ff00); /* bV5GreenMask */
		ptr = _write_uint32le(ptr, 0x000000ff); /* bV5BlueMask */
		ptr = _write_uint32le(ptr, 0xff000000); /* bV5AlphaMask */
		ptr = _write_uint32le(ptr, 0x73524742); /* bV5CSType: LCS_sRGB */
		/* (N/A for bV5CSType != LCS_CALIBRATED_RGB) */
		ptr = _write_uint32le(ptr, 0); /* bV5Endpoints: ciexyzRed: X */
		ptr = _write_uint32le(ptr, 0); /* bV5Endpoints: ciexyzRed: Y */
		ptr = _write_uint32le(ptr, 0); /* bV5Endpoints: ciexyzRed: Z */
		ptr = _write_uint32le(ptr, 0); /* bV5Endpoints: ciexyzGreen: X */
		ptr = _write_uint32le(ptr, 0); /* bV5Endpoints: ciexyzGreen: Y */
		ptr = _write_uint32le(ptr, 0); /* bV5Endpoints: ciexyzGreen: Z */
		ptr = _write_uint32le(ptr, 0); /* bV5Endpoints: ciexyzBlue: X */
		ptr = _write_uint32le(ptr, 0); /* bV5Endpoints: ciexyzBlue: Y */
		ptr = _write_uint32le(ptr, 0); /* bV5Endpoints: ciexyzBlue: Z */
		ptr = _write_uint32le(ptr, 1); /* bV5GammaRed */
		ptr = _write_uint32le(ptr, 1); /* bV5GammaGreen */
		ptr = _write_uint32le(ptr, 1); /* bV5GammaBlue */
		/* (N/A for bV5CSType != LCS_sRGB) */
		ptr = _write_uint32le(ptr, 8); /* bV5Intent: LCS_GM_ABS_COLORIMETRIC */
		/* (N/A for bV5CSType != PROFILE_LINKED and bV5CSType != PROFILE_EMBEDDED) */
		ptr = _write_uint32le(ptr, 0); /* bV5ProfileData */
		ptr = _write_uint32le(ptr, 0); /* bV5ProfileSize */
		ptr = _write_uint32le(ptr, 0); /* bV5Reserved */
	}

	/* write image data */
	y = height;
	while (y > 0) {
		--y;
		for (x = 0; x < width; x++) {
			int c = unsafeGetTrueColorPixel(im, x, y);
			*ptr++ = (byte_t)getB(c);
			*ptr++ = (byte_t)getG(c);
			*ptr++ = (byte_t)getR(c);
			*ptr++ = (byte_t)_alpha2gray(getA(c));
		}
	}

	if (size != NULL) {
		*size = buffer_size;
	}
	return buffer;
}

/* }}} */
/* {{{ output_image() */

/*
 * Output an image to either the output buffer or a file
 */
static zend_bool
_output_image(const char *filename, const byte_t *buffer, size_t buffer_size TSRMLS_DC)
{
	zend_bool success = 0;

	if (filename == NULL) {
		PHPWRITE((void *)buffer, (uint)buffer_size);
		success = 1;
	} else {
		php_stream *stream;

		stream = php_stream_open_wrapper((char *)filename, "wb",
				IGNORE_URL | ENFORCE_SAFE_MODE | REPORT_ERRORS, NULL);
		if (stream != NULL) {
			if (buffer_size != php_stream_write(stream, (char *)buffer, buffer_size)) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to write data");
			} else {
				success = 1;
			}
			php_stream_close(stream);
		}
	}

	return success;
}

/* }}} */
/* {{{ bool imagebmp_ex(resource im[, string filename]) */

/*
 * Output a BMP image to either the browser or a file
 */
GDEXTRA_LOCAL PHP_FUNCTION(imagebmp_ex)
{
	zval *zim = NULL;
	gdImagePtr im = NULL;
	char *filename = NULL;
	int filename_len = 0;
	byte_t *buffer = NULL;
	size_t buffer_size = 0;
	zend_bool success = 0;

	/* parse the arguments */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|s",
			&zim, &filename, &filename_len) == FAILURE)
	{
		return;
	}
	ZEND_FETCH_RESOURCE(im, gdImagePtr, &zim, -1, "Image", GDEXG(le_gd));

	/* determine whether to be a truecolor image */
	if (gdImageTrueColor(im)) {
		if (im->saveAlphaFlag) {
			buffer = gdimage_to_bmp32(im, &buffer_size, 1);
		} else {
			buffer = gdimage_to_bmp24(im, &buffer_size);
		}
	} else {
		int colors = gdImageColorsTotal(im);
		if (colors > 16) {
			buffer = gdimage_to_bmp8(im, &buffer_size, 0);
		} else if (colors > 2) {
			buffer = gdimage_to_bmp4(im, &buffer_size, 0);
		} else {
			buffer = gdimage_to_bmp1(im, &buffer_size);
		}
	}
	if (buffer == NULL) {
		RETURN_FALSE;
	}

	/* write the image */
	success = output_image(((filename_len > 0) ? filename : NULL), buffer, buffer_size);
	efree(buffer);
	RETURN_BOOL(success);
}

/* }}} */
/* {{{ bool imageicon_ex(resource im[, string filename]) */

/*
 * Output an Icon image to either the browser or a file
 */
GDEXTRA_LOCAL PHP_FUNCTION(imageicon_ex)
{
	zval *zim = NULL;
	gdImagePtr im = NULL;
	gdImagePtr *images = NULL;
	char *filename = NULL;
	int filename_len = 0;
	byte_t *icon;
	size_t num, icon_size = 0;
	zend_bool is_multiple = 0;
	zend_bool success = 0;

	/* parse the arguments */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|s",
			&zim, &filename, &filename_len) == FAILURE)
	{
		return;
	}

	if (Z_TYPE_P(zim) == IS_ARRAY) {
		is_multiple = 1;
	} else if (Z_TYPE_P(zim) != IS_RESOURCE) {
		zend_error(E_WARNING, "%s() expects parameter 1 to be resource or array, %s given",
				get_active_function_name(TSRMLS_C), zend_zval_type_name(zim));
		RETURN_FALSE;
	}

	/* fetch the images */
	if (is_multiple) {
		gdImagePtr *imagep;
		HashTable *imageh;
		HashPosition pos;
		zval **entry;

		imageh = Z_ARRVAL_P(zim);
		num = (size_t)zend_hash_num_elements(imageh);
		if (num == 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "No image given");
			RETURN_FALSE;
		}
		if (num > 0xffff) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Too many images given");
			RETURN_FALSE;
		}
		images = (gdImagePtr *)ecalloc(num, sizeof(gdImagePtr));
		imagep = images;

		zend_hash_internal_pointer_reset_ex(imageh, &pos);
		while (zend_hash_get_current_data_ex(imageh, (void **)&entry, &pos) == SUCCESS) {
			ZEND_FETCH_RESOURCE_NO_RETURN(im, gdImagePtr, entry, -1, "Image", GDEXG(le_gd));
			if (im == NULL) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING,
						"All entries of parameter 1 should be valid Image resources");
				efree(images);
				RETURN_FALSE;
			}
			if (_verify_icon_size(im) == FAILURE) {
				efree(images);
				RETURN_FALSE;
			}
			*imagep++ = im;
			zend_hash_move_forward_ex(imageh, &pos);
		}
	} else {
		ZEND_FETCH_RESOURCE(im, gdImagePtr, &zim, -1, "Image", GDEXG(le_gd));
		if (_verify_icon_size(im) == FAILURE) {
			RETURN_FALSE;
		}
		num = 1;
		images = &im;
	}

	/* create an icon data */
	icon = gdimages_to_icon(images, num, &icon_size);
	if (is_multiple) {
		efree(images);
	}

	/* write the image */
	success = output_image(((filename_len > 0) ? filename : NULL), icon, icon_size);
	efree(icon);
	RETURN_BOOL(success);
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
