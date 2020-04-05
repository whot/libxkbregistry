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

#include "xkbregistry.h"
#include "util-list.h"
#include <assert.h>
#include <stdint.h>

static void
test_models(void)
{
	struct rxkb_context *ctx;
	struct rxkb_model *m;

	ctx = rxkb_context_new(RXKB_CONTEXT_NO_FLAGS);
	assert(ctx);

	m = rxkb_model_first(ctx);
	while (m) {
		struct rxkb_model *next = rxkb_model_next(m);
		assert(next != m);
		assert(rxkb_model_get_name(m) != NULL);
		assert(rxkb_model_get_description(m) != NULL);
		//assert(rxkb_model_get_vendor(m) != NULL);
		m = next;
	}
}

static void
test_context_new(void)
{
	struct rxkb_context *ctx;

	ctx = rxkb_context_new(RXKB_CONTEXT_NO_FLAGS);
	assert(ctx);
	assert(rxkb_context_unref(ctx) == NULL);

	ctx = rxkb_context_new(RXKB_CONTEXT_NO_FLAGS);
	assert(ctx);
	assert(rxkb_context_ref(ctx) == ctx);
	assert(rxkb_context_unref(ctx) == NULL);
	assert(rxkb_context_unref(ctx) == NULL);
}

static void
test_list(void)
{
	struct listtest {
		struct list node;
		uint32_t value;
	};
	struct list head;
	struct listtest l1, l2, l3;
	struct listtest *a, *b;

	l1.value = 1;
	l2.value = 3;
	l3.value = 3;

	list_init(&l1.node);
	list_init(&l2.node);
	list_init(&l3.node);

	list_init(&head);
	assert(list_empty(&head));

	list_append(&head, &l1.node);
	a = list_first_entry(&head, a, node);
	b = &l1;
	assert(a == b);
	a = list_last_entry(&head, a, node);
	b = &l1;
	assert(a == b);
	assert(list_is_last(&head, &l1.node));

	list_append(&head, &l2.node);
	a = list_first_entry(&head, a, node);
	b = &l1;
	assert(a == b);
	a = list_last_entry(&head, a, node);
	b = &l2;
	assert(a == b);
	assert(!list_is_last(&head, &l1.node));
	assert(list_is_last(&head, &l2.node));

	list_append(&head, &l3.node);
	a = list_first_entry(&head, a, node);
	b = &l1;
	assert(a == b);
	a = list_last_entry(&head, a, node);
	b = &l3;
	assert(a == b);
	assert(!list_is_last(&head, &l1.node));
	assert(!list_is_last(&head, &l2.node));
	assert(list_is_last(&head, &l3.node));

	list_remove(&l1.node);
	a = list_first_entry(&head, a, node);
	b = &l2;
	assert(a == b);
	a = list_last_entry(&head, a, node);
	b = &l3;
	assert(a == b);
	assert(!list_is_last(&head, &l2.node));
	assert(list_is_last(&head, &l3.node));
}

int main(void)
{
	test_list();
	test_context_new();
	test_models();
}
