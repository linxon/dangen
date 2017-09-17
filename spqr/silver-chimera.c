/* $Id: silver-chimera.c,v 1.172 2005/06/24 14:42:42 oohara Exp $ */
/* [hard] Silver Chimera */

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

#include "silver-chimera.h"

static int silver_chimera_move(tenm_object *my, double turn_per_frame);
static int silver_chimera_hit(tenm_object *my, tenm_object *your);
static void silver_chimera_explode(tenm_object *my);
static int silver_chimera_act(tenm_object *my, const tenm_object *player);
static int silver_chimera_draw(tenm_object *my, int priority);
static int silver_chimera_green(const tenm_object *my);

static tenm_object *silver_chimera_spread_new(double x, double y,
                                              double speed_x, double speed_y,
                                              int t_spread);
static int silver_chimera_spread_move(tenm_object *my, double turn_per_frame);
static int silver_chimera_spread_hit(tenm_object *my, tenm_object *your);
static int silver_chimera_spread_act(tenm_object *my,
                                     const tenm_object *player);
static int silver_chimera_spread_draw(tenm_object *my, int priority);

static tenm_object *silver_chimera_lock_on_new(double x, double y);
static int silver_chimera_lock_on_act(tenm_object *my,
                                      const tenm_object *player);
static int silver_chimera_lock_on_draw(tenm_object *my, int priority);
static int silver_chimera_lock_on_draw_square(int x, int y, int length,
                                              tenm_color color);

static tenm_object *silver_chimera_bit_new(int table_index, int n);
static int silver_chimera_bit_move(tenm_object *my, double turn_per_frame);
static int silver_chimera_bit_hit(tenm_object *my, tenm_object *your);
static int silver_chimera_bit_signal(tenm_object *my, int n);
static int silver_chimera_bit_act(tenm_object *my, const tenm_object *player);
static int silver_chimera_bit_draw(tenm_object *my, int priority);
static int silver_chimera_bit_green(const tenm_object *my);

tenm_object *
silver_chimera_new(void)
{
  int i;
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = (double) (WINDOW_WIDTH / 2);
  double y = -53.0;

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 2);
  if (p == NULL)
  {
    fprintf(stderr, "silver_chimera_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + 96.0, y - 54.0,
                                             x - 48.0, y + 54.0,
                                             x - 65.28, y + 30.96,
                                             x + 78.72, y - 77.04);
  if (p[0] == NULL)
  {
    fprintf(stderr, "silver_chimera_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }
  p[1] = (tenm_primitive *) tenm_polygon_new(4,
                                             x - 96.0, y - 54.0,
                                             x + 48.0, y + 54.0,
                                             x + 65.28, y + 30.96,
                                             x - 78.72, y - 77.04);
  if (p[1] == NULL)
  {
    fprintf(stderr, "silver_chimera_new: cannot set p[1]\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 6);
  if (count == NULL)
  {
    fprintf(stderr, "silver_chimera_new: malloc(count) failed\n");
    for (i = 0; i < 2; i++)
      (p[i])->delete(p[i]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "silver_chimera_new: malloc(count_d) failed\n");
    free(count);
    for (i = 0; i < 2; i++)
      (p[i])->delete(p[i]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] for deal_damage
   * [1] "damaged" timer
   * [2] move mode
   * [3] move timer
   * [4] number of bits dead
   * [5] "was green when killed" flag
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
  count[5] = 0;

  count_d[0] = 0.0;
  count_d[1] = (((double) (WINDOW_HEIGHT / 4)) - y) / 120.0;

  new = tenm_object_new("Silver Chimera", ATTR_BOSS, ATTR_PLAYER_SHOT,
                        1000, x, y,
                        6, count, 2, count_d, 2, p,
                        (int (*)(tenm_object *, double))
                        (&silver_chimera_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&silver_chimera_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&silver_chimera_act),
                        (int (*)(tenm_object *, int))
                        (&silver_chimera_draw));
  if (new == NULL)
  {
    fprintf(stderr, "silver_chimera_new: tenm_object_new failed\n");
    if (count_d != NULL)
      free(count_d);
    if (count != NULL)
      free(count);
    for (i = 0; i < 2; i++)
      (p[i])->delete(p[i]);
    free(p);
    return NULL;
  }

  return new;
}

static int
silver_chimera_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "silver_chimera_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "silver_chimera_move: strange turn_per_frame (%f)\n",
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
silver_chimera_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "silver_chimera_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "silver_chimera_hit: your is NULL\n");
    return 0;
  }

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;
  if (my->count[2] != 1)
    return 0;

  deal_damage(my, your, 0);
  if (silver_chimera_green(my))
    add_chain(my, your);
  my->count[1] = 2;

  if (my->hit_point <= 0)
  {
    set_background(1);
    silver_chimera_explode(my);
    add_score(30000);
    return 0;
  }

  return 0;
}

static void
silver_chimera_explode(tenm_object *my)
{
  int n;

  if (my == NULL)
  {
    fprintf(stderr, "silver_chimera_explode: my is NULL\n");
    return;
  }

  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy_shot), 0);
  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy), 0);

  /* set "was green" flag before we change the life mode */
  if (silver_chimera_green(my))
  {
    my->count[5] = 1;
    n = 8;
  }
  else
  {
    my->count[5] = 0;
    n = 7;
  }

  my->count[2] = 2;
  my->count[3] = 0;
  my->count[1] = 0;

  my->count_d[0] = 0.0;
  my->count_d[1] = 0.5;

  tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                               1, 5000, n, 10.0, 6));

  /* don't modify my->attr or my->hit_mask here, or the player shot
   * may fly through the enemy */
  tenm_mass_delete(my->mass);
  my->mass = NULL;
}

