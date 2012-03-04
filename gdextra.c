/*
 * Extra image functions
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

#define PHP_GDEXTRA_MODULE_VERSION "0.4.0"

/* {{{ globals */

#include "svg_color.h"

static HashTable _svg_color_table;

static zend_class_entry *ce_util = NULL;

/* }}} */
/* {{{ module globals */

GDEXTRA_LOCAL ZEND_DECLARE_MODULE_GLOBALS(gdextra)

/* }}} */
/* {{{ module function prototypes */

static PHP_MINIT_FUNCTION(gdextra);
static PHP_MSHUTDOWN_FUNCTION(gdextra);
static PHP_RINIT_FUNCTION(gdextra);
static PHP_RSHUTDOWN_FUNCTION(gdextra);
static PHP_MINFO_FUNCTION(gdextra);
static PHP_GINIT_FUNCTION(gdextra);

/* }}} */
/* {{{ argument informations */

#if ZEND_EXTENSION_API_NO < 220090626
#define ARG_INFO_STATIC static
#else
#define ARG_INFO_STATIC
#endif

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(arginfo_image, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, im)
ZEND_END_ARG_INFO()

#if PHP_GDEXTRA_WITH_MAGICK
ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(arginfo_imagecreatebymagick, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, filename)
	ZEND_ARG_INFO(0, is_blob)
ZEND_END_ARG_INFO()
#endif /* PHP_GDEXTRA_WITH_MAGICK */

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(arginfo_imagehistgram, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, im)
	ZEND_ARG_INFO(0, colorspace)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(arginfo_imagechannelextract, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, im)
	ZEND_ARG_INFO(0, colorspace)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(arginfo_imagechannelmerge, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_ARRAY_INFO(0, channels, 0)
	ZEND_ARG_INFO(0, colorspace)
	ZEND_ARG_INFO(0, position)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(arginfo_imagealphamask, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 2)
	ZEND_ARG_INFO(0, im)
	ZEND_ARG_INFO(0, mask)
	ZEND_ARG_INFO(0, mode)
	ZEND_ARG_INFO(0, position)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(arginfo_imagewrite, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, im)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(arginfo_imagetowebsafepalette, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, im)
	ZEND_ARG_INFO(0, dither)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(arginfo_imagecolorallocatecss, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, im)
	ZEND_ARG_INFO(0, color)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(arginfo_imagecolorallocatecmyk, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 5)
	ZEND_ARG_INFO(0, im)
	ZEND_ARG_INFO(0, cyan)
	ZEND_ARG_INFO(0, magenta)
	ZEND_ARG_INFO(0, yellow)
	ZEND_ARG_INFO(0, black)
	ZEND_ARG_INFO(0, alpha)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(arginfo_imagecolorallocatehsl, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 4)
	ZEND_ARG_INFO(0, im)
	ZEND_ARG_INFO(0, hue)
	ZEND_ARG_INFO(0, saturation)
	ZEND_ARG_INFO(0, lightness)
	ZEND_ARG_INFO(0, alpha)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(arginfo_imagecolorallocatehsv, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 4)
	ZEND_ARG_INFO(0, im)
	ZEND_ARG_INFO(0, hue)
	ZEND_ARG_INFO(0, saturation)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, alpha)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(arginfo_imagecolorcorrect, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 2)
	ZEND_ARG_INFO(0, im)
	ZEND_ARG_ARRAY_INFO(0, params, 0)
	ZEND_ARG_INFO(0, colorspace)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(arginfo_imageflip, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, im)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(arginfo_imagescale, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 3)
	ZEND_ARG_INFO(0, im)
	ZEND_ARG_INFO(0, width)
	ZEND_ARG_INFO(0, height)
	ZEND_ARG_INFO(0, mode)
	ZEND_ARG_ARRAY_INFO(0, options, 1)
ZEND_END_ARG_INFO()

#if PHP_GDEXTRA_WITH_LQR
ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(arginfo_imagecarve, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 3)
	ZEND_ARG_INFO(0, im)
	ZEND_ARG_INFO(0, width)
	ZEND_ARG_INFO(0, height)
	ZEND_ARG_ARRAY_INFO(0, options, 1)
ZEND_END_ARG_INFO()
#endif /* PHP_GDEXTRA_WITH_LQR */

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(arginfo_util_conv_from_cmyk, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, cyan)
	ZEND_ARG_INFO(0, magenta)
	ZEND_ARG_INFO(0, yellow)
	ZEND_ARG_INFO(0, black)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(arginfo_util_conv_from_hsl, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, hue)
	ZEND_ARG_INFO(0, saturation)
	ZEND_ARG_INFO(0, lightness)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(arginfo_util_conv_from_hsv, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, hue)
	ZEND_ARG_INFO(0, saturation)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(arginfo_util_conv_from_rgb, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, red)
	ZEND_ARG_INFO(0, green)
	ZEND_ARG_INFO(0, blue)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(arginfo_util_parsecsscolor, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, color)
