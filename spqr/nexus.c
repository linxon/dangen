/* $Id: nexus.c,v 1.132 2005/01/01 16:41:00 oohara Exp $ */

#include <stdio.h>
/* malloc, rand */
#include <stdlib.h>
/* strlen, strcmp */
#include <string.h>

#include "const.h"
#include "tenm_object.h"
#include "tenm_graphic.h"
#include "tenm_primitive.h"
#include "util.h"
#include "player-shot.h"
#include "tenm_table.h"
#include "background.h"
#include "chain.h"
#include "laser.h"
#include "normal-shot.h"
#include "tenm_math.h"
#include "fragment.h"
#include "explosion.h"
#include "normal-enemy.h"
#include "score.h"
#include "warning.h"
#include "tadashi.h"

#include "nexus.h"

static int nexus_move(tenm_object *my, double turn_per_frame);
static int nexus_hit(tenm_object *my, tenm_object *your);
static int nexus_signal_kill_hatch(tenm_object *my, int n);
static int nexus_signal_kill_attacker(tenm_object *my, int n);
static int nexus_signal_kill_head(tenm_object *my, int n);
static int nexus_act(tenm_object *my, const tenm_object *player);
static int nexus_draw(tenm_object *my, int priority);
static int nexus_green(const tenm_object *my);

static tenm_object *nexus_hatch_new(int what);
static int nexus_hatch_move(tenm_object *my, double turn_per_frame);
static int nexus_hatch_act(tenm_object *my, const tenm_object *player);
static int nexus_hatch_open_time(int what, int side);
static int nexus_hatch_draw(tenm_object *my, int priority);

static tenm_object *nexus_attacker_new(void);
static int nexus_attacker_move(tenm_object *my, double turn_per_frame);
static int nexus_attacker_act(tenm_object *my, const tenm_object *player);
static int nexus_attacker_draw(tenm_object *my, int priority);

static tenm_object *nexus_head_new(void);
static int nexus_head_move(tenm_object *my, double turn_per_frame);
static int nexus_head_act(tenm_object *my, const tenm_object *player);
static int nexus_head_draw(tenm_object *my, int priority);

tenm_object *
nexus_new(void)
{
  int i;
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = (double) (WINDOW_WIDTH / 2);
  double y = -29.0;

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "nexus_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + 30.0, y - 30.0,
                                             x + 30.0, y + 30.0,
                                             x - 30.0, y + 30.0,
                                             x - 30.0, y - 30.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "nexus_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 10);
  if (count == NULL)
  {
    fprintf(stderr, "nexus_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 3);
  if (count_d == NULL)
  {
    fprintf(stderr, "nexus_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] for deal_damage
   * [1] "damaged" timer
   * [2] life mode
   * [3] life timer
   * [4] "was green when killed" flag
   * [5 -- 7] hatch index
   * [8] attacker index
   * [9] head index
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */


  count[0] = 0;
  count[1] = 0;
  count[2] = 0;
  count[3] = 0;
  count[4] = 0;
  for (i = 5; i <= 9; i++)
    count[i] = -1;

  count_d[0] = 0.0;
  count_d[1] = 1.0;

  new = tenm_object_new("Nexus", ATTR_ENEMY, ATTR_PLAYER_SHOT,
                        1000, x, y,
                        10, count, 3, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&nexus_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&nexus_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&nexus_act),
                        (int (*)(tenm_object *, int))
                        (&nexus_draw));
  if (new == NULL)
  {
    fprintf(stderr, "nexus_new: tenm_object_new failed\n");
    if (count_d != NULL)
      free(count_d);
    if (count != NULL)
      free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  return new;
}
static int
nexus_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "nexus_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "nexus_move: strange turn_per_frame (%f)\n",
            turn_per_frame);
    return 0;
  }

  dx_temp = my->count_d[0] / turn_per_frame;
  dy_temp = my->count_d[1] / turn_per_frame;
  my->x += dx_temp;
  my->y += dy_temp;
  if (my->mass != NULL)
    tenm_move_mass(my->mass, dx_temp, dy_temp);

  return 0;
}

static int
nexus_hit(tenm_object *my, tenm_object *your)
{
  int i;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "nexus_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "nexus_hit: your is NULL\n");
    return 0;
  }

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;
  if (my->count[2] != 0)
    return 0;

  deal_damage(my, your, 0);
  if (nexus_green(my))
    add_chain(my, your);
  my->count[1] = 41;

  if (my->hit_point <= 0)
  {
    add_score(80);

    if (nexus_green(my))
      i = 8;
    else
      i = 7;
    tenm_table_add(explosion_new(my->x, my->y,
                                 my->count_d[0] * 0.5,
                                 my->count_d[1] * 0.5,
                                 1, 1000, i, 8.0, 6));
    tenm_table_add(fragment_new(my->x, my->y,
                                my->count_d[0] * 0.5, my->count_d[1] * 0.5,
                                30.0, 30, i, 5.0, 0.0, 20));

    for (i = 0; i < 3; i++)
    {
      if (my->count[5 + i] >= 0)
        tenm_table_apply(my->count[5 + i],
                         (int (*)(tenm_object *, int))
                         nexus_signal_kill_hatch,
                         0);
    }
    if (my->count[8] >= 0)
      tenm_table_apply(my->count[8],
                       (int (*)(tenm_object *, int))
                       nexus_signal_kill_attacker,
                       0);
    if (my->count[9] >= 0)
      tenm_table_apply(my->count[9],
                       (int (*)(tenm_object *, int))
                       nexus_signal_kill_head,
                       0);

    my->count[2] = 1;
    my->count[3] = 0;
    /* don't modify my->attr or my->hit_mask here, or the player shot
     * may fly through the enemy */
    tenm_mass_delete(my->mass);
    my->mass = NULL;

    return 0;
  }

  return 0;
}

