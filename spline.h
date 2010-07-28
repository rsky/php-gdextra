/*
 * Functions for 3D spline interpolation.
 *
 * Copyright (c) 2009 Ryusuke SEKIYAMA. All rights reserved.
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
 * @copyright   2009 Ryusuke SEKIYAMA
 * @license     http://www.opensource.org/licenses/mit-license.php  MIT License
 * @version     SVN: $Id$
 */

#ifndef _SPLINE_H_
#define _SPLINE_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_GDEXTRA
#include "php_gdextra.h"
#define SPLINE_PUBLIC   GDEXTRA_LOCAL
#define spline_malloc   emalloc
#define spline_calloc   ecalloc
#define spline_realloc  erealloc
#define spline_free     efree
#define spline_isfinite zend_finite
#else
#ifndef SPLINE_PUBLIC
#define SPLINE_PUBLIC
#endif
#ifndef spline_malloc
#include <stdlib.h>
#define spline_malloc   malloc
#define spline_calloc   calloc
#define spline_realloc  realloc
#define spline_free     free
#endif
#ifndef spline_isfinite
#include <math.h>
#define spline_isfinite isfinite
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* {{{ type definitions */

typedef struct _spline_t spline_t;

/* }}} */

/* {{{ public function prototypes */

/*
 * Constructor.
 */
SPLINE_PUBLIC spline_t *
spline_new(void);

/*
 * Add a point.
 * 'x' must be greater than the last added one.
 */
SPLINE_PUBLIC int
spline_add_point(spline_t *spl, double x, double y);

/*
 * Counter.
 */
SPLINE_PUBLIC unsigned int
spline_num_points(const spline_t *spl);

/*
 * Get the x and y coordinate of the first point.
 */
SPLINE_PUBLIC int
spline_get_first_point(const spline_t *spl, double *x, double *y);

/*
 * Get the x and y coordinate of the last point.
 */
SPLINE_PUBLIC int
spline_get_last_point(const spline_t *spl, double *x, double *y);

/*
 * Get the x and y coordinate of the specified point.
 */
SPLINE_PUBLIC int
spline_get_nth_point(const spline_t *spl, unsigned int n, double *x, double *y);

/*
 * Determine whether the spline is closed.
 */
SPLINE_PUBLIC int
spline_is_closed(const spline_t *spl);

/*
 * Finalizer.
 */
SPLINE_PUBLIC int
spline_close(spline_t *spl);

/*
 * Reactivator.
 */
SPLINE_PUBLIC void
spline_reopen(spline_t *spl);

/*
 * Get an interpolated value.
 * 'spl' must be closed by spline_close().
 */
SPLINE_PUBLIC double
spline_interpolate(const spline_t *spl, double x);

/*
 * Destructor.
 */
SPLINE_PUBLIC void
spline_destroy(spline_t *spl);

/* }}} */

#ifdef __cplusplus
}
#endif

#endif /* _SPLINE_H_ */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
