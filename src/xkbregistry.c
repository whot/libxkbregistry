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
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libxml/parser.h>

#include "xkbregistry-private.h"

#define streq(a_, b_) (strcmp(a_, b_) == 0)

static bool
parse(struct rxkb_context *ctx, const char *path);

#define DECLARE_HELPERS_FOR_TYPE(type_) \
RXKB_EXPORT struct type_ * type_##_ref(struct type_ *object) { \
	rxkb_object_ref(&object->base); \
	return object; \
} \
RXKB_EXPORT struct type_ * type_##_unref(struct type_ *object) { \
	if (!object) return NULL; \
	return rxkb_object_unref(&object->base); \
} \
static inline struct type_ * type_##_create(struct rxkb_object *parent) { \
	struct type_ *t = calloc(1, sizeof *t); \
	if (t) \
		rxkb_object_init(&t->base, parent, (destroy_func_t)type_##_destroy); \
	return t; \
}

#define DECLARE_GETTER_FOR_TYPE(type_, field_) \
RXKB_EXPORT const char * type_##_get_##field_(struct type_ *object) { \
	return object->field_; \
}

#define DECLARE_FIRST_NEXT_FOR_TYPE(type_, parent_type_, parent_field_) \
RXKB_EXPORT struct type_ * type_##_first(struct parent_type_ *parent) { \
	struct type_ *o = NULL; \
	if (!list_empty(&parent->parent_field_)) \
		o = list_first_entry(&parent->parent_field_, o, base.link); \
	return o; \
} \
RXKB_EXPORT struct type_ * \
type_##_next(struct type_ *o) \
{ \
	struct parent_type_ *parent; \
	struct type_ *next; \
	parent = container_of(o->base.parent, struct parent_type_, base); \
	next = list_first_entry(&o->base.link, o, base.link); \
	if (list_is_last(&parent->parent_field_, &o->base.link)) \
		return NULL; \
	return next; \
}

static void
rxkb_object_init(struct rxkb_object *object, struct rxkb_object *parent, destroy_func_t destroy)
{
	object->refcount = 1;
	object->destroy = destroy;
	object->parent = parent;
	list_init(&object->link);
}

static void
rxkb_object_destroy(struct rxkb_object *object)
{
	if (object->destroy)
		object->destroy(object);
	list_remove(&object->link);
	free(object);
}

static void *
rxkb_object_ref(struct rxkb_object *object)
{
	assert(object->refcount >= 1);
	++object->refcount;
	return object;
}

static void *
rxkb_object_unref(struct rxkb_object *object)
{
	assert(object->refcount >= 1);
	if (--object->refcount == 0)
		rxkb_object_destroy(object);
	return NULL;
}

static void
rxkb_variant_destroy(struct rxkb_variant *v)
{
	free(v->name);
	free(v->brief);
	free(v->description);
}

DECLARE_HELPERS_FOR_TYPE(rxkb_variant);
DECLARE_GETTER_FOR_TYPE(rxkb_variant, name);
DECLARE_GETTER_FOR_TYPE(rxkb_variant, brief);
DECLARE_GETTER_FOR_TYPE(rxkb_variant, description);
DECLARE_FIRST_NEXT_FOR_TYPE(rxkb_variant, rxkb_layout, variants);

static void
rxkb_option_destroy(struct rxkb_option *o)
{
	free(o->name);
	free(o->brief);
	free(o->description);
}

DECLARE_HELPERS_FOR_TYPE(rxkb_option);
DECLARE_GETTER_FOR_TYPE(rxkb_option, name);
DECLARE_GETTER_FOR_TYPE(rxkb_option, brief);
DECLARE_GETTER_FOR_TYPE(rxkb_option, description);
DECLARE_FIRST_NEXT_FOR_TYPE(rxkb_option, rxkb_option_group, options);

static void
rxkb_layout_destroy(struct rxkb_layout *l)
{
	struct rxkb_variant *v, *vtmp;

	free(l->name);
	free(l->brief);
	free(l->description);

	list_for_each_safe(v, vtmp, &l->variants, base.link) {
		rxkb_variant_unref(v);
	}
}