static int
silver_chimera_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int theta;
  int t_spread;
  double x;
  double y;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "silver_chimera_act: my is NULL\n");
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

  /* encounter */
  if (my->count[2] == 0)
  {
    if (my->count[3] == 60)
    {
      for (i = 0; i < 2; i++)
        tenm_table_add(silver_chimera_bit_new(my->table_index, i));
      return 0;
    }

    if (my->count[3] >= 120)
    {
      my->count[2] = 1;
      my->count[3] = 0;

      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;

      return 0;
    }
    return 0;
  }

  /* dead */
  if (my->count[2] == 2)
  {
    if (silver_chimera_green(my))
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
                                   1, 5000, i, 10.0, 8));
      tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                                   2, 1000, i, 7.5, 12));

      tenm_table_add(fragment_new(my->x + 48.0, my->y - 36.0, 2.8, -2.1,
                                  30.0, 30, i, 5.0, 0.0, 16));
      tenm_table_add(fragment_new(my->x + 48.0, my->y + 36.0, 2.8, 2.1,
                                  30.0, 30, i, 5.0, 0.0, 16));
      tenm_table_add(fragment_new(my->x - 48.0, my->y - 36.0, -2.8, -2.1,
                                  30.0, 30, i, 5.0, 0.0, 16));
      tenm_table_add(fragment_new(my->x - 48.0, my->y + 36.0, -2.8, 2.1,
                                  30.0, 30, i, 5.0, 0.0, 16));

      tenm_table_add(stage_clear_new(100));
      return 1;
    }

    return 0;
  }

  /* self-destruction */
  if ((my->count[2] == 1) && (my->count[3] >= 5610))
  {
    set_background(2);
    clear_chain();
    silver_chimera_explode(my);
    return 0;
  }

  /* speed change */
  if (my->count[3] < 180)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
  }
  if ((my->count[3] == 180) || (my->count[3] == 2880))
  {
    my->count_d[0] = (((double) (WINDOW_WIDTH / 2)) - my->x) / 90.0;
    my->count_d[1] = (20.0 - my->y) / 90.0;
  }
  if (((my->count[3] > 180) && (my->count[3] < 270))
      || ((my->count[3] > 2880) && (my->count[3] < 2970)))
  {
    ;
  }
  if (((my->count[3] >= 270) && (my->count[3] < 990))
      || ((my->count[3] >= 2970) && (my->count[3] < 3690)))
  {
    x = (double) (WINDOW_WIDTH / 2);
    x += ((double) (WINDOW_WIDTH / 2))
      * tenm_cos(my->count[3] * 2) * tenm_cos(my->count[3]);
    my->count_d[0] = x - my->x;
    my->count_d[1] = 0.0;
  }
  if (((my->count[3] >= 990) && (my->count[3] < 1080))
      || ((my->count[3] >= 3690) && (my->count[3] < 3780)))
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
  }
  if ((my->count[3] == 1080) || (my->count[3] == 4680))
  {
    my->count_d[0] = (((double) (WINDOW_WIDTH / 2)) - my->x) / 90.0;
    my->count_d[1] = (((double) (WINDOW_HEIGHT / 2)) - my->y) / 90.0;
  }
  if (((my->count[3] > 1080) && (my->count[3] < 1170))
      || ((my->count[3] > 4680) && (my->count[3] < 4770)))
  {
    ;
  }
  if (((my->count[3] >= 1170) && (my->count[3] < 1890))
      || (my->count[3] >= 4770))
  {
    x = (double) (WINDOW_WIDTH / 2);
    x += ((double) (WINDOW_WIDTH / 2))
      * tenm_cos(my->count[3] * 2) * tenm_cos(my->count[3]);
    y = (double) (WINDOW_HEIGHT / 4);
    y += ((double) (WINDOW_HEIGHT / 4))
      * tenm_sin(my->count[3]);
    my->count_d[0] = x - my->x;
    my->count_d[1] = y - my->y;
  }
  if ((my->count[3] >= 1890) && (my->count[3] < 1980))
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
  }
  if ((my->count[3] == 1980) || (my->count[3] == 3780))
  {
    my->count_d[0] = (((double) (WINDOW_WIDTH / 2)) - my->x) / 90.0;
    my->count_d[1] = (((double) (WINDOW_HEIGHT / 4)) - my->y) / 90.0;
  }
  if (((my->count[3] > 1980) && (my->count[3] < 2070))
      || ((my->count[3] > 3780) && (my->count[3] < 3870)))
  {
    ;
  }
  if (((my->count[3] >= 2070) && (my->count[3] < 2880))
      || ((my->count[3] >= 3870) && (my->count[3] < 4680)))
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
  }

  if (my->count[2] != 1)
    return 0;

  /* shoot */
  if (((my->count[3] >= 360) && (my->count[3] < 990))
      || ((my->count[3] >= 2970) && (my->count[3] < 3690)))
  {
    if (my->count[3] % 7 == 0)
    {
      if ((my->count[3] + 90) % 360 < 180)
        theta = 80;
      else
        theta = 100;

      if (my->count[4] >= 2)
      {
        tenm_table_add(laser_angle_new(my->x, my->y + 18.0,
                                       12.0, theta,
                                       30.0, 1));
        tenm_table_add(laser_angle_new(my->x + 9.0, my->y + 18.0,
                                       9.0, theta,
                                       30.0, 1));
        tenm_table_add(laser_angle_new(my->x - 9.0, my->y + 18.0,
                                       9.0, theta,
                                       30.0, 1));
        tenm_table_add(laser_angle_new(my->x + 27.0, my->y + 18.0,
                                       7.0, theta,
                                       30.0, 1));
        tenm_table_add(laser_angle_new(my->x - 27.0, my->y + 18.0,
                                       7.0, theta,
                                       30.0, 1));
        tenm_table_add(laser_angle_new(my->x + 45.0, my->y + 18.0,
                                       5.0, theta,
                                       30.0, 1));
        tenm_table_add(laser_angle_new(my->x - 45.0, my->y + 18.0,
                                       5.0, theta,
                                       30.0, 1));
      }
      else
      {
        tenm_table_add(laser_angle_new(my->x, my->y + 18.0,
                                       8.0, theta,
                                       30.0, 1));
        if (my->count[4] >= 1)
        {
          tenm_table_add(laser_angle_new(my->x + 15.0, my->y + 18.0,
                                         6.5, theta,
                                         30.0, 1));
          tenm_table_add(laser_angle_new(my->x - 15.0, my->y + 18.0,
                                         6.5, theta,
                                         30.0, 1));
        }
        tenm_table_add(laser_angle_new(my->x + 45.0, my->y + 18.0,
                                       5.0, theta,
                                       30.0, 1));
        tenm_table_add(laser_angle_new(my->x - 45.0, my->y + 18.0,
                                       5.0, theta,
                                       30.0, 1));
      }
    }
  }

  if (((my->count[3] >= 1170) && (my->count[3] < 1890))
      || ((my->count[3] >= 4770) && (my->count[3] < 5490)))
  {
    if (my->count[3] % 46 == 0)
    {
      if (my->count[4] >= 2)
        t_spread = 20;
      else if (my->count[4] == 1)
        t_spread = 55;
      else
        t_spread = 70;
      tenm_table_add(silver_chimera_spread_new(my->x + 36.0, my->y + 30.96,
                                               -6.0, 2.5,
                                               t_spread + rand() % 31));
      tenm_table_add(silver_chimera_spread_new(my->x - 36.0, my->y + 30.96,
                                               6.0, 2.5,
                                               t_spread + rand() % 31));
    }
    if (my->count[3] % 46 == 23)
    {
      if (my->count[4] >= 2)
        t_spread = 20;
      else if (my->count[4] == 1)
        t_spread = 50;
      else
        t_spread = 80;
      tenm_table_add(silver_chimera_spread_new(my->x, my->y,
                                               0.0, 6.5,
                                               t_spread + rand() % 31));
    }
  }

  if (((my->count[3] >= 2070) && (my->count[3] < 2790))
      || ((my->count[3] >= 3840) && (my->count[3] < 4590)))
  {
    if ((my->count[3] % 37 == 0)
        && ((my->count[4] >= 2)
            || ((my->count[4] >= 1) && (my->count[3] % 148 < 74))
            || (my->count[3] % 148 == 0)))
    {
      tenm_table_add(silver_chimera_lock_on_new(player->x, player->y));
    }
  }

  return 0;
}

