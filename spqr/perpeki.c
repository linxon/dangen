/* $Id: perpeki.c,v 1.225 2004/10/02 16:31:57 oohara Exp $ */
/* [very hard] Perpeki */

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
#include "stage-clear.h"
#include "score.h"

#include "perpeki.h"

#define NEAR_ZERO 0.0001

static int perpeki_move(tenm_object *my, double turn_per_frame);
static int perpeki_hit(tenm_object *my, tenm_object *your);
static void perpeki_explode(tenm_object *my);
static int perpeki_act(tenm_object *my, const tenm_object *player);
static void perpeki_normal_shot(tenm_object *my, const tenm_object *player,
                                int what);
static int perpeki_green(const tenm_object *my);
static int perpeki_draw(tenm_object *my, int priority);

static tenm_object *perpeki_chip_new(int what);
static int perpeki_chip_act(tenm_object *my, const tenm_object *player);
static int perpeki_chip_draw(tenm_object *my, int priority);

static tenm_object *perpeki_backdancer_new(int table_index, int what);
static int perpeki_backdancer_move(tenm_object *my, double turn_per_frame);
static int perpeki_backdancer_hit(tenm_object *my, tenm_object *your);
static int perpeki_backdancer_signal(tenm_object *my, int n);
static int perpeki_backdancer_act(tenm_object *my, const tenm_object *player);
static int perpeki_backdancer_green(const tenm_object *my);
static int perpeki_backdancer_draw(tenm_object *my, int priority);

tenm_object *
perpeki_new(void)
{
  int i;
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = 0.0;
  double y = 0.0;

  x = (double) (WINDOW_WIDTH / 2);
  y = -29.0;

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 3);
  if (p == NULL)
  {
    fprintf(stderr, "perpeki_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + 30.0,
                                             y + 30.0,
                                             x - 30.0,
                                             y + 30.0,
                                             x - 30.0,
                                             y - 30.0,
                                             x + 30.0,
                                             y - 30.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "perpeki_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }
  p[1] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + 10.0,
                                             y - 30.0,
                                             x + 20.0,
                                             y - 30.0,
                                             x + 30.0,
                                             y - 80.0,
                                             x + 20.0,
                                             y - 80.0);
  if (p[1] == NULL)
  {
    fprintf(stderr, "perpeki_new: cannot set p[1]\n");
      (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  p[2] = (tenm_primitive *) tenm_polygon_new(4,
                                             x - 10.0,
                                             y - 30.0,
                                             x - 20.0,
                                             y - 30.0,
                                             x - 30.0,
                                             y - 70.0,
                                             x - 20.0,
                                             y - 70.0);
  if (p[2] == NULL)
  {
    fprintf(stderr, "perpeki_new: cannot set p[2]\n");
    for (i = 0; i < 2; i++)
      (p[i])->delete(p[i]);
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 18);
  if (count == NULL)
  {
    fprintf(stderr, "perpeki_new: malloc(count) failed\n");
    for (i = 0; i < 2; i++)
      (p[i])->delete(p[i]);
    free(p);
    return NULL;
  }

  count_d = (double *) malloc(sizeof(double) * 6);
  if (count_d == NULL)
  {
    fprintf(stderr, "perpeki_new: malloc(count_d) failed\n");
    if (count != NULL)
      free(count);
    for (i = 0; i < 2; i++)
      (p[i])->delete(p[i]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] for deal_damage
   * [1] "damaged" timer
   * [2] normal move direction
   * [3] evasion type
   * [4] evasion direction
   * [5] evasion timer
   * [6] shoot timer
   * [7 -- 8] shoot theta
   * [9] number of backdancers dead
   * [10] life mode
   * [11] "was green when killed" flag
   */
  count[0] = 0;
  count[1] = 0;
  if (rand() % 2 == 0)
    count[2] = 1;
  else
    count[2] = -1;
  count[3] = 0;
  count[4] = 0;
  count[5] = 0;
  count[6] = 0;
  count[7] = 0;
  count[8] = 0;
  count[9] = 0;
  count[10] = 0;
  count[11] = 0;

  /* list of count_d
   * [0] speed x
   * [1] speed y
   * [2] evasion move center x
   * [3] evasion move center y
   */
  count_d[0] = (((double) (WINDOW_WIDTH / 2)) - x) / 120.0;
  count_d[1] = (((double) (WINDOW_HEIGHT / 3)) - y) / 120.0;
  count_d[2] = 0.0;
  count_d[3] = 0.0;

  new = tenm_object_new("Perpeki", ATTR_BOSS, ATTR_PLAYER_SHOT,
                        2000, x, y,
                        18, count, 6, count_d, 3, p,
                        (int (*)(tenm_object *, double))
                        (&perpeki_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&perpeki_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&perpeki_act),
                        (int (*)(tenm_object *, int))
                        (&perpeki_draw));
  if (new == NULL)
  {
    fprintf(stderr, "perpeki_new: tenm_object_new failed\n");
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
perpeki_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
    return 0;
  if (turn_per_frame <= 0.5)
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
perpeki_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (your == NULL)
    return 0;

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;
  if (my->count[10] != 1)
    return 0;

  deal_damage(my, your, 0);
  if (perpeki_green(my))
    add_chain(my, your);
  my->count[1] = 2;

  if (my->hit_point <= 0)
  {
    set_background(1);
    add_score(100000);
    perpeki_explode(my);
    return 0;
  }

  return 0;
}

static
void perpeki_explode(tenm_object *my)
{
  int n;

  /* sanity check */
  if (my == NULL)
    return;

  /* set "was green" flag before we change the life mode */
  if (perpeki_green(my))
  {
    my->count[11] = 1;
    n = 8;
  }
  else
  {
    my->count[11] = 0;
    n = 7;
  }

  my->count[0] = 0;
  my->count[1] = 0;
  my->count[6] = -1;
  my->count[10] = 2;

  my->count_d[0] = 0.0;
  my->count_d[1] = 0.0;

  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy_shot), 0);
  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy), 0);

  tenm_table_add(fragment_new(my->x - 20.0, my->y - 50.0, 0.0, 0.0,
                              30.0, 20, n, 4.0, 0.0, 20));
  tenm_table_add(fragment_new(my->x + 20.0, my->y - 50.0, 0.0, 0.0,
                              30.0, 20, n, 4.0, 0.0, 20));
  tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                               1, 5000, n, 10.0, 6));

  /* don't modify my->attr or my->hit_mask here, or the player shot
   * may fly through the enemy */
  tenm_mass_delete(my->mass);
  my->mass = NULL;
}

