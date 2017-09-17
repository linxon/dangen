/* $Id: hatsuda.c,v 1.231 2005/07/12 20:34:49 oohara Exp $ */
/* [hard] 0x82da3104 */
/* read: hatsuda satoshi */

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

#include "hatsuda.h"

#define NEAR_ZERO 0.0001

static int hatsuda_move(tenm_object *my, double turn_per_frame);
static int hatsuda_hit(tenm_object *my, tenm_object *your);
static void hatsuda_next(tenm_object *my);
static int hatsuda_signal_shroud(tenm_object *my, int n);
static int hatsuda_act(tenm_object *my, const tenm_object *player);
static void hatsuda_act_reflect(tenm_object *my, const tenm_object *player,
                                int direction);
static int hatsuda_draw(tenm_object *my, int priority);
static int hatsuda_green(const tenm_object *my);

static tenm_object *hatsuda_wall_triangle_new(double y, double size,
                                              double speed, int n, int t);
static int hatsuda_wall_triangle_move(tenm_object *my, double turn_per_frame);
static int hatsuda_wall_triangle_act(tenm_object *my,
                                     const tenm_object *player);
static double hatsuda_wall_triangle_edge(int t);
static int hatsuda_wall_triangle_draw(tenm_object *my, int priority);

static tenm_object *hatsuda_wall_reflect_new(double x, double y,
                                             double speed_x, double speed_y,
                                             int number_reflect);
static int hatsuda_wall_reflect_move(tenm_object *my, double turn_per_frame);
static int hatsuda_wall_reflect_act(tenm_object *my,
                                     const tenm_object *player);
static int hatsuda_wall_reflect_draw(tenm_object *my, int priority);

static tenm_object *hatsuda_shroud_new(int n);
static int hatsuda_shroud_move(tenm_object *my, double turn_per_frame);
static int hatsuda_shroud_act(tenm_object *my, const tenm_object *player);
static int hatsuda_shroud_draw(tenm_object *my, int priority);

tenm_object *
hatsuda_new(void)
{
  int i;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = (double) (WINDOW_WIDTH / 2);
  double y = ((double) (WINDOW_HEIGHT)) - 286.0 * tenm_cos(45);

  count = (int *) malloc(sizeof(int) * 8);
  if (count == NULL)
  {
    fprintf(stderr, "hatsuda_new: malloc(count) failed\n");
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "hatsuda_new: malloc(count_d) failed\n");
    free(count);
    return NULL;
  }

  /* list of count
   * [0] for deal_damage
   * [1] "damaged" timer
   * [2] move mode
   * [3] move timer
   * [4] "was green when killed" flag
   * [5 -- 7] shroud index
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
  for (i = 0; i < 3; i++)
    count[5 + i] = -1;

  count_d[0] = 0.0;
  count_d[1] = 0.0;

  new = tenm_object_new("0x82da3104", ATTR_BOSS, ATTR_PLAYER_SHOT,
                        400, x, y,
                        8, count, 2, count_d, 0, NULL,
                        (int (*)(tenm_object *, double))
                        (&hatsuda_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&hatsuda_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&hatsuda_act),
                        (int (*)(tenm_object *, int))
                        (&hatsuda_draw));
  if (new == NULL)
  {
    fprintf(stderr, "hatsuda_new: tenm_object_new failed\n");
    if (count_d != NULL)
      free(count_d);
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
hatsuda_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "hatsuda_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "hatsuda_move: strange turn_per_frame (%f)\n",
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
hatsuda_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "hatsuda_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "hatsuda_hit: your is NULL\n");
    return 0;
  }

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;
  if ((my->count[2] != 1) && (my->count[2] != 3) && (my->count[2] != 5))
    return 0;
  if ((my->count[2] == 5) && (my->count[3] < 290))
    return 0;

  deal_damage(my, your, 0);
  if (hatsuda_green(my))
    add_chain(my, your);
  my->count[1] = 2;

  if (my->hit_point <= 0)
  {
    if (my->count[2] == 1)
      add_score(10000);
    else if (my->count[2] == 3)
      add_score(15000);
    else if (my->count[2] == 5)
      add_score(35000);
    set_background(1);
    hatsuda_next(my);
    return 0;
  }

  return 0;
}

static void
hatsuda_next(tenm_object *my)
{
  int n;
  int i;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "hatsuda_next: my is NULL\n");
    return;
  }

  if ((my->count[2] != 1) && (my->count[2] != 3) && (my->count[2] != 5))
    return;

  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy_shot), 0);
  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy), 0);

  /* don't modify my->attr or my->hit_mask here, or the player shot
   * may fly through the enemy */
  tenm_mass_delete(my->mass);
  my->mass = NULL;

  /* set "was green" flag before we change the life mode */
  if (hatsuda_green(my))
  {
    n = 8;
    my->count[4] = 1;
  }
  else
  {
    n = 7;
    my->count[4] = 0;
  }

  if (my->count[2] == 1)
  {
    tenm_table_add(explosion_new(my->x, my->y,
                                 0.0, 0.0,
                                 1, 3000, n, 10.0, 8));
    tenm_table_add(explosion_new(my->x, my->y,
                                 0.0, 0.0,
                                 2, 800, n, 6.0, 8));
    my->hit_point = 750;
    my->count[2] = 2;
    my->count[3] = 0;
    my->count[1] = 0;
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
  }
  if (my->count[2] == 3)
  {
    tenm_table_add(explosion_new(my->x, my->y,
                                 0.0, 0.0,
                                 1, 3000, n, 10.0, 8));
    tenm_table_add(fragment_new(my->x, my->y, 0.0, 0.0,
                                30.0, 100, n, 4.0, 0.0, 16));
    tenm_table_add(fragment_new(my->x, my->y, 0.0, 0.0,
                                50.0, 30, n, 2.5, 0.0, 12));
    my->hit_point = 400;
    my->count[2] = 4;
    my->count[3] = 0;
    my->count[1] = 0;
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
  }
  if (my->count[2] == 5)
  {
    tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                                 1, 5000, n, 10.0, 6));
    my->count[2] = 6;
    my->count[3] = 0;
    my->count[1] = 0;
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.5;

    for (i = 0; i < 3; i++)
      if (my->count[5 + i] >= 0)
        tenm_table_apply(my->count[5 + i],
                         (int (*)(tenm_object *, int))
                         (&hatsuda_signal_shroud),
                         0);
  }
}