DECLARE_HELPERS_FOR_TYPE(rxkb_layout);
DECLARE_GETTER_FOR_TYPE(rxkb_layout, name);
DECLARE_GETTER_FOR_TYPE(rxkb_layout, brief);
DECLARE_GETTER_FOR_TYPE(rxkb_layout, description);
DECLARE_FIRST_NEXT_FOR_TYPE(rxkb_layout, rxkb_context, layouts);

static void
rxkb_model_destroy(struct rxkb_model *m)
{
	free(m->name);
	free(m->vendor);
	free(m->description);
}

DECLARE_HELPERS_FOR_TYPE(rxkb_model);
DECLARE_GETTER_FOR_TYPE(rxkb_model, name);
DECLARE_GETTER_FOR_TYPE(rxkb_model, vendor);
DECLARE_GETTER_FOR_TYPE(rxkb_model, description);
DECLARE_FIRST_NEXT_FOR_TYPE(rxkb_model, rxkb_context, models);

static void
rxkb_option_group_destroy(struct rxkb_option_group *og)
{
	struct rxkb_option *o, *otmp;

	list_for_each_safe(o, otmp, &og->options, base.link) {
		rxkb_option_unref(o);
	}
}

RXKB_EXPORT bool
rxkb_option_group_allows_multiple(struct rxkb_option_group *g)
{
	return g->allow_multiple;
}

DECLARE_HELPERS_FOR_TYPE(rxkb_option_group);
DECLARE_GETTER_FOR_TYPE(rxkb_option_group, name);
DECLARE_GETTER_FOR_TYPE(rxkb_option_group, description);
DECLARE_FIRST_NEXT_FOR_TYPE(rxkb_option_group, rxkb_context, option_groups);

static void
rxkb_context_destroy(struct rxkb_context *ctx)
{
	struct rxkb_model *m, *mtmp;
	struct rxkb_layout *l, *ltmp;
	struct rxkb_option_group *og, *ogtmp;
	char **path;

	list_for_each_safe(m, mtmp, &ctx->models, base.link)
		rxkb_model_unref(m);

	list_for_each_safe(l, ltmp, &ctx->layouts, base.link)
		rxkb_layout_unref(l);

	list_for_each_safe(og, ogtmp, &ctx->option_groups, base.link)
		rxkb_option_group_unref(og);

	darray_foreach(path, ctx->includes)
		free(*path);
}

DECLARE_HELPERS_FOR_TYPE(rxkb_context);

RXKB_EXPORT struct rxkb_context *
rxkb_context_new(enum rxkb_context_flags flags)
{
	struct rxkb_context *ctx = rxkb_context_create(NULL);

	if (!ctx)
		return NULL;

	list_init(&ctx->models);
	list_init(&ctx->layouts);
	list_init(&ctx->option_groups);

	if (!(flags & RXKB_CONTEXT_NO_DEFAULT_INCLUDES) &&
	    !rxkb_context_include_path_append_default(ctx)) {
		rxkb_context_unref(ctx);
		return NULL;
	}

	return ctx;
}

RXKB_EXPORT bool
rxkb_context_include_path_append(struct rxkb_context *ctx, const char *path)
{
	struct stat stat_buf;
	int err;
	char *tmp;

	tmp = strdup(path);
	if (!tmp)
		goto err;

	err = stat(path, &stat_buf);
	if (err != 0)
		goto err;
	if (!S_ISDIR(stat_buf.st_mode))
		goto err;

#if defined(HAVE_EACCESS)
	if (eaccess(path, R_OK | X_OK) != 0)
		goto err;
#elif defined(HAVE_EUIDACCESS)
	if (euidaccess(path, R_OK | X_OK) != 0)
		goto err;
#endif

	darray_append(ctx->includes, tmp);
	return true;

err:
	return false;
}