static int
perpeki_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int theta;
  double speed;

  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  /* for deal_damage */
  my->count[0] = 0;

  /* "damaged" count down */
  if (my->count[1] > 0)
    (my->count[1])--;

  /* encounter */
  if (my->count[10] == 0)
  {
    (my->count[6])++;
    if (my->count[6] >= 120)
    {
      my->count[6] = -192;
      my->count[10] = 1;
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
      return 0;
    }
    return 0;
  }

  /* dead */
  if (my->count[10] == 2)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.5;

    if (my->count[11] != 0)
      i = 8;
    else
      i = 7;

    (my->count[6])++;
    if ((my->count[6] >= 30) && (my->count[6] <= 75)
        && (my->count[6] % 15 == 0))
    {
      theta = rand() % 360;
      tenm_table_add(explosion_new(my->x + 30.0 * tenm_cos(theta),
                                   my->y + 30.0 * tenm_sin(theta),
                                   0.0, 0.0,
                                   2, 300, i, 5.0, 8));
    }

    if (my->count[6] > 120)
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
  if (my->count[6] >= 6366)
  {
    set_background(2);
    perpeki_explode(my);
    clear_chain();

    return 0;
  }

  /* normal move */
  if (my->x < 50.0)
    my->count[2] = 1;
  else if (my->x > ((double) WINDOW_WIDTH) - 50.0)
    my->count[2] = -1;

  my->count_d[0] = 2.0 * ((double) (my->count[2]));
  my->count_d[1] = 0.0;

  /* evasion begin */
  if (my->count[3] == 0)
  {
    if ((my->x - 30.0 < player->x) && (my->x + 30.0 > player->x))
    {
      if (rand() % 3 != 0)
      {  
        my->count[3] = 1;
      }
      else
      {  
        my->count[3] = 2;
      }
      if (my->x < 160.0)
        my->count[4] = 1;
      else if (my->x > ((double) WINDOW_WIDTH) - 160.0)
        my->count[4] = -1;
      else if ((double) (rand() % WINDOW_WIDTH) > my->x)
        my->count[4] = 1;
      else
        my->count[4] = -1;
      my->count[5] = 0;
      my->count_d[2] = my->x + 86.6025 * ((double) (my->count[4]));
      my->count_d[3] = my->y - 50.0;
    }
  }

  /* evasion */
  if (my->count[3] != 0)
  {
    if (my->count[3] == 1)
    {
      if (my->count[5] < 20)
      {
        my->count_d[0] = 4.3301 * ((double) (my->count[4]));
        my->count_d[1] = 2.5;
      }
      else if (my->count[5] < 30)
      {
        my->count_d[0] = 0.0;
        my->count_d[1] = -5.0;
      }
      else
      {
        my->count[3] = 0;
        my->count[4] = 0;
        my->count[5] = 0;
      }
    }
    else
    {
      if (my->count[5] < 30)
      {
        theta = 150 - (my->count[5] + 1) * 4;
        if (my->count[4] < 0)
          theta = 180 - theta;
        my->count_d[0] = my->count_d[2] + 100.0 * tenm_cos(theta);
        my->count_d[1] = my->count_d[3] + 100.0 * tenm_sin(theta);
        my->count_d[0] -= my->x;
        my->count_d[1] -= my->y;
      }
      else
      {
        my->count[3] = 0;
        my->count[4] = 0;
        my->count[5] = 0;
      }
    }
    (my->count[5])++;
  }

  /* shoot */
  (my->count[6])++;
  if (((my->count[6] >= 0) && (my->count[6] < 768))
      || ((my->count[6] >= 3120) && (my->count[6] < 3888))
      || ((my->count[6] >= 6000) && (my->count[6] < 6240)))
  {
    perpeki_normal_shot(my, player, 0);
    if (my->count[9] >= 1)
      perpeki_normal_shot(my, player, 1);
    if (my->count[9] >= 2)
      perpeki_normal_shot(my, player, 2);
  }

  if (((my->count[6] >= 864) && (my->count[6] < 2304))
      || ((my->count[6] >= 3888) && (my->count[6] < 5184)))
  {
    if (my->count[9] >= 1)
      perpeki_normal_shot(my, player, 1);
    if (my->count[9] >= 2)
      perpeki_normal_shot(my, player, 0);
  }

  if (((my->count[6] >= 2304) && (my->count[6] < 3072))
      || ((my->count[6] >= 5184) && (my->count[6] < 5952)))
  {
    switch (my->count[6] % 192)
    {
    case 0:
      for (i = 0; i < 7; i++)
        tenm_table_add(laser_point_new(my->x, my->y,
                                       1.5 + 1.5 * ((double) i),
                                       player->x + 200.0,
                                       player->y - 100.0,
                                       25.0, 2));
      break;
    case 12:
      for (i = 0; i < 7; i++)
        tenm_table_add(laser_point_new(my->x, my->y,
                                       1.5 + 1.5 * ((double) i),
                                       player->x,
                                       player->y,
                                       25.0, 2));
      break;
    case 24:
      for (i = 0; i < 7; i++)
        tenm_table_add(laser_point_new(my->x, my->y,
                                       1.5 + 1.5 * ((double) i),
                                       player->x - 200.0,
                                       player->y + 100.0,
                                       25.0, 2));
      break;
    case 72:
      for (i = 0; i < 7; i++)
        tenm_table_add(laser_point_new(my->x, my->y,
                                       1.5 + 1.5 * ((double) i),
                                       player->x - 400.0,
                                       player->y,
                                       25.0, 2));
      break;
    case 96:
      for (i = 0; i < 7; i++)
        tenm_table_add(laser_point_new(my->x, my->y,
                                       1.5 + 1.5 * ((double) i),
                                       player->x - 150.0,
                                       player->y - 100.0,
                                       25.0, 2));
      break;
    case 102:
      for (i = 0; i < 7; i++)
        tenm_table_add(laser_point_new(my->x, my->y,
                                       1.5 + 1.5 * ((double) i),
                                       player->x,
                                       player->y,
                                       25.0, 2));
      break;
    case 108:
      for (i = 0; i < 7; i++)
        tenm_table_add(laser_point_new(my->x, my->y,
                                       1.5 + 1.5 * ((double) i),
                                       player->x + 150.0,
                                       player->y + 100.0,
                                       25.0, 2));
      break;
    case 114:
      for (i = 0; i < 7; i++)
        tenm_table_add(laser_point_new(my->x, my->y,
                                       1.5 + 1.5 * ((double) i),
                                       player->x + 400.0,
                                       player->y,
                                       25.0, 2));
      break;
    default:
      break;
    }

    if (my->count[6] % 192 == 120)
      my->count[7] = rand() % 360;
    if (my->count[6] % 192 >= 120)
    {
      if (my->count[6] % 2 == 0)
        speed = 4.0;
      else
        speed = 4.5;
      theta = my->count[7] - ((my->count[6] % 192) - 120) * 5;
      if (my->count[7] % 2 == 0)
        theta *= -1;
      tenm_table_add(laser_angle_new(my->x, my->y, speed, theta,
                                     25.0, 5));
    }

    if (my->count[6] % 192 == 156)
      my->count[8] = rand() % 360;
    if (my->count[6] % 192 >= 156)
    {
      if (my->count[6] % 2 == 0)
        speed = 9.0;
      else
        speed = 9.5;
      theta = my->count[8] + ((my->count[6] % 192) - 120) * 10;
      if (my->count[7] % 2 == 0)
        theta *= -1;
      tenm_table_add(laser_angle_new(my->x, my->y, speed, theta,
                                     25.0, 5));
    }
  }

  /* add backdancer */
  if (my->count[6] == -144)
  {
    for (i = 0; i < 3; i++)
      tenm_table_add(perpeki_backdancer_new(my->table_index, i));
  }
  
  /* add chip */
  if (my->count[6] == 900)
    tenm_table_add(perpeki_chip_new(0));
  if (my->count[6] == 1100)
    tenm_table_add(perpeki_chip_new(1));
  if (my->count[6] == 1300)
    tenm_table_add(perpeki_chip_new(2));
  if (my->count[6] == 1500)
    tenm_table_add(perpeki_chip_new(3));
  if (my->count[6] == 1700)
    tenm_table_add(perpeki_chip_new(4));
  if (my->count[6] == 1900)
    tenm_table_add(perpeki_chip_new(5));
  if (my->count[6] == 2100)
    tenm_table_add(perpeki_chip_new(6));

  if (my->count[6] == 3888)
    tenm_table_add(perpeki_chip_new(7));
  if (my->count[6] == 4088)
    tenm_table_add(perpeki_chip_new(8));
  if (my->count[6] == 4288)
    tenm_table_add(perpeki_chip_new(9));
  if (my->count[6] == 4488)
    tenm_table_add(perpeki_chip_new(10));
  if (my->count[6] == 4688)
    tenm_table_add(perpeki_chip_new(11));
  if (my->count[6] == 4888)
    tenm_table_add(perpeki_chip_new(12));
  if (my->count[6] == 5088)
    tenm_table_add(perpeki_chip_new(13));

  return 0;
}