static int
hatsuda_signal_shroud(tenm_object *my, int n)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (strcmp(my->name, "0x82da3104 shroud") != 0)
    return 0;

  tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                               1, 1000, 9, 4.0, 6));
  tenm_table_add(fragment_new(my->x, my->y, 0.0, 0.0,
                              30.0, 100, 9, 4.0, 0.0, 16));
  tenm_table_add(fragment_new(my->x, my->y, 0.0, 0.0,
                              50.0, 30, 9, 2.5, 0.0, 12));

  return 1;
}

static int
hatsuda_act(tenm_object *my, const tenm_object *player)
{
  int theta;
  int i;
  tenm_primitive **p = NULL;
  int n;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "hatsuda_act: my is NULL\n");
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
  /* speed change */
  if (my->count[2] == 1)
  {
    my->count_d[0] = hatsuda_wall_triangle_edge(my->count[3]) - my->x;
    my->count_d[1] = 0.0;
  }

  /* encounter */
  if (my->count[2] == 0)
  {
    if (my->count[3] == 60)
    {
      my->count_d[0] = (((double) (WINDOW_WIDTH / 2)) - my->x) / 60.0;
      my->count_d[1] = (40.0 - my->y) / 60.0;
      return 0;
    }
    if (my->count[3] >= 120)
    {
      my->count[2] = 1;
      my->count[3] = 0;
      my->count[4] = 0;
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;

      p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
      if (p == NULL)
      {
        fprintf(stderr, "hatsuda_act: malloc(p) failed (form 1)\n");
        return 0;
      }

      p[0] = (tenm_primitive *) tenm_circle_new(my->x, my->y, 25.0);
      if (p[0] == NULL)
      {
        fprintf(stderr, "hatsuda_act: cannot set p[0] (form 1)\n");
        free(p);
        return 0;
      }

      my->mass = tenm_mass_new(1, p);
      if (my->mass == NULL)
      {
        fprintf(stderr, "hatsuda_act: tenm_mass_new failed (form 1)\n");
        (p[0])->delete(p[0]);
        free(p);
        return 0;
      }

      return 0;
    }

    return 0;
  }
  else if (my->count[2] == 2)
  {
    if (my->count[3] == 30)
    {
      my->count_d[0] = (((double) (WINDOW_WIDTH / 2)) - my->x) / 60.0;
      my->count_d[1] = (((double) (WINDOW_HEIGHT / 2)) - my->y) / 60.0;
      return 0;
    }
    if (my->count[3] == 90)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
      return 0;
    }
    if (my->count[3] >= 120)
    {
      my->count[2] = 3;
      my->count[3] = 0;
      my->count[4] = 0;
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;

      p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
      if (p == NULL)
      {
        fprintf(stderr, "hatsuda_act: malloc(p) failed (form 2)\n");
        return 0;
      }

      p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                                 my->x + 50.0 * tenm_cos(45),
                                                 my->y + 50.0 * tenm_cos(45),
                                                 my->x - 50.0 * tenm_cos(15),
                                                 my->y + 50.0 * tenm_cos(45),
                                                 my->x - 50.0 * tenm_cos(15),
                                                 my->y - 50.0 * tenm_cos(15),
                                                 my->x + 50.0 * tenm_cos(45),
                                                 my->y - 50.0 * tenm_cos(15));
      if (p[0] == NULL)
      {
        fprintf(stderr, "hatsuda_act: cannot set p[0] (form 2)\n");
        free(p);
        return 0;
      }

      my->mass = tenm_mass_new(1, p);
      if (my->mass == NULL)
      {
        fprintf(stderr, "hatsuda_act: tenm_mass_new failed (form 2)\n");
        (p[0])->delete(p[0]);
        free(p);
        return 0;
      }

      return 0;
    }

    return 0;
  }
  else if (my->count[2] == 4)
  {
    if (my->count[3] >= 90)
    {
      my->count[2] = 5;
      my->count[3] = 0;
      my->count[4] = 0;
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;

      p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
      if (p == NULL)
      {
        fprintf(stderr, "hatsuda_act: malloc(p) failed (form 3)\n");
        return 0;
      }

      p[0] = (tenm_primitive *) tenm_polygon_new(3,
                                                 my->x + 50.0 * tenm_cos(45),
                                                 my->y + 50.0 * tenm_sin(45),
                                                 my->x + 50.0 * tenm_cos(165),
                                                 my->y + 50.0 * tenm_sin(165),
                                                 my->x + 50.0 * tenm_cos(285),
                                                 my->y + 50.0 * tenm_sin(285));
      if (p[0] == NULL)
      {
        fprintf(stderr, "hatsuda_act: cannot set p[0] (form 3)\n");
        free(p);
        return 0;
      }

      my->mass = tenm_mass_new(1, p);
      if (my->mass == NULL)
      {
        fprintf(stderr, "hatsuda_act: tenm_mass_new failed (form 3)\n");
        (p[0])->delete(p[0]);
        free(p);
        return 0;
      }

      return 0;
    }

    return 0;
  }

  /* dead */
  if (my->count[2] == 6)
  {
    if (hatsuda_green(my))
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

  /* self-destruction */
  if (((my->count[2] == 1) && (my->count[3] >= 2052))
      || ((my->count[2] == 3) && (my->count[3] >= 1850))
      || ((my->count[2] == 5) && (my->count[3] >= 2230)))
  {
    set_background(2);
    clear_chain();
    hatsuda_next(my);
    return 0;
  }
  
  /* shoot */
  if ((my->count[2] == 1) && (my->count[3] <= 1872))
  {
    if (my->count[3] % 17 == 0)
    {
      tenm_table_add(laser_point_new(my->x, my->y, 5.0,
                                     player->x, player->y, 25.0, 4));
    }

    if (my->count[3] % 26 == 13)
      n = 1;
    else
      n = 0;
    if (my->count[3] % 312 >= 156)
      n = 1 - n;
    switch (my->count[3] % 156)
    {
    case 13:
      tenm_table_add(hatsuda_wall_triangle_new(40.0, 40.0, 5.5, n,
                                               my->count[3]));
      break;
    case 26:
      tenm_table_add(hatsuda_wall_triangle_new(200.0, 40.0, -5.5, n,
                                               my->count[3]));
      break;
    case 39:
      if (my->count[3] > 312)
        tenm_table_add(hatsuda_wall_triangle_new(360.0, 40.0, 5.5, n,
                                                 my->count[3]));
      break;
    case 52:
      tenm_table_add(hatsuda_wall_triangle_new(120.0, 40.0, -5.5, n,
                                               my->count[3]));
      break;
    case 65:
      tenm_table_add(hatsuda_wall_triangle_new(280.0, 40.0, 5.5, n,
                                               my->count[3]));
      break;
    case 78:
      if (my->count[3] > 312)
        tenm_table_add(hatsuda_wall_triangle_new(440.0, 40.0, -5.5, n,
                                                 my->count[3]));
      break;
    case 91:
      tenm_table_add(hatsuda_wall_triangle_new(40.0, 40.0, -5.5, n,
                                               my->count[3]));
      break;
    case 104:
      tenm_table_add(hatsuda_wall_triangle_new(200.0, 40.0, 5.5, n,
                                               my->count[3]));
      break;
    case 117:
      if (my->count[3] > 312)
        tenm_table_add(hatsuda_wall_triangle_new(360.0, 40.0, -5.5, n,
                                                 my->count[3]));
      break;
    case 130:
      tenm_table_add(hatsuda_wall_triangle_new(120.0, 40.0, 5.5, n,
                                               my->count[3]));
      break;
    case 143:
      if (my->count[3] > 312)
        tenm_table_add(hatsuda_wall_triangle_new(280.0, 40.0, -5.5, 0,
                                                 my->count[3]));
      break;
    case 0:
      if (my->count[3] > 312)
        tenm_table_add(hatsuda_wall_triangle_new(440.0, 40.0, 5.5, n,
                                                 my->count[3]));
      break;
    default:
      break;
    }
  }

  if ((my->count[2] == 3) && (my->count[3] <= 1720))
  {
    if (my->count[3] % 86 == 0)
    {
      hatsuda_act_reflect(my, player, 3);
      hatsuda_act_reflect(my, player, 7);
    } 
    if (my->count[3] % 86 == 43)
    {
      hatsuda_act_reflect(my, player, 9);
      hatsuda_act_reflect(my, player, 1);
    }
    if ((my->count[3] > 172) && (my->count[3] % 7 == 0))
    {
      theta = -90 + (my->count[3] -172) * (-6);
      tenm_table_add(laser_angle_new(my->x, my->y, 4.0,
                                     theta, 25.0, 3));
      tenm_table_add(laser_angle_new(my->x, my->y, 2.5,
                                     theta - 5, 25.0, 3));
      tenm_table_add(laser_angle_new(my->x, my->y, 2.5,
                                     theta + 5, 25.0, 3));
      tenm_table_add(laser_angle_new(my->x, my->y, 6.0,
                                     theta - 11, 25.0, 3));
      tenm_table_add(laser_angle_new(my->x, my->y, 6.0,
                                     theta + 11, 25.0, 3));
    }
  }

  if (my->count[2] == 5)
  {
    if (my->count[3] == 30)
      for (i = 0; i < 3; i++)
        my->count[5 + i] = tenm_table_add(hatsuda_shroud_new(i));

    if ((my->count[3] >= 530) && (my->count[3] <= 2100)
        && (my->count[3] % 5 == 0))
      tenm_table_add(normal_shot_point_new(my->x, my->y, 7.0,
                                           player->x
                                           + (double) ((rand() % 151) - 75),
                                           player->y
                                           + (double) ((rand() % 151) - 75),
                                           4));
  }
  
  return 0;
}

