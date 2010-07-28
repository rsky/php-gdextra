/*
 * Extra image functions: color correction functions
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
#include "spline.h"

#define GAMMA_MAX 1000.0
#define GAMMA_MIN 0.001

#define COLORCORRECT_PARAMETERS \
	gdImagePtr im, HashTable *params TSRMLS_DC

#define COLORCORRECT_PARAMS_PASSTHRU \
	im, params TSRMLS_CC

/* {{{ private type definitions */

typedef enum _correct_result {
	CORRECT_SUCCESS =  1,
	CORRECT_NOTHING =  0,
	CORRECT_ERROR   = -1
} correct_result;

/* }}} */

/* {{{ private function prototypes */

static int
_get_levels(zval *zv TSRMLS_DC, float *inclination,
            float *min_in,  float *max_in,  float *in_range,
            float *min_out, float *max_out, float *out_range);

static float
_get_rgamma(zval *zv);

static float
_get_rotation(zval *zv);

static spline_t *
_get_tonecurve(zval *zv, zend_bool no_edge TSRMLS_DC);

static correct_result
_get_parameters(HashTable *ht TSRMLS_DC,
                int *levels, float *inclination,
                float *min_in,  float *max_in,  float *in_range,
                float *min_out, float *max_out, float *out_range,
                float *r_gamma, spline_t **tonecurve, int *negate);

static correct_result
_color_correct_rgb(COLORCORRECT_PARAMETERS),
_color_correct_hsv(COLORCORRECT_PARAMETERS, zend_bool is_hsl),
_color_correct_cmyk(COLORCORRECT_PARAMETERS),
_color_correct_alpha(COLORCORRECT_PARAMETERS);

/* }}} */

/* {{{ _get_levels()
 * Get levels.
 */
static int
_get_levels(zval *zv TSRMLS_DC, float *inclination,
            float *min_in,  float *max_in,  float *in_range,
            float *min_out, float *max_out, float *out_range)
{
	long mni = 0L, mxi = 255L, mno = 0L, mxo = 255L;
	float irn, orn;
	HashTable *levels;
	zval **entry;
	int n;

	if (zv == NULL || Z_TYPE_P(zv) != IS_ARRAY) {
		goto invalid_levels;
	}

	levels = Z_ARRVAL_P(zv);
	n = zend_hash_num_elements(levels);
	if (n != 2 && n != 4) {
		goto invalid_levels;
	}

	/* get the minimum input value from index 0 */
	if (zend_hash_index_find(levels, 0, (void**)&entry) == FAILURE) {
		goto invalid_levels;
	}
	mni = gdex_get_lval(*entry);

	/* get the maximum input value from index 1 */
	if (zend_hash_index_find(levels, 1, (void**)&entry) == FAILURE) {
		goto invalid_levels;
	}
	mxi = gdex_get_lval(*entry);

	/* get the output values */
	if (n == 4) {
		/* get the minimum output value from index 2 */
		if (zend_hash_index_find(levels, 2, (void**)&entry) == FAILURE) {
			goto invalid_levels;
		}
		mno = gdex_get_lval(*entry);

		/* get the maximum output value from index 3 */
		if (zend_hash_index_find(levels, 3, (void**)&entry) == FAILURE) {
			goto invalid_levels;
		}
		mxo = gdex_get_lval(*entry);
	}

	/* verify */
	if (mni < 0L) {
		mni = 0L;
	}
	if (mxi > 255L) {
		mxi = 255L;
	}
	if (mno < 0L) {
		mno = 0L;
	}
	if (mxo > 255L) {
		mxo = 255L;
	}
	if (mni >= mxi || mno >= mxo) {
		goto invalid_levels;
	}

	irn = (float)(mxi - mni);
	orn = (float)(mxo - mno);

	*min_in = (float)mni / 255.0f;
	*max_in = (float)mxi / 255.0f;
	*in_range = irn / 255.0f;
	*min_out = (float)mno / 255.0f;
	*max_out = (float)mxo / 255.0f;
	*out_range = orn / 255.0f;
	*inclination = orn / irn;

	return SUCCESS;

  invalid_levels:
	php_error_docref(NULL TSRMLS_CC, E_WARNING,
			"Invalid levels option given");
	return FAILURE;
}
/* }}} */

/* {{{ _get_rgamma()
 * Get reciprocal gamma.
 */
