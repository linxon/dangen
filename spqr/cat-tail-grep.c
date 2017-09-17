/* $Id: cat-tail-grep.c,v 1.403 2011/08/24 17:31:23 oohara Exp $ */
/* [hard] cat tail (grep mix) */

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
#include "stage-clear.h"
#include "score.h"
#include "ship.h"
/* tenm_same_side, tenm_line_nearer */
#include "tenm_collision.h"

#include "cat-tail-grep.h"

#define NEAR_ZERO 0.0001

static int cat_tail_grep_move(tenm_object *my, double turn_per_frame);
static int cat_tail_grep_hit(tenm_object *my, tenm_object *your);
static void cat_tail_grep_second_form(tenm_object *my);
static void cat_tail_grep_explode(tenm_object *my);
static int cat_tail_grep_signal_turret(tenm_object *my, int n);
static int cat_tail_grep_act(tenm_object *my, const tenm_object *player);
static int cat_tail_grep_draw(tenm_object *my, int priority);
static int cat_tail_grep_green(const tenm_object *my);

static tenm_object *cat_tail_grep_turret_new(int what, int index_body,
                                             int index_turret);
static int cat_tail_grep_turret_move(tenm_object *my, double turn_per_frame);
static int cat_tail_grep_turret_hit(tenm_object *my, tenm_object *your);
static int cat_tail_grep_turret_signal_turret(tenm_object *my, int n);
static int cat_tail_grep_turret_signal_body(tenm_object *my, int n);
static void cat_tail_grep_turret_explode(tenm_object *my);
static int cat_tail_grep_turret_act(tenm_object *my,
                                    const tenm_object *player);
static int cat_tail_grep_turret_draw(tenm_object *my, int priority);
static int cat_tail_grep_turret_green(const tenm_object *my);

static int cat_tail_grep_position(double *result, int mode, int t);

static tenm_object *cat_tail_grep_shot_s_new(double x, double y,
                                             double speed_x, double speed_y,
                                             double target_x, double target_y);
static int cat_tail_grep_shot_s_move(tenm_object *my, double turn_per_frame);
static int cat_tail_grep_shot_s_hit(tenm_object *my, tenm_object *your);
static int cat_tail_grep_shot_s_act(tenm_object *my,
                                    const tenm_object *player);
static int cat_tail_grep_shot_s_draw(tenm_object *my, int priority);

static tenm_object *cat_tail_grep_shot_i_new(double x, double y, double speed,
                                             double rotate_direction,
                                             double target_x, double target_y,
                                             double center_x, double center_y,
                                             int t_fixed);
static int cat_tail_grep_shot_i_move(tenm_object *my, double turn_per_frame);
static int cat_tail_grep_shot_i_hit(tenm_object *my, tenm_object *your);
static int cat_tail_grep_shot_i_act(tenm_object *my,
                                    const tenm_object *player);
static int cat_tail_grep_shot_i_draw(tenm_object *my, int priority);

static tenm_object *cat_tail_grep_shot_v_new(double x, double y,
                                             double speed_x, double speed_y,
                                             int theta_turn,
                                             double target_x, double target_y);
static int cat_tail_grep_shot_v_move(tenm_object *my, double turn_per_frame);
static int cat_tail_grep_shot_v_hit(tenm_object *my, tenm_object *your);
static int cat_tail_grep_shot_v_act(tenm_object *my,
                                    const tenm_object *player);
static int cat_tail_grep_shot_v_draw(tenm_object *my, int priority);

tenm_object *
cat_tail_grep_new(void)
{
  int i;
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double result[2];
  double x;
  double y;
  
  result[0] = 0.0;
  result[1] = 0.0;
  cat_tail_grep_position(result, 0, 0);
  x = result[0];
  y = result[1];

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 3);
  if (p == NULL)
  {
    fprintf(stderr, "cat_tail_grep_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_polygon_new(6,
                                             x + 48.0, y,
                                             x + 24.0, y + 41.5692,
                                             x - 24.0, y + 41.5692,
                                             x - 48.0, y,
                                             x - 24.0, y - 41.5692,
                                             x + 24.0, y - 41.5692);
  if (p[0] == NULL)
  {
    fprintf(stderr, "cat_tail_grep_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }
  p[1] = (tenm_primitive *) tenm_polygon_new(3,
                                             x - 24.0, y - 41.5692,
                                             x - 44.7846, y - 53.5692,
                                             x - 36.0, y - 20.7846);
  if (p[1] == NULL)
  {
    fprintf(stderr, "cat_tail_grep_new: cannot set p[1]\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  p[2] = (tenm_primitive *) tenm_polygon_new(3,
                                             x + 24.0, y - 41.5692,
                                             x + 44.7846, y - 53.5692,
                                             x + 36.0, y - 20.7846);
  if (p[2] == NULL)
  {
    fprintf(stderr, "cat_tail_grep_new: cannot set p[2]\n");
    for (i = 0; i < 2; i++)
      (p[i])->delete(p[i]);
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 10);
  if (count == NULL)
  {
    fprintf(stderr, "cat_tail_grep_new: malloc(count) failed\n");
    for (i = 0; i < 3; i++)
      (p[i])->delete(p[i]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "cat_tail_grep_new: malloc(count_d) failed\n");
    free(count);
    for (i = 0; i < 3; i++)
      (p[i])->delete(p[i]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] for deal_damage
   * [1] "damaged" timer
   * [2] move mode
   * [3] move timer
   * [4 -- 6] turret index
   * [7] hand direction
   * [8] hand theta
   * [9] "was green when killed" flag
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */
  count[0] = 0;
  count[1] = 0;
  count[2] = 0;
  count[3] = 0;
  count[4] = -1;
  count[5] = -1;
  count[6] = -1;
  count[7] = -30;
  count[8] = 0;
  count[9] = 0;

  count_d[0] = 0.0;
  count_d[1] = 0.0;

  new = tenm_object_new("cat tail (grep mix)", ATTR_BOSS, ATTR_PLAYER_SHOT,
                        750, x, y,
                        10, count, 2, count_d, 3, p,
                        (int (*)(tenm_object *, double))
                        (&cat_tail_grep_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&cat_tail_grep_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&cat_tail_grep_act),
                        (int (*)(tenm_object *, int))
                        (&cat_tail_grep_draw));
  if (new == NULL)
  {
    fprintf(stderr, "cat_tail_grep_new: tenm_object_new failed\n");
    if (count_d != NULL)
      free(count_d);
    if (count != NULL)
      free(count);
    for (i = 0; i < 3; i++)
      (p[i])->delete(p[i]);
    free(p);
    return NULL;
  }

  return new;
}

static int
cat_tail_grep_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "cat_tail_grep_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "cat_tail_grep_move: strange turn_per_frame (%f)\n",
            turn_per_frame);
    return 0;
  }

  if ((my->count[2] == 2) && (my->count[3] < 0))
    return 0;

  dx_temp = my->count_d[0] / turn_per_frame;
  dy_temp = my->count_d[1] / turn_per_frame;
  my->x += dx_temp;
  my->y += dy_temp;
  if (my->mass != NULL)
    tenm_move_mass(my->mass, dx_temp, dy_temp);

  return 0;
}

static int
cat_tail_grep_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "cat_tail_grep_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "cat_tail_grep_hit: your is NULL\n");
    return 0;
  }

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;
  if ((my->count[2] != 1) && (my->count[2] != 3))
    return 0;

  deal_damage(my, your, 0);
  if (cat_tail_grep_green(my))
    add_chain(my, your);
  my->count[1] = 2;

  if (my->hit_point <= 0)
  {
    set_background(1);

    if (my->count[2] == 1)
    {
      add_score(7500);
      cat_tail_grep_second_form(my);
      return 0;
    }
    else
    {
      add_score(22500);
      cat_tail_grep_explode(my);
      return 0;
    }
  }

  return 0;
}

static void
cat_tail_grep_second_form(tenm_object *my)
{
  int i;
  int n;

  /* sanity check */
  if (my == NULL)
    return;

  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy_shot), 0);

  for (i = 0; i < 3; i++)
    if (my->count[4 + i] >= 0)
      tenm_table_apply(my->count[4 + i],
                       (int (*)(tenm_object *, int))
                       (&cat_tail_grep_signal_turret),
                       0);

  /* set "was green" flag before we change the life mode */
  if (cat_tail_grep_green(my))
  {
    n = 8;
    my->count[9] = 1;
  }
  else
  {
    n = 7;
    my->count[9] = 0;
  }

  tenm_table_add(explosion_new(my->x, my->y - 31.1769,
                               my->count_d[0] * 0.5,
                               my->count_d[1] * 0.5,
                               1, 1000, n, 3.0, 9));
  tenm_table_add(fragment_new(my->x, my->y - 31.1769,
                              my->count_d[0] * 0.5, my->count_d[1] * 0.5,
                              50.0, 20, n, 5.0, 0.0, 20));

  my->hit_point = 750;
  my->count[2] = 2;
  my->count[3] = -30;
  my->count[1] = 0;
  my->count_d[0] = (((double) (WINDOW_WIDTH / 2)) - my->x) / 240.0;
  my->count_d[1] = (((double) (WINDOW_HEIGHT / 2)) - my->y) / 240.0;
}