/* reflection shot
 * direction
 * 789
 * 4*6
 * 123
 */
static void
hatsuda_act_reflect(tenm_object *my, const tenm_object *player,
                    int direction)
{
  double x_aim;
  double y_aim;
  int number_reflect;
  double dx;
  double dy;
  double length;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "hatsuda_act_reflect: my is NULL\n");
    return;
  }
  if (player == NULL)
    return;
  if ((direction < 1) || (direction > 9) || (direction == 5))
  {
    fprintf(stderr, "hatsuda_act_reflect: strange direction(%d)\n",
            direction);
    return;
  }

  x_aim = player->x;
  y_aim = player->y;
  number_reflect = 0;
  if ((direction == 7) || (direction == 4) || (direction == 1))
  {  
    x_aim = -x_aim;
    number_reflect++;
  }
  if ((direction == 9) || (direction == 6) || (direction == 3))
  {
    x_aim = 2.0 * ((double) WINDOW_WIDTH) - x_aim;
    number_reflect++;
  }
  if ((direction == 1) || (direction == 2) || (direction == 3))
  {  
    y_aim = -y_aim;
    number_reflect++;
  }
  if ((direction == 7) || (direction == 8) || (direction == 9))
  {
    y_aim = 2.0 * ((double) WINDOW_HEIGHT) - y_aim;
    number_reflect++;
  }

  dx = x_aim - my->x;
  dy = y_aim - my->y;
  length = tenm_sqrt((int) (dx * dx + dy * dy));
  if (length < NEAR_ZERO)
    length = 1.0;
  tenm_table_add(hatsuda_wall_reflect_new(my->x, my->y, 
                                          6.0 * dx / length,
                                          6.0 * dy / length,
                                          number_reflect));
}

