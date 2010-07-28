/*
 * Extra image functions: GD's function wrappers
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

#ifndef _PHP_GDEXTRA_WRAPPERS_H_
#define _PHP_GDEXTRA_WRAPPERS_H_

#include "php_gdextra.h"

BEGIN_EXTERN_C()

GDEXTRA_LOCAL int
gdex_wrappers_init(int module_number TSRMLS_DC);

GDEXTRA_LOCAL void
gdex_wrappers_shutdown(TSRMLS_D);

GDEXTRA_LOCAL gdImagePtr gdex_gdImageCreate(int sx, int sy, zend_bool truecolor);
GDEXTRA_LOCAL void gdex_gdImageDestroy(gdImagePtr im);
GDEXTRA_LOCAL int gdex_gdImageColorResolveAlpha(gdImagePtr im, int r, int g, int b, int a);
GDEXTRA_LOCAL void gdex_gdImageCopy(gdImagePtr dst, gdImagePtr src, int dstX, int dstY, int srcX, int srcY, int w, int h);
GDEXTRA_LOCAL void gdex_gdImageCopyResampled(gdImagePtr dst, gdImagePtr src, int dstX, int dstY, int srcX, int srcY, int dstW, int dstH, int srcW, int srcH);
GDEXTRA_LOCAL void gdex_gdImageSaveAlpha(gdImagePtr im, int saveAlphaArg);

END_EXTERN_C()

#if GDEXTRA_USE_WRAPPERS

#undef gdImageCreate
#undef gdImageCreateTrueColor
#undef gdImageDestroy
#undef gdImageColorResolveAlpha
#undef gdImageCopy
#undef gdImageCopyResampled
#undef gdImageSaveAlpha

#define gdImageCreate(sx, sy) gdex_gdImageCreate((sx), (sy), 0)
#define gdImageCreateTrueColor(sx, sy) gdex_gdImageCreate((sx), (sy), 1)
#define gdImageDestroy gdex_gdImageDestroy
#define gdImageColorResolveAlpha gdex_gdImageColorResolveAlpha
#define gdImageCopy gdex_gdImageCopy
#define gdImageCopyResampled gdex_gdImageCopyResampled
#define gdImageSaveAlpha gdex_gdImageSaveAlpha

#endif /* GDEXTRA_USE_WRAPPERS */

#endif /* _PHP_GDEXTRA_WRAPPERS_H_ */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