ZEND_END_ARG_INFO()

/* }}} */
/* {{{ gdextra_functions[] */

#if PHP_VERSION_ID >= 50300
#define GDEX_NS_FALIAS(name, alias, arg_info) \
	{ "gdextra\\" #name, ZEND_FN(alias), arg_info, \
		(zend_uint) (sizeof(arg_info)/sizeof(struct _zend_arg_info)-1), 0 },
#endif

static zend_function_entry gdextra_functions[] = {
#if PHP_GDEXTRA_WITH_MAGICK
	GDEX_FE(imagecreatebymagick,     arginfo_imagecreatebymagick)
#endif
	GDEX_FE(imageclone,              arginfo_image)
	GDEX_FE(imagehistgram,           arginfo_imagehistgram)
	GDEX_FE(imagehistgram216,        arginfo_image)
	GDEX_FE(imagechannelextract,     arginfo_imagechannelextract)
	GDEX_FE(imagechannelmerge,       arginfo_imagechannelmerge)
	GDEX_FE(imagealphamask,          arginfo_imagealphamask)
	GDEX_FE(imagebmp,                arginfo_imagewrite)
	GDEX_FE(imageicon,               arginfo_imagewrite)
	GDEX_FE(imagepalettetotruecolor, arginfo_image)
	GDEX_FE(imagetowebsafepalette,   arginfo_imagetowebsafepalette)
	GDEX_FE(imagecolorallocatecss,   arginfo_imagecolorallocatecss)
	GDEX_FE(imagecolorallocatecmyk,  arginfo_imagecolorallocatecmyk)
	GDEX_FE(imagecolorallocatehsl,   arginfo_imagecolorallocatehsl)
	GDEX_FE(imagecolorallocatehsv,   arginfo_imagecolorallocatehsv)
	GDEX_FE(imagecolorcorrect,       arginfo_imagecolorcorrect)
	GDEX_FE(imageflip,               arginfo_imageflip)
	GDEX_FE(imagescale,              arginfo_imagescale)
#if PHP_GDEXTRA_WITH_LQR
	GDEX_FE(imagecarve,              arginfo_imagecarve)
#endif
	{ NULL, NULL, NULL }
};

/* }}} */
/* {{{ gdextra_colorutility_methods[] */