static int
hatsuda_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  char temp[32];
  double x;
  int theta;
  int i;
  double c;
  double length;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "hatsuda_draw: my is NULL\n");
    return 0;
  }

  /* decoration */
  if (priority == 0)
  {  
    if (hatsuda_green(my))
    {
      if (my->count[1] >= 1)
        color = tenm_map_color(181, 190, 92);
      else
        color = tenm_map_color(157, 182, 123);
    }
    else
    {
      if (my->count[1] >= 1)
        color = tenm_map_color(200, 164, 92);
      else
        color = tenm_map_color(182, 147, 123);
    }

    if (my->count[2] == 0)
    {
      if (my->count[3] <= 60)
      {
        c = ((double) (my->count[3])) / 60.0;
        for (i = 0; i < 360; i += 120)
        {
          if (tenm_draw_line((int) (my->x + 286.0 * tenm_cos(45 + i)),
                             (int) (my->y + 286.0 * tenm_sin(45 + i)),
                             (int) (my->x
                                    + 286.0 * tenm_cos(45 + i) * (1.0 - c)
                                    + 286.0 * tenm_cos(165 + i) * c),
                             (int) (my->y
                                    + 286.0 * tenm_sin(45 + i) * (1.0 - c)
                                    + 286.0 * tenm_sin(165 + i) * c),
                             1, color))
            status = 1;
        }
      }
      else if (my->count[3] <= 120)
      {
        c = ((double) (my->count[3] - 60)) / 60.0;
        length = 286.0 * (1.0 - c) + 50.0 * c;
        for (i = 0; i < 360; i += 120)
        {
          theta = 45 + i + (my->count[3] - 60) * 6;
          if (tenm_draw_line((int) (my->x + length * tenm_cos(theta)),
                             (int) (my->y + length * tenm_sin(theta)),
                             (int) (my->x + length * tenm_cos(theta + 120)),
                             (int) (my->y + length * tenm_sin(theta + 120)),
                             1, color))
            status = 1;
        }
      }
    }
    else if ((my->count[2] == 1) || (my->count[2] == 3) || (my->count[2] == 4))
    {
      for (i = 0; i < 360; i += 120)
      {
        if (tenm_draw_line((int) (my->x + 50.0 * tenm_cos(45 + i)),
                           (int) (my->y + 50.0 * tenm_sin(45 + i)),
                           (int) (my->x + 50.0 * tenm_cos(165 + i)),
                           (int) (my->y + 50.0 * tenm_sin(165 + i)),
                           1, color))
          status = 1;
      }
    }
    else if (my->count[2] == 2)
    {
      for (i = 0; i < 360; i += 120)
      {
        if ((my->count[3] < 30) || (my->count[3] >= 90))
          theta = 45 + i;
        else
          theta = 45 + i + (my->count[3] - 30) * 6;
        if (tenm_draw_line((int) (my->x + 50.0 * tenm_cos(theta)),
                           (int) (my->y + 50.0 * tenm_sin(theta)),
                           (int) (my->x + 50.0 * tenm_cos(theta + 120)),
                           (int) (my->y + 50.0 * tenm_sin(theta + 120)),
                           1, color))
          status = 1;
      }
    }

    if (my->count[2] == 1)
    {
      color = tenm_map_color(158, 158, 158);
      x = hatsuda_wall_triangle_edge(my->count[3]);
      if (tenm_draw_line((int) x, 0, (int) x, WINDOW_HEIGHT, 1, color))
        status = 1;
    } 

    if ((my->count[2] == 2)
        &&(my->count[3] > 90) && (my->count[3] <= 120))
    {
      color = tenm_map_color(95, 47, 13);
      length = 50.0 + ((double) (120 - my->count[3])) * 3.0;
      if (tenm_draw_line((int) (my->x + length * tenm_cos(45)),
                         (int) (my->y + length * tenm_cos(45)),
                         (int) (my->x - length * tenm_cos(15)),
                         (int) (my->y + length * tenm_cos(45)),
                         1, color))
        status = 1;
      if (tenm_draw_line((int) (my->x - length * tenm_cos(15)),
                         (int) (my->y + length * tenm_cos(45)),
                         (int) (my->x - length * tenm_cos(15)),
                         (int) (my->y - length * tenm_cos(15)),
                         1, color))
        status = 1;
      if (tenm_draw_line((int) (my->x - length * tenm_cos(15)),
                         (int) (my->y - length * tenm_cos(15)),
                         (int) (my->x + length * tenm_cos(45)),
                         (int) (my->y - length * tenm_cos(15)),
                         1, color))
        status = 1;
      if (tenm_draw_line((int) (my->x + length * tenm_cos(45)),
                         (int) (my->y - length * tenm_cos(15)),
                         (int) (my->x + length * tenm_cos(45)),
                         (int) (my->y + length * tenm_cos(45)),
                         1, color))
        status = 1;
    }
    if ((my->count[2] == 4)
        &&(my->count[3] > 30) && (my->count[3] <= 90))
    {
      color = tenm_map_color(95, 47, 13);
      length = 50.0 + ((double) (90 - my->count[3])) * 3.0;
      for (i = 0; i < 360; i += 120)
      {
        theta = 45 + i + (my->count[3] - 30) * 6;
        if (tenm_draw_line((int) (my->x + length * tenm_cos(theta)),
                           (int) (my->y + length * tenm_sin(theta)),
                           (int) (my->x + length * tenm_cos(theta + 120)),
                           (int) (my->y + length * tenm_sin(theta + 120)),
                           1, color))
          status = 1;
      }
    }
  }

  /* body */
  /* dead enemy has low priority */
  if (((my->count[2] <= 1) && (priority == 0))
      || ((my->count[2] > 1) && (priority == -1)))
  {  
    if (hatsuda_green(my))
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

    if (my->count[2] == 1)
    {
      if (tenm_draw_circle((int) (my->x), (int) (my->y), 25, 3, color) != 0)
        status = 1;
    }
    else if (my->count[2] == 3)
    {
      length = 50.0;
      if (tenm_draw_line((int) (my->x + length * tenm_cos(45)),
                         (int) (my->y + length * tenm_cos(45)),
                         (int) (my->x - length * tenm_cos(15)),
                         (int) (my->y + length * tenm_cos(45)),
                         3, color))
        status = 1;
      if (tenm_draw_line((int) (my->x - length * tenm_cos(15)),
                         (int) (my->y + length * tenm_cos(45)),
                         (int) (my->x - length * tenm_cos(15)),
                         (int) (my->y - length * tenm_cos(15)),
                         3, color))
        status = 1;
      if (tenm_draw_line((int) (my->x - length * tenm_cos(15)),
                         (int) (my->y - length * tenm_cos(15)),
                         (int) (my->x + length * tenm_cos(45)),
                         (int) (my->y - length * tenm_cos(15)),
                         3, color))
        status = 1;
      if (tenm_draw_line((int) (my->x + length * tenm_cos(45)),
                         (int) (my->y - length * tenm_cos(15)),
                         (int) (my->x + length * tenm_cos(45)),
                         (int) (my->y + length * tenm_cos(45)),
                         3, color))
        status = 1;
    }
    else if ((my->count[2] == 5) || (my->count[2] == 6))
    {
      length = 50.0;
      for (i = 0; i < 360; i += 120)
      {
        theta = 45 + i;
        if (tenm_draw_line((int) (my->x + length * tenm_cos(theta)),
                           (int) (my->y + length * tenm_sin(theta)),
                           (int) (my->x + length * tenm_cos(theta + 120)),
                           (int) (my->y + length * tenm_sin(theta + 120)),
                           3, color))
          status = 1;
      }
    }
  }
  
  /* hit point stat */
  if ((priority == 0)
      &&((my->count[2] == 1)
         || (my->count[2] == 3)
         || ((my->count[2] == 5) && (my->count[3] >= 290))))
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 5, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "hatsuda_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