static void
cat_tail_grep_explode(tenm_object *my)
{
  int n;
  double x;
  double y;
  double dx;
  double dy;
  int i;

  /* sanity check */
  if (my == NULL)
    return;

  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy_shot), 0);

  /* set "was green" flag before we change the life mode */
  if (cat_tail_grep_green(my))
  {
    n = 8;
    my->count[9] = 1;
  }
  else
  {
    n = 7;
    my->count[9] = 0;
  }

  x = my->x;
  y = my->y;
  for (i = 1; i <= 4; i++)
  {
    dx = 120.0 * tenm_cos(my->count[7] + my->count[8] * i);
    dy = 120.0 * tenm_sin(my->count[7] + my->count[8] * i);
    tenm_table_add(fragment_new(x, y, 0.0, 0.0,
                                50.0, 10, n, 4.0, 15.0, 12));
    tenm_table_add(explosion_new(x + dx / 2.0, y + dy / 2.0, 0.0, 0.0,
                                 1, 1000, n, 10.0, 9));
    x += dx;
    y += dy;
  }
  tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                               1, 5000, n, 10.0, 6));

  my->count[2] = 4;
  my->count[3] = 0;
  my->count[1] = 0;
  my->count_d[0] = 0.0;
  my->count_d[1] = 0.5;

  /* don't modify my->attr or my->hit_mask here, or the player shot
   * may fly through the enemy */
  tenm_mass_delete(my->mass);
  my->mass = NULL;
}

static int
cat_tail_grep_signal_turret(tenm_object *my, int n)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (strcmp(my->name, "cat tail (grep mix) turret") != 0)
    return 0;

  my->count[2] = 3;
  /* don't modify my->attr or my->hit_mask here, or the player shot
   * may fly through the enemy */
  if (my->mass != NULL)
  {
    tenm_mass_delete(my->mass);
    my->mass = NULL;
  }

  return 0;
}