static int
nexus_signal_kill_hatch(tenm_object *my, int n)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (strcmp(my->name, "Nexus hatch") != 0)
    return 0;

  tenm_table_add(explosion_new(my->x, my->y,
                               my->count_d[0] * 0.5,
                               my->count_d[1] * 0.5,
                               1, 1000, 9, 8.0, 6));
  tenm_table_add(fragment_new(my->x, my->y,
                              my->count_d[0] * 0.5, my->count_d[1] * 0.5,
                              30.0, 30, 9, 5.0, 0.0, 20));

  return 1;
}

static int
nexus_signal_kill_attacker(tenm_object *my, int n)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (strcmp(my->name, "Nexus attacker") != 0)
    return 0;

  tenm_table_add(explosion_new(my->x, my->y,
                               my->count_d[0] * 0.5,
                               my->count_d[1] * 0.5,
                               1, 1000, 9, 8.0, 6));
  tenm_table_add(fragment_new(my->x, my->y,
                              my->count_d[0] * 0.5, my->count_d[1] * 0.5,
                              30.0, 30, 9, 5.0, 0.0, 20));

  return 1;
}

static int
nexus_signal_kill_head(tenm_object *my, int n)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (strcmp(my->name, "Nexus head") != 0)
    return 0;

  tenm_table_add(explosion_new(my->x, my->y,
                               my->count_d[0] * 0.5,
                               my->count_d[1] * 0.5,
                               1, 1000, 9, 8.0, 6));
  tenm_table_add(fragment_new(my->x, my->y,
                              my->count_d[0] * 0.5, my->count_d[1] * 0.5,
                              30.0, 30, 9, 5.0, 0.0, 20));

  return 1;
}

