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

#ifndef _PHP_GDEXTRA_H_
#define _PHP_GDEXTRA_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef PHP_GDEXTRA_WITH_LQR
#define PHP_GDEXTRA_WITH_LQR 0
#endif

#ifndef PHP_GDEXTRA_WITH_MAGICK
#define PHP_GDEXTRA_WITH_MAGICK 0
#endif

#ifndef PHP_GDEXTRA_EXPERIMENTAL
#define PHP_GDEXTRA_EXPERIMENTAL 0
#endif

#ifndef PHP_GDEXTRA_ADD_FUNCTION_NAME_SUFFIX
#define PHP_GDEXTRA_ADD_FUNCTION_NAME_SUFFIX 0
#endif

#include <math.h>
#ifdef ZTS
#include "TSRM.h"
#endif
#include <php.h>
#include <php_ini.h>
#include <SAPI.h>
#include <ext/gd/libgd/gd.h>
#include <ext/standard/info.h>
#include <Zend/zend_extensions.h>

#if defined(__GNUC__) && __GNUC__ >= 4
#define GDEXTRA_LOCAL __attribute__((visibility("hidden")))
#else
#define GDEXTRA_LOCAL
#endif

#if defined(HAVE_GD_BUNDLED) && HAVE_GD_BUNDLED && defined(PHP_VERSION_ID) && PHP_VERSION_ID >= 50300
#define GDEXTRA_USE_WRAPPERS 1
#else
#define GDEXTRA_USE_WRAPPERS 0
#endif

/* {{{ flags and modes */

#define COLORSPACE_RGB  0
#define COLORSPACE_HSV  1
#define COLORSPACE_HSL  2
#define COLORSPACE_CMYK 3
#define COLORSPACE_GRAY 4
#define COLORSPACE_ALPHA 128
#define COLORSPACE_RAW 1024
#define COLORSPACE_RAW_ALPHA (COLORSPACE_ALPHA | COLORSPACE_RAW)

#define MASK_SET        0
#define MASK_MERGE      1
#define MASK_SCREEN     2
#define MASK_AND        3
#define MASK_OR         4
#define MASK_XOR        5
#define MASK_NOT        8
#define MASK_TILE      16

#define MASK_OFFSET_NOT 6
#define MASK_NUM_MODES 12

#define POSITION_LEFT   0
#define POSITION_CENTER 1
#define POSITION_RIGHT  2
#define POSITION_TOP    0
#define POSITION_MIDDLE 4
#define POSITION_BOTTOM 8
#define POSITION_TOP_LEFT       (POSITION_TOP    | POSITION_LEFT)
#define POSITION_TOP_CENTER     (POSITION_TOP    | POSITION_CENTER)
#define POSITION_TOP_RIGHT      (POSITION_TOP    | POSITION_RIGHT)
#define POSITION_MIDDLE_LEFT    (POSITION_MIDDLE | POSITION_LEFT)
#define POSITION_MIDDLE_CENTER  (POSITION_MIDDLE | POSITION_CENTER)
#define POSITION_MIDDLE_RIGHT   (POSITION_MIDDLE | POSITION_RIGHT)
#define POSITION_BOTTOM_LEFT    (POSITION_BOTTOM | POSITION_LEFT)
#define POSITION_BOTTOM_CENTER  (POSITION_BOTTOM | POSITION_CENTER)
#define POSITION_BOTTOM_RIGHT   (POSITION_BOTTOM | POSITION_RIGHT)
#define POSITION_DEFAULT        POSITION_MIDDLE_CENTER
#define POSITION_HMASK  (POSITION_LEFT  | POSITION_CENTER | POSITION_RIGHT)
#define POSITION_VMASK  (POSITION_TOP   | POSITION_MIDDLE | POSITION_BOTTOM)
#define POSITION_MASK   (POSITION_HMASK | POSITION_VMASK)

/* MERGE_CROP = MAX(POSITION_XXX) << 1 */
#define MERGE_CROP     16

#define ADJUST_TO_HEAD  0
#define ADJUST_TO_BODY  1
#define ADJUST_TO_TAIL  2

#define FLIP_NONE       0
#define FLIP_HORIZONTAL 1
#define FLIP_VERTICAL   2
#define FLIP_BOTH       (FLIP_HORIZONTAL | FLIP_VERTICAL)
#define FLIP_MASK       FLIP_BOTH

#define SCALE_NONE      0
#define SCALE_CROP      1
#define SCALE_FIT       2
#define SCALE_FILL      3
#define SCALE_PAD       4
#define SCALE_STRETCH   5
#define SCALE_TILE      6
#define SCALE_CARVE     7

/* }}} */
/* {{{ shorthand macros */

#define MINMAX(_val, _min, _max) \
	(((_val) < (_min)) ? (_min) : (((_val) > (_max)) ? (_max) : (_val)))

#define hash_find(_ht, _key, _pp) \
	zend_hash_find((_ht), (_key), sizeof((_key)), (void **)(_pp))
