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
#include <stdarg.h>

/* {{{ globals */

ZEND_EXTERN_MODULE_GLOBALS(gdextra);
static int le_fake;

/* }}} */
/* {{{ internal function prototypes */

static zval *
_gdex_init_args(int argc TSRMLS_DC, ...);

static void
_gdex_fake_resource(zval *rsrc TSRMLS_DC);

/* }}} */

/* {{{ gdex_wrappers_init() */

GDEXTRA_LOCAL int
gdex_wrappers_init(INIT_FUNC_ARGS)
{
	le_fake = zend_register_list_destructors(NULL, NULL, module_number);

	return le_fake;
}

/* }}} */
/* {{{ gdex_fcall_info_init() */

GDEXTRA_LOCAL int
gdex_fcall_info_init(const char *name, gdextra_fcall_info *info TSRMLS_DC)
{
	MAKE_STD_ZVAL(info->name);
	ZVAL_STRING(info->name, (char *)name, 1);

#if ZEND_EXTENSION_API_NO >= 220090626
	return zend_fcall_info_init(info->name, 0, &info->fci, &info->fcc, NULL, NULL TSRMLS_CC);
#else
	return zend_fcall_info_init(info->name, &info->fci, &info->fcc TSRMLS_CC);
#endif
}

/* }}} */
/* {{{ gdex_fcall_info_destroy() */

GDEXTRA_LOCAL void
gdex_fcall_info_destroy(gdextra_fcall_info *info TSRMLS_DC)
{
	zval_ptr_dtor(&info->name);
}

/* }}} */
/* {{{ _ex_gdImageCreate() */

GDEXTRA_LOCAL gdImagePtr
_ex_gdImageCreate(int sx, int sy, zend_bool truecolor)
{
	TSRMLS_FETCH();
	gdImagePtr im = NULL;
	zval *retval = NULL, *args, *zsx, *zsy;

	MAKE_STD_ZVAL(zsx);
	MAKE_STD_ZVAL(zsy);
	ZVAL_LONG(zsx, sx);
	ZVAL_LONG(zsy, sy);

	args = _gdex_init_args(2 TSRMLS_CC, zsx, zsy);
	if (truecolor) {
		zend_fcall_info_call(&GDEXG(func_createtruecolor).fci,
		                     &GDEXG(func_createtruecolor).fcc,
		                     &retval, args TSRMLS_CC);
	} else {
		zend_fcall_info_call(&GDEXG(func_create).fci,
		                     &GDEXG(func_create).fcc,
		                     &retval, args TSRMLS_CC);
	}
	if (retval) {
		if (Z_TYPE_P(retval) == IS_RESOURCE) {
			ZEND_FETCH_RESOURCE_NO_RETURN(im, gdImagePtr, &retval, -1, "Image", GDEXG(le_gd));
			_gdex_fake_resource(retval TSRMLS_CC);
		}
		zval_ptr_dtor(&retval);
	}
	zval_ptr_dtor(&args);

	return im;
}

/* }}} */
/* {{{ _ex_gdImageDestroy() */

GDEXTRA_LOCAL void
_ex_gdImageDestroy(gdImagePtr im)
{
	TSRMLS_FETCH();
	zval *zim;

	MAKE_STD_ZVAL(zim);
	ZEND_REGISTER_RESOURCE(zim, im, GDEXG(le_gd));
	zval_ptr_dtor(&zim);
}

/* }}} */
/* {{{ _ex_gdImageColorResolveAlpha() */

GDEXTRA_LOCAL int
_ex_gdImageColorResolveAlpha(gdImagePtr im, int r, int g, int b, int a)
{
	if (gdImageTrueColor(im)) {
		return gdTrueColorAlpha(r, g, b, a);
	} else {
		TSRMLS_FETCH();
		int color = -1;
		zval *retval = NULL, *args, *zim, *zr, *zg, *zb, *za;

		MAKE_STD_ZVAL(zim);
		MAKE_STD_ZVAL(zr);
		MAKE_STD_ZVAL(zg);
		MAKE_STD_ZVAL(zb);
		MAKE_STD_ZVAL(za);
		ZEND_REGISTER_RESOURCE(zim, im, GDEXG(le_gd));
		ZVAL_LONG(zr, r);
		ZVAL_LONG(zg, g);
		ZVAL_LONG(zb, b);
		ZVAL_LONG(za, a);

		args = _gdex_init_args(5 TSRMLS_CC, zim, zr, zg, zb, za);
		zend_fcall_info_call(&GDEXG(func_colorresolvealpha).fci,
		                     &GDEXG(func_colorresolvealpha).fcc,
		                     &retval, args TSRMLS_CC);
		if (retval) {
			if (Z_TYPE_P(retval) == IS_LONG) {
				color = (int)Z_LVAL_P(retval);
			}
			zval_ptr_dtor(&retval);
		}
		_gdex_fake_resource(zim TSRMLS_CC);
		zval_ptr_dtor(&args);

		return color;
	}
}

/* }}} */
/* {{{ _ex_gdImageCopy() */

