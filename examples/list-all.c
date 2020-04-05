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
#include <stdio.h>

#include "xkbregistry.h"

int main(int argc, char **argv)
{
	struct rxkb_context *ctx;
	const char *path = NULL;
	struct rxkb_model *m;
	struct rxkb_layout *l;
	struct rxkb_option_group *g;

	if (argc > 1)
		path = argv[1];

	ctx = rxkb_context_new(RXKB_CONTEXT_NO_DEFAULT_INCLUDES);
	if (path)
		rxkb_context_include_path_append(ctx, path);
	rxkb_context_include_path_append_default(ctx);
	rxkb_context_parse_default_ruleset(ctx);

	printf("Models:\n");
	m = rxkb_model_first(ctx);
	while (m) {
		printf("- %s:%s:%s\n",
		       rxkb_model_get_name(m),
		       rxkb_model_get_vendor(m),
		       rxkb_model_get_description(m));
		m = rxkb_model_next(m);
	}

	printf("Layouts:\n");
	l = rxkb_layout_first(ctx);
	while (l) {
		struct rxkb_variant *v;
		printf("- %s:%s:%s\n",
		       rxkb_layout_get_name(l),
		       rxkb_layout_get_brief(l),
		       rxkb_layout_get_description(l));

		v = rxkb_variant_first(l);
		while (v) {
			printf("  - %s:%s:%s\n",
			       rxkb_variant_get_name(v),
			       rxkb_variant_get_brief(v),
			       rxkb_variant_get_description(v));
			v = rxkb_variant_next(v);
		}

		l = rxkb_layout_next(l);
	}
	printf("Options:\n");
	g = rxkb_option_group_first(ctx);
	while(g) {
		struct rxkb_option *o;

		printf("- %s:%s (%s)\n",
		       rxkb_option_group_get_name(g),
		       rxkb_option_group_get_description(g),
		       rxkb_option_group_allows_multiple(g) ? "multiple" : "single");

		o = rxkb_option_first(g);
		while (o) {
			printf("  - %s:%s:%s\n",
			       rxkb_option_get_name(o),
			       rxkb_option_get_brief(o),
			       rxkb_option_get_description(o));
			o = rxkb_option_next(o);
		}

		g = rxkb_option_group_next(g);
	}

	rxkb_context_unref(ctx);
	return 1;
}