RXKB_EXPORT bool
rxkb_context_include_path_append_default(struct rxkb_context *ctx)
{
	const char *home, *xdg, *root;
	char *user_path;
	int err;
	bool ret = false;

	home = secure_getenv("HOME");

	xdg = secure_getenv("XDG_CONFIG_HOME");
	if (xdg != NULL) {
		err = asprintf(&user_path, "%s/xkb", xdg);
		if (err >= 0) {
			ret |= rxkb_context_include_path_append(ctx, user_path);
			free(user_path);
		}
	} else if (home != NULL) {
		/* XDG_CONFIG_HOME fallback is $HOME/.config/ */
		err = asprintf(&user_path, "%s/.config/xkb", home);
		if (err >= 0) {
			ret |= rxkb_context_include_path_append(ctx, user_path);
			free(user_path);
		}
	}

	if (home != NULL) {
		err = asprintf(&user_path, "%s/.xkb", home);
		if (err >= 0) {
			ret |= rxkb_context_include_path_append(ctx, user_path);
			free(user_path);
		}
	}

	root = secure_getenv("XKB_CONFIG_ROOT");
	if (root != NULL)
		ret |= rxkb_context_include_path_append(ctx, root);
	else
		ret |= rxkb_context_include_path_append(ctx, DFLT_XKB_CONFIG_ROOT);

	return ret;
}

RXKB_EXPORT bool
rxkb_context_parse_default_ruleset(struct rxkb_context *ctx)
{
	return rxkb_context_parse(ctx, "evdev");
}

RXKB_EXPORT bool
rxkb_context_parse(struct rxkb_context *ctx, const char *ruleset)
{
	char **path;
	bool success = false;

	darray_foreach(path, ctx->includes) {
		char rules[PATH_MAX];

		snprintf(rules, sizeof(rules), "%s/rules/%s.xml", *path, ruleset);
		success = parse(ctx, rules);
		if (success)
			break;
	}
	return success;
}


RXKB_EXPORT void
rxkb_context_set_user_data(struct rxkb_context *ctx, void *userdata)
{
	ctx->userdata = userdata;
}

RXKB_EXPORT void *
rxkb_context_get_user_data(struct rxkb_context *ctx)
{
	return ctx->userdata;
}

static inline bool
is_node(xmlNode *node, const char *name)
{
	return node->type == XML_ELEMENT_NODE &&
		xmlStrcmp(node->name, (const xmlChar*)name) == 0;
}

/* return a copy of the text content from the first text node of this node */
static char *
extract_text(xmlNode *node)
{
	xmlNode *n;

	for (n = node->children; n; n = n->next) {
		if (n->type == XML_TEXT_NODE)
			return (char *)xmlStrdup(n->content);
	}
	return NULL;
}

static bool
parse_config_item(xmlNode *parent,
		  char **name,
		  char **description,
		  char **brief_or_vendor)
{
	xmlNode *node = NULL;
	xmlNode *ci = NULL;

	for (ci = parent->children; ci; ci = ci->next) {
		if (is_node(ci, "configItem")) {
			*name = NULL;
			*description = NULL;
			*brief_or_vendor = NULL;

			for (node = ci->children; node; node = node->next) {
				if (is_node(node, "name"))
					*name = extract_text(node);
				else if (is_node(node, "description"))
					*description = extract_text(node);
				else if (is_node(node, "shortDescription") ||
					 is_node(node, "vendor"))
					*brief_or_vendor = extract_text(node);
			}
			return true; /* only one configItem allowed in the dtd */
		}
	}
	return false;
}

static void
parse_model(struct rxkb_context *ctx, xmlNode *model)
{
	char *name, *description, *brief_or_vendor;

	if (parse_config_item(model, &name, &description, &brief_or_vendor)) {
		struct rxkb_model *m = rxkb_model_create(&ctx->base);

		m->name = name;
		m->description = description;
		m->vendor = brief_or_vendor;
		list_append(&ctx->models, &m->base.link);
	}
}

static void
parse_model_list(struct rxkb_context *ctx, xmlNode *model_list)
{
	xmlNode *node = NULL;

	for (node = model_list->children; node; node = node->next) {
		if (is_node(node, "model"))
			parse_model(ctx, node);
	}
}