static void
perpeki_normal_shot(tenm_object *my, const tenm_object *player, int what)
{
  int i;
  int theta;

  /* sanity check */
  if (my == NULL)
    return;
  if (player == NULL)
    return;

  if (what == 0)
  {
    switch (my->count[6] % 48)
    {
    case 0:
      tenm_table_add(normal_shot_point_new(my->x, my->y, 5.5,
                                           player->x,
                                           player->y,
                                           1));
      break;
    case 9:
      tenm_table_add(normal_shot_point_new(my->x, my->y, 6.5,
                                           player->x + 60.0,
                                           player->y,
                                           1));
      break;
    case 18:
      tenm_table_add(normal_shot_point_new(my->x, my->y, 7.5,
                                           player->x - 60.0,
                                           player->y,
                                           1));
      break;
    case 30:
      tenm_table_add(normal_shot_point_new(my->x, my->y, 5.5,
                                           player->x,
                                           player->y,
                                           1));
      break;
    case 36:
      tenm_table_add(normal_shot_point_new(my->x, my->y, 6.5,
                                           player->x - 60.0,
                                           player->y,
                                           1));
      break;
    case 42:
      tenm_table_add(normal_shot_point_new(my->x, my->y, 7.5,
                                           player->x + 60.0,
                                           player->y,
                                           1));
      break;
    default:
      break;
    }
  }

  if (what == 1)
  {
    switch (my->count[6] % 48)
    {
    case 0:
      tenm_table_add(normal_shot_angle_new(my->x, my->y, 5.5,
                                           170, 0));
      tenm_table_add(normal_shot_angle_new(my->x, my->y, 5.5,
                                           10, 0));
      break;
    case 9:
      tenm_table_add(normal_shot_angle_new(my->x, my->y, 6.0,
                                           150, 0));
      tenm_table_add(normal_shot_angle_new(my->x, my->y, 6.0,
                                           30, 0));
      break;
    case 18:
      tenm_table_add(normal_shot_angle_new(my->x, my->y, 6.5,
                                           130, 0));
      tenm_table_add(normal_shot_angle_new(my->x, my->y, 6.5,
                                           50, 0));
      break;
    case 30:
      tenm_table_add(normal_shot_angle_new(my->x, my->y, 5.5,
                                           160, 0));
      tenm_table_add(normal_shot_angle_new(my->x, my->y, 5.5,
                                           20, 0));
      break;
    case 36:
      tenm_table_add(normal_shot_angle_new(my->x, my->y, 6.0,
                                           140, 0));
      tenm_table_add(normal_shot_angle_new(my->x, my->y, 6.0,
                                           40, 0));
      break;
    case 42:
      tenm_table_add(normal_shot_angle_new(my->x, my->y, 6.5,
                                           120, 0));
      tenm_table_add(normal_shot_angle_new(my->x, my->y, 6.5,
                                           60, 0));
      break;
    default:
      break;
    }
  }

  if (what == 2)
  {  
    if (my->count[6] % 48 == 24)
    {
      theta = rand() % 360;
      for (i = 0; i < 360; i += 30)
        tenm_table_add(normal_shot_new(my->x, my->y,
                                       2.0 * tenm_cos(theta + i),
                                       3.5 + 2.0 * tenm_sin(theta + i),
                                       2, -2, 0));
    }
    if (my->count[6] % 48 == 0)
    {    
      theta = rand() % 360;
      for (i = 0; i < 360; i += 30)
        tenm_table_add(normal_shot_new(my->x, my->y,
                                       4.0 + 2.0 * tenm_cos(theta + i),
                                       0.5 + 2.0 * tenm_sin(theta + i),
                                       2, -2, 0));
      theta = rand() % 360;
      for (i = 0; i < 360; i += 30)
        tenm_table_add(normal_shot_new(my->x, my->y,
                                       -4.0 + 2.0 * tenm_cos(theta + i),
                                       0.5 + 2.0 * tenm_sin(theta + i),
                                       2, -2, 0));
    }
  }
}