static int
cat_tail_grep_act(tenm_object *my, const tenm_object *player)
{
  int i;
  double result[2];
  double x;
  double y;
  double dx;
  double dy;
  double length;
  int theta;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "cat_tail_grep_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  /* for deal_damage */
  my->count[0] = 0;

  /* "damaged" count down */
  if (my->count[1] > 0)
    (my->count[1])--;

  /* speed change */
  (my->count[3])++;
  result[0] = 0.0;
  result[1] = 0.0;
  if (my->count[2] <= 1)
  {
    cat_tail_grep_position(result, my->count[2], my->count[3]);
    my->count_d[0] = result[0] - my->x;
    my->count_d[1] = result[1] - my->y;
  }
  else if (my->count[2] == 2)
  {
    if (my->count[3] >= 240)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
  }
  else if (my->count[2] == 3)
  {
    dx = player->x - my->x;
    dy = player->y - my->y;
    length = tenm_sqrt((int) (dx * dx + dy * dy));
    if (length < NEAR_ZERO)
      length = 1.0;
    my->count_d[0] = 1.5 * dx / length;
    my->count_d[1] = 1.5 * dy / length;
    if (length < 10.0)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    if ((length > 300.0) || (length < 100.0))
    {
      my->count_d[0] *= 2.0;
      my->count_d[1] *= 2.0;
    }
    if (my->count[3] < 260)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    /* don't move if the player is immutable */
    if ((get_ship() < 0) || (player->count[1] > 0))
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
  }
  else if (my->count[2] == 4)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.5;
  }
  else
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
  }

  /* encounter */
  if (my->count[2] == 0)
  {
    if (my->count[3] == 1)
    {
      my->count[4] = tenm_table_add(cat_tail_grep_turret_new(0,
                                                             my->table_index,
                                                             -1));
      for (i = 1; i < 3; i++)
        my->count[4+i]=tenm_table_add(cat_tail_grep_turret_new(i,
                                                               my->table_index,
                                                               my->count[4]));
    }

    if (my->count[3] >= 120)
    {
      my->count[2] = 1;
      my->count[3] = 0;
    }
    
    return 0;
  }

  /* self-destruction */
  if ((my->count[2] == 1) && (my->count[3] >= 3190))
  {
    set_background(2);
    clear_chain();
    cat_tail_grep_second_form(my);
    return 0;
  }
  if ((my->count[2] == 3) && (my->count[3] >= 2290))
  {
    set_background(2);
    clear_chain();
    cat_tail_grep_explode(my);
    return 0;
  }

  /* second form */
  if (my->count[2] == 2)
  {
    if (my->count[3] >= 250)
    {
      my->count[2] = 3;
      my->count[3] = 0;
    }

    return 0;
  }

  /* dead */
  if (my->count[2] == 4)
  {
    if (cat_tail_grep_green(my))
      i = 8;
    else
      i = 7;

    if ((my->count[3] >= 30) && (my->count[3] <= 75)
        && (my->count[3] % 15 == 0))
    {
      theta = rand() % 360;
      tenm_table_add(explosion_new(my->x + 30.0 * tenm_cos(theta),
                                   my->y + 30.0 * tenm_sin(theta),
                                   0.0, 0.0,
                                   2, 300, i, 5.0, 8));
    }

    if (my->count[3] > 120)
    {
      tenm_table_add(explosion_new(my->x, my->y,
                                   0.0, 0.0,
                                   1, 3000, i, 10.0, 8));
      tenm_table_add(fragment_new(my->x, my->y, 0.0, 0.0,
                                  30.0, 100, i, 4.0, 0.0, 16));
      tenm_table_add(fragment_new(my->x, my->y, 0.0, 0.0,
                                  50.0, 30, i, 2.5, 0.0, 12));

      tenm_table_add(stage_clear_new(100));
      return 1;
    }

    return 0;
  }

  /* move hand */
  if (my->count[2] == 3)
  {
    if (my->count[3] < 120)
    {
      my->count[7] += 2;
      my->count[8] -= 2;
    }
    else if (my->count[3] < 150)
    {
      ;
    }
    else if (my->count[3] < 230)
    {
      my->count[7] -= 3;
      my->count[8] += 3;
    }
    else if (my->count[3] < 260)
    {
      ;
    } 
    else
    {
      if ((my->count[3] - 260) % 195 < 95)
      {
        my->count[7] += 7;
        my->count[8] -= 7;
      }
      else if (((my->count[3] - 260) % 195 >= 100)
               && ((my->count[3] - 260) % 195 < 190))
      {
        my->count[7] -= 7;
        my->count[8] += 7;
      }
    }

    if (my->count[8] > 0)
      my->count[8] -= (my->count[8] + 5) / 6;
    else if (my->count[8] < 0)
      my->count[8] -= (my->count[8] - 5) / 6;
  }

  /* shoot */
  if (my->count[2] == 1)
  {    
    if ((my->count[3] >= 600) && (my->count[3] < 1150))
    { 
      if (my->count[3] % 17 == 0)
      {
        tenm_table_add(laser_angle_new(my->x + 40.0 * tenm_sin(my->count[3]),
                                       my->y,
                                       4.0, 90, 25.0, 3));
      }

      if (my->count[3] % 7 == 0)
      {
        tenm_table_add(laser_angle_new(my->x + 40.0 * tenm_sin(my->count[3]),
                                       my->y,
                                       5.0, 150 - (my->count[3]%50), 25.0, 3));
        tenm_table_add(laser_angle_new(my->x + 40.0 * tenm_sin(my->count[3]),
                                       my->y,
                                       5.0, 30 + (my->count[3]%50), 25.0, 3));
      }

      if ((my->count[5] < 0)
          && (my->count[3] >= 616) && (my->count[3] <= 1141)
          && (my->count[3] % 7 == 0))
      {
        if (my->count[3] % 56 < 28)
          tenm_table_add(cat_tail_grep_shot_i_new(my->x, my->y, 7.0, 1.0,
                                                  player->x, player->y,
                                                  my->x + 30.0,
                                                  my->y + 90.0,
                                                  25));
        else
          tenm_table_add(cat_tail_grep_shot_i_new(my->x, my->y, 7.0, -1.0,
                                                  player->x, player->y,
                                                  my->x - 30.0,
                                                  my->y + 90.0,
                                                  25));
      }
    }

    if ((my->count[3] >= 1350) && (my->count[3] <= 1841))
    {
      if (my->count[3] % 7 == 0)
      {
        tenm_table_add(laser_point_new(my->x + 40.0 * tenm_sin(my->count[3]),
                                       my->y,
                                       9.0,
                                       160.0 + 120.0*tenm_sin(my->count[3]*2),
                                       360.0,
                                       25.0, 3));
        tenm_table_add(laser_point_new(my->x + 40.0 * tenm_sin(my->count[3]),
                                       my->y,
                                       9.0,
                                       480.0 - 120.0*tenm_sin(my->count[3]*2),
                                       360.0,
                                       25.0, 3));
      }

      if ((my->count[6] < 0) && (my->count[3] % 37 == 0))
      {
        tenm_table_add(cat_tail_grep_shot_i_new(my->x, my->y, 7.0, 1.0,
                                                player->x, player->y,
                                                my->x + 15.0,
                                                my->y + 45.0,
                                                12));
        tenm_table_add(cat_tail_grep_shot_i_new(my->x, my->y, 7.0, -1.0,
                                                player->x, player->y,
                                                my->x - 15.0,
                                                my->y + 45.0,
                                                12));
        tenm_table_add(cat_tail_grep_shot_i_new(my->x, my->y, 8.0, 1.0,
                                                player->x, player->y,
                                                my->x + 30.0,
                                                my->y + 90.0,
                                                21));
        tenm_table_add(cat_tail_grep_shot_i_new(my->x, my->y, 8.0, -1.0,
                                                player->x, player->y,
                                                my->x - 30.0,
                                                my->y + 90.0,
                                                21));
        tenm_table_add(cat_tail_grep_shot_i_new(my->x, my->y, 9.0, 1.0,
                                                player->x, player->y,
                                                my->x + 45.0,
                                                my->y + 135.0,
                                                28));
        tenm_table_add(cat_tail_grep_shot_i_new(my->x, my->y, 9.0, -1.0,
                                                player->x, player->y,
                                                my->x - 45.0,
                                                my->y + 135.0,
                                                28));
      }
    }

    if ((my->count[3] >= 1900) && (my->count[3] < 2420))
    {
      if (my->count[3] % 29 == 0)
      {
        tenm_table_add(laser_angle_new(my->x - 20.0,
                                       my->y + 20.0,
                                       4.0, 55, 25.0, 3));
        tenm_table_add(laser_angle_new(my->x + 20.0,
                                       my->y + 20.0,
                                       4.0, 125, 25.0, 3));
        tenm_table_add(laser_angle_new(my->x - 20.0,
                                       my->y - 20.0,
                                       5.5, 60, 25.0, 3));
        tenm_table_add(laser_angle_new(my->x + 20.0,
                                       my->y - 20.0,
                                       5.5, 120, 25.0, 3));

        tenm_table_add(laser_angle_new(my->x - 20.0,
                                       my->y + 20.0,
                                       4.0, 25, 25.0, 3));
        tenm_table_add(laser_angle_new(my->x + 20.0,
                                       my->y + 20.0,
                                       4.0, 155, 25.0, 3));
        tenm_table_add(laser_angle_new(my->x - 20.0,
                                       my->y - 20.0,
                                       5.5, 35, 25.0, 3));
        tenm_table_add(laser_angle_new(my->x + 20.0,
                                       my->y - 20.0,
                                       5.5, 145, 25.0, 3));
      }

      if ((my->count[5] < 0) && (my->count[3] % 7 == 0))
      {
        tenm_table_add(cat_tail_grep_shot_i_new(my->x, my->y, 7.0, 1.0,
                                                player->x, player->y,
                                                my->x + 30.0,
                                                my->y + 90.0,
                                                25));
        tenm_table_add(cat_tail_grep_shot_i_new(my->x, my->y, 7.0, -1.0,
                                                player->x, player->y,
                                                my->x - 30.0,
                                                my->y + 90.0,
                                                25));
      }
    }

    if ((my->count[3] >= 2520) && (my->count[3] < 3060))
    {
      if (my->count[3] % 17 == 0)
      {
        tenm_table_add(laser_angle_new(my->x + 40.0 * tenm_sin(my->count[3]),
                                       my->y,
                                       4.0, 90, 25.0, 3));
      }
      if (my->count[3] % 7 == 0)
      {

        if ((my->count[3] + 270) % 360 < 180)
        {
          tenm_table_add(laser_angle_new(my->x - 40.0 * tenm_sin(my->count[3]),
                                         my->y,
                                         9.0,
                                         75 + ((my->count[3] + 270) % 360) / 2,
                                         25.0, 3));
          tenm_table_add(laser_angle_new(my->x - 40.0 * tenm_sin(my->count[3]),
                                         my->y,
                                         9.0,
                                         15 + ((my->count[3] + 270) % 360) / 2,
                                         25.0, 3));
        }
        else
        {
          tenm_table_add(laser_angle_new(my->x - 40.0 * tenm_sin(my->count[3]),
                                         my->y,
                                         9.0,
                                         255 - ((my->count[3]+270) % 360) / 2,
                                         25.0, 3));
          tenm_table_add(laser_angle_new(my->x - 40.0 * tenm_sin(my->count[3]),
                                         my->y,
                                         9.0,
                                         195 - ((my->count[3]+270) % 360) / 2,
                                         25.0, 3));
        }
      }

      if ((my->count[6] < 0) && (my->count[3] % 5 == 0)
          && (my->count[3] % 20 != 0))
      {
        if (my->count[3] % 60 < 20)
        {
          tenm_table_add(cat_tail_grep_shot_i_new(my->x, my->y, 7.0, 1.0,
                                                  player->x, player->y,
                                                  my->x + 15.0,
                                                  my->y + 45.0,
                                                  12));
          tenm_table_add(cat_tail_grep_shot_i_new(my->x, my->y, 7.0, -1.0,
                                                  player->x, player->y,
                                                  my->x - 15.0,
                                                  my->y + 45.0,
                                                  12));
        }
        else if (my->count[3] % 60 < 40)
        {
          tenm_table_add(cat_tail_grep_shot_i_new(my->x, my->y, 8.0, 1.0,
                                                  player->x, player->y,
                                                  my->x + 30.0,
                                                  my->y + 90.0,
                                                  21));
          tenm_table_add(cat_tail_grep_shot_i_new(my->x, my->y, 8.0, -1.0,
                                                  player->x, player->y,
                                                  my->x - 30.0,
                                                  my->y + 90.0,
                                                  21));
        }
        else
        {
          tenm_table_add(cat_tail_grep_shot_i_new(my->x, my->y, 9.0, 1.0,
                                                  player->x, player->y,
                                                  my->x + 45.0,
                                                  my->y + 135.0,
                                                  28));
          tenm_table_add(cat_tail_grep_shot_i_new(my->x, my->y, 9.0, -1.0,
                                                  player->x, player->y,
                                                  my->x - 45.0,
                                                  my->y + 135.0,
                                                  28));
        }
      }
    }
  }

  if ((my->count[2] == 3) && (my->count[3] % 11 == 0))
  {
    x = my->x;
    y = my->y;
    for (i = 1; i <= 4; i++)
    {
      dx = 120.0 * tenm_cos(my->count[7] + my->count[8] * i);
      dy = 120.0 * tenm_sin(my->count[7] + my->count[8] * i);
      x += dx;
      y += dy;
      tenm_table_add(laser_point_new(x, y, 3.5,
                                     x - dy, y + dx,
                                     25.0, 3));
      tenm_table_add(laser_point_new(x, y, 3.5,
                                     x + dy, y - dx,
                                     25.0, 3));
    }
  }

  return 0;
}