static void
parse_variant(struct rxkb_layout *l, xmlNode *variant)
{
	char *name, *description, *brief_or_vendor;

	if (parse_config_item(variant, &name, &description, &brief_or_vendor)) {
		struct rxkb_variant *v = rxkb_variant_create(&l->base);

		v->name = name;
		v->description = description;
		v->brief = brief_or_vendor;
		list_append(&l->variants, &v->base.link);
	}
}

static void
parse_variant_list(struct rxkb_layout *l, xmlNode *variant_list)
{
	xmlNode *node = NULL;

	for (node = variant_list->children; node; node = node->next) {
		if (is_node(node, "variant"))
			parse_variant(l, node);
	}
}

static void
parse_layout(struct rxkb_context *ctx, xmlNode *layout)
{
	char *name, *description, *brief_or_vendor;
	struct rxkb_layout *l;
	xmlNode *node = NULL;

	if (!parse_config_item(layout, &name, &description, &brief_or_vendor))
		return;

	l = rxkb_layout_create(&ctx->base);

	list_init(&l->variants);
	l->name = name;
	l->description = description;
	l->brief = brief_or_vendor;
	list_append(&ctx->layouts, &l->base.link);

	for (node = layout->children; node; node = node->next) {
		if (is_node(node, "variantList")) {
			parse_variant_list(l, node);
			break;
		}
	}
}

static void
parse_layout_list(struct rxkb_context *ctx, xmlNode *layout_list)
{
	xmlNode *node = NULL;

	for (node = layout_list->children; node; node = node->next) {
		if (is_node(node, "layout"))
			parse_layout(ctx, node);
	}
}

static void
parse_option(struct rxkb_option_group *group, xmlNode *option)
{
	char *name, *description, *brief_or_vendor;

	if (parse_config_item(option, &name, &description, &brief_or_vendor)) {
		struct rxkb_option *o = rxkb_option_create(&group->base);

		o->name = name;
		o->description = description;
		list_append(&group->options, &o->base.link);
	}
}

static void
parse_group(struct rxkb_context *ctx, xmlNode *group)
{
	char *name, *description, *brief_or_vendor;
	struct rxkb_option_group *g;
	xmlNode *node = NULL;
	const xmlChar *multiple;

	if (!parse_config_item(group, &name, &description, &brief_or_vendor))
		return;

	g = rxkb_option_group_create(&ctx->base);
	g->name = name;
	g->description = description;

	multiple = xmlGetProp(group, (const xmlChar*)"allowMultipleSelection");
	if (multiple && xmlStrcmp(multiple, (const xmlChar*)"true"))
		g->allow_multiple = true;

	list_init(&g->options);
	list_append(&ctx->option_groups, &g->base.link);

	for (node = group->children; node; node = node->next) {
		if (is_node(node, "option"))
			parse_option(g, node);
	}
}

static void
parse_option_list(struct rxkb_context *ctx, xmlNode *option_list)
{
	xmlNode *node = NULL;

	for (node = option_list->children; node; node = node->next) {
		if (is_node(node, "group"))
			parse_group(ctx, node);
	}
}

static void
parse_rules_xml(struct rxkb_context *ctx, xmlNode *root)
{
	xmlNode *node = NULL;

	for (node = root->children; node; node = node->next) {
		if (is_node(node, "modelList"))
			parse_model_list(ctx, node);
		else if (is_node(node, "layoutList"))
			parse_layout_list(ctx, node);
		else if (is_node(node, "optionList"))
			parse_option_list(ctx, node);
	}
}

static bool
parse(struct rxkb_context *ctx, const char *path)
{
	xmlDoc *doc = NULL;
	xmlNode *root = NULL;

	doc = xmlReadFile(path, NULL, 0);
	if (!doc)
		return false;

	root = xmlDocGetRootElement(doc);
	parse_rules_xml(ctx, root);
	xmlFreeDoc(doc);
	xmlCleanupParser();

	return true;
}
