/* $Id: util.h,v 1.22 2005/06/29 05:49:21 oohara Exp $ */

#ifndef __DANGEN_UTIL_H__
#define __DANGEN_UTIL_H__

#include "tenm_object.h"

int util_init(int width, int height);
void util_quit(void);

int draw_string(int x, int y, const char *string, int length);
int draw_string_int(int x, int y, const int *string, int length);

int in_window_object(const tenm_object *p);
int in_window_primitive(const tenm_primitive *p);

void vector_rotate(double *result, const double *v, int theta);
void vector_rotate_bounded(double *result, const double *v,
                           const double *a, int theta);

/* table manipulation function */
int delete_enemy_shot(tenm_object *my, int n);
int delete_enemy(tenm_object *my, int n);

#endif /* not __DANGEN_UTIL_H__ */