/* return 1 (true) or 0 (false) */
static int
hatsuda_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if ((my->count[2] == 1)
      && (my->count[3] > 312) && (my->count[3] < 2022))
    return 1;
  if ((my->count[2] == 2) && (my->count[4] != 0)
      && (my->count[3] < 30))
    return 1;
  if ((my->count[2] == 3)
      && (my->count[3] > 172) && (my->count[3] < 1820))
    return 1;
  if ((my->count[2] == 4) && (my->count[4] != 0)
      && (my->count[3] < 30))
    return 1;
  if ((my->count[2] == 5)
      && (my->count[3] >= 530) && (my->count[3] < 2200))
    return 1;
  if ((my->count[2] == 6) && (my->count[4] != 0))
    return 1;

  return 0;
}

static tenm_object *
hatsuda_wall_triangle_new(double y, double size, double speed, int n, int t)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x;

  /* sanity check */
  if (size < NEAR_ZERO)
  {
    fprintf(stderr, "hatsuda_wall_triangle_new: size is non-positive "
            "(%f)\n", size);
    return NULL;
  }
  if ((n < 0) || (n > 1))
  {
    fprintf(stderr, "hatsuda_wall_triangle_new: strange n (%d)\n", n);
    return NULL;
  }

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "hatsuda_wall_triangle_new: malloc(p) failed\n");
    return NULL;
  }

  if (speed > 0.0)
    x = 1.0 - size;
  else
    x = ((double) (WINDOW_WIDTH)) - 1.0 + size;

  if (n == 0)
  {
    p[0] = (tenm_primitive *) tenm_polygon_new(3,
                                               x - size, y - size,
                                               x - size, y + size,
                                               x + size, y + size);
  }
  else
  {
    p[0] = (tenm_primitive *) tenm_polygon_new(3,
                                               x + size, y - size,
                                               x + size, y + size,
                                               x - size, y + size);
  }
  if (p[0] == NULL)
  {
    fprintf(stderr, "hatsuda_wall_triangle_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 6);
  if (count == NULL)
  {
    fprintf(stderr, "hatsuda_wall_triangle_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 3);
  if (count_d == NULL)
  {
    fprintf(stderr, "hatsuda_wall_triangle_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] n
   * [1] move mode
   * [2] move timer
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   * [2] size
   */

  count[0] = n;
  count[1] = 0;
  count[2] = t;

  count_d[0] = speed;
  count_d[1] = 0.0;
  count_d[2] = size;

  new = tenm_object_new("0x82da3104 wall triangle",
                        ATTR_ENEMY | ATTR_OPAQUE, 0,
                        1, x, y,
                        6, count, 3, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&hatsuda_wall_triangle_move),
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&hatsuda_wall_triangle_act),
                        (int (*)(tenm_object *, int))
                        (&hatsuda_wall_triangle_draw));
  if (new == NULL)
  {
    fprintf(stderr, "hatsuda_wall_triangle_new: tenm_object_new failed\n");
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
hatsuda_wall_triangle_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "hatsuda_wall_triangle_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "hatsuda_wall_triangle_move: "
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

  if (!in_window_object(my))
    return 1;

  return 0;
}

static int
hatsuda_wall_triangle_act(tenm_object *my, const tenm_object *player)
{
  double edge_x;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "hatsuda_wall_triangle_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[2])++;
  if (my->count[1] == 0)
  {
    edge_x = hatsuda_wall_triangle_edge(my->count[2]);
    if ((my->x + my->count_d[2] >= edge_x)
        && (my->x - my->count_d[2] <= edge_x))
    {
      my->count[1] = 1;
      my->count[2] = 0;
      if (my->count_d[0] > 0.0)
        my->count_d[1] = my->count_d[0];
      else
        my->count_d[1] = -my->count_d[0];
      if (my->y - my->count_d[2] > player->y)
        my->count_d[1] *= -1.0;
      my->count_d[0] = 0.0;
    }
  }

  return 0;
}

static double
hatsuda_wall_triangle_edge(int t)
{
  if (t < 180)
    return ((double) (WINDOW_WIDTH / 2));

  return ((double) (WINDOW_WIDTH / 2))
    + ((double) (WINDOW_WIDTH / 3)) * tenm_sin(t * 2);
}

static int
hatsuda_wall_triangle_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  double size;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "hatsuda_wall_triangle_draw: my is NULL\n");
    return 0;
  }

  if (priority != 0)
    return 0;

  /* body */
  color = tenm_map_color(95, 13, 68);
  size = my->count_d[2];

  if (my->count[0] == 0)
  {
    if (tenm_draw_line((int) (my->x - size), (int) (my->y - size),
                       (int) (my->x - size), (int) (my->y + size),
                       3, color))
      status = 1;
    if (tenm_draw_line((int) (my->x - size), (int) (my->y + size),
                       (int) (my->x + size), (int) (my->y + size),
                       3, color))
      status = 1;
    if (tenm_draw_line((int) (my->x + size), (int) (my->y + size),
                       (int) (my->x - size), (int) (my->y - size),
                       3, color))
      status = 1;
  }
  else
  {
    if (tenm_draw_line((int) (my->x + size), (int) (my->y - size),
                       (int) (my->x + size), (int) (my->y + size),
                       3, color))
      status = 1;
    if (tenm_draw_line((int) (my->x + size), (int) (my->y + size),
                       (int) (my->x - size), (int) (my->y + size),
                       3, color))
      status = 1;
    if (tenm_draw_line((int) (my->x - size), (int) (my->y + size),
                       (int) (my->x + size), (int) (my->y - size),
                       3, color))
      status = 1;
  }

  return status;
}

static tenm_object *
hatsuda_wall_reflect_new(double x, double y, double speed_x, double speed_y,
                         int number_reflect)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;

  /* sanity check */
  if (number_reflect < 0)
  {
    fprintf(stderr, "hatsuda_wall_reflect_new: number_reflect is negative "
            "(%d)\n", number_reflect);
    return NULL;
  }

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "hatsuda_wall_reflect_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + 20.0, y + 20.0,
                                             x - 20.0, y + 20.0,
                                             x - 20.0, y - 20.0,
                                             x + 20.0, y - 20.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "hatsuda_wall_reflect_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 1);
  if (count == NULL)
  {
    fprintf(stderr, "hatsuda_wall_reflect_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "hatsuda_wall_reflect_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] number_reflect
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */

  count[0] = number_reflect;

  count_d[0] = speed_x;
  count_d[1] = speed_y;

  new = tenm_object_new("0x82da3104 wall reflect",
                        ATTR_ENEMY | ATTR_OPAQUE, 0,
                        1, x, y,
                        1, count, 2, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&hatsuda_wall_reflect_move),
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&hatsuda_wall_reflect_act),
                        (int (*)(tenm_object *, int))
                        (&hatsuda_wall_reflect_draw));
  if (new == NULL)
  {
    fprintf(stderr, "hatsuda_wall_reflect_new: tenm_object_new failed\n");
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
hatsuda_wall_reflect_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "hatsuda_wall_reflect_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "hatsuda_wall_reflect_move: "
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

  if (!in_window_object(my))
    return 1;

  return 0;
}

