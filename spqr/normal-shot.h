/* $Id: normal-shot.h,v 1.7 2004/07/06 02:50:33 oohara Exp $ */

#ifndef __DANGEN_NORMAL_SHOT_H__
#define __DANGEN_NORMAL_SHOT_H__

#include "tenm_object.h"

tenm_object *normal_shot_angle_new(double x, double y, double speed, int theta,
                                   int color);
tenm_object *normal_shot_point_new(double x, double y, double speed,
                             double target_x, double target_y,
                             int color);
tenm_object *normal_shot_new(double x, double y,
                             double speed_x, double speed_y,
                             int color, int life, int immortal_time);

#endif /* not __DANGEN_NORMAL_SHOT_H__ */