static int
nexus_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int what;
  int t_shoot;
  double v[2];
  double result[2];

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "nexus_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  /* for deal_damage */
  my->count[0] = 0;

  /* "damaged" count down */
  if (my->count[1] > 0)
    (my->count[1])--;

  (my->count[3])++;

  /* add boss */
  if (my->count[2] == 1)
  {
    if (my->count[3] == 200)
      tenm_table_add(warning_new());

    if (my->count[3] == 330)
    {  
      tenm_table_add(tadashi_new());
      return 1;
    }

    return 0;
  }
  
  /* escaped */
  if ((my->count[2] == 0) && (my->count[3] >= 2400))
  {
    my->count[2] = 1;
    my->count[3] = 170;
    /* don't modify my->attr or my->hit_mask here, or the player shot
     * may fly through the enemy */
    tenm_mass_delete(my->mass);
    my->mass = NULL;
    return 0;
  }

  if (my->count[3] == 1)
  {
    for (i = 0; i < 3; i++)
      my->count[5 + i] = tenm_table_add(nexus_hatch_new(i));
    my->count[8] = tenm_table_add(nexus_attacker_new());
    my->count[9] = tenm_table_add(nexus_head_new());
  }

  /* speed change */
  if (my->count[3] == 359)
      my->count_d[1] = -6.0 + 1.0;
  if (my->count[3] == 409)
    my->count_d[1] = 1.0;
  if ((my->count[3] >= 709) && (my->count[3] < 1739))
  {
    switch ((my->count[3] - 709) % 360)
    {
    case 0:
      my->count_d[1] = -6.0 + 1.0;
      break;
    case 60:
      my->count_d[1] = 1.0;
      break;
    default:
      break;
    }
  }
  if (my->count[3] == 1739)
      my->count_d[1] = -6.0 + 1.0;
  if (my->count[3] == 1789)
      my->count_d[1] = 1.0;
  if (my->count[3] == 1950)
      my->count_d[1] = 0.5;
  if (my->count[3] == 2230)
      my->count_d[1] = 1.0;

  /* add normal enemy */
  if ((my->count[3] >= 10) && (my->count[3] < 42)
      && ((my->count[3] - 10) % 8 == 0))
  {
    if (my->count[3] == 10)
      t_shoot = 23;
    else
      t_shoot = 9999;
    tenm_table_add(normal_enemy_new(-19.0,
                                    100.0 + (double) (my->count[3] - 10),
                                    BALL_SOLDIER, 0,
                                    0, -1, 0, -1, 0, 3, 2,
                                    /* move 0 */
                                    48, 5.0, 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    31, 0.0, 1.0, 0.0, 0.0,
                                    221.0,
                                    210.0 + (double) (my->count[3] - 10),
                                    0.0, 0.5, 2,
                                    /* move 2 */
                                    9999, -5.0, 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 2,
                                    /* shoot 0 */
                                    48, t_shoot, 0, 0, 0, 1,
                                    /* shoot 1 */
                                    9999, t_shoot, 48 % t_shoot, 0, 1, 1));
    tenm_table_add(normal_enemy_new(((double) (WINDOW_WIDTH)) + 19.0,
                                    100.0 + (double) (my->count[3] - 10),
                                    BALL_SOLDIER, 0,
                                    0, -1, 0, -1, 0, 3, 2,
                                    /* move 0 */
                                    48, -5.0, 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    31, 0.0, 1.0, 0.0, 0.0,
                                    ((double) (WINDOW_WIDTH)) - 221.0,
                                    210.0 + (double) (my->count[3] - 10),
                                    0.0, -0.5, 2,
                                    /* move 2 */
                                    9999, 5.0, 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 2,
                                    /* shoot 0 */
                                    48, t_shoot, 0, 0, 0, 1,
                                    /* shoot 1 */
                                    9999, t_shoot, 48 % t_shoot, 0, 1, 1));
  }
  if ((my->count[3] >= 850) && (my->count[3] < 914)
      && ((my->count[3] - 850) % 8 == 0))
  {
    if ((my->count[3] - 850) % 16 == 0)
      t_shoot = 23;
    else
      t_shoot = 9999;
    tenm_table_add(normal_enemy_new(-19.0,
                                    140.0 + (double) (my->count[3] - 769),
                                    BALL_SOLDIER, 0,
                                    0, -1, 0, -1, 0, 1, 2,
                                    /* move 0 */
                                    9999, 5.0, 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    55, t_shoot, 0, 0, 0, 1,
                                    /* shoot 1 */
                                    9999, t_shoot, 55 % t_shoot, 0, 1, 1));
  }
  if ((my->count[3] >= 1000) && (my->count[3] <= 1032)
      && ((my->count[3] - 1000) % 8 == 0))
  {
    if (my->count[3] == 1000)
    {
      what = BRICK;
      t_shoot = 37;
    }
    else
    {
      what = BALL_SOLDIER;
      t_shoot = 9999;
    }
    
    tenm_table_add(normal_enemy_new(32.0,
                                    -19.0,
                                    what, 0,
                                    0, -1, 0, -1, 0, 1, 1,
                                    /* move 0 */
                                    9999, 0.0, 5.0 + 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    9999, t_shoot, 0, 0, 1, 0));
  }
  if ((my->count[3] >= 1050) && (my->count[3] <= 1082)
      && ((my->count[3] - 1050) % 8 == 0))
  {
    if (my->count[3] == 1050)
    {
      what = BRICK;
      t_shoot = 37;
    }
    else
    {
      what = BALL_SOLDIER;
      t_shoot = 9999;
    }
    tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) - 32.0,
                                    -19.0,
                                    what, 0,
                                    0, -1, 0, -1, 0, 1, 1,
                                    /* move 0 */
                                    9999, 0.0, 5.0 + 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    9999, t_shoot, 0, 0, 1, 0));
  }
  if ((my->count[3] >= 1439) && (my->count[3] < 1559)
      && ((my->count[3] - 1439) % 8 == 0))
  {
    if ((my->count[3] - 1439) % 24 == 0)
      t_shoot = 31;
    else
      t_shoot = 9999;
    tenm_table_add(normal_enemy_new(-19.0,
                                    140.0 + (double) (my->count[3] - 1489),
                                    BALL_SOLDIER, 0,
                                    0, -1, 0, -1, 0, 1, 2,
                                    /* move 0 */
                                    9999, 5.0, 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    70, t_shoot, 0, 0, 0, 1,
                                    /* shoot 1 */
                                    9999, t_shoot, 70 % t_shoot, 0, 1, 1));
  }
  if ((my->count[3] >= 1489) && (my->count[3] < 1609)
      && ((my->count[3] - 1489) % 8 == 0))
  {
    if ((my->count[3] - 1489) % 24 == 0)
      t_shoot = 31;
    else
      t_shoot = 9999;
    tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) + 19.0,
                                    20.0 + (double) (my->count[3] - 1489),
                                    BALL_SOLDIER, 0,
                                    0, -1, 0, -1, 0, 1, 2,
                                    /* move 0 */
                                    9999, -5.0, 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    35, t_shoot, 0, 0, 1, 1,
                                    /* shoot 1 */
                                    9999, t_shoot, 35 % t_shoot, 0, 0, 1));
  }
  if ((my->count[3] >= 1870) && (my->count[3] < 1932)
      && ((my->count[3] - 1870) % 8 == 0))
  {
    if ((my->count[3] - 1870) % 24 == 0)
      t_shoot = 31;
    else
      t_shoot = 9999;
    tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) + 19.0,
                                    260.0,
                                    BALL_SOLDIER, 0,
                                    0, -1, 0, -1, 0, 2, 3,
                                    /* move 0 */
                                    60, -3.0, -4.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 0 */
                                    9999, -5.0, 0.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* shoot 0 */
                                    65, t_shoot, 0, 0, 0, 1,
                                    /* shoot 1 */
                                    40, 9999, 0, 0, 1, 2,
                                    /* shoot 2 */
                                    9999, 9999, 0, 0, 0, 2));
    tenm_table_add(normal_enemy_new(-19.0,
                                    260.0,
                                    BALL_SOLDIER, 0,
                                    0, -1, 0, -1, 0, 2, 3,
                                    /* move 0 */
                                    60, 3.0, -4.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 0 */
                                    9999, 5.0, 0.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* shoot 0 */
                                    65, t_shoot, 0, 0, 0, 1,
                                    /* shoot 1 */
                                    40, 9999, 0, 0, 1, 2,
                                    /* shoot 2 */
                                    9999, 9999, 0, 0, 0, 2));
  }

  /* shoot */
  if (my->count[3] < 2200)
  {  
    if (my->count[3] % 130 == 0)
    {
      for (i = 0; i < 360; i += 90)
      {
        tenm_table_add(laser_angle_new(my->x, my->y, 6.0,
                                       45 + i, 25.0, 2));
        tenm_table_add(laser_angle_new(my->x, my->y, 3.5,
                                       45 + i, 25.0, 2));
      }
      for (i = 0; i < 360; i += 180)
      {
        tenm_table_add(laser_angle_new(my->x, my->y, 3.0,
                                       -25 + i, 25.0, 2));
        tenm_table_add(laser_angle_new(my->x, my->y, 3.0,
                                       25 + i, 25.0, 2));
      }
    }
    if (my->count[3] % 43 == 0)
    {
      for (i = -1; i <= 1; i += 2)
      {
        v[0] = player->x - my->x;
        v[1] = player->y - my->y;
        result[0] = v[0];
        result[1] = v[1];
        vector_rotate(result, v, i * 5);
        tenm_table_add(laser_point_new(my->x, my->y, 4.5,
                                       my->x + result[0],
                                       my->y + result[1],
                                       25.0, 1));
      }
    }
  }

  return 0;
}