static int
hatsuda_wall_reflect_act(tenm_object *my, const tenm_object *player)
{
  int reflected = 0;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "hatsuda_wall_reflect_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  /* don't reflect if the player is immutable */
  if ((get_ship() < 0) || (player->count[1] > 0))
    my->count[0] = 0;

  if (my->count[0] > 0)
  {
    if ((my->x < 0.0) || (my->x > ((double) WINDOW_WIDTH)))
    {
      my->count_d[0] *= -1.0;
      reflected = 1;
    }
    if ((my->y < 0.0) || (my->y > ((double) WINDOW_HEIGHT)))
    {
      my->count_d[1] *= -1.0;
      reflected = 1;
    }
    if (reflected != 0)
      (my->count[0])--;
  }

  return 0;
}

static int
hatsuda_wall_reflect_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "hatsuda_wall_reflect_draw: my is NULL\n");
    return 0;
  }

  if (priority != 0)
    return 0;

  /* body */
  color = tenm_map_color(95, 13, 68);

  if (tenm_draw_line((int) (my->x + 20.0), (int) (my->y + 20.0),
                     (int) (my->x - 20.0), (int) (my->y + 20.0),
                     3, color))
    status = 1;
  if (tenm_draw_line((int) (my->x - 20.0), (int) (my->y + 20.0),
                     (int) (my->x - 20.0), (int) (my->y - 20.0),
                     3, color))
    status = 1;
  if (tenm_draw_line((int) (my->x - 20.0), (int) (my->y - 20.0),
                     (int) (my->x + 20.0), (int) (my->y - 20.0),
                     3, color))
    status = 1;
  if (tenm_draw_line((int) (my->x + 20.0), (int) (my->y - 20.0),
                     (int) (my->x + 20.0), (int) (my->y + 20.0),
                     3, color))
    status = 1;

  return status;
}