static float
_get_rgamma(zval *zv)
{
	double gamma = gdex_get_dval(zv);

	if (zend_finite(gamma) && gamma > 0.0) {
		if (gamma > GAMMA_MAX) {
			gamma = GAMMA_MAX;
		} else if (gamma < GAMMA_MIN) {
			gamma = GAMMA_MIN;
		}
		return (float)(1.0 / gamma);
	}

	return 1.0f;
}
/* }}} */

/* {{{ _get_rotation()
 * Get hue rotation.
 */
static float
_get_rotation(zval *zv)
{
	float rotation = 0.0f;

	if (Z_TYPE_P(zv) == IS_LONG) {
		rotation = (float)(Z_LVAL_P(zv) % 360L) / 360.0f;
	} else {
		double degrees, di;

		degrees = gdex_get_dval(zv);
		if (zend_finite(degrees)) {
			rotation = (float)modf(degrees / 360.0, &di);
		}
	}

	if (rotation < 0.0f) {
		rotation += 1.0f;
	}
	return rotation;
}
/* }}} */

/* {{{ _get_tonecurve()
 * Get tone curve.
 */
static spline_t *
_get_tonecurve(zval *zv, zend_bool no_edge TSRMLS_DC)
{
	HashTable *points;
	HashPosition pos;
	zval **entry, **zx, **zy;
	spline_t *spl;

	/* verify */
	if (zv == NULL || Z_TYPE_P(zv) != IS_ARRAY) {
		goto invalid_curve;
	}

	points = Z_ARRVAL_P(zv);
	if (zend_hash_num_elements(points) < ((no_edge) ? 1 : 2)) {
		goto invalid_curve;
	}

	/* create a spline */
	spl = spline_new();
	if (spl == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Failed to create a tone curve");
		return NULL;
	}

	/* set the first point */
	if (no_edge) {
		spline_add_point(spl, 0.0, 0.0);
	}

	/* add points */
	zend_hash_internal_pointer_reset_ex(points, &pos);
	while (zend_hash_get_current_data_ex(points, (void **)&entry, &pos) == SUCCESS) {
		if (Z_TYPE_PP(entry) != IS_ARRAY) {
			goto invalid_curve;
		}
		if (zend_hash_num_elements(Z_ARRVAL_PP(entry)) != 2 ||
			zend_hash_index_find(Z_ARRVAL_PP(entry), 0, (void**)&zx) == FAILURE ||
			zend_hash_index_find(Z_ARRVAL_PP(entry), 1, (void**)&zy) == FAILURE)
		{
			goto invalid_curve;
		}
		if (!spline_add_point(spl, gdex_get_dval(*zx), gdex_get_dval(*zy))) {
			goto invalid_curve;
		}
		zend_hash_move_forward_ex(points, &pos);
	}

	/* set the terminal point */
	if (no_edge && !spline_add_point(spl, 1.0, 1.0)) {
		goto invalid_curve;
	}

	/* close a spline */
	if (!spline_close(spl)) {
		spline_destroy(spl);
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Failed to close a tone curve");
		return NULL;
	}

	return spl;

  invalid_curve:
	if (spl != NULL)  {
		spline_destroy(spl);
	}
	php_error_docref(NULL TSRMLS_CC, E_WARNING,
			"Invalid tone curve option given");
	return NULL;
}
/* }}} */

/* {{{ _get_parameters()
 * Get color correction parameters.
 */
static correct_result
_get_parameters(HashTable *ht TSRMLS_DC,
                int *levels, float *inclination,
                float *min_in,  float *max_in,  float *in_range,
                float *min_out, float *max_out, float *out_range,
                float *r_gamma, spline_t **tonecurve, int *negate)
{
	zval **entry = NULL;
	spline_t *spl = NULL;
	int nparams = 0;

	/* get levels */
	if (hash_find(ht, "levels", &entry) == SUCCESS) {
		if (_get_levels(*entry TSRMLS_CC, inclination,
		                min_in,  max_in,  in_range,
		                min_out, max_out, out_range) == FAILURE)
		{
			return CORRECT_ERROR;
		}
		*levels = 1;
		nparams++;
	}

	/* get reciprocal gamma */
	if (hash_find(ht, "gamma", &entry) == SUCCESS) {
		*r_gamma = _get_rgamma(*entry);
		nparams++;
	}

	/* get tone curve */
	if (hash_find(ht, "tonecurve2", &entry) == SUCCESS) {
		spl = _get_tonecurve(*entry, 0 TSRMLS_CC);
		if (spl == NULL) {
			return CORRECT_ERROR;
		}
		*tonecurve = spl;
		nparams++;
	} else if (hash_find(ht, "tonecurve", &entry) == SUCCESS) {
		spl = _get_tonecurve(*entry, 1 TSRMLS_CC);
		if (spl == NULL) {
			return CORRECT_ERROR;
		}
		*tonecurve = spl;
		nparams++;
	}

	/* get negation */
	if (hash_find(ht, "negate", &entry) == SUCCESS) {
		*negate = zval_is_true(*entry);
		nparams++;
	}

	return (nparams > 0) ? CORRECT_SUCCESS : CORRECT_NOTHING;
}
/* }}} */