static int
nexus_draw(tenm_object *my, int priority)
{
  int i;
  double y;
  int status = 0;
  tenm_color color;
  char temp[32];

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "nexus_draw: my is NULL\n");
    return 0;
  }

  if (priority != 0)
    return 0;
  if (my->count[2] != 0)
    return 0;

  /* shaft */
  color = tenm_map_color(95, 13, 68);
  for (i = 0; i < 4; i++)
  {
    y = -29.0 + ((double) (my->count[3])) - 210.0 - 360.0 * ((double) i);
    if (tenm_draw_line((int) (my->x + 30.0), (int) (y),
                       (int) (my->x + 30.0), (int) (y - 180.0),
                       1, color) != 0)
    status = 1;
    if (tenm_draw_line((int) (my->x - 30.0), (int) (y),
                       (int) (my->x - 30.0), (int) (y - 180.0),
                       1, color) != 0)
    status = 1;
  }
  
  /* body */
  if (nexus_green(my))
  {
    if (my->count[1] >= 40)
      color = tenm_map_color(109, 125, 9);
    else
      color = tenm_map_color(61, 95, 13);
  }
  else
  {
    if (my->count[1] >= 40)
      color = tenm_map_color(135, 89, 9);
    else
      color = tenm_map_color(95, 47, 13);
  }

  if (tenm_draw_line((int) (my->x + 30.0), (int) (my->y - 30.0),
                     (int) (my->x + 30.0), (int) (my->y + 30.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x + 30.0), (int) (my->y + 30.0),
                     (int) (my->x - 30.0), (int) (my->y + 30.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 30.0), (int) (my->y + 30.0),
                     (int) (my->x - 30.0), (int) (my->y - 30.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 30.0), (int) (my->y - 30.0),
                     (int) (my->x + 30.0), (int) (my->y - 30.0),
                     3, color) != 0)
    status = 1;

  /* hit point stat */
  if (my->count[1] >= 1)
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 10, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "nexus_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

