/*
 * Extra image functions: GD's function wrappers
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

#include "gdex_wrappers.h"

/* {{{ globals */

static int le_fake_rsrc, le_gd;
static zval _fn_imagecreate, _fn_imagecreatetruecolor,
            _fn_imagecolorresolvealpha, _fn_imagecopy, _fn_imagecopyresampled;

/* }}} */
/* {{{ macros */

#define MAKE_FUNC_NAME(name) \
	ZVAL_STRINGL(&_fn_##name, #name, sizeof(#name) - 1, 1)

#define FREE_FUNC_NAME(name) efree(Z_STRVAL(_fn_##name))

#define COMMON_VARS(num_args) \
	zval *argv[num_args], **argp[num_args], *retval = NULL; \
	zend_uint argc = 0;

#define MAKE_ARG_RSRC(im) \
	MAKE_STD_ZVAL(argv[argc]); \
	ZEND_REGISTER_RESOURCE(argv[argc], (im), le_gd); \
	argp[argc] = &argv[argc]; \
	argc++;

#define MAKE_ARG_LONG(n) \
	MAKE_STD_ZVAL(argv[argc]); \
	ZVAL_LONG(argv[argc], (n)); \
	argp[argc] = &argv[argc]; \
	argc++;

#define MAKE_ARG_BOOL(b) \
	MAKE_STD_ZVAL(argv[argc]); \
	ZVAL_BOOL(argv[argc], (b)); \
	argp[argc] = &argv[argc]; \
	argc++;

#define FREE_ARGS() \
	while (argc > 0) { \
		zval_ptr_dtor(argp[--argc]); \
	}

#define CALL_FUNCTION_EX(name) \
	call_user_function_ex(CG(function_table), NULL, (name), &retval, argc, argp, 0, NULL TSRMLS_CC)

#define CALL_FUNCTION(name) CALL_FUNCTION_EX(&_fn_##name)

/* }}} */
/* {{{ _rsrc_to_fake() */

static void
_rsrc_to_fake(zval **rsrc TSRMLS_DC)
{
	zend_rsrc_list_entry *le;

	if (zend_hash_index_find(&EG(regular_list), (ulong)Z_LVAL_PP(rsrc), (void **)&le) == SUCCESS) {
		le->ptr = NULL;
		le->type = le_fake_rsrc;
	}
}

/* }}} */
/* {{{ gdex_wrappers_init() */

GDEXTRA_LOCAL int
gdex_wrappers_init(int module_number TSRMLS_DC)
{
	le_fake_rsrc = zend_register_list_destructors(NULL, NULL, module_number);
	le_gd = phpi_get_le_gd();

	INIT_ZVAL(_fn_imagecreate);
	INIT_ZVAL(_fn_imagecreatetruecolor);
	INIT_ZVAL(_fn_imagecolorresolvealpha);
	INIT_ZVAL(_fn_imagecopy);
	INIT_ZVAL(_fn_imagecopyresampled);
	MAKE_FUNC_NAME(imagecreate);
	MAKE_FUNC_NAME(imagecreatetruecolor);
	MAKE_FUNC_NAME(imagecolorresolvealpha);
	MAKE_FUNC_NAME(imagecopy);
	MAKE_FUNC_NAME(imagecopyresampled);

	return le_fake_rsrc;
}

/* }}} */
/* {{{ gdex_wrappers_shutdown() */

GDEXTRA_LOCAL void
gdex_wrappers_shutdown(TSRMLS_D)
{
	FREE_FUNC_NAME(imagecreate);
	FREE_FUNC_NAME(imagecreatetruecolor);
	FREE_FUNC_NAME(imagecolorresolvealpha);
	FREE_FUNC_NAME(imagecopy);
	FREE_FUNC_NAME(imagecopyresampled);
}

/* }}} */
/* {{{ gdex_gdImageCreate() */

GDEXTRA_LOCAL gdImagePtr
gdex_gdImageCreate(int sx, int sy, zend_bool truecolor)
{
	COMMON_VARS(2);
	gdImagePtr im = NULL;
	zval *ctor;
	TSRMLS_FETCH();

	MAKE_ARG_LONG(sx);
	MAKE_ARG_LONG(sy);

	if (truecolor) {
		ctor = &_fn_imagecreatetruecolor;
	} else {
		ctor = &_fn_imagecreate;
	}

	if (CALL_FUNCTION_EX(ctor) == SUCCESS) {
		if (retval != NULL) {
			if (Z_TYPE_P(retval) == IS_RESOURCE) {
				ZEND_FETCH_RESOURCE_NO_RETURN(im, gdImagePtr, &retval, -1, "Image", le_gd);
				_rsrc_to_fake(&retval TSRMLS_CC);
			}
			zval_ptr_dtor(&retval);
		}
	}

	FREE_ARGS();

	return im;
}

/* }}} */
/* {{{ gdex_gdImageDestroy() */

GDEXTRA_LOCAL void
gdex_gdImageDestroy(gdImagePtr im)
{
	zval *zim;
	TSRMLS_FETCH();

	MAKE_STD_ZVAL(zim);
	ZEND_REGISTER_RESOURCE(zim, im, le_gd);
	zval_ptr_dtor(&zim);
}

/* }}} */
/* {{{ gdex_gdImageColorResolveAlpha() */

GDEXTRA_LOCAL int
gdex_gdImageColorResolveAlpha(gdImagePtr im, int r, int g, int b, int a)
{
	if (gdImageTrueColor(im)) {
		return gdTrueColorAlpha(r, g, b, a);
	} else {
		COMMON_VARS(5);
		int color = -1;
		TSRMLS_FETCH();

		MAKE_ARG_RSRC(im);
		MAKE_ARG_LONG(r);
		MAKE_ARG_LONG(g);
		MAKE_ARG_LONG(b);
		MAKE_ARG_LONG(a);

		if (CALL_FUNCTION(imagecolorresolvealpha) == SUCCESS) {
			if (retval != NULL) {
				if (Z_TYPE_P(retval) == IS_LONG) {
					color = (int)Z_LVAL_P(retval);
				}
				zval_ptr_dtor(&retval);
			}
		}

		_rsrc_to_fake(argp[0] TSRMLS_CC);

		FREE_ARGS();

		return color;
	}
}

/* }}} */
/* {{{ gdex_gdImageCopy() */

GDEXTRA_LOCAL void
gdex_gdImageCopy(gdImagePtr dst, gdImagePtr src,
                 int dstX, int dstY, int srcX, int srcY, int w, int h)
{
	COMMON_VARS(8);
	TSRMLS_FETCH();

	MAKE_ARG_RSRC(dst);
	MAKE_ARG_RSRC(src);
	MAKE_ARG_LONG(dstX);
	MAKE_ARG_LONG(dstY);
	MAKE_ARG_LONG(srcX);
	MAKE_ARG_LONG(srcY);
	MAKE_ARG_LONG(w);
	MAKE_ARG_LONG(h);

	if (CALL_FUNCTION(imagecopy) == SUCCESS) {
		if (retval != NULL) {
			zval_ptr_dtor(&retval);
		}
	}

	_rsrc_to_fake(argp[0] TSRMLS_CC);
	_rsrc_to_fake(argp[1] TSRMLS_CC);

	FREE_ARGS();
}

/* }}} */
/* {{{ gdex_gdImageCopyResampled() */

GDEXTRA_LOCAL void
gdex_gdImageCopyResampled(gdImagePtr dst, gdImagePtr src,
                          int dstX, int dstY, int srcX, int srcY,
                          int dstW, int dstH, int srcW, int srcH)
{
	COMMON_VARS(10);
	TSRMLS_FETCH();

	MAKE_ARG_RSRC(dst);
	MAKE_ARG_RSRC(src);
	MAKE_ARG_LONG(dstX);
	MAKE_ARG_LONG(dstY);
	MAKE_ARG_LONG(srcX);
	MAKE_ARG_LONG(srcY);
	MAKE_ARG_LONG(dstW);
	MAKE_ARG_LONG(dstH);
	MAKE_ARG_LONG(srcW);
	MAKE_ARG_LONG(srcH);

	if (CALL_FUNCTION(imagecopyresampled) == SUCCESS) {
		if (retval != NULL) {
			zval_ptr_dtor(&retval);
		}
	}

	_rsrc_to_fake(argp[0] TSRMLS_CC);
	_rsrc_to_fake(argp[1] TSRMLS_CC);

	FREE_ARGS();
}

/* }}} */
/* {{{ gdex_gdImageSaveAlpha() */

GDEXTRA_LOCAL void
gdex_gdImageSaveAlpha(gdImagePtr im, int saveAlphaArg)
{
	im->saveAlphaFlag = saveAlphaArg;
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
