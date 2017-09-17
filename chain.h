/* $Id: chain.h,v 1.14 2005/06/30 15:05:14 oohara Exp $ */

#ifndef __DANGEN_CHAIN_H__
#define __DANGEN_CHAIN_H__

#include "tenm_object.h"

void clear_chain(void);
int get_chain(void);
int add_chain(tenm_object *my, tenm_object *your);

#endif /* not __DANGEN_CHAIN_H__ */
