/* $Id: info.h,v 1.4 2005/06/30 15:10:38 oohara Exp $ */

#ifndef __DANGEN_INFO_H__
#define __DANGEN_INFO_H__

#include "tenm_object.h"

void clear_chain_scroll(void);
int show_chain(const tenm_object *player);
void clear_ship_scroll(void);
int show_ship(const tenm_object *player);
void clear_score_scroll(void);
int show_score(const tenm_object *player);

#endif /* not __DANGEN_INFO_H__ */
