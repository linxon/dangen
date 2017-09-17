/* $Id: tutor.h,v 1.6 2005/07/09 06:34:45 oohara Exp $ */

#ifndef __DANGEN_TUTOR_H__
#define __DANGEN_TUTOR_H__

#include "tenm_object.h"

tenm_object *tutor_new(int what, double x, double y, int parent_index,
                       int rank);

#endif /* not __DANGEN_TUTOR_H__ */