#define hash_exists(_ht, _key) \
	zend_hash_exists((_ht), (_key), sizeof((_key)))

#define isValidTrueColor(c) (((unsigned long)(c) & 0x7fffffffUL) == (unsigned long)(c))
#define getR gdTrueColorGetRed
#define getG gdTrueColorGetGreen
#define getB gdTrueColorGetBlue
#define getA gdTrueColorGetAlpha

#define paletteR(im, c) ((im)->red[(c)])
#define paletteG(im, c) ((im)->green[(c)])
#define paletteB(im, c) ((im)->blue[(c)])
#define paletteA(im, c) ((im)->alpha[(c)])
#define paletteOpened(im, c) ((im)->open[(c)])
#define paletteExists(im, c) (c >= 0 && c < gdImageColorsTotal((im)))

/* }}} */

BEGIN_EXTERN_C()

/* {{{ type definitions for module globals */
#if GDEXTRA_USE_WRAPPERS

typedef struct _gdextra_fcall_info {
	zval *name;
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
} gdextra_fcall_info;

#endif
/* }}} */
/* {{{ module globals */

ZEND_BEGIN_MODULE_GLOBALS(gdextra)
	int le_gd;
#if GDEXTRA_USE_WRAPPERS
	gdextra_fcall_info func_create;
	gdextra_fcall_info func_createtruecolor;
/*	gdextra_fcall_info func_destroy;*/
	gdextra_fcall_info func_colorresolvealpha;
	gdextra_fcall_info func_copy;
	gdextra_fcall_info func_copyresampled;
/*	gdextra_fcall_info func_savealpha;*/
#endif
ZEND_END_MODULE_GLOBALS(gdextra)

#ifdef ZTS
#define GDEXG(v) TSRMG(gdextra_globals_id, zend_gdextra_globals *, v)
#else
#define GDEXG(v) (gdextra_globals.v)
#endif

/* }}} */
/* {{{ funcptr type definitions */

/*
 * Type of color converter functions.
 */
typedef void (*gdex_rgb_to_3ch_func_t)(int r, int g, int b, float *x, float *y, float *z);
typedef void (*gdex_3ch_to_rgb_func_t)(float x, float y, float z, int *r, int *g, int *b);
typedef void (*gdex_rgb_to_4ch_func_t)(int r, int g, int b, float *w, float *x, float *y, float *z);
typedef void (*gdex_4ch_to_rgb_func_t)(float w, float x, float y, float z, int *r, int *g, int *b);

/* }}} */
/* {{{ utility function prototypes */

/*
 * Auto unicode (ascii) and size supported version of array_init()
 * and shorthand macros of add_assoc_{long,double}().
 */
#if ZEND_EXTENSION_API_NO < 220090626

GDEXTRA_LOCAL int
_gdex_array_init(zval *arg, uint size);

#define array_init_size(_arg, _size) _gdex_array_init((_arg), (_size))

#endif

#define gdex_add_assoc_long(_arr, _key, _val) \
	add_assoc_long_ex((_arr), (_key), sizeof((_key)), (_val))

#define gdex_add_assoc_double(_arr, _key, _val) \
	add_assoc_double_ex((_arr), (_key), sizeof((_key)), (_val))

/*
 * Get a long integer.
 */
GDEXTRA_LOCAL long
gdex_get_lval(zval *zv);

/*
 * Get a double precision number.
 */
GDEXTRA_LOCAL double
gdex_get_dval(zval *zv);

/*
 * Get a string value.
 */
GDEXTRA_LOCAL char *
gdex_get_strval(zval *zv, int *length, zend_bool *is_copy);

/*
 * Get the lowercased and trimmed copy of the string.
 */
GDEXTRA_LOCAL char *
gdex_str_tolower_trim(const char *str, int length, int *new_length);

/*
 * Get the SVG color table.
 */
GDEXTRA_LOCAL HashTable *
gdex_get_svg_color_table(void);

/*
 * Get the offset to the target.
 */
GDEXTRA_LOCAL int
gdex_calc_offset(int target, int length, int mode);

#define calc_x_offset(target, length, position) \
	gdex_calc_offset((target), (length), ((position) & POSITION_HMASK))
#define calc_y_offset(target, length, position) \
	gdex_calc_offset((target), (length), ((position) & POSITION_VMASK) >> 2)

/*
 * Get the name of the color space.
 */
GDEXTRA_LOCAL const char *
gdex_get_colorspace_name(int colorspace);

/*
 * Parse CSS3-style color strings.
 * @see http://www.w3.org/TR/css3-color/
 */
GDEXTRA_LOCAL int
gdex_parse_css_color(const char *color, int length, int *r, int *g, int *b, int *a);

/*
 * Fetch a color from zval.
 */
GDEXTRA_LOCAL int
gdex_fetch_color(zval *zv, const gdImagePtr im);

/*
 * Convert from one color representation to RGB color representation.
 *
 * All input parameters except for "h" (hue) should be normalized in range [0..1].
 * Hue should be normalized in range [0..1).
 *
 * All output parameters are in range [0..255].
 */