#define GDEX_UTIL_ME(name, arginfo) \
	PHP_ME(ColorUtility, name, arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

static zend_function_entry gdextra_colorutility_methods[] = {
	GDEX_UTIL_ME(cmykToRgb,         arginfo_util_conv_from_cmyk)
	GDEX_UTIL_ME(hslToRgb,          arginfo_util_conv_from_hsl)
	GDEX_UTIL_ME(hsvToRgb,          arginfo_util_conv_from_hsv)
	GDEX_UTIL_ME(rgbToCmyk,         arginfo_util_conv_from_rgb)
	GDEX_UTIL_ME(rgbToHsl,          arginfo_util_conv_from_rgb)
	GDEX_UTIL_ME(rgbToHsv,          arginfo_util_conv_from_rgb)
	GDEX_UTIL_ME(parseCssColor,     arginfo_util_parsecsscolor)
	GDEX_UTIL_ME(getSvgColorTable,  NULL)
	{ NULL, NULL, NULL }
};

/* }}} */
/* {{{ cross-extension dependencies */

static zend_module_dep gdextra_deps[] = {
	ZEND_MOD_REQUIRED("gd")
	{NULL, NULL, NULL, 0}
};

/* }}} */
/* {{{ gdextra_module_entry */

static zend_module_entry gdextra_module_entry = {
	STANDARD_MODULE_HEADER_EX,
	NULL,
	gdextra_deps,
	"gdextra",
	gdextra_functions,
	PHP_MINIT(gdextra),
	PHP_MSHUTDOWN(gdextra),
	PHP_RINIT(gdextra),
	PHP_RSHUTDOWN(gdextra),
	PHP_MINFO(gdextra),
	PHP_GDEXTRA_MODULE_VERSION,
	PHP_MODULE_GLOBALS(gdextra),
	PHP_GINIT(gdextra),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};

/* }}} */

#ifdef COMPILE_DL_GDEXTRA
ZEND_GET_MODULE(gdextra)
#endif

#define GDEX_REGISTER_CONSTANT(name) \
	REGISTER_LONG_CONSTANT("IMAGE_EX_" #name, name, CONST_PERSISTENT | CONST_CS)

/* {{{ PHP_MINIT_FUNCTION */

static PHP_MINIT_FUNCTION(gdextra)
{
	zend_class_entry ce;
	int i;

	/* initialize SVG color table */
	if (zend_hash_init(&_svg_color_table, SVG_COLOR_NUM, NULL, NULL, 1) == FAILURE) {
		return FAILURE;
	}
	for (i = 0; i < SVG_COLOR_NUM; i++) {
		zend_hash_add(&_svg_color_table,
				(char *)_svg_colors[i].name, strlen(_svg_colors[i].name) + 1,
				(void *)&(_svg_colors[i].value), sizeof(gdex_rgba_t), NULL);
	}

	gdex_mask_alpha_funcs_init();

#if GDEXTRA_USE_WRAPPERS
	if (gdex_wrappers_init(INIT_FUNC_ARGS_PASSTHRU) == FAILURE) {
		return FAILURE;
	}
#endif

	/* register constants */
	GDEX_REGISTER_CONSTANT(COLORSPACE_RGB);
	GDEX_REGISTER_CONSTANT(COLORSPACE_HSV);
	GDEX_REGISTER_CONSTANT(COLORSPACE_HSL);
	GDEX_REGISTER_CONSTANT(COLORSPACE_CMYK);
	GDEX_REGISTER_CONSTANT(COLORSPACE_ALPHA);
	GDEX_REGISTER_CONSTANT(COLORSPACE_RAW_ALPHA);
	GDEX_REGISTER_CONSTANT(MASK_SET);
	GDEX_REGISTER_CONSTANT(MASK_MERGE);
	GDEX_REGISTER_CONSTANT(MASK_SCREEN);
	GDEX_REGISTER_CONSTANT(MASK_AND);
	GDEX_REGISTER_CONSTANT(MASK_OR);
	GDEX_REGISTER_CONSTANT(MASK_XOR);
	GDEX_REGISTER_CONSTANT(MASK_NOT);
	GDEX_REGISTER_CONSTANT(MASK_TILE);
	GDEX_REGISTER_CONSTANT(POSITION_TOP_LEFT);
	GDEX_REGISTER_CONSTANT(POSITION_TOP_CENTER);
	GDEX_REGISTER_CONSTANT(POSITION_TOP_RIGHT);
	GDEX_REGISTER_CONSTANT(POSITION_MIDDLE_LEFT);
	GDEX_REGISTER_CONSTANT(POSITION_MIDDLE_CENTER);
	GDEX_REGISTER_CONSTANT(POSITION_MIDDLE_RIGHT);
	GDEX_REGISTER_CONSTANT(POSITION_BOTTOM_LEFT);
	GDEX_REGISTER_CONSTANT(POSITION_BOTTOM_CENTER);
	GDEX_REGISTER_CONSTANT(POSITION_BOTTOM_RIGHT);
	GDEX_REGISTER_CONSTANT(MERGE_CROP);
	GDEX_REGISTER_CONSTANT(FLIP_NONE);
	GDEX_REGISTER_CONSTANT(FLIP_HORIZONTAL);
	GDEX_REGISTER_CONSTANT(FLIP_VERTICAL);
	GDEX_REGISTER_CONSTANT(FLIP_BOTH);
	GDEX_REGISTER_CONSTANT(SCALE_NONE);
	GDEX_REGISTER_CONSTANT(SCALE_CROP);
	GDEX_REGISTER_CONSTANT(SCALE_FIT);
	GDEX_REGISTER_CONSTANT(SCALE_FILL);
	GDEX_REGISTER_CONSTANT(SCALE_PAD);
	GDEX_REGISTER_CONSTANT(SCALE_STRETCH);
	GDEX_REGISTER_CONSTANT(SCALE_TILE);
#if PHP_GDEXTRA_WITH_LQR
	GDEX_REGISTER_CONSTANT(SCALE_CARVE);
#endif

	/* register class ColorUtility */
	memset(&ce, 0, sizeof(zend_class_entry));
	INIT_CLASS_ENTRY(ce, "ColorUtility", gdextra_colorutility_methods);
	if ((ce_util = zend_register_internal_class(&ce TSRMLS_CC)) == NULL) {
		return FAILURE;
	}

	return SUCCESS;
}

/* }}} */
/* {{{ PHP_MSHUTDOWN_FUNCTION */

static PHP_MSHUTDOWN_FUNCTION(gdextra)
{
	zend_hash_destroy(&_svg_color_table);

	return SUCCESS;
}

/* }}} */
/* {{{ PHP_RINIT_FUNCTION */

static PHP_RINIT_FUNCTION(gdextra)
{
#if GDEXTRA_USE_WRAPPERS
#define GDEX_FCALL_INFO_INIT(name) \
	if (gdex_fcall_info_init("image" #name, \
			&GDEXG(func_##name) TSRMLS_CC) == FAILURE) { \
		return FAILURE; \
	}
	GDEX_FCALL_INFO_INIT(create);
	GDEX_FCALL_INFO_INIT(createtruecolor);
/*	GDEX_FCALL_INFO_INIT(destroy);*/
	GDEX_FCALL_INFO_INIT(colorresolvealpha);
	GDEX_FCALL_INFO_INIT(copy);
	GDEX_FCALL_INFO_INIT(copyresampled);
/*	GDEX_FCALL_INFO_INIT(savealpha);*/
#undef GDEX_FCALL_INFO_INIT
#endif
	return SUCCESS;
}

/* }}} */
/* {{{ PHP_RSHUTDOWN_FUNCTION */

static PHP_RSHUTDOWN_FUNCTION(gdextra)
{
#if GDEXTRA_USE_WRAPPERS
#define GDEX_FCALL_INFO_DESTROY(name) \
	gdex_fcall_info_destroy(&GDEXG(func_##name) TSRMLS_CC)
	GDEX_FCALL_INFO_DESTROY(create);
	GDEX_FCALL_INFO_DESTROY(createtruecolor);
/*	GDEX_FCALL_INFO_DESTROY(destroy);*/
	GDEX_FCALL_INFO_DESTROY(colorresolvealpha);
	GDEX_FCALL_INFO_DESTROY(copy);
	GDEX_FCALL_INFO_DESTROY(copyresampled);
/*	GDEX_FCALL_INFO_DESTROY(savealpha);*/
#undef GDEX_FCALL_INFO_DESTROY
#endif
	return SUCCESS;
}

/* }}} */
/* {{{ PHP_MINFO_FUNCTION */

static PHP_MINFO_FUNCTION(gdextra)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "Extra Image Functions Support", "enabled");
	php_info_print_table_row(2, "Module Version", PHP_GDEXTRA_MODULE_VERSION);
#if PHP_GDEXTRA_WITH_LQR
	php_info_print_table_row(2, "Liquid Rescaling Support", "enabled");
	php_info_print_table_row(2, "Liblqr Version", PHP_GDEXTRA_LIBLQR_VERSION_STRING);
#else
	php_info_print_table_row(2, "Liquid Rescaling Support", "disabled");
#endif
#if PHP_GDEXTRA_WITH_MAGICK
	php_info_print_table_row(2, "ImageMagick Loader Support", "enabled");
	php_info_print_table_row(2, "ImageMagick Version", gdex_get_magick_version());
#else
	php_info_print_table_row(2, "ImageMagick Loader Support", "disabled");
#endif
	php_info_print_table_end();
}

/* }}} */
/* {{{ PHP_GINIT_FUNCTION */

static PHP_GINIT_FUNCTION(gdextra)
{
	gdextra_globals->le_gd = phpi_get_le_gd();
}

/* }}} */

/* {{{ _gdex_array_init() */

/*
 * Size supported version of array_init().
 */
#if ZEND_EXTENSION_API_NO < 220090626
GDEXTRA_LOCAL int
_gdex_array_init(zval *zv, uint size)
{
	ALLOC_HASHTABLE(Z_ARRVAL_P(zv));
	zend_hash_init(Z_ARRVAL_P(zv), size, NULL, ZVAL_PTR_DTOR, 0);
	Z_TYPE_P(zv) = IS_ARRAY;
	return SUCCESS;
}
#endif

/* }}} */
/* {{{ gdex_get_lval() */

/*
 * Get a long integer.
 */
GDEXTRA_LOCAL long
gdex_get_lval(zval *zv)
{
	if (Z_TYPE_P(zv) == IS_LONG) {
		return Z_LVAL_P(zv);
	} else if (Z_TYPE_P(zv) == IS_DOUBLE) {
		double d = Z_DVAL_P(zv);

		if (zend_isnan(d)) {
			return 0;
		} else if (d > LONG_MAX) {
			return LONG_MAX;
		} else if (d < LONG_MIN) {
			return LONG_MIN;
		} else {
			return d;
		}
	} else {
		zval val;

		INIT_ZVAL(val);
		ZVAL_ZVAL(&val, zv, 1, 0);
		convert_to_long(&val);

		return Z_LVAL(val);
	}
}

/* }}} */
/* {{{ gdex_get_dval() */

/*
 * Get a double precision number.
 */
GDEXTRA_LOCAL double
gdex_get_dval(zval *zv)
{
	if (Z_TYPE_P(zv) == IS_LONG) {
		return (double)Z_LVAL_P(zv);
	} else if (Z_TYPE_P(zv) == IS_DOUBLE) {
		return Z_DVAL_P(zv);
	} else {
		zval val;

		INIT_ZVAL(val);
		ZVAL_ZVAL(&val, zv, 1, 0);
		convert_to_double(&val);

		return Z_DVAL(val);
	}
}

/* }}} */
/* {{{ gdex_get_strval() */

/*
 * Get a string value.
 */
GDEXTRA_LOCAL char *
gdex_get_strval(zval *zv, int *length, zend_bool *is_copy)
{
	if (Z_TYPE_P(zv) == IS_STRING) {
		*is_copy = 0;
		*length = Z_STRLEN_P(zv);
		return Z_STRVAL_P(zv);
	} else {
		zval val;

		INIT_ZVAL(val);
		ZVAL_ZVAL(&val, zv, 1, 0);
		convert_to_string(&val);

		*is_copy = 1;
		*length = Z_STRLEN(val);
		return Z_STRVAL(val);
	}
}

/* }}} */
/* {{{ gdex_str_tolower_trim() */

/*
 * Get the lowercased and trimmed copy of the string.
 */
GDEXTRA_LOCAL char *
gdex_str_tolower_trim(const char *str, int length, int *new_length)
{
	TSRMLS_FETCH();
	zval trimmed;

	INIT_ZVAL(trimmed);
	php_trim((char *)str, length, NULL, 0, &trimmed, 3 TSRMLS_CC);
	zend_str_tolower(Z_STRVAL(trimmed), Z_STRLEN(trimmed));

	*new_length = Z_STRLEN(trimmed);
	return Z_STRVAL(trimmed);
}

/* }}} */
/* {{{ gdex_get_svg_color_table() */

/*
 * Get the SVG color table.
 */
GDEXTRA_LOCAL HashTable *
gdex_get_svg_color_table(void)
{
	return &_svg_color_table;
}

/* }}} */
/* {{{ gdex_calc_offset() */

/*
 * Calculate the offset to the target.
 */
GDEXTRA_LOCAL int
gdex_calc_offset(int target, int length, int mode)
{
	switch (mode) {
		case ADJUST_TO_HEAD:
			return 0;
		case ADJUST_TO_BODY:
			return (target - length) / 2;
		case ADJUST_TO_TAIL:
			return target - length;
		default:
			/* fallback */
			return 0;
	}
}

/* }}} */
/* {{{ gdex_get_colorspace_name() */

/*
 * Get the name of the color space.
 */
GDEXTRA_LOCAL const char *
gdex_get_colorspace_name(int colorspace)
{
	if (colorspace & COLORSPACE_ALPHA) {
		switch (colorspace ^ COLORSPACE_ALPHA) {
			case COLORSPACE_RGB:
				return "RGBA";
			case COLORSPACE_HSV:
				return "HSVA";
			case COLORSPACE_HSL:
				return "HSLA";
			case COLORSPACE_CMYK:
				return "CMYKA";
		}
	} else {
		switch (colorspace) {
			case COLORSPACE_RGB:
				return "RGB";
			case COLORSPACE_HSV:
				return "HSV";
			case COLORSPACE_HSL:
				return "HSL";
			case COLORSPACE_CMYK:
				return "CMYK";
		}
	}
	return "unknown";
}

/* }}} */
/* {{{ resource imageclone(resource im) */

/*
 * Clone an image.
 */
GDEXTRA_LOCAL GDEX_FUNCTION(imageclone)
{
	zval *zim = NULL;
	gdImagePtr src, dst;
	int width, height;

	/* parse the arguments */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zim) == FAILURE) {
		return;
	}
	ZEND_FETCH_RESOURCE(src, gdImagePtr, &zim, -1, "Image", GDEXG(le_gd));

	/* clone the image */
	width = gdImageSX(src);
	height = gdImageSY(src);
	if (gdImageTrueColor(src)) {
		dst = gdImageCreateTrueColor(width, height);
	} else {
		dst = gdImageCreate(width, height);
	}

	if (dst == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to clone the image");
		RETURN_FALSE;
	}

	gdImageCopy(dst, src, 0, 0, 0, 0, width, height);

	/* register the cloned image to the return value */
	ZEND_REGISTER_RESOURCE(return_value, dst, GDEXG(le_gd));
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