static int
cat_tail_grep_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  char temp[32];
  double x;
  double y;
  double dx;
  double dy;
  int i;
  int theta;
  double length;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "cat_tail_grep_draw: my is NULL\n");
    return 0;
  }

  /* body */
  /* dead enemy has low priority */
  if (((my->count[2] <= 3) && (priority == 0))
      || ((my->count[2] > 3) && (priority == -1)))
  {
    if (cat_tail_grep_green(my))
    {
      if (my->count[1] >= 1)
        color = tenm_map_color(109, 125, 9);
      else
        color = tenm_map_color(61, 95, 13);
    }
    else
    {
      if (my->count[1] >= 1)
        color = tenm_map_color(135, 89, 9);
      else
        color = tenm_map_color(95, 47, 13);
    }

    if (my->count[2] <= 1)
    {
      if (tenm_draw_line((int) (my->x + 36.0), (int) (my->y - 20.7846),
                         (int) (my->x + 48.0), (int) (my->y),
                         3, color))
        status = 1;
      if (tenm_draw_line((int) (my->x - 36.0), (int) (my->y - 20.7846),
                         (int) (my->x + 36.0), (int) (my->y - 20.7846),
                         2, color))
        status = 1;
      if (tenm_draw_line((int) (my->x - 48.0), (int) (my->y),
                         (int) (my->x - 36.0), (int) (my->y - 20.7846),
                         3, color))
        status = 1;
    }
    else
    {
      if (tenm_draw_line((int) (my->x + 24.0), (int) (my->y - 41.5692),
                         (int) (my->x + 48.0), (int) (my->y),
                         3, color))
        status = 1;
      if (tenm_draw_line((int) (my->x - 48.0), (int) (my->y),
                         (int) (my->x - 24.0), (int) (my->y - 41.5692),
                         3, color))
        status = 1;
    }

    if (tenm_draw_line((int) (my->x + 48.0), (int) (my->y),
                       (int) (my->x + 24.0), (int) (my->y + 41.5692),
                       3, color))
      status = 1;
    if (tenm_draw_line((int) (my->x + 24.0), (int) (my->y + 41.5692),
                       (int) (my->x - 24.0), (int) (my->y + 41.5692),
                       3, color))
      status = 1;
    if (tenm_draw_line((int) (my->x - 24.0), (int) (my->y + 41.5692),
                       (int) (my->x - 48.0), (int) (my->y),
                       3, color))
      status = 1;
    if (tenm_draw_line((int) (my->x - 24.0), (int) (my->y - 41.5692),
                       (int) (my->x + 24.0), (int) (my->y - 41.5692),
                       2, color))
      status = 1;

    if (tenm_draw_line((int) (my->x - 24.0), (int) (my->y - 41.5692),
                       (int) (my->x - 44.7846), (int) (my->y - 53.5692),
                       3, color))
      status = 1;
    if (tenm_draw_line((int) (my->x - 44.7846), (int) (my->y - 53.5692),
                       (int) (my->x - 36.0), (int) (my->y - 20.7846),
                       3, color))
      status = 1;

    if (tenm_draw_line((int) (my->x + 24.0), (int) (my->y - 41.5692),
                       (int) (my->x + 44.7846), (int) (my->y - 53.5692),
                       3, color))
      status = 1;
    if (tenm_draw_line((int) (my->x + 44.7846), (int) (my->y - 53.5692),
                       (int) (my->x + 36.0), (int) (my->y - 20.7846),
                       3, color))
      status = 1;

    /* hand */
    if ((my->count[2] == 2) && (my->count[3] > 0))
    {
      x = my->x;
      y = my->y;
      if (my->count[3] < 0)
        length = 0.5;
      else if (my->count[3] < 240)
        length = ((double) (my->count[3])) * 0.5;
      else
        length = 120.0;
      for (i = 1; i <= 4; i++)
      {
        if (my->count[3] < 120)
          theta = 360;
        else if (my->count[3] < 240)
          theta = (120 - my->count[3]) * 3;
        else
          theta = 0;
        dx = length * tenm_cos(my->count[7] + theta * i);
        dy = length * tenm_sin(my->count[7] + theta * i);
        if (tenm_draw_line((int) (x), (int) (y),
                           (int) (x + dx), (int) (y + dy),
                           1, color))
          status = 1;
        if (tenm_draw_circle((int) (x + dx), (int) (y + dy), 5, 1,
                             tenm_map_color(0, 111, 223)) != 0)
          status = 1;
        x += dx;
        y += dy;
      }
    }
    else if (my->count[2] == 3)
    {
      x = my->x;
      y = my->y;
      for (i = 1; i <= 4; i++)
      {
        dx = 120.0 * tenm_cos(my->count[7] + my->count[8] * i);
        dy = 120.0 * tenm_sin(my->count[7] + my->count[8] * i);
        if (tenm_draw_line((int) (x), (int) (y),
                           (int) (x + dx), (int) (y + dy),
                           1, color))
          status = 1;
        if (tenm_draw_circle((int) (x + dx), (int) (y + dy), 5, 1,
                             tenm_map_color(0, 111, 223)) != 0)
          status = 1;
        x += dx;
        y += dy;
      }
    }
  }

  /* hit point stat */
  if ((priority == 0)
      && ((my->count[2] == 1) || (my->count[2] == 3)))
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 5, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "cat_tail_grep_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

/* return 1 (true) or 0 (false) */
static int
cat_tail_grep_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if ((my->count[2] == 1)
      && (my->count[3] >= 600) && (my->count[3] < 3160))
    return 1;
  if ((my->count[2] == 2)
      && (my->count[3] < 0) && (my->count[9] != 0))
    return 1;
  if ((my->count[2] == 3)
      && (my->count[3] >= 260) && (my->count[3] <= 2260))
    return 1;
  if ((my->count[2] == 4) && (my->count[9] != 0))
    return 1;

  return 0;
}

static tenm_object *
cat_tail_grep_turret_new(int what, int index_body, int index_turret)
{
  int i;
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double result[2];
  double x;
  double y;

  result[0] = 0.0;
  result[1] = 0.0;
  cat_tail_grep_position(result, 0, 1);
  x = result[0];
  y = result[1];

  /* sanity check */
  if ((what < 0) || (what > 2))
  {
    fprintf(stderr, "cat_tail_grep_turret_new: strange what (%d)\n", what);
    return NULL;
  }

  switch (what)
  {
  case 0:
    x += 180.0;
    y += 75.0;
    break;
  case 1:
    x += 180.0;
    y += 75.0 + 42.0;
    break;
  case 2:
    x += 180.0;
    y += 75.0 - 42.0;
    break;
  default:
    fprintf(stderr, "cat_tail_grep_turret_new: undefined what (%d)\n", what);
    break;
  }
    
  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "cat_tail_grep_turret_new: malloc(p) failed\n");
    return NULL;
  }

  switch (what)
  {
  case 0:
    p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                               x + 18.0, y + 24.0,
                                               x - 18.0, y + 24.0,
                                               x - 18.0, y - 24.0,
                                               x + 18.0, y - 24.0);
    break;
  case 1:
    p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                               x + 24.0, y + 18.0,
                                               x - 24.0, y + 18.0,
                                               x - 24.0, y - 18.0,
                                               x + 24.0, y - 18.0);
    break;
  case 2:
    p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                               x - 24.0, y - 18.0,
                                               x + 24.0, y - 18.0,
                                               x + 24.0, y + 18.0,
                                               x - 24.0, y + 18.0);
    break;
  default:
    fprintf(stderr, "cat_tail_grep_turret_new: undefined what (%d)\n", what);
    break;
  }

  if (p[0] == NULL)
  {
    fprintf(stderr, "cat_tail_grep_turret_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 9);
  if (count == NULL)
  {
    fprintf(stderr, "cat_tail_grep_turret_new: malloc(count) failed\n");
    for (i = 0; i < 3; i++)
      (p[i])->delete(p[i]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 4);
  if (count_d == NULL)
  {
    fprintf(stderr, "cat_tail_grep_turret_new: malloc(count_d) failed\n");
    free(count);
    for (i = 0; i < 3; i++)
      (p[i])->delete(p[i]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] for deal_damage
   * [1] "damaged" timer
   * [2] move mode
   * [3] move timer
   * [4] what
   * [5] index body
   * [6] index turret center
   * [7] number of turret heads dead
   * [8] shoot timer
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   * [2] front x
   * [3] front y
   */
  count[0] = 0;
  count[1] = 0;
  count[2] = 0;
  count[3] = 1;
  count[4] = what;
  count[5] = index_body;
  count[6] = index_turret;
  count[7] = 0;
  count[8] = 0;

  count_d[0] = 0.0;
  count_d[1] = 0.0;
  switch (what)
  {
  case 0:
    count_d[2] = 0.0;
    count_d[3] = 24.0;
    break;
  case 1:
    count_d[2] = 0.0;
    count_d[3] = 18.0;
    break;
  case 2:
    count_d[2] = 0.0;
    count_d[3] = -18.0;
    break;
  default:
    fprintf(stderr, "cat_tail_grep_turret_new: undefined what (%d)\n", what);
    break;
  }
  
  new = tenm_object_new("cat tail (grep mix) turret",
                        ATTR_BOSS, ATTR_PLAYER_SHOT,
                        750, x, y,
                        9, count, 4, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&cat_tail_grep_turret_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&cat_tail_grep_turret_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&cat_tail_grep_turret_act),
                        (int (*)(tenm_object *, int))
                        (&cat_tail_grep_turret_draw));
  if (new == NULL)
  {
    fprintf(stderr, "cat_tail_grep_turret_new: tenm_object_new failed\n");
    if (count_d != NULL)
      free(count_d);
    if (count != NULL)
      free(count);
    for (i = 0; i < 3; i++)
      (p[i])->delete(p[i]);
    free(p);
    return NULL;
  }

  return new;
}

static int
cat_tail_grep_turret_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "cat_tail_grep_turret_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "cat_tail_grep_turret_move: strange turn_per_frame (%f)\n",
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
cat_tail_grep_turret_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "cat_tail_grep_turret_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "cat_tail_grep_turret_hit: your is NULL\n");
    return 0;
  }

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;
  if (my->count[2] != 1)
    return 0;
  if (my->count[4] == 0)
    return 0;

  deal_damage(my, your, 0);
  if (cat_tail_grep_turret_green(my))
    add_chain(my, your);
  my->count[1] = 41;

  if (my->hit_point <= 0)
  {
    add_score(20000);
    cat_tail_grep_turret_explode(my);

    tenm_table_apply(my->count[6],
                     (int (*)(tenm_object *, int))
                     (&cat_tail_grep_turret_signal_turret),
                     0);
    tenm_table_apply(my->count[5],
                     (int (*)(tenm_object *, int))
                     (&cat_tail_grep_turret_signal_body),
                     my->count[4]);
    return 1;
  }

  return 0;
}

