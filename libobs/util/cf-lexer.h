/******************************************************************************
  Copyright (c) 2013 by Hugh Bailey <obs.jim@gmail.com>

  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

     1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.

     2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.

     3. This notice may not be removed or altered from any source
     distribution.
******************************************************************************/

#ifndef CF_LEXER_H
#define CF_LEXER_H

#include "lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

EXPORT char *cf_literal_to_str(const char *literal, size_t count);

/* ------------------------------------------------------------------------- */
/*
 * A C-family lexer token is defined as:
 *   1.) A generic 'name' token.  (abc123_def456)
 *   2.) A numeric sequence (usually starting with a number)
 *   3.) A sequence of generic whitespace defined as spaces and tabs
 *   4.) A newline
 *   5.) A string or character sequence (surrounded by single or double quotes)
 *   6.) A single character of a type not specified above
 */
enum cf_token_type {
	CFTOKEN_NONE,
	CFTOKEN_NAME,
	CFTOKEN_NUM,
	CFTOKEN_SPACETAB,
	CFTOKEN_NEWLINE,
	CFTOKEN_STRING,
	CFTOKEN_OTHER
};

struct cf_token {
	const struct cf_lexer *lex;
	struct strref str;
	struct strref unmerged_str;
	enum cf_token_type type;
};

static inline void cf_token_clear(struct cf_token *t)
{
	memset(t, 0, sizeof(struct cf_token));
}

static inline void cf_token_copy(struct cf_token *dst,
		const struct cf_token *src)
{
	memcpy(dst, src, sizeof(struct cf_token));
}

static inline void cf_token_add(struct cf_token *dst,
		const struct cf_token *add)
{
	strref_add(&dst->str, &add->str);
	strref_add(&dst->unmerged_str, &add->unmerged_str);
}

/* ------------------------------------------------------------------------- */
/*
 *   The c-family lexer is a base lexer for generating a list of string
 * reference tokens to be used with c-style languages.
 *
 *   This base lexer is meant to be used as a stepping stone for an actual
 * language lexer/parser.
 *
 *   It reformats the text in the two following ways:
 *     1.) Spliced lines (escaped newlines) are merged
 *     2.) All comments are converted to a single space
 */

struct cf_lexer {
	char *file;
	struct lexer base_lexer;
	char *reformatted, *write_offset;
	DARRAY(struct cf_token) tokens;
	bool unexpected_eof; /* unexpected multi-line comment eof */
};

EXPORT void cf_lexer_init(struct cf_lexer *lex);
EXPORT void cf_lexer_free(struct cf_lexer *lex);

static inline struct cf_token *cf_lexer_gettokens(struct cf_lexer *lex)
{
	return lex->tokens.array;
}

EXPORT bool cf_lexer_lex(struct cf_lexer *lex, const char *str,
		const char *file);

/* ------------------------------------------------------------------------- */
/* c-family preprocessor definition */

struct cf_def {
	struct cf_token name;
	DARRAY(struct cf_token) params;
	DARRAY(struct cf_token) tokens;
	bool macro;
};

static inline void cf_def_init(struct cf_def *cfd)
{
	cf_token_clear(&cfd->name);
	da_init(cfd->params);
	da_init(cfd->tokens);
	cfd->macro = false;
}

static inline void cf_def_addparam(struct cf_def *cfd, struct cf_token *param)
{
	da_push_back(cfd->params, param);
}

static inline void cf_def_addtoken(struct cf_def *cfd, struct cf_token *token)
{
	da_push_back(cfd->tokens, token);
}

static inline struct cf_token *cf_def_getparam(const struct cf_def *cfd,
		size_t idx)
{
	return cfd->params.array+idx;
}

static inline void cf_def_free(struct cf_def *cfd)
{
	cf_token_clear(&cfd->name);
	da_free(cfd->params);
	da_free(cfd->tokens);
}

/* ------------------------------------------------------------------------- */
/*
 * C-family preprocessor
 *
 *   This preprocessor allows for standard c-style preprocessor directives
 * to be applied to source text, such as:
 *
 *   + #include
 *   + #define/#undef
 *   + #ifdef/#ifndef/#if/#elif/#else/#endif
 *
 *   Still left to implement (TODO):
 *   + #if/#elif
 *   + "defined" preprocessor keyword
 *   + system includes 
 *   + variadic macros
 *   + custom callbacks (for things like pragma)
 *   + option to exclude features such as #import, variadic macros, and other
 *     features for certain language implementations
 *   + macro parameter string operator #
 *   + macro parameter token concactenation operator ##
 *   + predefined macros
 *   + restricted macros
 */

struct cf_preprocessor {
	struct cf_lexer *lex;
	struct error_data *ed;
	DARRAY(struct cf_def)   defines;
	DARRAY(char*)           sys_include_dirs;
	DARRAY(struct cf_lexer) dependencies;
	DARRAY(struct cf_token) tokens;
	bool ignore_state;
};

EXPORT void cf_preprocessor_init(struct cf_preprocessor *pp);
EXPORT void cf_preprocessor_free(struct cf_preprocessor *pp);

EXPORT bool cf_preprocess(struct cf_preprocessor *pp, struct cf_lexer *lex,
		struct error_data *ed);

static inline void cf_preprocessor_add_sys_include_dir(
		struct cf_preprocessor *pp, const char *include_dir)
{
	if (include_dir)
		da_push_back(pp->sys_include_dirs, bstrdup(include_dir));
}

EXPORT void cf_preprocessor_add_def(struct cf_preprocessor *pp,
		struct cf_def *def);
EXPORT void cf_preprocessor_remove_def(struct cf_preprocessor *pp,
		const char *def_name);

static inline struct cf_token *cf_preprocessor_gettokens(
		struct cf_preprocessor *pp)
{
	return pp->tokens.array;
}

#ifdef __cplusplus
}
#endif

#endif