/* {{{ macros for declaration of variables */

#define COLORCORRECT_DECLARE_COMMON() \
	zval **entry = NULL; \
	correct_result has_params = CORRECT_NOTHING;

#define COLORCORRECT_DECLARE_EX(_Z) \
	    float mni##_Z = 0.0f; /* minimum input   [0..1] */ \
	    float mxi##_Z = 1.0f; /* maximum input   [0..1] */ \
	    float irn##_Z = 1.0f; /* input range     [0..1] */ \
	    float mno##_Z = 0.0f; /* minimum output  [0..1] */ \
	    float mxo##_Z = 1.0f; /* maximum output  [0..1] */ \
	    float orn##_Z = 1.0f; /* output range    [0..1] */ \
	    float icl##_Z = 1.0f; /* inclination (orn/irn)  */ \
	    float rgm##_Z = 1.0f; /* reciprocal gamma (> 0) */ \
	      int lvl##_Z = 0;    /* levels (bool)          */ \
	      int ngt##_Z = 0;    /* negate (bool)          */ \
	spline_t *tcv##_Z = NULL; /* tone curve (3D spline) */

#define COLORCORRECT_DECLARE(_z, _Z) \
	float _z = 0.0f; /* channel value   [0..1] */ \
	COLORCORRECT_DECLARE_EX(_Z)

/* }}} */

/* {{{ macros for fetching parameters */