/* return 1 (true) or 0 (false) */
static int
perpeki_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if ((my->count[10] == 1) && (my->count[6] >= 6336))
    return 0;
  if (my->count[10] == 2)
  {
    if (my->count[11] != 0)
      return 1;
    else
      return 0;
  }

  if (my->count[9] >= 2)
    return 1;
  if (((my->count[6] >= 864) && (my->count[6] < 2304))
      || ((my->count[6] >= 3888) && (my->count[6] < 5184)))
    return 1;

  return 0;
}

static int
perpeki_draw(tenm_object *my, int priority)
{
  tenm_color color;
  char temp[32];
  int status = 0;

  /* sanity check */
  if (my == NULL)
    return 0;

  if (((my->count[10] != 2) && (priority != 0))
      || ((my->count[10] == 2) && (priority != -1)))
    return 0;

  /* body */
  if (perpeki_green(my))
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
  if (tenm_draw_line(((int) (my->x)) + 30,
                     ((int) (my->y)) + 30,
                     ((int) (my->x)) - 30,
                     ((int) (my->y)) + 30,
                     2, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x)) - 30,
                     ((int) (my->y)) + 30,
                     ((int) (my->x)) - 30,
                     ((int) (my->y)) - 30,
                     2, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x)) - 30,
                     ((int) (my->y)) - 30,
                     ((int) (my->x)) + 30,
                     ((int) (my->y)) - 30,
                     2, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x)) + 30,
                     ((int) (my->y)) - 30,
                     ((int) (my->x)) + 30,
                     ((int) (my->y)) + 30,
                     2, color) != 0)
    status = 1;

  if (my->count[10] != 2)
  {
    
    if (tenm_draw_line(((int) (my->x)) + 10,
                       ((int) (my->y)) - 30,
                       ((int) (my->x)) + 20,
                       ((int) (my->y)) - 80,
                       2, color) != 0)
      status = 1;
    if (tenm_draw_line(((int) (my->x)) + 20,
                       ((int) (my->y)) - 30,
                       ((int) (my->x)) + 30,
                       ((int) (my->y)) - 80,
                       2, color) != 0)
      status = 1;
    if (tenm_draw_line(((int) (my->x)) + 20,
                       ((int) (my->y)) - 80,
                       ((int) (my->x)) + 30,
                       ((int) (my->y)) - 80,
                       2, color) != 0)
      status = 1;

    if (tenm_draw_line(((int) (my->x)) - 10,
                       ((int) (my->y)) - 30,
                       ((int) (my->x)) - 30,
                       ((int) (my->y)) - 70,
                       2, color) != 0)
      status = 1;
    if (tenm_draw_line(((int) (my->x)) - 20,
                       ((int) (my->y)) - 30,
                       ((int) (my->x)) - 40,
                       ((int) (my->y)) - 70,
                       2, color) != 0)
      status = 1;
    if (tenm_draw_line(((int) (my->x)) - 30,
                       ((int) (my->y)) - 70,
                       ((int) (my->x)) - 40,
                       ((int) (my->y)) - 70,
                       2, color) != 0)
      status = 1;
  }
  
  /* hit point stat */
  if (my->count[10] == 1)
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 5, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "perpeki_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

