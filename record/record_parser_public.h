/* $Id: record_parser_public.h,v 1.8 2005/06/29 17:08:56 oohara Exp $ */

#ifndef __DANGEN_RECORD_PARSER_PUBLIC_H__
#define __DANGEN_RECORD_PARSER_PUBLIC_H__

#include "record_data.h"

/* yyparse */
int record_parse(void);

void set_game_record(game_record *q);
game_record *get_game_record(void);

#endif /* not __DANGEN_RECORD_PARSER_PUBLIC_H__ */