static int
cat_tail_grep_turret_signal_turret(tenm_object *my, int n)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (strcmp(my->name, "cat tail (grep mix) turret") != 0)
    return 0;

  (my->count[7])++;
  if (my->count[7] >= 2)
  {  
    my->count[2] = 2;
    /* don't modify my->attr or my->hit_mask here, or the player shot
     * may fly through the enemy */
    if (my->mass != NULL)
    {
      tenm_mass_delete(my->mass);
      my->mass = NULL;
    }
  }

  return 0;
}

static int
cat_tail_grep_turret_signal_body(tenm_object *my, int n)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (strcmp(my->name, "cat tail (grep mix)") != 0)
    return 0;

  my->count[4 + n] = -1;

  return 0;
}

static void
cat_tail_grep_turret_explode(tenm_object *my)
{
  int n;

  /* sanity check */
  if (my == NULL)
    return;

  if (my->count[4] == 0)
    n = 9;
  else if (cat_tail_grep_turret_green(my))
    n = 8;
  else
    n = 7;

  tenm_table_add(explosion_new(my->x, my->y,
                               my->count_d[0] * 0.5,
                               my->count_d[1] * 0.5,
                               1, 1000, n, 8.0, 6));
  tenm_table_add(fragment_new(my->x, my->y,
                              my->count_d[0] * 0.5, my->count_d[1] * 0.5,
                              30.0, 30, n, 5.0, 0.0, 20));
}

static int
cat_tail_grep_turret_act(tenm_object *my, const tenm_object *player)
{
  double result[2];
  double temp[2];
  double v[2];
  double a[2];
  double x;
  double y;
  double temp_x;
  double temp_y;
  double result_length;
  int n;
  double length;
  int dtheta;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "cat_tail_grep_turret_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  /* for deal_damage */
  my->count[0] = 0;

  /* "damaged" count down */
  if (my->count[1] > 0)
    (my->count[1])--;
  
  /* self-destruction */
  if (my->count[2] == 3)
  {
    cat_tail_grep_turret_explode(my);
    return 1;
  }

  /* aim at the player */
  (my->count[3])++;
  temp[0] = 0.0;
  temp[1] = 0.0;
  cat_tail_grep_position(temp, my->count[2], my->count[3]);

  if ((my->count[2] == 0) || (my->count[2] == 2))
  {
    result[0] = my->count_d[2];
    result[1] = my->count_d[3];
  }
  else
  {  
    v[0] = my->count_d[2];
    v[1] = my->count_d[3];
    a[0] = player->x - (temp[0] + 180.0);
    a[1] = player->y - (temp[1] + 75.0);
    if ((my->count[3] < 250)
        || ((my->count[3] >= 530) && (my->count[3] < 1200))
        || ((my->count[3] >= 1840) && (my->count[3] < 2440)))
    {
      if (my->count[4] == 2)
      {
        a[0] *= -1.0;
        a[1] *= -1.0;
      }
    }
    else
    {
      if (my->count[4] != 2)
      {
        a[0] *= -1.0;
        a[1] *= -1.0;
      }
    }
    if (((my->count[3] >= 250) && (my->count[3] < 310))
        || ((my->count[3] >= 530) && (my->count[3] < 590))
        || ((my->count[3] >= 1200) && (my->count[3] < 1260))
        || ((my->count[3] >= 1840) && (my->count[3] < 1900))
        || ((my->count[3] >= 2440) && (my->count[3] < 2500)))
      dtheta = 3;
    else
      dtheta = 1;
    result[0] = 0.0;
    result[1] = 0.0;
    vector_rotate_bounded(result, v, a, dtheta);
  }

  /* speed change */
  result_length = result[0] * result[0] + result[1] * result[1];
  result_length = tenm_sqrt((int) result_length);
  if ((my->count[4] == 0) || (result_length < NEAR_ZERO))
  {
    my->count_d[0] = (temp[0] + 180.0) - my->x;
    my->count_d[1] = (temp[1] + 75.0) - my->y;
  }
  else
  {
    x = result[0] * 42.0 / result_length;
    y = result[1] * 42.0 / result_length;
    x += temp[0] + 180.0;
    y += temp[1] + 75.0;

    my->count_d[0] = x - my->x;
    my->count_d[1] = y - my->y;
  }

  /* encounter */
  if (my->count[2] == 0)
  {
    if (my->count[3] >= 120)
    {  
      my->count[2] = 1;
      my->count[3] = 0;
    }
    return 0;
  }

  /* dead */
  if (my->count[2] == 2)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.5;

    (my->count[8])++;
    if ((my->count[8] >= 10) && (my->count[8] <= 55)
        && (my->count[8] % 15 == 10))
    {
      if (my->count[4] == 0)
        n = 9;
      else if (cat_tail_grep_turret_green(my))
        n = 8;
      else
        n = 7;
      tenm_table_add(explosion_new(my->x + ((double) (-30 + (rand() % 61))),
                                   my->y + ((double) (-30 + (rand() % 61))),
                                   0.0, 0.0,
                                   2, 300, n, 5.0, 8));
    }

    if (my->count[8] >= 100)
      my->count[2] = 3;
    return 0;
  }

  /* shape change */
  my->count_d[2] = result[0];
  my->count_d[3] = result[1];

  x = my->count_d[2];
  y = my->count_d[3];
  temp_x = y;
  temp_y = -x;
  if (my->count[4] == 0)
  {
    temp_x *= 3.0 / 4.0;
    temp_y *= 3.0 / 4.0;
  }
  else
  {
    temp_x *= 4.0 / 3.0;
    temp_y *= 4.0 / 3.0;
  }

  if (my->mass != NULL)
  {
    (((tenm_polygon *)(my->mass->p[0]))->v[0])->x = my->x + x + temp_x;
    (((tenm_polygon *)(my->mass->p[0]))->v[0])->y = my->y + y + temp_y;
    (((tenm_polygon *)(my->mass->p[0]))->v[1])->x = my->x + x - temp_x;
    (((tenm_polygon *)(my->mass->p[0]))->v[1])->y = my->y + y - temp_y;
    (((tenm_polygon *)(my->mass->p[0]))->v[2])->x = my->x - x - temp_x;
    (((tenm_polygon *)(my->mass->p[0]))->v[2])->y = my->y - y - temp_y;
    (((tenm_polygon *)(my->mass->p[0]))->v[3])->x = my->x - x + temp_x;
    (((tenm_polygon *)(my->mass->p[0]))->v[3])->y = my->y - y + temp_y;
  }

  if (my->count[2] != 1)
    return 0;

  /* shoot */
  length = tenm_sqrt((int) (my->count_d[2] * my->count_d[2]
                            + my->count_d[3] * my->count_d[3]));
  if (length < NEAR_ZERO)
    length = 1.0;

  if ((my->count[3] >= 70) && (my->count[3] < 200)
      && (my->count[4] == 1) && (my->count[3] % 10 == 0))
  {
    tenm_table_add(cat_tail_grep_shot_v_new(my->x, my->y,
                                            7.0 * my->count_d[3] / length,
                                            7.0 * (-my->count_d[2]) / length,
                                            60 + (((my->count[3]+60) % 130)/2),
                                            player->x, player->y));
    tenm_table_add(cat_tail_grep_shot_v_new(my->x, my->y,
                                            7.0 * (-my->count_d[3]) / length,
                                            7.0 * my->count_d[2] / length,
                                            60 + (((my->count[3]+60) % 130)/2),
                                            player->x, player->y));
  }

  if ((((my->count[3] >= 350) && (my->count[3] < 480))
       || ((my->count[3] >= 1300) && (my->count[3] < 1820))
       || ((my->count[3] >= 2520) && (my->count[3] < 3040)))
      && (my->count[4] == 2) && (my->count[3] % 10 == 0))
  {
    tenm_table_add(cat_tail_grep_shot_s_new(my->x, my->y,
                                            7.0 * my->count_d[3] / length,
                                            7.0 * (-my->count_d[2]) / length,
                                            player->x, player->y));
    tenm_table_add(cat_tail_grep_shot_s_new(my->x, my->y,
                                            7.0 * (-my->count_d[3]) / length,
                                            7.0 * my->count_d[2] / length,
                                            player->x, player->y));
  }

  if ((((my->count[3] >= 600) && (my->count[3] < 1120))
       || ((my->count[3] >= 1900) && (my->count[3] < 2420)))
      && (my->count[4] == 1) && (my->count[3] % 10 == 0))
  {
    tenm_table_add(cat_tail_grep_shot_v_new(my->x, my->y,
                                            7.0 * my->count_d[2] / length,
                                            7.0 * my->count_d[3] / length,
                                            60 + (((my->count[3]+50) % 130)/2),
                                            player->x, player->y));
    tenm_table_add(cat_tail_grep_shot_v_new(my->x, my->y,
                                            7.0 * my->count_d[3] / length,
                                            7.0 * (-my->count_d[2]) / length,
                                            60 + (((my->count[3]+110)%130)/2),
                                            player->x, player->y));
    tenm_table_add(cat_tail_grep_shot_v_new(my->x, my->y,
                                            7.0 * (-my->count_d[3]) / length,
                                            7.0 * my->count_d[2] / length,
                                            60 + (((my->count[3]+110)%130)/2),
                                            player->x, player->y));
  }

  return 0;
}

