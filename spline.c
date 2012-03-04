/*
 * Functions for 3D spline interpolation.
 *
 * Copyright (c) 2009-2012 Ryusuke SEKIYAMA. All rights reserved.
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
 * @copyright   2009-2012 Ryusuke SEKIYAMA
 * @license     http://www.opensource.org/licenses/mit-license.php  MIT License
 */

#include "spline.h"

/* {{{ private type definitions */

typedef struct _spline_factor_t {
	double x;   /* x-coordinate */
	double y;   /* y-coordinate */
	double q;
	double r;
	double s;
} spline_factor_t;

struct _spline_t {
	unsigned int m;     /* maximum number of points */
	unsigned int n;     /* number of points */
	spline_factor_t *v; /* factors */
	int closed;         /* (bool) */
};

/* }}} */

#define INITIAL_VECTOR_SIZE 8
#define SPLx(i) spl->v[(i)].x
#define SPLy(i) spl->v[(i)].y
#define SPLq(i) spl->v[(i)].q
#define SPLr(i) spl->v[(i)].r
#define SPLs(i) spl->v[(i)].s

/* {{{ spline_new() */

/*
 * Constructor.
 */
SPLINE_PUBLIC spline_t *
spline_new(void)
{
	spline_t *spl;

	spl = (spline_t *)spline_calloc(1, sizeof(spline_t));
	if (spl == NULL) {
		return NULL;
	}

	spl->v = (spline_factor_t *)spline_malloc(sizeof(spline_factor_t) * INITIAL_VECTOR_SIZE);
	if (spl->v == NULL) {
		spline_free(spl);
		return NULL;
	}
	spl->m = INITIAL_VECTOR_SIZE;

	return spl;
}

/* }}} */
/* {{{ spline_add_point() */

/*
 * Add a point.
 * 'x' must be greater than the last added one.
 */
SPLINE_PUBLIC int
spline_add_point(spline_t *spl, double x, double y)
{
	if (spl->closed) {
		return 0;
	}

	if (!(spline_isfinite(x) && spline_isfinite(y))) {
		return 0;
	}

	if (spl->n > 0 &&  x <= SPLx(spl->n - 1)) {
		return 0;
	}

	if (spl->m == spl->n) {
		spline_factor_t *v;
		unsigned int m;

		m = spl->m << 1;
		if (spl->m > m) {
			return 0;
		}
		v = (spline_factor_t *)spline_realloc(spl->v, sizeof(spline_factor_t) * m);
		if (v == NULL) {
			return 0;
		}
		spl->v = v;
		spl->m = m;
	}

	SPLx(spl->n) = x;
	SPLy(spl->n) = y;
	spl->n++;

	return 1;
}

/* }}} */
/* {{{ spline_num_points() */

/*
 * Counter.
 */
SPLINE_PUBLIC unsigned int
spline_num_points(const spline_t *spl)
{
	return spl->n;
}

/* }}} */
/* {{{ spline_get_first_point() */

/*
 * Get the x and y coordinate of the first point.
 */
SPLINE_PUBLIC int
spline_get_first_point(const spline_t *spl, double *x, double *y)
{
	if (spl->n > 0) {
		*x = SPLx(0);
		*y = SPLy(0);
		return 1;
	} else {
		return 0;
	}
}

/* }}} */
/* {{{ spline_get_last_point() */

/*
 * Get the x and y coordinate of the last point.
 */
SPLINE_PUBLIC int
spline_get_last_point(const spline_t *spl, double *x, double *y)
{
	if (spl->n > 0) {
		*x = SPLx(spl->n - 1);
		*y = SPLy(spl->n - 1);
		return 1;
	} else {
		return 0;
	}
}

/* }}} */
/* {{{ spline_get_nth_point() */

/*
 * Get the x and y coordinate of the specified point.
 */
SPLINE_PUBLIC int
spline_get_nth_point(const spline_t *spl, unsigned int n, double *x, double *y)
{
	if (n > 0 && n <= spl->n) {
		*x = SPLx(n - 1);
		*y = SPLy(n - 1);
		return 1;
	} else {
		return 0;
	}
}

/* }}} */
/* {{{ spline_is_closed() */

/*
 * Determine whether the spline is closed.
 */
