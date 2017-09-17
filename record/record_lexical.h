/* $Id: record_lexical.h,v 1.5 2011/08/24 16:30:17 oohara Exp $ */

#ifndef __DANGEN_RECORD_LEXICAL_H__
#define __DANGEN_RECORD_LEXICAL_H__

/* FILE */
#include <stdio.h>

/* yylineno */
extern int record_lineno;

/* yylex */
int record_lex(void);
/* yylex_destroy */
int record_lex_destroy(void);

void record_read_from_file(FILE *file);
void record_read_from_string(const char *string);

#endif /* not __DANGEN_RECORD_LEXICAL_H__ */