static int
silver_chimera_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  char temp[32];

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "silver_chimera_draw: my is NULL\n");
    return 0;
  }

  /* body */
  if (priority == 0)
  {
    if (silver_chimera_green(my))
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

    if (tenm_draw_line((int) (my->x), (int) (my->y - 18.0),
                       (int) (my->x + 78.72), (int) (my->y - 77.04),
                       3, color))
      status = 1;
    if (tenm_draw_line((int) (my->x + 78.72), (int) (my->y - 77.04),
                       (int) (my->x + 96.0), (int) (my->y - 54.0),
                       3, color))
      status = 1;
    if (tenm_draw_line((int) (my->x + 96.0), (int) (my->y - 54.0),
                       (int) (my->x + 24.0), (int) (my->y),
                       3, color))
      status = 1;
    if (tenm_draw_line((int) (my->x + 24.0), (int) (my->y),
                       (int) (my->x + 65.28), (int) (my->y + 30.96),
                       3, color))
      status = 1;
    if (tenm_draw_line((int) (my->x + 65.28), (int) (my->y + 30.96),
                       (int) (my->x + 48.0), (int) (my->y + 54.0),
                       3, color))
      status = 1;
    if (tenm_draw_line((int) (my->x + 48.0), (int) (my->y + 54.0),
                       (int) (my->x), (int) (my->y + 18.0),
                       3, color))
      status = 1;

    if (tenm_draw_line((int) (my->x), (int) (my->y - 18.0),
                       (int) (my->x - 78.72), (int) (my->y - 77.04),
                       3, color))
      status = 1;
    if (tenm_draw_line((int) (my->x - 78.72), (int) (my->y - 77.04),
                       (int) (my->x - 96.0), (int) (my->y - 54.0),
                       3, color))
      status = 1;
    if (tenm_draw_line((int) (my->x - 96.0), (int) (my->y - 54.0),
                       (int) (my->x - 24.0), (int) (my->y),
                       3, color))
      status = 1;
    if (tenm_draw_line((int) (my->x - 24.0), (int) (my->y),
                       (int) (my->x - 65.28), (int) (my->y + 30.96),
                       3, color))
      status = 1;
    if (tenm_draw_line((int) (my->x - 65.28), (int) (my->y + 30.96),
                       (int) (my->x - 48.0), (int) (my->y + 54.0),
                       3, color))
      status = 1;
    if (tenm_draw_line((int) (my->x - 48.0), (int) (my->y + 54.0),
                       (int) (my->x), (int) (my->y + 18.0),
                       3, color))
      status = 1;
  }
  
  /* hit point stat */
  if ((priority == 0) && (my->count[2] == 1))
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 10, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "silver_chimera_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

