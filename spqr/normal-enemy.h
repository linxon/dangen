/* $Id: normal-enemy.h,v 1.23 2004/12/01 13:33:32 oohara Exp $ */

#ifndef __DANGEN_NORMAL_ENEMY_H__
#define __DANGEN_NORMAL_ENEMY_H__

/* what */
#define BALL_SOLDIER 1
#define BALL_CAPTAIN 2
#define BRICK 3
#define SQUARE 4
#define TRIANGLE 5

/* type
 * these values must be one of 2^n
 */
/* gets hit by ATTR_OBSTACLE */
#define ENEMY_TYPE_WEAK 1
/* counts as ATTR_OBSTACLE as well as ATTR_ENEMY */
#define ENEMY_TYPE_OBSTACLE 2

tenm_object * normal_enemy_new(double x, double y, int what, int type,
                               int time_no_escape,
                               int signal_index_killed,
                               int signal_suffix_killed,
                               int signal_index_escaped,
                               int signal_suffix_escaped,
                               int number_mode_move,
                               int number_mode_shoot, ...);

#endif /* not __DANGEN_NORMAL_ENEMY_H__ */