SPLINE_PUBLIC int
spline_is_closed(const spline_t *spl)
{
	return spl->closed;
}

/* }}} */
/* {{{ spline_close() */

/*
 * Finalizer.
 */
SPLINE_PUBLIC int
spline_close(spline_t *spl)
{
	unsigned int i, j, k, m, n;
	double *w, *h, *b, *d, *g, *u;
	double l;

	/* verify */
	if (spl->closed) {
		return 1;
	} else if (spl->n < 2) {
		return 0;
	} else if (spl->n == 2) {
		SPLq(0) = (SPLy(1) - SPLy(0)) / (SPLx(1) - SPLx(0));
		SPLr(0) = 0.0;
		SPLs(0) = 0.0;
		SPLq(1) = 0.0;
		SPLr(1) = 0.0;
		SPLs(1) = 0.0;
		spl->closed = 1;
		return 1;
	}
	m = spl->n;
	n = m - 1;

	/* allocate working area */
	w = (double *)spline_calloc(5, sizeof(double) * m);
	if (w == NULL) {
		return 0;
	}
	h = w;
	b = h + m;
	d = b + m;
	g = d + m;
	u = g + m;

	/* step 1 */
	h[0] = SPLx(1) - SPLx(0);
	for (i = 1; i < n; i++) {
		j = i - 1;
		k = i + 1;
		h[i] = SPLx(k) - SPLx(i);
		b[i] = 2.0 * (h[i] + h[j]);
		d[i] = 3.0 * ((SPLy(k) - SPLy(i)) / h[i] - (SPLy(i) - SPLy(j)) / h[j]);
	}

	/* step 2 */
	g[1] = h[1] / b[1];
	u[1] = d[1] / b[1];
	k = n - 1;
	for (i = 2; i < k; i++) {
		j = i - 1;
		l = b[i] - h[j] * g[j];
		g[i] = h[i] / l;
		u[i] = (d[i] - h[j] * u[j]) / l;
	}
	i = n - 1;
	j = n - 2;
	u[i] = (d[i] - h[j] * u[j]) / (b[i] - h[j] * g[j]);

	/* step 3 */
	SPLr(n) = 0.0;
	SPLr(n - 1) = u[n - 1];
	for (i = n - 2; i > 0; i--) {
		SPLr(i) = u[i] - g[i] * SPLr(i + 1);
	}
	SPLr(0) = 0.0;

	/* step 4 */
	for (i = 0; i < n; i++) {
		k = i + 1;
		SPLq(i) = (SPLy(k) - SPLy(i)) / h[i] - h[i] * (SPLr(k) + 2.0 * SPLr(i)) / 3.0;
		SPLs(i) = (SPLr(k) - SPLr(i)) / (3.0 * h[i]);
	}
	SPLq(n) = 0.0;
	SPLs(n) = 0.0;

	spline_free(w);
	spl->closed = 1;

	return 1;
}

/* }}} */
/* {{{ spline_reopen() */

/*
 * Reactivator.
 */
SPLINE_PUBLIC void
spline_reopen(spline_t *spl)
{
	spl->closed = 0;
}

/* }}} */
/* {{{ spline_interpolate() */

/*
 * Get an interpolated value.
 * 'spl' must be closed by spline_close().
 */
SPLINE_PUBLIC double
spline_interpolate(const spline_t *spl, double x)
{
	unsigned int i, m, n;
	double xi;

	m = 0;
	n = spl->n - 1;

	if (!spl->closed) {
		return strtod("NAN", NULL);
	} else if (x <= SPLx(0)) {
		return SPLy(0);
	} else if (x >= SPLx(n)) {
		return SPLy(n);
	}

	do {
		i = (m + n) / 2;
		xi = SPLx(i);
		if (x > xi) {
			m = i;
		} else {
			n = i;
		}
	} while (!(xi <= x && x < SPLx(i + 1)));

	x -= xi;
	return SPLy(i) + x * (SPLq(i) + x * (SPLr(i) + x * SPLs(i)));
}

/* }}} */
/* {{{ spline_destroy() */

/*
 * Destructor.
 */
void
spline_destroy(spline_t *spl)
{
	if (spl->v != NULL) {
		spline_free(spl->v);
	}
	spline_free(spl);
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