static tenm_object *
hatsuda_shroud_new(int n)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x;
  double y;
  double dx_a;
  double dy_a;
  double dx_b;
  double dy_b;

  /* sanity check */
  if ((n < 0) || (n > 2))
  {
    fprintf(stderr, "hatsuda_shroud_new: strange n (%d)\n", n);
    return NULL;
  }

  x = (double) (WINDOW_WIDTH / 2);
  y = (double) (WINDOW_HEIGHT / 2);
  x += 200.0 * tenm_cos(n * 120);
  y += 200.0 * tenm_sin(n * 120);
  dx_a = 150.0 * tenm_cos(n * 120);
  dy_a = 150.0 * tenm_sin(n * 120);
  dx_b = 5.0 * tenm_cos(n * 120 + 90);
  dy_b = 5.0 * tenm_sin(n * 120 + 90);

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "hatsuda_shroud_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + dx_a + dx_b,
                                             y + dy_a + dy_b,
                                             x - dx_a + dx_b,
                                             y - dy_a + dy_b,
                                             x - dx_a - dx_b,
                                             y - dy_a - dy_b,
                                             x + dx_a - dx_b,
                                             y + dy_a - dy_b);
  if (p[0] == NULL)
  {
    fprintf(stderr, "hatsuda_shroud_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 10);
  if (count == NULL)
  {
    fprintf(stderr, "hatsuda_shroud_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "hatsuda_shroud_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] n
   * [1] move mode
   * [2] move timer
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */

  count[0] = n;
  count[1] = 0;
  count[2] = 0;

  count_d[0] = 0.0;
  count_d[1] = 0.0;

  new = tenm_object_new("0x82da3104 shroud",
                        0, 0,
                        1, x, y,
                        10, count, 2, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&hatsuda_shroud_move),
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&hatsuda_shroud_act),
                        (int (*)(tenm_object *, int))
                        (&hatsuda_shroud_draw));
  if (new == NULL)
  {
    fprintf(stderr, "hatsuda_shroud_new: tenm_object_new failed\n");
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
hatsuda_shroud_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "hatsuda_shroud_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "hatsuda_shroud_move: strange turn_per_frame (%f)\n",
            turn_per_frame);
    return 0;
  }

  dx_temp = my->count_d[0] / turn_per_frame;
  dy_temp = my->count_d[1] / turn_per_frame;
  my->x += dx_temp;
  my->y += dy_temp;
  if (my->mass != NULL)
    tenm_move_mass(my->mass, dx_temp, dy_temp);

  if (!in_window_object(my))
    return 1;

  return 0;
}