static int
cat_tail_grep_turret_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  char temp[32];
  double x;
  double y;
  double temp_x;
  double temp_y;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "cat_tail_grep_turret_draw: my is NULL\n");
    return 0;
  }

  /* dead enemy has low priority */
  if (((my->count[2] <= 1) && (priority == 0))
      || ((my->count[2] > 1) && (priority == -1)))
  {
    /* decoration */
    if ((my->count[4] == 0) && (my->count[2] <= 1))
    {
      color = tenm_map_color(182, 123, 162);

      if (tenm_draw_line((int) (my->x), (int) (my->y),
                         (int) (my->x - 180.0), (int) (my->y - 75.0),
                         1, color))
        status = 1;
    }

    /* body */
    if (my->count[4] == 0)
    {
      if (my->count[2] > 1)
        color = tenm_map_color(182, 123, 162);
      else
        color = tenm_map_color(95, 13, 68);
    }
    else if (cat_tail_grep_turret_green(my))
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

    x = my->count_d[2];
    y = my->count_d[3];
    temp_x = y;
    temp_y = -x;
    if (my->count[4] == 0)
    {
      temp_x *= 3.0 / 4.0;
      temp_y *= 3.0 / 4.0;
    }
    else
    {
      temp_x *= 4.0 / 3.0;
      temp_y *= 4.0 / 3.0;
    }

    if (tenm_draw_line((int) (my->x + x + temp_x), (int) (my->y + y + temp_y),
                       (int) (my->x + x - temp_x), (int) (my->y + y - temp_y),
                       3, color))
      status = 1;
    if (tenm_draw_line((int) (my->x + x - temp_x), (int) (my->y + y - temp_y),
                       (int) (my->x - x - temp_x), (int) (my->y - y - temp_y),
                       3, color))
      status = 1;
    if (tenm_draw_line((int) (my->x - x - temp_x), (int) (my->y - y - temp_y),
                       (int) (my->x - x + temp_x), (int) (my->y - y + temp_y),
                       3, color))
      status = 1;
    if (tenm_draw_line((int) (my->x - x + temp_x), (int) (my->y - y + temp_y),
                       (int) (my->x + x + temp_x), (int) (my->y + y + temp_y),
                       3, color))
      status = 1;
  }

  /* hit point stat */
  if ((priority == 0) && (my->count[4] != 0) && (my->count[1] > 0))
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 10, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "cat_tail_grep_turret_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

/* return 1 (true) or 0 (false) */
static int
cat_tail_grep_turret_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if (my->count[4] == 0)
    return 0;

  if (my->count[2] == 1)
  {
    if (my->count[3] >= 3160)
      return 0;
    if ((my->count[3] < 250)
        || ((my->count[3] >= 530) && (my->count[3] < 1200))
        || ((my->count[3] >= 1840) && (my->count[3] < 2440)))
    {
      if (my->count[4] == 1)
        return 1;
    }
    else
    {
      if (my->count[4] == 2)
        return 1;
    }
  }

  return 0;
}

/* set result (arg 1) to the position of the boss at time t (arg 3)
 * result (arg 1) must be double[2] (you must allocate enough memory
 * before calling this function)
 * return 0 on success, 1 on error
 */
static int
cat_tail_grep_position(double *result, int mode, int t)
{
  /* sanity check */
  if (result == NULL)
  {
    fprintf(stderr, "cat_tail_grep_position: result is NULL\n");
    return 1;
  }
  if (t < 0)
  {
    fprintf(stderr, "cat_tail_grep_position: t is negative (%d)\n", t);
    return 1;
  }

  if (mode == 0)
  {
    result[0] = (double) (WINDOW_WIDTH / 2);
    result[1] = -150.0
      + ((double) (WINDOW_HEIGHT / 4) -  (-150.0)) * ((double) t) / 120.0;
  }
  else
  {
    result[0] = (double) (WINDOW_WIDTH / 2) + 100.0 * tenm_sin(t);
    result[1] = (double) (WINDOW_HEIGHT / 4);
  }
  
  return 0;
}

static tenm_object *
cat_tail_grep_shot_s_new(double x, double y, double speed_x, double speed_y,
                         double target_x, double target_y)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double temp;
  double t;
  double temp_x;
  double temp_y;
  double next_x;
  double next_y;
  double next_length;

  /* sanity check */
  if (speed_x * speed_x + speed_y * speed_y < NEAR_ZERO)
  {
    fprintf(stderr, "cat_tail_grep_shot_s_new: speed is too small "
            "(%f, %f)\n", speed_x, speed_y);
    return NULL;
  }

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_s_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_circle_new(x, y, 5.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_s_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 3);
  if (count == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_s_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 7);
  if (count_d == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_s_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  temp = speed_x * (y - target_y) - speed_y * (x - target_x);
  if ((temp > -NEAR_ZERO) && (temp < NEAR_ZERO))
  {
    /* the target is exactly in front of me or behind me */
    count[1] = 0;
    count[2] = -1;
    count_d[2] = 0.0;
    count_d[3] = 0.0;
    count_d[4] = 0.0;
    count_d[5] = 0.0;
    count_d[6] = 0.0;
  }
  else
  {
    count[1] = 1;
    t = (x - target_x) * (x - target_x) + (y - target_y) * (y - target_y);
    t /= 2.0 * temp;
    count_d[2] = x + t * speed_y;
    count_d[3] = y - t * speed_x;
    temp_x = x - count_d[2];
    temp_y = y - count_d[3];
    count_d[4] = tenm_sqrt((int) (temp_x * temp_x + temp_y * temp_y));
    if (count_d[4] < NEAR_ZERO)
    {
      /* something is wrong, fall back to straight move  */
      count[1] = 0;
      count[2] = -1;
      count_d[2] = 0.0;
      count_d[3] = 0.0;
      count_d[4] = 0.0;
      count_d[5] = 0.0;
      count_d[6] = 0.0;
    }
    else
    { 
      if (-temp_y * (temp_x + speed_x) + temp_x * (temp_y + speed_y) > 0.0)
        count_d[5] = 1.0;
      else
        count_d[5] = -1.0;
      count_d[6] = tenm_sqrt((int) (speed_x * speed_x + speed_y * speed_y));
      if (count_d[6] < NEAR_ZERO)
        count_d[6] = 1.0;

      count[2] = 1 + (int) (count_d[4] * 2.0 * 3.1415 / count_d[6]);
      if (count[2] <= 0)
        count[2] = 1;

      next_x = x + speed_x - count_d[2];
      next_y = y + speed_y - count_d[3];
      next_length = tenm_sqrt((int) (next_x * next_x + next_y * next_y));
      if (next_length < NEAR_ZERO)
        next_length = 1.0;
      next_x *= count_d[4] / next_length;
      next_y *= count_d[4] / next_length;

      speed_x = count_d[2] + next_x - x;
      speed_y = count_d[3] + next_y - y;
    }
  }

  /* list of count
   * [0] color (for delete_enemy_shot)
   * [1] homing flag
   * [2] rotate timer
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   * [2] center x
   * [3] center y
   * [4] rotate radius
   * [5] rotate direction
   * [6] rotate speed
   */
  count[0] = 0;
  count_d[0] = speed_x;
  count_d[1] = speed_y;

  new = tenm_object_new("cat tail shot s", ATTR_ENEMY_SHOT, ATTR_OPAQUE,
                        1, x, y,
                        3, count, 7, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&cat_tail_grep_shot_s_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&cat_tail_grep_shot_s_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&cat_tail_grep_shot_s_act),
                        (int (*)(tenm_object *, int))
                        (&cat_tail_grep_shot_s_draw));
  if (new == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_s_new: tenm_object_new failed\n");
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
cat_tail_grep_shot_s_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_s_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "cat_tail_grep_shot_s_move: strange turn_per_frame (%f)\n",
            turn_per_frame);
    return 0;
  }

  dx_temp = my->count_d[0] / turn_per_frame;
  dy_temp = my->count_d[1] / turn_per_frame;
  my->x += dx_temp;
  my->y += dy_temp;
  tenm_move_mass(my->mass, dx_temp, dy_temp);

  if (!in_window_object(my))
    return 1;

  return 0;
}

static int
cat_tail_grep_shot_s_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_s_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_s_hit: your is NULL\n");
    return 0;
  }

  if (your->attr & ATTR_OPAQUE)
    return 1;

  return 0;
}

