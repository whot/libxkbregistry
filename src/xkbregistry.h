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


#pragma once

#include <stdbool.h>

/**
 * @file
 * libxkbregistry API.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup API libxkbregistry API
 *
 * @{
 */

/**
 * @struct rxkb_context
 *
 * Opaque top level library context object.
 *
 * The context contains general library state, like include paths and parsed
 * data. Objects are created in a specific context, and multiple contexts
 * may coexist simultaneously. Objects from different contexts are
 * completely separated and do not share any memory or state.
 */
struct rxkb_context;

/**
 * @struct rxkb_model
 *
 * Opaque struct representing an XKB model.
 */
struct rxkb_model;

/**
 * @struct rxkb_layout
 *
 * Opaque struct representing an XKB layout. This layout is the base layout
 * equivalent to a null variant. Variants are a sublevel of the layout.
 */
struct rxkb_layout;

/**
 * @struct rxkb_variant
 *
 * Opaque struct representing an XKB variant. Variants are a sublevel of the
 * layout.
 */
struct rxkb_variant;

/**
 * @struct rxkb_option_group
 *
 * Opaque struct representing an option group. Option groups divide the
 * individual options into logical group. Their main purpose is to indicate
 * whether some options are mutually exclusive or not.
 */
struct rxkb_option_group;

/**
 * @struct rxkb_option
 *
 * Opaque struct representing an XKB option.
 */
struct rxkb_option;

/** Flags for context creation. */
enum rxkb_context_flags {
    RXKB_CONTEXT_NO_FLAGS = 0,
    RXKB_CONTEXT_NO_DEFAULT_INCLUDES = (1 << 0),
};

/**
 * Create a new xkb registry context.
 *
 * @param flags Flags affecting context behavior
 * @return A new xkb registry context or NULL on failure
 */
struct rxkb_context *
rxkb_context_new(enum rxkb_context_flags flags);

/**
 * Parse the given ruleset. This can only be called once per context, once
 * parsed the data in the context is considered constant and will never
 * change.
 */
bool
rxkb_context_parse(struct rxkb_context *ctx, const char *ruleset);

/**
 * Parse the default ruleset as configured at build time. See
 * rxkb_context_parse() for details.
 */
bool
rxkb_context_parse_default_ruleset(struct rxkb_context *ctx);

/**
 * Increases the refcount of this object by one and returns the object.
 *
 * @param ctx The xkb registry context
 * @return The passed in object
 */
struct rxkb_context*
rxkb_context_ref(struct rxkb_context *ctx);

/**
 * Decreases the refcount of this object by one. Where the refcount of an
 * object hits zero, associated resources will be freed.
 *
 * @param ctx The xkb registry context
 * @return always NULL
 */
struct rxkb_context*
rxkb_context_unref(struct rxkb_context *ctx);

/**
 * Assign user-specific data. libxkbregistry will not look at or modify the
 * data, it will merely return the same pointer in
 * rxkb_context_get_user_data().
 *
 * @param ctx The xkb registry context
 * @param user_data User-specific data pointer
 */
void
rxkb_context_set_user_data(struct rxkb_context *ctx, void *user_data);

/**
 * Return the pointer passed into rxkb_context_get_user_data().
 *
 * @param ctx The xkb registry context
 * @return User-specific data pointer
 */
void *
rxkb_context_get_user_data(struct rxkb_context *ctx);

/**
 * Append a new entry to the context's include path.
 *
 * @returns true on success, or false if the include path could not be added
 * or is inaccessible.
 */
bool
rxkb_context_include_path_append(struct rxkb_context *ctx, const char *path);

/**
 * Append the default include paths to the context's include path.
 *
 * @returns true on success, or false if the include path could not be added
 * or is inaccessible.
 */
bool
rxkb_context_include_path_append_default(struct rxkb_context *ctx);

/**
 * Return the first model for this context. Use this to start iterating over
 * the models, followed by calls to rxkb_model_next().
 *
 * @return The first model in the model list.
 */
struct rxkb_model *
rxkb_model_first(struct rxkb_context *ctx);

/**
 * Return the next model for this context. Returns NULL when no more models
 * are available.
 *
 * @return the next model or NULL at the end of the list
 */
struct rxkb_model *
rxkb_model_next(struct rxkb_model *m);

struct rxkb_model *
rxkb_model_ref(struct rxkb_model *m);

struct rxkb_model *
rxkb_model_unref(struct rxkb_model *m);

const char *
rxkb_model_get_name(struct rxkb_model *m);

const char *
rxkb_model_get_description(struct rxkb_model *m);

const char *
rxkb_model_get_vendor(struct rxkb_model *m);

struct rxkb_layout *
rxkb_layout_first(struct rxkb_context *ctx);

struct rxkb_layout *
rxkb_layout_next(struct rxkb_layout *l);

struct rxkb_layout *
rxkb_layout_ref(struct rxkb_layout *l);

struct rxkb_layout *
rxkb_layout_unref(struct rxkb_layout *l);

const char *
rxkb_layout_get_name(struct rxkb_layout *l);

const char *
rxkb_layout_get_name(struct rxkb_layout *l);

const char *
rxkb_layout_get_brief(struct rxkb_layout *l);

const char *
rxkb_layout_get_description(struct rxkb_layout *l);

struct rxkb_variant *
rxkb_variant_first(struct rxkb_layout *v);

struct rxkb_variant *
rxkb_variant_next(struct rxkb_variant *v);

struct rxkb_variant *
rxkb_variant_ref(struct rxkb_variant *v);

struct rxkb_variant *
rxkb_variant_unref(struct rxkb_variant *v);

const char *
rxkb_variant_get_name(struct rxkb_variant *v);

const char *
rxkb_variant_get_name(struct rxkb_variant *v);

const char *
rxkb_variant_get_brief(struct rxkb_variant *v);

const char *
rxkb_variant_get_description(struct rxkb_variant *v);

struct rxkb_option_group *
rxkb_option_group_first(struct rxkb_context *ctx);

struct rxkb_option_group *
rxkb_option_group_next(struct rxkb_option_group *g);

struct rxkb_option_group *
rxkb_option_group_ref(struct rxkb_option_group *g);

struct rxkb_option_group *
rxkb_option_group_unref(struct rxkb_option_group *g);

const char *
rxkb_option_group_get_name(struct rxkb_option_group *m);

const char *
rxkb_option_group_get_description(struct rxkb_option_group *m);

/**
 * @return true if multiple options within this option group can be selected
 *	   simultaneously, true if all options within this option group are
 *	   mutually exclusive.
 */
bool
rxkb_option_group_allows_multiple(struct rxkb_option_group *g);

struct rxkb_option *
rxkb_option_first(struct rxkb_option_group *group);

struct rxkb_option *
rxkb_option_next(struct rxkb_option *o);

struct rxkb_option *
rxkb_option_ref(struct rxkb_option *o);

struct rxkb_option *
rxkb_option_unref(struct rxkb_option *o);

const char *
rxkb_option_get_name(struct rxkb_option *o);

const char *
rxkb_option_get_name(struct rxkb_option *o);

const char *
rxkb_option_get_brief(struct rxkb_option *o);

const char *
rxkb_option_get_description(struct rxkb_option *o);

/** @} */

#ifdef __cplusplus
}
#endif