/* return 1 (true) or 0 (false) */
static int
nexus_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if ((my->count[2] == 0)
      && (my->count[3] >= 359) && (my->count[3] < 2200))
    return 1;

  return 0;
}

static tenm_object *
nexus_hatch_new(int what)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = (double) (WINDOW_WIDTH / 2);
  double y;

  /* sanity check */
  if ((what < 0) || (what > 2))
  {
    fprintf(stderr, "nexus_hatch_new: strange what (%d)\n", what);
    return NULL;
  }

  y = -28.0 - 120.0 - 360.0 * ((double) what);
  if (what == 2)
    y -= 360.0;
  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "nexus_hatch_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + 90.0, y - 90.0,
                                             x + 90.0, y + 90.0,
                                             x - 90.0, y + 90.0,
                                             x - 90.0, y - 90.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "nexus_hatch_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 6);
  if (count == NULL)
  {
    fprintf(stderr, "nexus_hatch_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 3);
  if (count_d == NULL)
  {
    fprintf(stderr, "nexus_hatch_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] life timer
   * [1] what
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */

  count[0] = 1;
  count[1] = what;

  count_d[0] = 0.0;
  count_d[1] = 1.0;

  new = tenm_object_new("Nexus hatch", ATTR_ENEMY, 0,
                        1, x, y,
                        6, count, 3, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&nexus_hatch_move),
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&nexus_hatch_act),
                        (int (*)(tenm_object *, int))
                        (&nexus_hatch_draw));
  if (new == NULL)
  {
    fprintf(stderr, "nexus_hatch_new: tenm_object_new failed\n");
    if (count_d != NULL)
      free(count_d);
    if (count != NULL)
      free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  return new;
}

static int
nexus_hatch_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "nexus_hatch_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "nexus_hatch_move: strange turn_per_frame (%f)\n",
            turn_per_frame);
    return 0;
  }

  dx_temp = my->count_d[0] / turn_per_frame;
  dy_temp = my->count_d[1] / turn_per_frame;
  my->x += dx_temp;
  my->y += dy_temp;
  if (my->mass != NULL)
    tenm_move_mass(my->mass, dx_temp, dy_temp);

  return 0;
}

static int
nexus_hatch_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int t;
  double dy;
  int t_shoot;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "nexus_hatch_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[0])++;

  /* escaped */
  if (my->count[0] > 2400)
    return 1;

  /* speed change */
  if (my->count[0] == 1950)
    my->count_d[1] = 0.5;
  if (my->count[0] == 2230)
    my->count_d[1] = 1.0;

  /* open hatch */
  for (i = -1; i <= 1; i += 2)
  {
    t = my->count[0] - nexus_hatch_open_time(my->count[1], i);
    if ((t >= 40) && (t < 72) && ((t - 40) % 8 == 0))
    {
      if (player->y < my->y - 20.0)
        dy = -5.0;
      else
        dy = 5.0;

      if (t == 40)
        t_shoot = 23;
      else
        t_shoot = 9999;

      tenm_table_add(normal_enemy_new(my->x + 60.0 * ((double) i), my->y,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 2, 1,
                                      /* move 0 */
                                      40, 5.0 * ((double) i), 1.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      9999, 0.0, dy + 1.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* shoot 0 */
                                      9999, t_shoot, 17, 0, 1, 0));
    }
  }

  return 0;
}