static int
cat_tail_grep_shot_s_act(tenm_object *my, const tenm_object *player)
{
  double r_x;
  double r_y;
  double r_length;
  double dr_x;
  double dr_y;
  double next_x;
  double next_y;
  double next_length;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_s_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  if (my->count[1] == 0)
    return 0;
  (my->count[2])--;
  if (my->count[2] < 0)
    return 0;

  r_x = my->x - my->count_d[2];
  r_y = my->y - my->count_d[3];
  r_length = tenm_sqrt((int) (r_x * r_x + r_y * r_y));
  if (r_length < NEAR_ZERO)
    r_length = 1.0;
  r_x /= r_length;
  r_y /= r_length;

  dr_x = -r_y * my->count_d[5];
  dr_y = r_x * my->count_d[5];

  next_x = my->x + my->count_d[6] * dr_x - my->count_d[2];
  next_y = my->y + my->count_d[6] * dr_y - my->count_d[3];
  next_length = tenm_sqrt((int) (next_x * next_x + next_y * next_y));
  if (next_length < NEAR_ZERO)
    next_length = 1.0;
  next_x *= my->count_d[4] / next_length;
  next_y *= my->count_d[4] / next_length;

  my->count_d[0] = my->count_d[2] + next_x - my->x;
  my->count_d[1] = my->count_d[3] + next_y - my->y;

  return 0;
}

static int
cat_tail_grep_shot_s_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_s_draw: my is NULL\n");
    return 0;
  }

  /* return if it is not my turn */
  if (priority != 1)
    return 0;

  color = tenm_map_color(0, 191, 47);

  if (tenm_draw_circle((int) (my->x), (int) (my->y), 5, 2, color) != 0)
    status = 1;

  return status;
}

static tenm_object *
cat_tail_grep_shot_i_new(double x, double y, double speed,
                         double rotate_direction,
                         double target_x, double target_y,
                         double center_x, double center_y, int t_fixed)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double speed_x;
  double speed_y;
  double temp;
  double temp_x;
  double temp_y;

  /* sanity check */
  if (speed < NEAR_ZERO)
  {
    fprintf(stderr, "cat_tail_grep_shot_i_new: speed is non-positive (%f)\n",
            speed);
    return NULL;
  }
  if (t_fixed <= 0)
  {
    fprintf(stderr, "cat_tail_grep_shot_i_new: t_fixed is non-positive (%d)\n",
            t_fixed);
    return NULL;
  }

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_i_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_circle_new(x, y, 5.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_i_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 3);
  if (count == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_i_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 11);
  if (count_d == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_i_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  temp_x = x - center_x;
  temp_y = y - center_y;
  temp = temp_x * temp_x + temp_y * temp_y;
  temp = tenm_sqrt((int) temp);
  if (temp < NEAR_ZERO)
    temp = 1.0;

  speed_x = speed * (-temp_y / temp);
  speed_y = speed * (temp_x / temp);

  /* list of count
   * [0] color (for delete_enemy_shot)
   * [1] homing mode
   * [2] homing timer
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   * [2] center x
   * [3] center y
   * [4] rotate radius
   * [5] rotate direction
   * [6] rotate speed
   * [7] target x
   * [8] target y
   * [9] rotate end x
   * [10] rotate end y
   */
  count[0] = 4;
  count[1] = 1;
  count[2] = t_fixed;
  count_d[0] = speed_x;
  count_d[1] = speed_y;
  count_d[2] = center_x;
  count_d[3] = center_y;
  count_d[4] = temp;
  if (rotate_direction > 0.0)
    count_d[5] = 1.0;
  else
    count_d[5] = -1.0;
  count_d[0] *= count_d[5];
  count_d[1] *= count_d[5];
  count_d[6] = speed;
  count_d[7] = target_x;
  count_d[8] = target_y;

  new = tenm_object_new("cat tail shot i", ATTR_ENEMY_SHOT, ATTR_OPAQUE,
                        1, x, y,
                        3, count, 11, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&cat_tail_grep_shot_i_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&cat_tail_grep_shot_i_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&cat_tail_grep_shot_i_act),
                        (int (*)(tenm_object *, int))
                        (&cat_tail_grep_shot_i_draw));
  if (new == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_i_new: tenm_object_new failed\n");
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
cat_tail_grep_shot_i_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_i_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "cat_tail_grep_shot_i_move: strange turn_per_frame (%f)\n",
            turn_per_frame);
    return 0;
  }

  dx_temp = my->count_d[0] / turn_per_frame;
  dy_temp = my->count_d[1] / turn_per_frame;
  my->x += dx_temp;
  my->y += dy_temp;
  tenm_move_mass(my->mass, dx_temp, dy_temp);

  if (!in_window_object(my))
    return 1;

  return 0;
}

static int
cat_tail_grep_shot_i_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_i_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_i_hit: your is NULL\n");
    return 0;
  }

  if (your->attr & ATTR_OPAQUE)
    return 1;

  return 0;
}

static int
cat_tail_grep_shot_i_act(tenm_object *my, const tenm_object *player)
{
  double r_x;
  double r_y;
  double r_length;
  double dr_x;
  double dr_y;
  double next_x;
  double next_y;
  double next_length;
  double temp;
  double speed_x;
  double speed_y;
  double t;
  double temp_x;
  double temp_y;
  double p_x[2];
  double p_y[2];
  double h_x;
  double h_y;
  int first;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_i_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  if (my->count[1] == 0)
    return 0;

  /* mode change */
  if (my->count[1] == 1)
  {
    (my->count[2])--;
    if (my->count[2] < 0)
    {
      speed_x = my->count_d[0];
      speed_y = my->count_d[1];
      temp = speed_x * (my->y - my->count_d[8])
        - speed_y * (my->x - my->count_d[7]);
      if ((temp > -NEAR_ZERO) && (temp < NEAR_ZERO))
      {
        /* the target is exactly in front of me or behind me */
        my->count[1] = 0;
        my->count[2] = -1;
        my->count_d[2] = 0.0;
        my->count_d[3] = 0.0;
        my->count_d[4] = 0.0;
        my->count_d[5] = 0.0;
        my->count_d[6] = 0.0;
        my->count_d[7] = 0.0;
        my->count_d[8] = 0.0;
        return 0;
      }
      else
      {
        my->count[1] = 2;
        t = (my->x - my->count_d[7]) * (my->x - my->count_d[7])
          + (my->y - my->count_d[8]) * (my->y - my->count_d[8]);
        t /= 2.0 * temp;
        my->count_d[2] = my->x + t * speed_y;
        my->count_d[3] = my->y - t * speed_x;
        temp_x = my->x - my->count_d[2];
        temp_y = my->y - my->count_d[3];
        my->count_d[4] = tenm_sqrt((int) (temp_x * temp_x + temp_y * temp_y));
        if (my->count_d[4] < NEAR_ZERO)
        {
          /* something is wrong, fall back to straight move */
          my->count[1] = 0;
          my->count[2] = -1;
          my->count_d[2] = 0.0;
          my->count_d[3] = 0.0;
          my->count_d[4] = 0.0;
          my->count_d[5] = 0.0;
          my->count_d[6] = 0.0;
          my->count_d[7] = 0.0;
          my->count_d[8] = 0.0;
          return 0;
        }

        t = (my->x - my->count_d[7]) * (my->x - my->count_d[7])
          + (my->y - my->count_d[8]) * (my->y - my->count_d[8]);
        t = tenm_sqrt((int) t);
        if (t < NEAR_ZERO)
          t = 1.0;
        speed_x = my->count_d[0];
        speed_y = my->count_d[1];
        temp_x = my->x - my->count_d[2];
        temp_y = my->y - my->count_d[3];
        my->count_d[4] = tenm_sqrt((int) (temp_x * temp_x + temp_y * temp_y));
        if (my->count_d[4] < NEAR_ZERO)
          my->count_d[4] = 1.0;
        if (-temp_y * (temp_x + speed_x) + temp_x * (temp_y + speed_y) > 0.0)
          my->count_d[5] = 1.0;
        else
          my->count_d[5] = -1.0;
        if ((speed_x * (my->count_d[7] - my->x)
            + speed_y * (my->count_d[8] - my->y)
            < tenm_cos(45) * my->count_d[6] * t)
             && (my->count_d[4] > 160.0))
        {
          r_length = 50.0;
          my->count[1] = 3;
          my->count_d[2] = -r_length * speed_y / my->count_d[6];
          my->count_d[3] = r_length * speed_x / my->count_d[6];
          my->count_d[2] *= my->count_d[5];
          my->count_d[3] *= my->count_d[5];
          my->count_d[2] += my->x;
          my->count_d[3] += my->y;
          my->count_d[4] = r_length;
          temp_x = my->count_d[7] - my->count_d[2];
          temp_y = my->count_d[8] - my->count_d[3];
          temp = temp_x * temp_x + temp_y * temp_y;
          temp = tenm_sqrt((int) temp);
          if (temp < NEAR_ZERO)
            temp = 1.0;
          t = r_length * r_length / temp;
          h_x = my->count_d[2] + t * temp_x / temp;
          h_y = my->count_d[3] + t * temp_y / temp;
          t = temp_x * temp_x + temp_y * temp_y - r_length * r_length;
          t = tenm_sqrt((int) t);
          if (t < NEAR_ZERO)
            t = 1.0;
          t *= r_length / temp;
          p_x[0] = h_x - t * temp_y / temp;
          p_y[0] = h_y + t * temp_x / temp;
          p_x[1] = h_x + t * temp_y / temp;
          p_y[1] = h_y - t * temp_x / temp;
          if (tenm_same_side(p_x[0], p_y[0], p_x[1], p_y[1],
                             my->count_d[2], my->count_d[3], my->x, my->y))
          {
            if (tenm_same_side(p_x[0], p_y[0], my->x, my->y,
                               my->count_d[2], my->count_d[3],
                               my->count_d[7], my->count_d[8]))
              first = 0;
            else
              first = 1;
            if (tenm_same_side(my->count_d[7], my->count_d[8],
                               my->x + my->count_d[0],
                               my->y + my->count_d[1],
                               my->count_d[2], my->count_d[3],
                               my->x, my->y))
              first = first;
            else
              first = 1 - first;
          }
          else
          {
            if (tenm_same_side(p_x[0], p_y[0],
                               my->x + my->count_d[0],
                               my->y + my->count_d[1],
                               my->count_d[2], my->count_d[3],
                               my->x, my->y))
              first = 0;
            else
              first = 1;
          }
          if (tenm_same_side(my->x, my->y, my->count_d[7], my->count_d[8],
                             p_x[0], p_y[0], p_x[1], p_y[1]))
          {
            my->count_d[9] = p_x[1 - first];
            my->count_d[10] = p_y[1 - first];
          }
          else
          {
            my->count_d[9] = p_x[first];
            my->count_d[10] = p_y[first];
          }
          my->count_d[6] *= 0.8;
          my->count[2] = (int)(2.0 * 3.1415 * my->count_d[4] / my->count_d[6]);
          my->count[2] += 1;
          if (my->count[2] <= 0)
            my->count[2] = 1;
        }
      }
    }
  }

  if (my->count[1] == 2)
  {
    temp_x = my->x - my->count_d[7];
    temp_y = my->y - my->count_d[8];
    if (temp_x * temp_x + temp_y * temp_y <= my->count_d[6] * my->count_d[6])
    {
      my->count[1] = 0;
      /* sqrt(1.5) = 1.22474... */
      my->count_d[0] *= 1.2247;
      my->count_d[1] *= 1.2247;
      return 0;
    }
  }

  if (my->count[1] == 3)
  {
    temp_x = my->x - my->count_d[9];
    temp_y = my->y - my->count_d[10];
    (my->count[2])--;
    if ((temp_x * temp_x + temp_y * temp_y <= my->count_d[6] * my->count_d[6])
        || (my->count[2] < 0))
    {
      my->count[1] = 0;
      my->count_d[6] *= 1.25 * 1.5;
      temp_x = my->count_d[7] - my->x;
      temp_y = my->count_d[8] - my->y;
      temp = temp_x * temp_x + temp_y * temp_y;
      temp = tenm_sqrt((int) temp);
      if (temp < NEAR_ZERO)
        temp = 1.0;
      my->count_d[0] = my->count_d[6] * temp_x / temp;
      my->count_d[1] = my->count_d[6] * temp_y / temp;
      return 0;
    }
  }
  
  /* rotate */
  r_x = my->x - my->count_d[2];
  r_y = my->y - my->count_d[3];
  r_length = tenm_sqrt((int) (r_x * r_x + r_y * r_y));
  if (r_length < NEAR_ZERO)
    r_length = 1.0;
  r_x /= r_length;
  r_y /= r_length;

  dr_x = -r_y * my->count_d[5];
  dr_y = r_x * my->count_d[5];

  next_x = my->x + my->count_d[6] * dr_x - my->count_d[2];
  next_y = my->y + my->count_d[6] * dr_y - my->count_d[3];
  next_length = tenm_sqrt((int) (next_x * next_x + next_y * next_y));
  if (next_length < NEAR_ZERO)
    next_length = 1.0;
  next_x *= my->count_d[4] / next_length;
  next_y *= my->count_d[4] / next_length;

  my->count_d[0] = my->count_d[2] + next_x - my->x;
  my->count_d[1] = my->count_d[3] + next_y - my->y;

  return 0;
}