/* return 1 (true) or 0 (false) */
static int
silver_chimera_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if ((my->count[2] == 1)
      && (my->count[3] >= 270) && (my->count[3] < 5580))
    return 1;

  if ((my->count[2] == 2) && (my->count[5] != 0))
    return 1;

  return 0;
}

static tenm_object *
silver_chimera_spread_new(double x, double y, double speed_x, double speed_y,
                          int t_spread)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;

  /* sanity check */
  if (t_spread <= 0)
  {
    fprintf(stderr, "silver_chimera_spread_new: t_spread is non-positive "
            "(%d)\n", t_spread);
    return NULL;
  }

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "silver_chimera_spread_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_circle_new(x, y, 10.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "silver_chimera_spread_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 3);
  if (count == NULL)
  {
    fprintf(stderr, "silver_chimera_spread_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "silver_chimera_spread_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] color (for delete_enemy_shot)
   * [1] spread mode
   * [2] spread timer
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */
  count[0] = 2;
  count[1] = 0;
  count[2] = -t_spread;

  count_d[0] = speed_x;
  count_d[1] = speed_y;

  new = tenm_object_new("Silver Chimera spread", ATTR_ENEMY_SHOT, ATTR_OPAQUE,
                        1, x, y,
                        3, count, 2, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&silver_chimera_spread_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&silver_chimera_spread_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&silver_chimera_spread_act),
                        (int (*)(tenm_object *, int))
                        (&silver_chimera_spread_draw));
  if (new == NULL)
  {
    fprintf(stderr, "silver_chimera_spread_new: tenm_object_new failed\n");
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
silver_chimera_spread_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "silver_chimera_spread_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "silver_chimera_spread_move: "
            "strange turn_per_frame (%f)\n",
            turn_per_frame);
    return 0;
  }

  dx_temp = my->count_d[0] / turn_per_frame;
  dy_temp = my->count_d[1] / turn_per_frame;
  my->x += dx_temp;
  my->y += dy_temp;
  if (my->mass != NULL)
    tenm_move_mass(my->mass, dx_temp, dy_temp);

  /* shape change */
  if ((my->count[1] == 1) && (my->mass != NULL))
    ((tenm_circle *)(my->mass->p[0]))->r += 2.0 / turn_per_frame;

  if (!in_window_object(my))
    return 1;

  return 0;
}