GDEXTRA_LOCAL void
_ex_gdImageCopy(gdImagePtr dst,
                gdImagePtr src,
                int dstX, int dstY,
                int srcX, int srcY,
                int w,    int h)
{
	TSRMLS_FETCH();
	zval *retval = NULL, *args, *zdst, *zsrc;
	zval *zdstX, *zdstY, *zsrcX, *zsrcY, *zw, *zh;

	MAKE_STD_ZVAL(zdst);
	MAKE_STD_ZVAL(zsrc);
	MAKE_STD_ZVAL(zdstX);
	MAKE_STD_ZVAL(zdstY);
	MAKE_STD_ZVAL(zsrcX);
	MAKE_STD_ZVAL(zsrcY);
	MAKE_STD_ZVAL(zw);
	MAKE_STD_ZVAL(zh);
	ZEND_REGISTER_RESOURCE(zdst, dst, GDEXG(le_gd));
	ZEND_REGISTER_RESOURCE(zsrc, src, GDEXG(le_gd));
	ZVAL_LONG(zdstX, dstX);
	ZVAL_LONG(zdstY, dstY);
	ZVAL_LONG(zsrcX, srcX);
	ZVAL_LONG(zsrcY, srcY);
	ZVAL_LONG(zw, w);
	ZVAL_LONG(zh, h);

	args = _gdex_init_args(8 TSRMLS_CC, zdst, zsrc,
	                       zdstX, zdstY, zsrcX, zsrcY, zw, zh);
	zend_fcall_info_call(&GDEXG(func_copy).fci,
	                     &GDEXG(func_copy).fcc,
	                     &retval, args TSRMLS_CC);
	if (retval) {
		zval_ptr_dtor(&retval);
	}
	_gdex_fake_resource(zdst TSRMLS_CC);
	_gdex_fake_resource(zsrc TSRMLS_CC);
	zval_ptr_dtor(&args);
}

/* }}} */
/* {{{ _ex_gdImageCopyResampled() */

GDEXTRA_LOCAL void
_ex_gdImageCopyResampled(gdImagePtr dst,
                         gdImagePtr src,
                         int dstX, int dstY,
                         int srcX, int srcY,
                         int dstW, int dstH,
                         int srcW, int srcH)
{
	TSRMLS_FETCH();
	zval *retval = NULL, *args, *zdst, *zsrc;
	zval *zdstX, *zdstY, *zsrcX, *zsrcY;
	zval *zdstW, *zdstH, *zsrcW, *zsrcH;

	MAKE_STD_ZVAL(zdst);
	MAKE_STD_ZVAL(zsrc);
	MAKE_STD_ZVAL(zdstX);
	MAKE_STD_ZVAL(zdstY);
	MAKE_STD_ZVAL(zsrcX);
	MAKE_STD_ZVAL(zsrcY);
	MAKE_STD_ZVAL(zdstW);
	MAKE_STD_ZVAL(zdstH);
	MAKE_STD_ZVAL(zsrcW);
	MAKE_STD_ZVAL(zsrcH);
	ZEND_REGISTER_RESOURCE(zdst, dst, GDEXG(le_gd));
	ZEND_REGISTER_RESOURCE(zsrc, src, GDEXG(le_gd));
	ZVAL_LONG(zdstX, dstX);
	ZVAL_LONG(zdstY, dstY);
	ZVAL_LONG(zsrcX, srcX);
	ZVAL_LONG(zsrcY, srcY);
	ZVAL_LONG(zdstW, dstW);
	ZVAL_LONG(zdstH, dstH);
	ZVAL_LONG(zsrcW, srcW);
	ZVAL_LONG(zsrcH, srcH);

	args = _gdex_init_args(10 TSRMLS_CC, zdst, zsrc,
	                       zdstX, zdstY, zsrcX, zsrcY,
	                       zdstW, zdstH, zsrcW, zsrcH);
	zend_fcall_info_call(&GDEXG(func_copyresampled).fci,
	                     &GDEXG(func_copyresampled).fcc,
	                     &retval, args TSRMLS_CC);
	if (retval) {
		zval_ptr_dtor(&retval);
	}
	_gdex_fake_resource(zdst TSRMLS_CC);
	_gdex_fake_resource(zsrc TSRMLS_CC);
	zval_ptr_dtor(&args);
}

/* }}} */
/* {{{ _ex_gdImageSaveAlpha() */

GDEXTRA_LOCAL void
_ex_gdImageSaveAlpha(gdImagePtr im, int saveAlphaArg)
{
	im->saveAlphaFlag = saveAlphaArg;
}

/* }}} */
/* {{{ _gdex_init_args() */

static zval *
_gdex_init_args(int argc TSRMLS_DC, ...)
{
	va_list ap;
	zval *args;

	if (argc < 0) {
		return NULL;
	}

	MAKE_STD_ZVAL(args);
	array_init(args);

#ifdef ZTS
	va_start(ap, TSRMLS_C);
#else
	va_start(ap, argc);
#endif
	while (argc > 0) {
		add_next_index_zval(args, (zval *)va_arg(ap, zval *));
		argc--;
	}
	va_end(ap);

	return args;
}

/* }}} */
/* {{{ _gdex_fake_resource() */

static void
_gdex_fake_resource(zval *rsrc TSRMLS_DC)
{
	zend_rsrc_list_entry *le;

	if (zend_hash_index_find(&EG(regular_list), (ulong)Z_LVAL_P(rsrc), (void **)&le) == SUCCESS) {
		le->ptr = NULL;
		le->type = le_fake;
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
