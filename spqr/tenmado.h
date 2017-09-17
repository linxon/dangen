/* $Id: tenmado.h,v 1.9 2004/10/07 14:09:32 oohara Exp $ */

#ifndef __DANGEN_TENMADO_H__
#define __DANGEN_TENMADO_H__

#include "tenm_object.h"

tenm_object *tenmado_new(double x, double y, int n, double dx, int t,
                         int table_index, int t_shoot);

#endif /* not __DANGEN_TENMADO_H__ */