static int
nexus_hatch_open_time(int what, int side)
{
  /* sanity check */
  if ((what < 0) || (what > 2))
  {
    fprintf(stderr, "nexus_hatch_open_time: strange what (%d)\n", what);
    return 0;
  }
  if ((side != -1) && (side != 1))
  {
    fprintf(stderr, "nexus_hatch_open_time: strange side (%d)\n", side);
    return 0;
  }

  if (side == -1)
  {
    switch (what)
    {
    case 0:
      return 200;
      break;
    case 1:
      return 600;
      break;
    case 2:
      return 1200;
      break;
    default:
      fprintf(stderr, "nexus_hatch_open_time: undefined what (%d)\n", what);
      return 0;
      break;
    }
  }
  else
  {
    switch (what)
    {
    case 0:
      return 400;
      break;
    case 1:
      return 500;
      break;
    case 2:
      return 1300;
      break;
    default:
      fprintf(stderr, "nexus_hatch_open_time: undefined what (%d)\n", what);
      return 0;
      break;
    }
  }

  fprintf(stderr, "nexus_hatch_open_time: fall off\n");
  return 0;
}

static int
nexus_hatch_draw(tenm_object *my, int priority)
{
  int i;
  int t;
  int theta;
  double c;
  int status = 0;
  tenm_color color;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "nexus_hatch_draw: my is NULL\n");
    return 0;
  }

  if (priority != 0)
    return 0;

  /* decoration */
  color = tenm_map_color(182, 123, 162);

  if (tenm_draw_line((int) (my->x + 30.0), (int) (my->y - 90.0),
                     (int) (my->x + 30.0), (int) (my->y + 90.0),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 30.0), (int) (my->y - 90.0),
                     (int) (my->x - 30.0), (int) (my->y + 90.0),
                     1, color) != 0)
    status = 1;

  /* hatch */
  color = tenm_map_color(95, 13, 68);

  for (i = -1; i <= 1; i += 2)
  {  
    if (tenm_draw_circle((int) (my->x + 60.0 * ((double) i)), (int) (my->y),
                         20, 1, color) != 0)
      status = 1;
    t = my->count[0] - nexus_hatch_open_time(my->count[1], i);
    if (t < 0)
      theta = -45;
    else if (t < 30)
      theta = -45 + t * 3;
    else
      theta = 45;
    if (i == 1)
      theta = 180 - theta;
    c = 30.0 * tenm_sqrt(2);
    if (tenm_draw_line((int) (my->x + 90.0 * ((double) i)),
                       (int) (my->y - 30.0),
                       (int) (my->x + 90.0 * ((double) i)
                              + c * tenm_cos(180 + theta)),
                       (int) (my->y - 30.0 + c * tenm_sin(180 + theta)),
                       1, color) != 0)
      status = 1;
    if (tenm_draw_line((int) (my->x + 90.0 * ((double) i)),
                       (int) (my->y + 30.0),
                       (int) (my->x + 90.0 * ((double) i)
                              + c * tenm_cos(180 - theta)),
                       (int) (my->y + 30.0 + c * tenm_sin(180 - theta)),
                       1, color) != 0)
      status = 1;
  }

  /* body */
  if (tenm_draw_line((int) (my->x + 90.0), (int) (my->y - 90.0),
                     (int) (my->x + 90.0), (int) (my->y + 90.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x + 90.0), (int) (my->y + 90.0),
                     (int) (my->x - 90.0), (int) (my->y + 90.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 90.0), (int) (my->y + 90.0),
                     (int) (my->x - 90.0), (int) (my->y - 90.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 90.0), (int) (my->y - 90.0),
                     (int) (my->x + 90.0), (int) (my->y - 90.0),
                     3, color) != 0)
    status = 1;

  return status;
}