static int
silver_chimera_spread_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "silver_chimera_spread_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "silver_chimera_spread_hit: your is NULL\n");
    return 0;
  }

  if (your->attr & ATTR_OPAQUE)
    return 1;

  return 0;
}

static int
silver_chimera_spread_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int theta;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "silver_chimera_spread_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[2])++;
  switch (my->count[1])
  {
  case 0:
    if (my->count[2] >= 0)
    {  
      my->count[1] = 1;
      my->count[2] = 0;
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    break;
  case 1:
    if (my->count[2] == 10)
    {
      tenm_table_add(laser_point_new(my->x, my->y, 5.0,
                                     player->x, player->y, 25.0, 2));
    }
    if (my->count[2] >= 20)
    {  
      my->count[0] = 3;
      my->count[1] = 2;
      my->count[2] = 0;
    }
    break;
  case 2:
    if (my->count[2] % 5 == 0)
    {
      theta = rand() % 360;
      for (i = 0; i < 360; i += 120)
        tenm_table_add(laser_angle_new(my->x, my->y, 7.0,
                                       theta + i, 25.0, 3));
    }
    if (my->count[2] >= 20)
    {  
      return 1;
    }
    break;
  default:
    fprintf(stderr, "silver_chimera_spread_act: strange my->count[1] (%d)\n",
            my->count[1]);
    break;
  }

  return 0;
}

static int
silver_chimera_spread_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  int r = 1;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "silver_chimera_spread_draw: my is NULL\n");
    return 0;
  }

  if ((priority == 0) && (my->count[1] == 0))
  {
    color = tenm_map_color(99, 143, 158);
    color = tenm_map_color(0, 167, 223);
    color = tenm_map_color(167, 196, 206);
    if (tenm_draw_line((int) (my->x), (int) (my->y),
                       (int) (my->x + my->count_d[0] * (double)(my->count[2])),
                       (int) (my->y + my->count_d[1] * (double)(my->count[2])),
                       1, color))
      status = 1;
  }
  if (priority == 1)
  {
    switch (my->count[1])
    {
    case 0:
      r = 10;
      color = tenm_map_color(0, 167, 223);
      break;
    case 1:
      r = 10 + my->count[2] * 2;
      color = tenm_map_color(0, 167, 223);
      break;
    case 2:
      r = 50;
      color = tenm_map_color(0, 111, 223);
      break;
    default:
      fprintf(stderr, "silver_chimera_spread_draw: strange my->count[1] "
              "(%d)\n",
              my->count[1]);
      r = 1;
      color = tenm_map_color(0, 0, 0);
      break;
    }
    if (tenm_draw_circle((int) (my->x), (int) (my->y), r, 3, color) != 0)
      status = 1;
  }
  
  return status;
}