GDEXTRA_LOCAL void
gdex_hsl_to_rgb(float h, float s, float l, int *r, int *g, int *b),
gdex_hsv_to_rgb(float h, float s, float v, int *r, int *g, int *b),
gdex_cmyk_to_rgb(float c, float m, float y, float k, int *r, int *g, int *b);

/*
 * Convert from RGB color representation to another color representation.
 *
 * All input parameters should be normalized in range [0..255].
 *
 * All output parameters except for "h" (hue) are normalized in range [0..1].
 * Hue is in range [0..1).
 */
GDEXTRA_LOCAL void
gdex_rgb_to_hsl(int r, int g, int b, float *h, float *s, float *l),
gdex_rgb_to_hsv(int r, int g, int b, float *h, float *s, float *v),
gdex_rgb_to_cmyk(int r, int g, int b, float *c, float *m, float *y, float *k);

/*
 * Convert a palette image to a true color image.
 */
GDEXTRA_LOCAL int
gdex_palette_to_truecolor(gdImagePtr im TSRMLS_DC);

/*
 * Convert an image to the web-safe palette.
 */
GDEXTRA_LOCAL int
gdex_image_to_web216(gdImagePtr im, zend_bool dither TSRMLS_DC);

/*
 * Initialize callback functions for imagealphamask().
 */
GDEXTRA_LOCAL void
gdex_mask_alpha_funcs_init(void);

#if PHP_GDEXTRA_WITH_LQR
/*
 *  Do liquid rescaling.
 */
GDEXTRA_LOCAL gdImagePtr
gdex_liquid_rescale(const gdImagePtr src, int width, int height,
                    HashTable *options TSRMLS_DC);
#endif

/* }}} */
/* {{{ PHP function prototypes */

#if PHP_GDEXTRA_ADD_FUNCTION_NAME_SUFFIX
#define GDEX_FUNCTION(fn) PHP_FUNCTION(fn##_ex)
#define GDEX_FE(fn, ai) PHP_FE(fn##_ex, ai)
#else
#define GDEX_FUNCTION PHP_FUNCTION
#define GDEX_FE PHP_FE
#endif

#if PHP_GDEXTRA_WITH_MAGICK
GDEXTRA_LOCAL GDEX_FUNCTION(imagecreatebymagick);
GDEXTRA_LOCAL GDEX_FUNCTION(imagecreatefromstring);
#endif
GDEXTRA_LOCAL GDEX_FUNCTION(imageclone);
GDEXTRA_LOCAL GDEX_FUNCTION(imagehistgram);
GDEXTRA_LOCAL GDEX_FUNCTION(imagehistgram216);
GDEXTRA_LOCAL GDEX_FUNCTION(imagechannelextract);
GDEXTRA_LOCAL GDEX_FUNCTION(imagechannelmerge);
GDEXTRA_LOCAL GDEX_FUNCTION(imagealphamask);
GDEXTRA_LOCAL GDEX_FUNCTION(imagebmp);
GDEXTRA_LOCAL GDEX_FUNCTION(imageicon);
GDEXTRA_LOCAL GDEX_FUNCTION(imagepalettetotruecolor);
GDEXTRA_LOCAL GDEX_FUNCTION(imagetowebsafepalette);
GDEXTRA_LOCAL GDEX_FUNCTION(imagecolorallocatecss);
GDEXTRA_LOCAL GDEX_FUNCTION(imagecolorallocatecmyk);
GDEXTRA_LOCAL GDEX_FUNCTION(imagecolorallocatehsl);
GDEXTRA_LOCAL GDEX_FUNCTION(imagecolorallocatehsv);
GDEXTRA_LOCAL GDEX_FUNCTION(imagecolorcorrect);
GDEXTRA_LOCAL GDEX_FUNCTION(imageflip);
GDEXTRA_LOCAL GDEX_FUNCTION(imagescale);
#if PHP_GDEXTRA_WITH_LQR
GDEXTRA_LOCAL GDEX_FUNCTION(imagecarve);
#endif

GDEXTRA_LOCAL PHP_METHOD(ColorUtility, cmykToRgb);
GDEXTRA_LOCAL PHP_METHOD(ColorUtility, hslToRgb);
GDEXTRA_LOCAL PHP_METHOD(ColorUtility, hsvToRgb);
GDEXTRA_LOCAL PHP_METHOD(ColorUtility, rgbToCmyk);
GDEXTRA_LOCAL PHP_METHOD(ColorUtility, rgbToHsl);
GDEXTRA_LOCAL PHP_METHOD(ColorUtility, rgbToHsv);
GDEXTRA_LOCAL PHP_METHOD(ColorUtility, parseCssColor);
GDEXTRA_LOCAL PHP_METHOD(ColorUtility, getSvgColorTable);

/* }}} */

END_EXTERN_C()

#if GDEXTRA_USE_WRAPPERS
#include "gdex_wrappers.h"
#endif

/* load common inline functions */
#include "inline.h"

#endif /* _PHP_GDEXTRA_H_ */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