static tenm_object *
nexus_attacker_new(void)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = (double) (WINDOW_WIDTH / 2);
  double y = -28.0 - 120.0 - 360.0 * 2.0;

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "nexus_attacker_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + 90.0, y - 90.0,
                                             x + 90.0, y + 90.0,
                                             x - 90.0, y + 90.0,
                                             x - 90.0, y - 90.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "nexus_attacker_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 6);
  if (count == NULL)
  {
    fprintf(stderr, "nexus_attacker_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 3);
  if (count_d == NULL)
  {
    fprintf(stderr, "nexus_attacker_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] life timer
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */

  count[0] = 1;

  count_d[0] = 0.0;
  count_d[1] = 1.0;

  new = tenm_object_new("Nexus attacker", ATTR_ENEMY, 0,
                        1, x, y,
                        6, count, 3, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&nexus_attacker_move),
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&nexus_attacker_act),
                        (int (*)(tenm_object *, int))
                        (&nexus_attacker_draw));
  if (new == NULL)
  {
    fprintf(stderr, "nexus_attacker_new: tenm_object_new failed\n");
    if (count_d != NULL)
      free(count_d);
    if (count != NULL)
      free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  return new;
}

static int
nexus_attacker_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "nexus_attacker_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "nexus_attacker_move: strange turn_per_frame (%f)\n",
            turn_per_frame);
    return 0;
  }

  dx_temp = my->count_d[0] / turn_per_frame;
  dy_temp = my->count_d[1] / turn_per_frame;
  my->x += dx_temp;
  my->y += dy_temp;
  if (my->mass != NULL)
    tenm_move_mass(my->mass, dx_temp, dy_temp);

  return 0;
}

static int
nexus_attacker_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int theta;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "nexus_attacker_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[0])++;

  /* escaped */
  if (my->count[0] > 2400)
    return 1;

  /* speed change */
  if (my->count[0] == 1950)
    my->count_d[1] = 0.5;
  if (my->count[0] == 2230)
    my->count_d[1] = 1.0;

  /* shoot */
  if (my->count[0] % 37 == 0)
  {
    if (my->count[0] % 74 == 0)
      theta = 10;
    else
      theta = -10;
    for (i = -1; i <= 1; i++)
    {
      tenm_table_add(laser_angle_new(my->x - 90.0,
                                     my->y + 90.0 * ((double) i),
                                     2.5, 180 + theta, 25.0, 0));
      tenm_table_add(laser_angle_new(my->x + 90.0,
                                     my->y + 90.0 * ((double) i),
                                     2.5, -theta, 25.0, 0));
    }
  }
  
  return 0;
}

static int
nexus_attacker_draw(tenm_object *my, int priority)
{
  int i;
  int j;
  int status = 0;
  tenm_color color;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "nexus_attacker_draw: my is NULL\n");
    return 0;
  }

  if (priority != 0)
    return 0;

  /* decoration */
  color = tenm_map_color(182, 123, 162);

  for (i = -1; i <= 1; i++)
  {
    for (j = -10; j <= 10; j += 20)
    {
      if (tenm_draw_line((int) (my->x - 90.0),
                         (int) (my->y + 90.0 * ((double) i)),
                         (int) (my->x - 90.0 + 25.0 * tenm_cos(180 + j)),
                         (int) (my->y + 90.0 * ((double) i)
                                + 25.0 * tenm_sin(180 + j)),
                         1, color) != 0)
        status = 1;
      if (tenm_draw_line((int) (my->x + 90.0),
                         (int) (my->y + 90.0 * ((double) i)),
                         (int) (my->x + 90.0 + 25.0 * tenm_cos(j)),
                         (int) (my->y + 90.0 * ((double) i)
                                + 25.0 * tenm_sin(j)),
                         1, color) != 0)
        status = 1;
    }
  }

  if (tenm_draw_line((int) (my->x + 40.0), (int) (my->y),
                     (int) (my->x + 80.0), (int) (my->y),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 40.0), (int) (my->y),
                     (int) (my->x - 80.0), (int) (my->y),
                     1, color) != 0)
    status = 1;

  if (tenm_draw_line((int) (my->x + 30.0), (int) (my->y - 90.0),
                     (int) (my->x + 30.0), (int) (my->y + 90.0),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 30.0), (int) (my->y - 90.0),
                     (int) (my->x - 30.0), (int) (my->y + 90.0),
                     1, color) != 0)
    status = 1;

  /* body */
  color = tenm_map_color(95, 13, 68);

  if (tenm_draw_line((int) (my->x + 90.0), (int) (my->y - 90.0),
                     (int) (my->x + 90.0), (int) (my->y + 90.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x + 90.0), (int) (my->y + 90.0),
                     (int) (my->x - 90.0), (int) (my->y + 90.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 90.0), (int) (my->y + 90.0),
                     (int) (my->x - 90.0), (int) (my->y - 90.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 90.0), (int) (my->y - 90.0),
                     (int) (my->x + 90.0), (int) (my->y - 90.0),
                     3, color) != 0)
    status = 1;

  return status;
}