static int
cat_tail_grep_shot_i_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_i_draw: my is NULL\n");
    return 0;
  }

  /* return if it is not my turn */
  if (priority != 1)
    return 0;

  color = tenm_map_color(75, 0, 239);

  if (tenm_draw_circle((int) (my->x), (int) (my->y), 5, 2, color) != 0)
    status = 1;

  return status;
}

static tenm_object *
cat_tail_grep_shot_v_new(double x, double y, double speed_x, double speed_y,
                         int theta_turn, double target_x, double target_y)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double temp[2];
  double v[2];
  double t;

  /* sanity check */
  if (speed_x * speed_x + speed_y * speed_y < NEAR_ZERO)
  {
    fprintf(stderr, "cat_tail_grep_shot_v_new: speed is non-positive "
            "(%f, %f)\n", speed_x, speed_y);
    return NULL;
  }
  if ((theta_turn <= 0) || (theta_turn >= 180))
  {
    fprintf(stderr, "cat_tail_grep_shot_v_new: strange theta_turn (%d)\n",
            theta_turn);
    return NULL;
  }

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_v_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_circle_new(x, y, 5.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_v_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 4);
  if (count == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_v_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 6);
  if (count_d == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_v_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  if (-speed_y * (target_x - x)
      + speed_x * (target_y - y) > 0.0)
    count[3] = theta_turn;
  else
    count[3] = -theta_turn;

  temp[0] = 0.0;
  temp[1] = 0.0;
  v[0] = speed_x;
  v[1] = speed_y;
  vector_rotate(temp, v, count[3]);

  t = (x * speed_y - y * speed_x)
    - (target_x * speed_y - target_y * speed_x);
  t /= temp[0] * speed_y
    - temp[1] * speed_x;

  /* list of count
   * [0] color (for delete_enemy_shot)
   * [1] homing flag
   * [2] homing timer
   * [3] theta turn
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   * [2] turn point x
   * [3] turn point y
   * [4] next speed x
   * [5] next speed y
   */
  count[0] = 2;
  if (tenm_line_nearer(target_x, target_y,
                       x, y, x + speed_x, y + speed_y, 5.0))
    count[1] = 2;
  else
    count[1] = 0;
  count[2] = 0;
  /* count[3] is set above */
  count_d[0] = speed_x;
  count_d[1] = speed_y;
  count_d[2] = target_x + t * temp[0];
  count_d[3] = target_y + t * temp[1];
  count_d[4] = temp[0];
  count_d[5] = temp[1];

  new = tenm_object_new("cat tail shot v", ATTR_ENEMY_SHOT, ATTR_OPAQUE,
                        1, x, y,
                        4, count, 6, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&cat_tail_grep_shot_v_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&cat_tail_grep_shot_v_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&cat_tail_grep_shot_v_act),
                        (int (*)(tenm_object *, int))
                        (&cat_tail_grep_shot_v_draw));
  if (new == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_v_new: tenm_object_new failed\n");
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
cat_tail_grep_shot_v_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_v_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "cat_tail_grep_shot_v_move: strange turn_per_frame (%f)\n",
            turn_per_frame);
    return 0;
  }

  dx_temp = my->count_d[0] / turn_per_frame;
  dy_temp = my->count_d[1] / turn_per_frame;
  my->x += dx_temp;
  my->y += dy_temp;
  tenm_move_mass(my->mass, dx_temp, dy_temp);

  if (!in_window_object(my))
    return 1;

  return 0;
}

static int
cat_tail_grep_shot_v_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_v_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_v_hit: your is NULL\n");
    return 0;
  }

  if (your->attr & ATTR_OPAQUE)
    return 1;

  return 0;
}

static int
cat_tail_grep_shot_v_act(tenm_object *my, const tenm_object *player)
{
  double dx;
  double dy;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_v_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  switch (my->count[1])
  {
  case 0:
    dx = my->count_d[2] - my->x;
    dy = my->count_d[3] - my->y;
    if (dx * dx + dy * dy <= my->count_d[4] * my->count_d[4])
    {
      my->count[1] = 1;
      my->count_d[0] = dx;
      my->count_d[1] = dy;
    }
    break;
  case 1:
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
    (my->count[2])++;
    if (my->count[2] >= 15)
    {
      my->count[0] = 3;
      my->count[1] = 2;
      my->count_d[0] = my->count_d[4];
      my->count_d[1] = my->count_d[5];
    }
    break;
  case 2:
    break;
  default:
    fprintf(stderr, "cat_tail_grep_shot_v_act: undefined mode (%d)\n",
            my->count[1]);
    break;
  }
  
  return 0;
}

static int
cat_tail_grep_shot_v_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "cat_tail_grep_shot_v_draw: my is NULL\n");
    return 0;
  }

  /* return if it is not my turn */
  if (priority != 1)
    return 0;

  if (my->count[1] < 2)
    color = tenm_map_color(0, 167, 223);
  else
    color = tenm_map_color(0, 111, 223);

  if (tenm_draw_circle((int) (my->x), (int) (my->y), 5, 2, color) != 0)
    status = 1;

  return status;
}