static tenm_object *
silver_chimera_lock_on_new(double x, double y)
{
  tenm_object *new = NULL;
  int *count = NULL;

  count = (int *) malloc(sizeof(int) * 4);
  if (count == NULL)
  {
    fprintf(stderr, "silver_chimera_spread_new: malloc(count) failed\n");
    return NULL;
  }

  /* list of count
   * [0] color (for delete_enemy_shot)
   * [1] shoot mode
   * [2] shoot timer
   * [3] shoot direction
   */
  count[0] = 6;
  count[1] = 0;
  count[2] = 1;
  count[3] = rand() % 2;

  /* ATTR_ENEMY_SHOT is only to clear it when the player is killed */
  new = tenm_object_new("Silver Chimera lock on", ATTR_ENEMY_SHOT, 0,
                        1, x, y,
                        4, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double)) NULL,
                        (int (*)(tenm_object *, tenm_object *)) NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&silver_chimera_lock_on_act),
                        (int (*)(tenm_object *, int))
                        (&silver_chimera_lock_on_draw));
  if (new == NULL)
  {
    fprintf(stderr, "silver_chimera_lock_on_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
silver_chimera_lock_on_act(tenm_object *my, const tenm_object *player)
{
  double length;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "silver_chimera_lock_on_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[2])++;
  switch (my->count[1])
  {
  case 0:
    if (my->count[2] >= 20)
    {  
      my->count[1] = 1;
      my->count[2] = 0;
    }
    break;
  case 1:
    length = 5.0 * (double) (140 - my->count[2]);
    if (my->count[2] % 10 == 0)
    { 
      if (my->count[2] % 20 == 10 * my->count[3])
      {
        tenm_table_add(laser_angle_new(my->x + length, my->y + length, 9.0,
                                       180, 25.0, 0));
        tenm_table_add(laser_angle_new(my->x + length, my->y + length, 9.0,
                                       -90, 25.0, 0));
        tenm_table_add(laser_angle_new(my->x - length, my->y - length, 9.0,
                                       0, 25.0, 0));
        tenm_table_add(laser_angle_new(my->x - length, my->y - length, 9.0,
                                       90, 25.0, 0));
      }
      else
      {
        tenm_table_add(laser_angle_new(my->x + length, my->y - length, 9.0,
                                       180, 25.0, 0));
        tenm_table_add(laser_angle_new(my->x + length, my->y - length, 9.0,
                                       90, 25.0, 0));
        tenm_table_add(laser_angle_new(my->x - length, my->y + length, 9.0,
                                       0, 25.0, 0));
        tenm_table_add(laser_angle_new(my->x - length, my->y + length, 9.0,
                                       -90, 25.0, 0));
      }
    }
    if (my->count[2] >= 140)
    {  
      return 1;
    }
    break;
  default:
    fprintf(stderr, "silver_chimera_lock_on_act: strange my->count[1] (%d)\n",
            my->count[1]);
    break;
  }

  return 0;
}

static int
silver_chimera_lock_on_draw(tenm_object *my, int priority)
{
  int status = 0;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "silver_chimera_lock_on_draw: my is NULL\n");
    return 0;
  }

  if (priority != 0)
    return 0;

  switch (my->count[1])
  {
  case 0:
    if (silver_chimera_lock_on_draw_square((int) my->x, (int) my->y,
                                           200 + 25 * my->count[2],
                                           tenm_map_color(158, 158, 158)) != 0)
      status = 1;
    if (silver_chimera_lock_on_draw_square((int) my->x, (int) my->y,
                                           200 - 10 * my->count[2],
                                           tenm_map_color(158, 158, 158)) != 0)
      status = 1;
    break;
  case 1:
    if (silver_chimera_lock_on_draw_square((int) my->x, (int) my->y,
                                           5 * (140 - my->count[2]),
                                           tenm_map_color(99, 158, 114)) != 0)
      status = 1;

    if (tenm_draw_line(((int) (my->x)) - 25, ((int) (my->y)),
                       ((int) (my->x)) + 25, ((int) (my->y)),
                       1, tenm_map_color(158, 158, 158)) != 0)
      status = 1;
    if (tenm_draw_line(((int) (my->x)), ((int) (my->y)) - 25,
                       ((int) (my->x)), ((int) (my->y)) + 25,
                       1, tenm_map_color(158, 158, 158)) != 0)
      status = 1;

    break;
  default:
    fprintf(stderr, "silver_chimera_lock_on_draw: strange my->count[1] "
            "(%d)\n",
            my->count[1]);
    break;
  }
  
  return status;
}