static tenm_object *
perpeki_chip_new(int what)
{
  int i;
  int n = 1;
  int suffix;
  double a_x;
  double a_y;
  double b_x;
  double b_y;
  double t;
  tenm_object *new = NULL;
  int *count = NULL;
  int x = 0;
  int y = 0;

  switch (what)
  {
  case 0:
    n = 37;
    break;
  case 1:
    n = 32;
    break;
  case 2:
    n = 64;
    break;
  case 3:
    n = 48;
    break;
  case 4:
    n = 48;
    break;
  case 5:
    n = 36;
    break;
  case 6:
    n = 60;
    break;
  case 7:
    n = 96;
    break;
  case 8:
    n = 120;
    break;
  case 9:
    n = 72;
    break;
  case 10:
    n = 96;
    break;
  case 11:
    n = 120;
    break;
  case 12:
    n = 90;
    break;
  case 13:
    n = 120;
    break;
  default:
    fprintf(stderr, "perpeki_chip_new: strange what (%d)\n", what);
    return NULL;
    break;
  }
  
  count = (int *) malloc(sizeof(int) * (3 + n * 3));
  if (count == NULL)
  {
    fprintf(stderr, "perpeki_chip_new: malloc(count) failed\n");
    return NULL;
  }

  /* list of count
   * [0] color (for delete_enemy_shot)
   * [1] border y
   * [2] number of lasers
   * [3 --] laser (x, y, theta)
   */
  count[0] = 6;
  count[1] = -200;
  count[2] = n;

  switch (what)
  {
  case 0:
    for (i = 0; i < n; i++)
    {
      suffix = 3 + i * 3;
      count[suffix + 0] = (int) (150.0 * tenm_cos(i * 5));
      count[suffix + 1] = (int) (150.0 * tenm_sin(i * 5));
      count[suffix + 0] += WINDOW_WIDTH / 2;
      count[suffix + 1] += WINDOW_HEIGHT / 8;
      count[suffix + 2] = i * 5;
    }
    break;
  case 1:
    for (i = 0; i < n; i++)
    {
      suffix = 3 + i * 3;
      if (i < 16)
      {        
        a_x = (double) (WINDOW_WIDTH / 2 - 20);
        a_y = (double) (WINDOW_HEIGHT / 8);
        b_x = 20.0;
        b_y = (double) (WINDOW_HEIGHT / 2);
      }
      else
      {        
        a_x = (double) (WINDOW_WIDTH / 2 + 20);
        a_y = (double) (WINDOW_HEIGHT / 8);
        b_x = (double) (WINDOW_WIDTH - 20);
        b_y = (double) (WINDOW_HEIGHT / 2);
      }
      t = ((double) (i % 16)) / 15.0;
      count[suffix + 0] = (int) (a_x * (1.0 - t) + b_x * t);
      count[suffix + 1] = (int) (a_y * (1.0 - t) + b_y * t);
      if (i < 16)
        count[suffix + 2] = 110 - i * 5;
      else
        count[suffix + 2] = 70 + (i - 16) * 5;
    }
    break;
  case 2:
    for (i = 0; i < n; i++)
    {
      suffix = 3 + i * 3;
      if (i < 16)
      {        
        a_x = 300.0;
        a_y = 20.0;
        b_x = 20.0;
        b_y = 300.0;
      }
      else if (i < 32)
      {        
        a_x = 20.0;
        a_y = 20.0;
        b_x = 300.0;
        b_y = 300.0;
      }
      else if (i < 48)
      {        
        a_x = 340.0;
        a_y = 20.0;
        b_x = 620.0;
        b_y = 300.0;
      }
      else
      {        
        a_x = 620.0;
        a_y = 20.0;
        b_x = 340.0;
        b_y = 300.0;
      }
      t = ((double) (i % 16)) / 15.0;
      count[suffix + 0] = (int) (a_x * (1.0 - t) + b_x * t);
      count[suffix + 1] = (int) (a_y * (1.0 - t) + b_y * t);
      if (i < 16)
        count[suffix + 2] = 90 - i * 10;
      else if (i < 32)
        count[suffix + 2] = 90 - (i - 16) * 10;
      else if (i < 48)
        count[suffix + 2] = 90 + (i - 32) * 10;
      else
        count[suffix + 2] = 90 + (i - 48) * 10;
    }
    break;
  case 3:
    for (i = 0; i < n; i++)
    {
      suffix = 3 + i * 3;
      count[suffix + 0] = (int) (100.0 * tenm_cos((i % 24) * 15));
      count[suffix + 1] = (int) (100.0 * tenm_sin((i % 24) * 15));
      if (i < 24)
      {
        count[suffix + 0] += WINDOW_WIDTH / 4;
        count[suffix + 1] += WINDOW_HEIGHT / 4;
      }
      else
      {
        count[suffix + 0] += (WINDOW_WIDTH * 3) / 4;
        count[suffix + 1] += WINDOW_HEIGHT / 4;
      }
      count[suffix + 2] = (i % 24) * 15;
      if (i < 24)
        count[suffix + 2] -= 45;
      else
        count[suffix + 2] += 45;
    }
    break;
  case 4:
    for (i = 0; i < n; i++)
    {
      suffix = 3 + i * 3;
      if (i < 12)
      {
        count[suffix + 0] = 230 + 16 * i;
        count[suffix + 1] = WINDOW_HEIGHT / 6;
        count[suffix + 2] = 35 + i * 10;
      }
      else if (i < 24)
      {
        count[suffix + 0] = 38 + 16 * (i - 12);
        count[suffix + 1] = WINDOW_HEIGHT / 3;
        count[suffix + 2] = 45 + (i - 12) * 10;
      }
      else if (i < 36)
      {
        count[suffix + 0] = 422 + 16 * (i - 24);
        count[suffix + 1] = WINDOW_HEIGHT / 3;
        count[suffix + 2] = 35 + (i - 24) * 10;
      }
      else
      {
        count[suffix + 0] = 230 + 16 * (i - 36);
        count[suffix + 1] = WINDOW_HEIGHT / 2;
        count[suffix + 2] = 35 + (i - 36) * 10;
      }
    }
    break;
  case 5:
    for (i = 0; i < n; i++)
    {
      suffix = 3 + i * 3;
      if (i < 18)
      {
        count[suffix + 0] = 7 + (int) (300.0 * tenm_cos(i * 5));
        count[suffix + 1] = 7 + (int) (300.0 * tenm_sin(i * 5));
        count[suffix + 2] = 30 + i * 5;
      }
      else
      {
        count[suffix + 0] = 633 + (int) (300.0 * tenm_cos(180 - (i - 18) * 5));
        count[suffix + 1] = 7 + (int) (300.0 * tenm_sin(180 - (i - 18) * 5));
        count[suffix + 2] = -30 + 180 - (i - 18) * 5;
      }
    }
    break;
  case 6:
    for (i = 0; i < n; i++)
    {
      suffix = 3 + i * 3;
      if (i < 15)
      {
        count[suffix + 0] = 20;
        count[suffix + 1] = 20 + 20 * i;
        count[suffix + 2] = 0 + i * 5;
      }
      else if (i < 30)
      {
        count[suffix + 0] = 120;
        count[suffix + 1] = 20 + 20 * (i - 15);
        count[suffix + 2] = 22 + (i - 15) * 10;
      }
      else if (i < 45)
      {
        count[suffix + 0] = WINDOW_WIDTH - 120;
        count[suffix + 1] = 20 + 20 * (i - 30);
        count[suffix + 2] = 158 - (i - 30) * 10;
      }
      else
      {
        count[suffix + 0] = WINDOW_WIDTH - 20;
        count[suffix + 1] = 20 + 20 * (i - 45);
        count[suffix + 2] = 180 - (i - 45) * 5;
      }
    }
    break;
  case 7:
    for (i = 0; i < n; i++)
    {
      suffix = 3 + i * 3;
      if (i < 48)
        count[suffix + 0] = (WINDOW_WIDTH * ((i % 48))) / 49;
      else
        count[suffix + 0] = (WINDOW_WIDTH * (48 - (i % 48))) / 49;
      count[suffix + 1] = (WINDOW_HEIGHT * ((i % 48) + 1)) / 49;
      if (i < 48)
        count[suffix + 2] = 90 + i * 10;
      else
        count[suffix + 2] = 0 - (i - 48) * 10;
    }
    break;
  case 8:
    for (i = 0; i < n; i++)
    {
      suffix = 3 + i * 3;
      count[suffix + 0] = (int) (100.0 * tenm_cos(i * 9));
      count[suffix + 1] = (int) (100.0 * tenm_sin(i * 9));
      if (i < 40)
      {
        count[suffix + 0] += WINDOW_WIDTH / 4;
        count[suffix + 1] += WINDOW_HEIGHT / 4;
      }
      else if (i < 80)
      {
        count[suffix + 0] += WINDOW_WIDTH / 2;
        count[suffix + 1] += WINDOW_HEIGHT / 2;
      }
      else
      {
        count[suffix + 0] += (WINDOW_WIDTH * 3) / 4;
        count[suffix + 1] += (WINDOW_HEIGHT * 3) / 4;
      }
      count[suffix + 2] = i * 9;
      if (i < 40)
        count[suffix + 2] -= 60;
      else if (i < 80)
        count[suffix + 2] -= 10;
      else
        count[suffix + 2] += 40;
    }
    break;
  case 9:
    for (i = 0; i < n; i++)
    {
      suffix = 3 + i * 3;
      if (i < 48)
      {
        count[suffix + 0] = (WINDOW_WIDTH * (((i % 16) / 4) * 2 + 1)) / 8;
        count[suffix + 1] = 36 + 180 * (i / 16) + 18 * (i % 4);
      }      
      else
      {
        count[suffix + 0] = (WINDOW_WIDTH * ((((i-48) % 12) / 4) * 2 + 2)) / 8;
        count[suffix + 1] = 36 + 90 + 180 * ((i-48) / 12) + 18 * (i % 4);
      }  
      count[suffix + 2] = i * 20;
    }
    break;
  case 10:
    for (i = 0; i < n; i++)
    {
      suffix = 3 + i * 3;
      count[suffix + 0] = ((WINDOW_WIDTH / 2) * ((i % 48))) / 49;
      count[suffix + 1] = (WINDOW_HEIGHT * ((i % 48) + 1)) / 49;
      if (i >= 48)
        count[suffix + 0] += WINDOW_WIDTH / 2;
      count[suffix + 2] = i * 10;
    }
    break;
  case 11:
    for (i = 0; i < n; i++)
    {
      suffix = 3 + i * 3;
      count[suffix + 0] = (int) (150.0 * tenm_cos(i * 3));
      count[suffix + 1] = (int) (150.0 * tenm_sin(i * 3));
      count[suffix + 0] += WINDOW_WIDTH / 2;
      count[suffix + 1] += WINDOW_HEIGHT / 2;
      count[suffix + 2] = i * 3;
      if (i % 3 == 0)
        count[suffix + 2] += 60;
      else if (i % 3 == 1)
        count[suffix + 2] -= 30;
    }
    break;
  case 12:
    for (i = 0; i < n; i++)
    {
      suffix = 3 + i * 3;
      if (i < 45)
      {
        count[suffix + 0] = 7 + (int) (300.0 * tenm_cos(-(i * 2)));
        count[suffix + 1] = 473 + (int) (300.0 * tenm_sin(-(i * 2)));
        count[suffix + 2] = -(i * 2);
      }
      else
      {
        count[suffix + 0] = 633 + (int) (300.0 * tenm_cos(90 + (i - 45) * 2));
        count[suffix + 1] = 7 + (int) (300.0 * tenm_sin(90 + (i - 45) * 2));
        count[suffix + 2] = 90 + (i - 45) * 2;
      }
      if (i % 2 == 0)
        count[suffix + 2] += 30;
    }
    break;
  case 13:
    for (i = 0; i < n; i++)
    {
      suffix = 3 + i * 3;
      a_x = 200.0 * tenm_cos((i / 24) * 72);
      a_y = 200.0 * tenm_sin((i / 24) * 72);
      b_x = 200.0 * 0.3820 * tenm_cos(((i+12) / 24) * 72 - 36);
      b_y = 200.0 * 0.3820 * tenm_sin(((i+12) / 24) * 72 - 36);
      t = ((double) (i % 12)) / 11.0;
      count[suffix + 0] = WINDOW_WIDTH / 2 + (int) (a_x * (1.0 - t) + b_x * t);
      count[suffix + 1] = WINDOW_HEIGHT / 2 + (int) (a_y * (1.0 -t) + b_y * t);
      count[suffix + 2] = i * 5;
      if (i % 2 == 0)
        count[suffix + 2] += 40;
    }
    break;
  default:
    fprintf(stderr, "perpeki_chip_new: undefined what (%d)\n", what);
    count[2] = 0;
    break;
  }

  /* add a little of randomness */
  if (rand() % 2 == 0)
  {
    for (i = 0; i < n; i++)
    {
      suffix = 3 + i * 3;
      count[suffix + 0] = WINDOW_WIDTH - count[suffix + 0];
      count[suffix + 2] = 180 - count[suffix + 2];
    }
  }
  x = -5 + (rand() % 11);
  y = -5 + (rand() % 11);
  for (i = 0; i < n; i++)
  {
    suffix = 3 + i * 3;
    count[suffix + 0] += x;
    count[suffix + 1] += y;
  }

  /* ATTR_ENEMY_SHOT is only to clear it when the player is killed */
  new = tenm_object_new("perpeki_chip", ATTR_ENEMY_SHOT, 0,
                        0,
                        (double) (x + WINDOW_WIDTH / 2),
                        (double) (y + WINDOW_HEIGHT / 2),
                        3 + n * 3, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double)) NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&perpeki_chip_act),
                        (int (*)(tenm_object *, int))
                        (&perpeki_chip_draw));
  if (new == NULL)
  {
    fprintf(stderr, "perpeki_chip_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
perpeki_chip_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int suffix;
  int theta;
  double dx;
  double dy;

  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  /* move border */
  my->count[1] += 4;
  if (my->count[1] > WINDOW_HEIGHT + 15)
    return 1;

  /* shoot */
  for (i = 0; i < my->count[2]; i++)
  {
    suffix = 3 + i * 3;
    if ((my->count[suffix + 1] > my->count[1])
        || (my->count[suffix + 1] <= my->count[1] - 4))
      continue;
      
    theta = my->count[suffix + 2];
    while (theta >= 360)
      theta -= 360;
    while (theta < 0)
      theta += 360;

    if ((double) (my->count[suffix + 1]) < player->y)
    {
      if (theta > 180)
        theta -= 180;
    }
    else
    {
      if (theta <= 180)
        theta -= 180;
    }

    dx = 15.0 * tenm_cos(theta);
    dy = 15.0 * tenm_sin(theta);
    tenm_table_add(laser_angle_new(my->count[suffix + 0] - dx,
                                   my->count[suffix + 1] - dy,
                                   8.0, theta, 30.0, 4));
  }

  return 0;
}

static int
perpeki_chip_draw(tenm_object *my, int priority)
{
  tenm_color color;
  int i;
  int suffix;
  int status = 0;
  int dx;
  int dy;
  int theta;

  /* sanity check */
  if (my == NULL)
    return 0;

  if (priority != 0)
    return 0;

  /* border */
  color = tenm_map_color(158, 158, 158);
  if (my->count[1] >= 0)
  {
    if (tenm_draw_line(0, my->count[1], WINDOW_WIDTH, my->count[1],
                       1, color) != 0)
      status = 1;
  }

  /* laser */
  color = tenm_map_color(118, 99, 158);
  for (i = 0; i < my->count[2]; i++)
  {
    suffix = 3 + i * 3;
    if (my->count[suffix + 1] <= my->count[1])
      continue;
      
    theta = my->count[suffix + 2];
    dx = (int) (15.0 * tenm_cos(theta));
    dy = (int) (15.0 * tenm_sin(theta));

    if (tenm_draw_line(my->count[suffix + 0] + dx,
                       my->count[suffix + 1] + dy,
                       my->count[suffix + 0] - dx,
                       my->count[suffix + 1] - dy,
                       1, color) != 0)
      status = 1;
  }

  return status;
}

static tenm_object *
perpeki_backdancer_new(int table_index, int what)
{
  int hit_mask;
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = 0.0;
  double y = 0.0;
  double target_y;

  /* sanity check */
  if (table_index < 0)
  {
    fprintf(stderr, "perpeki_backdancer_new: strange table_index "
            "(%d)\n", table_index);
    return NULL;
  }
  if ((what < 0) || (what > 2))
  {
    fprintf(stderr, "perpeki_backdancer_new: strange what (%d)\n", what);
    return NULL;
  }

  x = (double) (WINDOW_WIDTH * (what + 1) / 4);
  y = - 19.0;

  target_y = 60.0;

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "perpeki_backdancer_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + 20.0,
                                             y + 20.0,
                                             x - 20.0,
                                             y + 20.0,
                                             x - 20.0,
                                             y - 20.0,
                                             x + 20.0,
                                             y - 20.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "perpeki_backdancer_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 8);
  if (count == NULL)
  {
    fprintf(stderr, "perpeki_backdancer_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  count_d = (double *) malloc(sizeof(double) * 6);
  if (count_d == NULL)
  {
    fprintf(stderr, "perpeki_backdancer_new: malloc(count_d) failed\n");
    if (count != NULL)
      free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] for deal_damage
   * [1] "damaged" timer
   * [2] move/shoot timer
   * [3] what
   * [4] shoot timer
   * [5] core index
   * [6] life mode
   */
  count[0] = 0;
  count[1] = 0;
  count[2] = 0;
  count[3] = what;
  count[4] = 0;
  count[5] = table_index;
  count[6] = 0;

  /* list of count_d
   * [0] speed x
   * [1] speed y
   */
  count_d[0] = 0.0;
  count_d[1] = (target_y - y) / 60.0;

  if (what == 2)
    hit_mask = 0;
  else
    hit_mask = ATTR_PLAYER_SHOT;
  new = tenm_object_new("Perpeki backdancer", ATTR_ENEMY,
                        hit_mask,
                        1000, x, y,
                        8, count, 6, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&perpeki_backdancer_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&perpeki_backdancer_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&perpeki_backdancer_act),
                        (int (*)(tenm_object *, int))
                        (&perpeki_backdancer_draw));
  if (new == NULL)
  {
    fprintf(stderr, "perpeki_backdancer_new: tenm_object_new failed\n");
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
perpeki_backdancer_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
    return 0;
  if (turn_per_frame <= 0.5)
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
perpeki_backdancer_hit(tenm_object *my, tenm_object *your)
{
  int n;
  /* sanity check */
  if (my == NULL)
    return 0;
  if (your == NULL)
    return 0;

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;
  if (my->count[3] == 2)
    return 0;
  if (my->count[6] != 1)
    return 0;

  deal_damage(my, your, 0);
  if (perpeki_backdancer_green(my))
    add_chain(my, your);
  my->count[1] = 41;

  if (my->hit_point <= 0)
  {
    add_score(75000);
    tenm_table_apply(my->count[5],
                     (int (*)(tenm_object *, int))
                     (&perpeki_backdancer_signal),
                     0);
    if (perpeki_backdancer_green(my))
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
perpeki_backdancer_signal(tenm_object *my, int n)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (strcmp(my->name, "Perpeki") != 0)
    return 0;

  (my->count[9])++;

  return 0;
}

static int
perpeki_backdancer_act(tenm_object *my, const tenm_object *player)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  /* for deal_damage */
  my->count[0] = 0;

  /* "damaged" count down */
  if (my->count[1] > 0)
    (my->count[1])--;

  /* encounter */
  if (my->count[6] == 0)
  {
    (my->count[2])++;
    if (my->count[2] >= 60)
    {
      my->count[2] = -83;
      my->count[4] = my->count[3] * 3;
      my->count[6] = 1;
      my->count_d[0] = 3.0;
      my->count_d[1] = 0.0;
      return 0;
    }
    return 0;
  }

  /* speed change */
  if (my->x < ((double) (WINDOW_WIDTH * (my->count[3] + 1) / 4)) + NEAR_ZERO)
    my->count_d[0] += 0.1;
  else
    my->count_d[0] -= 0.1;

  /* shoot */
  (my->count[2])++;
  (my->count[4])++;
  if (my->count[4] >= 9)
  {
    if (((my->count[2] >= 0) && (my->count[2] < 768))
        || ((my->count[2] >= 864) && (my->count[2] < 2304))
        || ((my->count[2] >= 3120) && (my->count[2] < 5184))
        || ((my->count[2] >= 6000) && (my->count[2] < 6240)))
      tenm_table_add(normal_shot_point_new(my->x, my->y, 4.0,
                                         player->x, player->y, 3));
    my->count[4] = 0;
  }

  return 0;
}

static int
perpeki_backdancer_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if (my->count[3] == 2)
    return 0;
  if (my->count[6] != 1)
    return 0;
  if (my->count[2] >= 6336)
    return 0;

  return 1;
}

static int
perpeki_backdancer_draw(tenm_object *my, int priority)
{
  tenm_color color;
  char temp[32];
  int status = 0;

  /* sanity check */
  if (my == NULL)
    return 0;

  if (priority != 0)
    return 0;

  /* decoration */
  if (my->count[3] == 2)
  {
    color = tenm_map_color(182, 123, 162);
  }
  else if (perpeki_backdancer_green(my))
  {
    if (my->count[1] >= 40)
      color = tenm_map_color(181, 190, 92);
    else
      color = tenm_map_color(157, 182, 123);
  }
  else
  {
    if (my->count[1] >= 40)
      color = tenm_map_color(200, 164, 92);
    else
      color = tenm_map_color(182, 147, 123);
  }
  if (tenm_draw_line(((int) (my->x)) + 10,
                     ((int) (my->y)) - 20,
                     ((int) (my->x)) + 20,
                     ((int) (my->y)) - 50,
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x)) - 10,
                     ((int) (my->y)) - 20,
                     ((int) (my->x)) - 25,
                     ((int) (my->y)) - 40,
                     1, color) != 0)
    status = 1;

  /* body */
  if (my->count[3] == 2)
  {
    color = tenm_map_color(95, 13, 68);
  }
  else if (perpeki_backdancer_green(my))
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
  if (tenm_draw_line(((int) (my->x)) + 20,
                     ((int) (my->y)) + 20,
                     ((int) (my->x)) - 20,
                     ((int) (my->y)) + 20,
                     2, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x)) - 20,
                     ((int) (my->y)) + 20,
                     ((int) (my->x)) - 20,
                     ((int) (my->y)) - 20,
                     2, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x)) - 20,
                     ((int) (my->y)) - 20,
                     ((int) (my->x)) + 20,
                     ((int) (my->y)) - 20,
                     2, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x)) + 20,
                     ((int) (my->y)) - 20,
                     ((int) (my->x)) + 20,
                     ((int) (my->y)) + 20,
                     2, color) != 0)
    status = 1;

  /* hit point stat */
  if (my->count[1] > 0)
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string((int) (my->x - 10.0), (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "perpeki_backdancer_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}
