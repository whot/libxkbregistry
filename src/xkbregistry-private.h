/*
 * Copyright © 2020 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "config.h"

#include <assert.h>

#include "xkbregistry.h"
#include "darray.h"
#include "util-list.h"
#include "stdint.h"

#if defined(__GNUC__) && (__GNUC__ >= 4) && !defined(__CYGWIN__)
# define RXKB_EXPORT      __attribute__((visibility("default")))
#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x550)
# define RXKB_EXPORT      __global
#else /* not gcc >= 4 and not Sun Studio >= 8 */
# define RXKB_EXPORT
#endif

struct rxkb_object;

typedef void (*destroy_func_t)(struct rxkb_object *object);

struct rxkb_object {
	struct rxkb_object *parent;
	uint32_t refcount;
	struct list link;
	destroy_func_t destroy;
};

struct rxkb_context {
	struct rxkb_object base;

	struct list models;
	struct list layouts;
	struct list option_groups;

	darray(char *)includes;

	void *userdata;
};

struct rxkb_model {
	struct rxkb_object base;

	char *name;
	char *vendor;
	char *description;
};

struct rxkb_layout {
	struct rxkb_object base;

	char *name;
	char *brief;
	char *description;

	struct list variants;
};

struct rxkb_variant {
	struct rxkb_object base;

	char *name;
	char *brief;
	char *description;
};

struct rxkb_option_group {
	struct rxkb_object base;

	bool allow_multiple;
	struct list options;
	char *name;
	char *description;
};

struct rxkb_option {
	struct rxkb_object base;

	char *name;
	char *brief;
	char *description;
};