static tenm_object *
nexus_head_new(void)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = (double) (WINDOW_WIDTH / 2);
  double y = -28.0 - 120.0 - 360.0 * 4.0;

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "nexus_head_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + 90.0, y - 90.0,
                                             x + 90.0, y + 90.0,
                                             x - 90.0, y + 90.0,
                                             x - 90.0, y - 90.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "nexus_head_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 6);
  if (count == NULL)
  {
    fprintf(stderr, "nexus_head_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 3);
  if (count_d == NULL)
  {
    fprintf(stderr, "nexus_head_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] life timer
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */

  count[0] = 1;

  count_d[0] = 0.0;
  count_d[1] = 1.0;

  new = tenm_object_new("Nexus head", ATTR_ENEMY, 0,
                        1, x, y,
                        6, count, 3, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&nexus_head_move),
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&nexus_head_act),
                        (int (*)(tenm_object *, int))
                        (&nexus_head_draw));
  if (new == NULL)
  {
    fprintf(stderr, "nexus_head_new: tenm_object_new failed\n");
    if (count_d != NULL)
      free(count_d);
    if (count != NULL)
      free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  return new;
}

static int
nexus_head_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "nexus_head_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "nexus_head_move: strange turn_per_frame (%f)\n",
            turn_per_frame);
    return 0;
  }

  dx_temp = my->count_d[0] / turn_per_frame;
  dy_temp = my->count_d[1] / turn_per_frame;
  my->x += dx_temp;
  my->y += dy_temp;
  if (my->mass != NULL)
    tenm_move_mass(my->mass, dx_temp, dy_temp);

  return 0;
}

static int
nexus_head_act(tenm_object *my, const tenm_object *player)
{
  int i;
  double v[2];
  double result[2];
  double a[2];

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "nexus_head_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[0])++;

  /* escaped */
  if (my->count[0] > 2400)
    return 1;

  /* speed change */
  if (my->count[0] == 1950)
    my->count_d[1] = 0.5;
  if (my->count[0] == 2230)
    my->count_d[1] = 1.0;

  /* shoot */
  if ((my->count[0] < 2200) && (my->count[0] % 23 == 0))
  {
    v[0] = 0.0;
    v[1] = -180.0;
    a[0] = player->x - my->x;
    a[1] = player->y - my->y;
    result[0] = v[0];
    result[1] = v[1];
    vector_rotate_bounded(result, v, a, 20);
    tenm_table_add(laser_point_new(my->x, my->y, 7.0,
                                   my->x + result[0],
                                   my->y + result[1],
                                   25.0, 4));
    v[0] = result[0];
    v[1] = result[1];
    for (i = -1; i <= 1; i += 2)
    {
      result[0] = v[0];
      result[1] = v[1];
      vector_rotate(result, v, i * 7);
      tenm_table_add(laser_point_new(my->x, my->y, 5.5,
                                     my->x + result[0],
                                     my->y + result[1],
                                     25.0, 4));
    }
  }

  return 0;
}

static int
nexus_head_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "nexus_head_draw: my is NULL\n");
    return 0;
  }

  if (priority != 0)
    return 0;

  /* decoration */
  color = tenm_map_color(182, 123, 162);

  if (tenm_draw_line((int) (my->x), (int) (my->y),
                     (int) (my->x + 180.0 * tenm_cos(-63)),
                     (int) (my->y + 180.0 * tenm_sin(-63)),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x), (int) (my->y),
                     (int) (my->x + 180.0 * tenm_cos(-117)),
                     (int) (my->y + 180.0 * tenm_sin(-117)),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x + 180.0 * tenm_cos(-63)),
                     (int) (my->y + 180.0 * tenm_sin(-63)),
                     (int) (my->x + 180.0 * tenm_cos(-117)),
                     (int) (my->y + 180.0 * tenm_sin(-117)),
                     1, color) != 0)
    status = 1;

  if (tenm_draw_line((int) (my->x + 30.0), (int) (my->y - 90.0),
                     (int) (my->x + 30.0), (int) (my->y + 90.0),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 30.0), (int) (my->y - 90.0),
                     (int) (my->x - 30.0), (int) (my->y + 90.0),
                     1, color) != 0)
    status = 1;

  /* body */
  color = tenm_map_color(95, 13, 68);

  if (tenm_draw_line((int) (my->x + 90.0), (int) (my->y - 90.0),
                     (int) (my->x + 90.0), (int) (my->y + 90.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x + 90.0), (int) (my->y + 90.0),
                     (int) (my->x - 90.0), (int) (my->y + 90.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 90.0), (int) (my->y + 90.0),
                     (int) (my->x - 90.0), (int) (my->y - 90.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 90.0), (int) (my->y - 90.0),
                     (int) (my->x + 90.0), (int) (my->y - 90.0),
                     3, color) != 0)
    status = 1;

  return status;
}