#define COLORCORRECT_GETOPT_SP(_Z, _ht, _on_failure) \
	has_params = _get_parameters((_ht) TSRMLS_CC, \
			&lvl##_Z, &icl##_Z, \
			&mni##_Z, &mxi##_Z, &irn##_Z, \
			&mno##_Z, &mxo##_Z, &orn##_Z, \
			&rgm##_Z, &tcv##_Z, &ngt##_Z); \
	if (has_params == CORRECT_ERROR) { \
		_on_failure; \
	}

#define COLORCORRECT_GETOPT_EX(_Z, _ht) \
	COLORCORRECT_GETOPT_SP(_Z, _ht, return CORRECT_ERROR)

#define COLORCORRECT_GETOPT(_z, _Z) \
	if (hash_find(params, #_z, &entry) == SUCCESS) { \
		HashTable *params##_Z = HASH_OF(*entry); \
		if (params##_Z != NULL) { \
			COLORCORRECT_GETOPT_EX(_Z, params##_Z); \
		} \
	}

#define COLORCORRECT_SET_LEVELS(_X, _Y) \
	mni##_X = mni##_Y; \
	mxi##_X = mxi##_Y; \
	irn##_X = irn##_Y; \
	mno##_X = mno##_Y; \
	mxo##_X = mxo##_Y; \
	orn##_X = orn##_Y; \
	icl##_X = icl##_Y; \
	lvl##_X = lvl##_Y;

#define COLORCORRECT_SET_GAMMA(_X, _Y) rgm##_X = rgm##_Y;

#define COLORCORRECT_SET_TONECURVE(_X, _Y) tcv##_X = tcv##_Y;

#define COLORCORRECT_SET_NEGATE(_X, _Y) ngt##_X = ngt##_Y;

#define COLORCORRECT_FREE_TONECURVE(_X) \
	if (tcv##_X != NULL) { \
		spline_destroy(tcv##_X); \
	}

#define COLORCORRECT_FREE_TONECURVE2(_X, _Y) \
	if (tcv##_X != NULL && tcv##_X != tcv##_Y) { \
		spline_destroy(tcv##_X); \
	}

/* }}} */

/* {{{ macros for color correction */

#define COLORCORRECT_TO_TRUECOLOR(_i) \
	if (!gdImageTrueColor((_i))) { \
		if (gdex_palette_to_truecolor((_i) TSRMLS_CC) == FAILURE) { \
			return CORRECT_ERROR; \
		} \
	}

#define COLORCORRECT_ITERATE_BEGIN() { \
	int ic, ix, iy, width, height; \
	width = gdImageSX(im); \
	height = gdImageSY(im); \
	for (iy = 0; iy < height; iy++) { \
		for (ix = 0; ix < width; ix++) { \
			ic = unsafeGetTrueColorPixel(im, ix, iy);

#define COLORCORRECT_ITERATE_END(r, g, b, a) \
			unsafeSetTrueColorPixel(im, ix, iy, gdTrueColorAlpha((r), (g), (b), (a))); \
		} /* x */ \
	} /* y */ \
} /* block */

#define COLORCORRECT_DO(_z, _Z) { \
	if (lvl##_Z) { \
		if (_z <= mni##_Z) { \
			_z = mno##_Z; \
		} else if (_z >= mxi##_Z) { \
			_z = mxo##_Z; \
		} else if (rgm##_Z != 1.0f) { \
			_z = mno##_Z + orn##_Z * powf((_z - mni##_Z) / irn##_Z, rgm##_Z); \
		} else { \
			_z = mno##_Z + icl##_Z * (_z - mni##_Z); \
		} \
	} else if (rgm##_Z != 1.0f) { \
		_z = powf(_z, rgm##_Z); \
	} \
	if (tcv##_Z != NULL) { \
		_z = (float)spline_interpolate(tcv##_Z, (double)_z); \
	} \
	if (ngt##_Z) { \
		_z = 1.0f - _z; \
	} \
}

/* }}} */

/* {{{ _color_correct_rgb()
 * Correct color in RGB color space.
 */
static correct_result
_color_correct_rgb(COLORCORRECT_PARAMETERS)
{
	COLORCORRECT_DECLARE_COMMON();
	COLORCORRECT_DECLARE_EX(V);
	COLORCORRECT_DECLARE(r, R);
	COLORCORRECT_DECLARE(g, G);
	COLORCORRECT_DECLARE(b, B);

	/* get common parameters */
	COLORCORRECT_GETOPT_EX(V, params);
	if (lvlV) {
		COLORCORRECT_SET_LEVELS(R, V);
		COLORCORRECT_SET_LEVELS(G, V);
		COLORCORRECT_SET_LEVELS(B, V);
	}
	if (rgmV != 1.0f) {
		COLORCORRECT_SET_GAMMA(R, V);
		COLORCORRECT_SET_GAMMA(G, V);
		COLORCORRECT_SET_GAMMA(B, V);
	}
	if (tcvV != NULL) {
		COLORCORRECT_SET_TONECURVE(R, V);
		COLORCORRECT_SET_TONECURVE(G, V);
		COLORCORRECT_SET_TONECURVE(B, V);
	}
	if (ngtV) {
		COLORCORRECT_SET_NEGATE(R, V);
		COLORCORRECT_SET_NEGATE(G, V);
		COLORCORRECT_SET_NEGATE(B, V);
	}

	/* get channel specific parameters */
	COLORCORRECT_GETOPT(r, R);
	COLORCORRECT_GETOPT(g, G);
	COLORCORRECT_GETOPT(b, B);

	if (has_params == CORRECT_NOTHING) {
		return CORRECT_NOTHING;
	}

	/* convert to true color */
	COLORCORRECT_TO_TRUECOLOR(im);

	/* correct */
	COLORCORRECT_ITERATE_BEGIN();
	r = (float)getR(ic) / 255.0f;
	g = (float)getG(ic) / 255.0f;
	b = (float)getB(ic) / 255.0f;

	COLORCORRECT_DO(r, R);
	COLORCORRECT_DO(g, G);
	COLORCORRECT_DO(b, B);
	COLORCORRECT_ITERATE_END(_float2byte(r), _float2byte(g), _float2byte(b), getA(ic));

	/* cleanup */
	COLORCORRECT_FREE_TONECURVE2(R, V);
	COLORCORRECT_FREE_TONECURVE2(G, V);
	COLORCORRECT_FREE_TONECURVE2(B, V);
	COLORCORRECT_FREE_TONECURVE(V);

	return CORRECT_SUCCESS;
}
/* }}} */

/* {{{ _color_correct_hsv()
 * Correct color in HSV/HSL color space.
 */
static correct_result
_color_correct_hsv(COLORCORRECT_PARAMETERS, zend_bool is_hsl)
{
	COLORCORRECT_DECLARE_COMMON();
	int r = 0, g = 0, b = 0;
	float h = 0.0f, rotH = 0.0f;
	COLORCORRECT_DECLARE(s, S);
	COLORCORRECT_DECLARE(v, V);
	gdex_rgb_to_3ch_func_t rgb2hsv;
	gdex_3ch_to_rgb_func_t hsv2rgb;

	/* get parameters */
	if (hash_find(params, "h", &entry) == SUCCESS) {
		rotH += _get_rotation(*entry);
		if (rotH >= 1.0f) {
			rotH -= 1.0f;
		}
		has_params = CORRECT_SUCCESS;
	}
	COLORCORRECT_GETOPT(s, S);
	if (is_hsl) {
		COLORCORRECT_GETOPT(l, V);
	} else {
		COLORCORRECT_GETOPT(v, V);
	}
	if (has_params == CORRECT_NOTHING) {
		return CORRECT_NOTHING;
	}

	/* convert to true color */
	COLORCORRECT_TO_TRUECOLOR(im);

	/* determine conversion functions */
	if (is_hsl) {
		rgb2hsv = gdex_rgb_to_hsl;
		hsv2rgb = gdex_hsl_to_rgb;
	} else {
		rgb2hsv = gdex_rgb_to_hsv;
		hsv2rgb = gdex_hsv_to_rgb;
	}

	/* correct */
	COLORCORRECT_ITERATE_BEGIN();
	rgb2hsv(getR(ic), getG(ic), getB(ic), &h, &s, &v);

	if (rotH != 0.0f) {
		h += rotH;
		if (h >= 1.0f) {
			h -= 1.0f;
		}
	}
	COLORCORRECT_DO(s, S);
	COLORCORRECT_DO(v, V);

	hsv2rgb(h, s, v, &r, &g, &b);
	COLORCORRECT_ITERATE_END(r, g, b, getA(ic));

	/* cleanup */
	COLORCORRECT_FREE_TONECURVE(S);
	COLORCORRECT_FREE_TONECURVE(V);

	return CORRECT_SUCCESS;
}
/* }}} */

/* {{{ _color_correct_cmyk()
 * Correct color in CMYK color space.
 */
static correct_result
_color_correct_cmyk(COLORCORRECT_PARAMETERS)
{
	COLORCORRECT_DECLARE_COMMON();
	int r = 0, g = 0, b = 0;
	COLORCORRECT_DECLARE(c, C);
	COLORCORRECT_DECLARE(m, M);
	COLORCORRECT_DECLARE(y, Y);
	COLORCORRECT_DECLARE(k, K);

	/* get parameters */
	COLORCORRECT_GETOPT(c, C);
	COLORCORRECT_GETOPT(m, M);
	COLORCORRECT_GETOPT(y, Y);
	COLORCORRECT_GETOPT(k, K);

	if (has_params == CORRECT_NOTHING) {
		return CORRECT_NOTHING;
	}

	/* convert to true color */
	COLORCORRECT_TO_TRUECOLOR(im);

	/* correct */
	COLORCORRECT_ITERATE_BEGIN();
	gdex_rgb_to_cmyk(getR(ic), getG(ic), getB(ic), &c, &m, &y, &k);

	COLORCORRECT_DO(c, C);
	COLORCORRECT_DO(m, M);
	COLORCORRECT_DO(y, Y);
	COLORCORRECT_DO(k, K);

	gdex_cmyk_to_rgb(c, m, y, k, &r, &g, &b);
	COLORCORRECT_ITERATE_END(r, g, b, getA(ic));

	/* cleanup */
	COLORCORRECT_FREE_TONECURVE(C);
	COLORCORRECT_FREE_TONECURVE(M);
	COLORCORRECT_FREE_TONECURVE(Y);
	COLORCORRECT_FREE_TONECURVE(K);

	return CORRECT_SUCCESS;
}
/* }}} */

/* {{{ _color_correct_alpha()
 * Correct alpha channel.
 */
static correct_result
_color_correct_alpha(COLORCORRECT_PARAMETERS)
{
	COLORCORRECT_DECLARE_COMMON();
	COLORCORRECT_DECLARE(a, A);

	/* get parameters */
	COLORCORRECT_GETOPT(a, A);
	if (has_params == CORRECT_NOTHING) {
		return CORRECT_NOTHING;
	}

	/* convert to true color */
	COLORCORRECT_TO_TRUECOLOR(im);

	/* correct */
	COLORCORRECT_ITERATE_BEGIN();
	a = (float)(gdAlphaMax - getA(ic)) / (float)gdAlphaMax;

	COLORCORRECT_DO(a, A);
	COLORCORRECT_ITERATE_END(getR(ic), getG(ic), getB(ic), _float2alpha(a));

	/* cleanup */
	COLORCORRECT_FREE_TONECURVE(A);

	return CORRECT_SUCCESS;
}
/* }}} */

/* {{{ proto bool imagecolorcorrect_ex(resource im, array params[, int colorspace])
 * Correct color.
 */
GDEXTRA_LOCAL PHP_FUNCTION(imagecolorcorrect_ex)
{
	zval *zim = NULL;
	gdImagePtr im = NULL;
	zval *zparams = NULL;
	HashTable *params = NULL;
	long orig_colorspace = COLORSPACE_RGB;
	int colorspace;
	int use_alpha = 0;
	correct_result result = CORRECT_NOTHING;

	/* parse the arguments */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ra|l",
			&zim, &zparams, &orig_colorspace) == FAILURE)
	{
		return;
	}
	ZEND_FETCH_RESOURCE(im, gdImagePtr, &zim, -1, "Image", phpi_get_le_gd());
	params = Z_ARRVAL_P(zparams);

	/* verify the color space */
	if (orig_colorspace & COLORSPACE_ALPHA) {
		use_alpha = 1;
		colorspace = (int)(orig_colorspace ^ COLORSPACE_ALPHA);
	} else {
		colorspace = (int)orig_colorspace;
	}

	/* correct color */
	switch (colorspace) {
		case COLORSPACE_RGB:
			result = _color_correct_rgb(COLORCORRECT_PARAMS_PASSTHRU);
			break;
		case COLORSPACE_HSV:
			result = _color_correct_hsv(COLORCORRECT_PARAMS_PASSTHRU, 0);
			break;
		case COLORSPACE_HSL:
			result = _color_correct_hsv(COLORCORRECT_PARAMS_PASSTHRU, 1);
			break;
		case COLORSPACE_CMYK:
			result = _color_correct_cmyk(COLORCORRECT_PARAMS_PASSTHRU);
			break;
		default:
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"Unsupported color space given (%ld)", orig_colorspace);
			result = CORRECT_ERROR;
	}

	if (result == CORRECT_ERROR) {
		RETURN_FALSE;
	}

	/* correct alpha channel */
	if (use_alpha) {
		switch (_color_correct_alpha(COLORCORRECT_PARAMS_PASSTHRU)) {
			case CORRECT_SUCCESS:
				result = CORRECT_SUCCESS;
				break;
			case CORRECT_ERROR:
				RETURN_FALSE;
		}
	}

	if (result == CORRECT_NOTHING) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Nothing to do.");
	}

	RETURN_BOOL(result == CORRECT_SUCCESS);
}
/* }}} */

#if PHP_GDEXTRA_TESTING
/* {{{ proto array colorcorrectiontest(array params[, bool get_as_float])
 * Simulate color correction.
 */
GDEXTRA_LOCAL PHP_FUNCTION(colorcorrectiontest)
{
	zval *zparams = NULL;
	HashTable *params = NULL;
	zend_bool get_as_float = 0;
	int i = 0;
	COLORCORRECT_DECLARE_COMMON();
	COLORCORRECT_DECLARE(x, X);

	/* parse the arguments */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a|b",
			&zparams, &get_as_float) == FAILURE)
	{
		return;
	}
	params = Z_ARRVAL_P(zparams);

	/* get parameters */
	COLORCORRECT_GETOPT_SP(X, params, RETURN_FALSE);
	if (has_params == CORRECT_NOTHING) {
		RETURN_FALSE;
	}

	/* simulate */
	gdex_array_init_size(return_value, 256);
	while (i < 256) {
		x = (float)i / 255.0f;
		COLORCORRECT_DO(x, X);
		if (get_as_float) {
			add_next_index_double(return_value, (double)x);
		} else {
			add_next_index_long(return_value, (long)_float2byte(x));
		}
		i++;
	}
}
/* }}} */
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