static int
silver_chimera_lock_on_draw_square(int x, int y, int length,
                                   tenm_color color)
{
  int status = 0;

  /* sanity check */
  if (length <= 0)
  {
    fprintf(stderr, "silver_chimera_lock_on_draw_square: length is "
            "non-positive (%d)\n", length);
    return 1;
  }
  
  if (tenm_draw_line(x + length, y + length,
                     x - length, y + length,
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line(x - length, y + length,
                     x - length, y - length,
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line(x - length, y - length,
                     x + length, y - length,
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line(x + length, y - length,
                     x + length, y + length,
                     1, color) != 0)
    status = 1;

  return status;
}

static tenm_object *
silver_chimera_bit_new(int table_index, int n)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x;
  double y;

  /* sanity check */
  if ((n < 0) || (n > 1))
  {
    fprintf(stderr, "silver_chimera_bit_new: strange n (%d)\n", n);
    return NULL;
  }

  if (n == 0)
    x = (double) (WINDOW_WIDTH / 8);
  else
    x = (double) (WINDOW_WIDTH * 7 / 8);
  y = -19.0;

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "silver_chimera_bit_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + 20.0, y + 20.0,
                                             x - 20.0, y + 20.0,
                                             x - 20.0, y - 20.0,
                                             x + 20.0, y - 20.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "silver_chimera_bit_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 6);
  if (count == NULL)
  {
    fprintf(stderr, "silver_chimera_bit_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "silver_chimera_bit_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] for deal_damage
   * [1] "damaged" timer
   * [2] move mode
   * [3] move timer
   * [4] n
   * [5] core index
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */

  count[0] = 0;
  count[1] = 0;
  count[2] = 0;
  count[3] = 60;
  count[4] = n;
  count[5] = table_index;

  count_d[0] = 0.0;
  count_d[1] = (((double) (WINDOW_HEIGHT / 4)) - y) / 60.0;

  new = tenm_object_new("Silver Chimera bit", ATTR_ENEMY, ATTR_PLAYER_SHOT,
                        750, x, y,
                        6, count, 2, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&silver_chimera_bit_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&silver_chimera_bit_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&silver_chimera_bit_act),
                        (int (*)(tenm_object *, int))
                        (&silver_chimera_bit_draw));
  if (new == NULL)
  {
    fprintf(stderr, "silver_chimera_bit_new: tenm_object_new failed\n");
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
silver_chimera_bit_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "silver_chimera_bit_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "silver_chimera_bit_move: strange turn_per_frame (%f)\n",
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
silver_chimera_bit_hit(tenm_object *my, tenm_object *your)
{
  int n;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "silver_chimera_bit_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "silver_chimera_bit_hit: your is NULL\n");
    return 0;
  }

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;
  if (my->count[2] != 1)
    return 0;

  deal_damage(my, your, 0);
  if (silver_chimera_bit_green(my))
    add_chain(my, your);
  my->count[1] = 41;

  if (my->hit_point <= 0)
  {
    add_score(20000);
    tenm_table_apply(my->count[5],
                     (int (*)(tenm_object *, int))
                     (&silver_chimera_bit_signal),
                     0);

    if (silver_chimera_bit_green(my))
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
    return 1;
  }

  return 0;
}

static int
silver_chimera_bit_signal(tenm_object *my, int n)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (strcmp(my->name, "Silver Chimera") != 0)
    return 0;

  (my->count[4])++;

  return 0;
}

static int
silver_chimera_bit_act(tenm_object *my, const tenm_object *player)
{
  double x;
  double y;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "silver_chimera_bit_act: my is NULL\n");
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

  /* encounter */
  if (my->count[2] == 0)
  {
    if (my->count[3] >= 120)
    {
      my->count[2] = 1;
      my->count[3] = 0;

      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;

      return 0;
    }    
    return 0;
  }

  /* speed change */
  if ((my->count[3] == 1080) || (my->count[3] == 4680))
  {
    x = (double) (WINDOW_WIDTH / 2);
    if (my->count[4] == 0)
      x -= 96.0;
    else
      x += 96.0;
    my->count_d[0] = (x - my->x) / 90.0;
    my->count_d[1] = (((double) (WINDOW_HEIGHT / 2)) - my->y) / 90.0;
  }
  if (((my->count[3] > 1080) && (my->count[3] < 1170))
      || ((my->count[3] > 4680) && (my->count[3] < 4770)))
  {
    ;
  }
  if (((my->count[3] >= 1170) && (my->count[3] < 1890))
      || (my->count[3] >= 4770))
  {
    x = (double) (WINDOW_WIDTH / 2);
    x += ((double) (WINDOW_WIDTH / 2))
      * tenm_cos(my->count[3] * 2) * tenm_cos(my->count[3]);
    if (my->count[4] == 0)
      x -= 96.0;
    else
      x += 96.0;
    y = (double) (WINDOW_HEIGHT / 4);
    y += ((double) (WINDOW_HEIGHT / 4))
      * tenm_sin(my->count[3]);
    my->count_d[0] = x - my->x;
    my->count_d[1] = y - my->y;
  }
  if (my->count[3] == 1890)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
  }
  if (my->count[3] == 1980)
  {
    if (my->count[4] == 0)
      x = (double) (WINDOW_WIDTH / 8);
    else
      x = (double) (WINDOW_WIDTH * 7 / 8);
    my->count_d[0] = (x - my->x) / 90.0;
    my->count_d[1] = (((double) (WINDOW_HEIGHT / 4)) - my->y) / 90.0;
  }
  if (my->count[3] == 2070)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
  }

  if (my->count[2] != 1)
    return 0;

  /* shoot */
  if ((my->count[3] >= 35) && (my->count[3] < 5490)
      && (my->count[3] % 14 == my->count[4] * 7))
    tenm_table_add(normal_shot_point_new(my->x, my->y, 7.0,
                                         player->x
                                         +((double)(rand()%101))-50.0,
                                         player->y
                                         +((double)(rand()%101))-50.0,
                                         4));

  return 0;
}