static int
hatsuda_shroud_act(tenm_object *my, const tenm_object *player)
{
  double x;
  double y;
  int i;
  double dx_a;
  double dy_a;
  int theta;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "hatsuda_shroud_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  /* speed change */
  (my->count[2])++;
  if (my->count[1] == 1)
  {
    if (my->count[2] < 200)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    else
    {
      if (my->count[2] < 1100)
        theta = my->count[0] * 120 + my->count[2] - 200;
      else if (my->count[2] < 1200)
        theta = my->count[0] * 120 + 1100 - 200;
      else
        theta = my->count[0] * 120 + 1100 - 200 - (my->count[2] - 1200);
      x = (double) (WINDOW_WIDTH / 2);
      y = (double) (WINDOW_HEIGHT / 2);
      x += 200.0 * tenm_cos(theta);
      y += 200.0 * tenm_sin(theta);
      my->count_d[0] = x - my->x;
      my->count_d[1] = y - my->y;
    }
  }

  /* encounter */
  if (my->count[1] == 0)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
    if (my->count[2] > 60)
    {
      my->attr = ATTR_BOSS | ATTR_OPAQUE;
      my->hit_mask = 0;
      my->count[1] = 1;
      my->count[2] = 0;
      return 0;
    }
    return 0;
  }

  if (my->count[1] != 1)
    return 0;

  /* shoot */
  if ((my->count[2] <= 2010)
      && (((my->count[2] >= 132) && (my->count[2] % 11 == 0))
          || (my->count[2] % 33 == 0)))
  {
    dx_a = 150.0 * tenm_cos(my->count[0] * 120);
    dy_a = 150.0 * tenm_sin(my->count[0] * 120);
    for (i = -60; i <= 60; i += 60)
    {
      tenm_table_add(laser_angle_new(my->x + dx_a,
                                     my->y + dy_a,
                                     6.0, my->count[0] * 120 + i,
                                     25.0, 1));
      tenm_table_add(laser_angle_new(my->x - dx_a,
                                     my->y - dy_a,
                                     6.0, my->count[0] * 120 + i + 180,
                                     25.0, 1));
    }
  }

  return 0;
}

static int
hatsuda_shroud_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  int width;
  double dx_a;
  double dy_a;
  double dx_b;
  double dy_b;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "hatsuda_shroud_draw: my is NULL\n");
    return 0;
  }

  if (priority != 0)
    return 0;

  /* body */
  if (my->count[1] == 0)
    width = 1;
  else
    width = 3;
  if ((my->count[1] == 0) && (my->count[2] < 30))
    color = tenm_map_color(182, 123, 162);
  else
    color = tenm_map_color(95, 13, 68);
  dx_a = 150.0 * tenm_cos(my->count[0] * 120);
  dy_a = 150.0 * tenm_sin(my->count[0] * 120);
  dx_b = 5.0 * tenm_cos(my->count[0] * 120 + 90);
  dy_b = 5.0 * tenm_sin(my->count[0] * 120 + 90);

  if (tenm_draw_line((int) (my->x + dx_a + dx_b),
                     (int) (my->y + dy_a + dy_b),
                     (int) (my->x - dx_a + dx_b),
                     (int) (my->y - dy_a + dy_b),
                     width, color))
    status = 1;
  if (tenm_draw_line((int) (my->x - dx_a + dx_b),
                     (int) (my->y - dy_a + dy_b),
                     (int) (my->x - dx_a - dx_b),
                     (int) (my->y - dy_a - dy_b),
                     width, color))
    status = 1;
  if (tenm_draw_line((int) (my->x - dx_a - dx_b),
                     (int) (my->y - dy_a - dy_b),
                     (int) (my->x + dx_a - dx_b),
                     (int) (my->y + dy_a - dy_b),
                     width, color))
    status = 1;
  if (tenm_draw_line((int) (my->x + dx_a - dx_b),
                     (int) (my->y + dy_a - dy_b),
                     (int) (my->x + dx_a + dx_b),
                     (int) (my->y + dy_a + dy_b),
                     width, color))
    status = 1;

  return status;
}