static int
silver_chimera_bit_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  char temp[32];

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "silver_chimera_bit_draw: my is NULL\n");
    return 0;
  }

  if (priority != 0)
    return 0;

  /* decoration */
  if (silver_chimera_bit_green(my))
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
  if (tenm_draw_line((int) (my->x + 10.0), (int) (my->y - 20.0),
                     (int) (my->x + 20.0), (int) (my->y - 30.0),
                     1, color))
    status = 1;
  if (tenm_draw_line((int) (my->x + 20.0), (int) (my->y - 30.0),
                     (int) (my->x - 20.0), (int) (my->y - 30.0),
                     1, color))
    status = 1;
  if (tenm_draw_line((int) (my->x - 20.0), (int) (my->y - 30.0),
                     (int) (my->x - 10.0), (int) (my->y - 20.0),
                     1, color))
    status = 1;

  /* body */
  /*
  if (tenm_draw_mass(my->mass, tenm_map_color(0, 0, 0)) != 0)
    status = 1;
  */
  if (silver_chimera_bit_green(my))
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

  if (tenm_draw_line((int) (my->x + 20.0), (int) (my->y + 20.0),
                     (int) (my->x - 20.0), (int) (my->y + 20.0),
                     2, color))
    status = 1;
  if (tenm_draw_line((int) (my->x - 20.0), (int) (my->y + 20.0),
                     (int) (my->x - 20.0), (int) (my->y - 20.0),
                     2, color))
    status = 1;
  if (tenm_draw_line((int) (my->x - 20.0), (int) (my->y - 20.0),
                       (int) (my->x + 20.0), (int) (my->y - 20.0),
                     2, color))
    status = 1;
  if (tenm_draw_line((int) (my->x + 20.0), (int) (my->y - 20.0),
                     (int) (my->x + 20.0), (int) (my->y + 20.0),
                     2, color))
    status = 1;

  /* hit point stat */
  if ((my->count[2] == 1) && (my->count[1] >= 1))
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 5, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "silver_chimera_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

/* return 1 (true) or 0 (false) */
static int
silver_chimera_bit_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if ((my->count[2] == 1)
      && (my->count[3] >= 35) && (my->count[3] < 5580))
    return 1;

  return 0;
}
