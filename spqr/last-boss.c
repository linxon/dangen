/* $Id: last-boss.c,v 1.431 2005/07/12 20:33:12 oohara Exp $ */

#include <stdio.h>
/* malloc, rand */
#include <stdlib.h>
/* strlen */
#include <string.h>

#include "tenm_object.h"
#include "tenm_graphic.h"
#include "tenm_primitive.h"
#include "const.h"
#include "tenm_table.h"
#include "laser.h"
#include "normal-shot.h"
#include "tenm_math.h"
#include "util.h"
#include "background.h"
#include "chain.h"
#include "explosion.h"
#include "stage-clear.h"
#include "score.h"
#include "ship.h"
/* deal_damage */
#include "player-shot.h"

#include "last-boss.h"

#define NEAR_ZERO 0.0001

static int last_boss_hit(tenm_object *my, tenm_object *your);
static void last_boss_next(tenm_object *my);
static int last_boss_act(tenm_object *my, const tenm_object *player);
static int last_boss_act_0(tenm_object *my, const tenm_object *player);
static int last_boss_act_1(tenm_object *my, const tenm_object *player);
static int last_boss_act_2(tenm_object *my, const tenm_object *player);
static int last_boss_act_3(tenm_object *my, const tenm_object *player);
static int last_boss_act_4(tenm_object *my, const tenm_object *player);
static int last_boss_act_5(tenm_object *my, const tenm_object *player);
static int last_boss_draw(tenm_object *my, int priority);
static int last_boss_green(const tenm_object *my);

static tenm_object *last_boss_cage_new(int n);
static int last_boss_cage_act(tenm_object *my, const tenm_object *player);
static int last_boss_cage_draw(tenm_object *my, int priority);

static tenm_object *last_boss_star_dust_new(int n);
static int last_boss_star_dust_act(tenm_object *my, const tenm_object *player);
static int last_boss_star_dust_draw(tenm_object *my, int priority);

static tenm_object * last_boss_spotlight_new(int n);
static int last_boss_spotlight_act(tenm_object *my, const tenm_object *player);
static int last_boss_spotlight_draw(tenm_object *my, int priority);

static tenm_object *last_boss_horizon_circle_new(int n);
static int last_boss_horizon_circle_act(tenm_object *my,
                                       const tenm_object *player);
static int last_boss_horizon_circle_sights(double *result,
                                          const double *v1, const double *v2);
static int last_boss_horizon_circle_draw(tenm_object *my, int priority);

static tenm_object *last_boss_horizon_new(double x, double y, int n);
static int last_boss_horizon_act(tenm_object *my, const tenm_object *player);
static int last_boss_horizon_draw(tenm_object *my, int priority);

static tenm_object *last_boss_twist_circle_new(int n);
static int last_boss_twist_circle_act(tenm_object *my,
                                      const tenm_object *player);
static int last_boss_twist_circle_draw(tenm_object *my, int priority);

static tenm_object *last_boss_matrix_new(int n);
static int last_boss_matrix_act(tenm_object *my,
                                const tenm_object *player);

static tenm_object *last_boss_cross_circle_new(int n);
static int last_boss_cross_circle_act(tenm_object *my,
                                      const tenm_object *player);
static int last_boss_cross_circle_draw(tenm_object *my, int priority);

static tenm_object *last_boss_cross_new(double x, double y, int n);
static int last_boss_cross_act(tenm_object *my, const tenm_object *player);
static int last_boss_cross_draw(tenm_object *my, int priority);

tenm_object *
last_boss_new(void)
{
  int i;
  int suffix;
  tenm_object *new = NULL;
  int *count = NULL;
  tenm_primitive **p = NULL;
  double x = (double) (WINDOW_WIDTH / 2);
  double y = (double) (WINDOW_HEIGHT / 4);

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "last_boss_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_circle_new(x, y, 60.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "last_boss_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 19);
  if (count == NULL)
  {
    (p[0])->delete(p[0]);
    free(p);
    fprintf(stderr, "last_boss_new: malloc(count) failed\n");
    return NULL;
  }

  /* list of count
   * [0] mode
   * [1] shoot timer
   * [2] demo timer
   * [3] for deal_damage
   * [4 -- 15] decoration circle management
   *   suffix + 0: center x
   *   suffix + 1: center y
   *   suffix + 2: radius
   * [16] immutable timer
   * [17] "damaged" timer
   * [18] "was green when killed" flag
   */
  count[0] = 0;
  count[1] = 0;
  count[2] = 0;
  count[3] = 0;
  for (i = 0; i < 4; i++)
  {
    suffix = i * 3 + 4;
    if (i == 0)
    {
      count[suffix + 0] = WINDOW_WIDTH / 2;
      count[suffix + 1] = WINDOW_HEIGHT / 4;
      count[suffix + 2] = -240 + 2;
    }
    else
    {
      count[suffix + 0] = WINDOW_WIDTH / 2 - 50 + (rand() % 100);
      count[suffix + 1] = WINDOW_HEIGHT / 4 - 50 + (rand() % 100);
      count[suffix + 2] = (i - 1) * (-80) + 2;
    }
  }
  count[16] = 0;
  count[17] = 0;
  count[18] = 0;

  new = tenm_object_new("L", 0, 0,
                        750, x, y,
                        19, count, 0, NULL, 1, p,
                        (int (*)(tenm_object *, double)) NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        (&last_boss_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&last_boss_act),
                        (int (*)(tenm_object *, int)) (&last_boss_draw));
  if (new == NULL)
  {
    fprintf(stderr, "last_boss_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  return new;
}

static int
last_boss_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (your == NULL)
    return 0;

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;
  if (my->count[2] >= 0)
    return 0;
  if (my->count[16] > 0)
    return 0;

  deal_damage(my, your, 3);
  if (last_boss_green(my))
    add_chain(my, your);
  my->count[17] = 2;

  if (my->hit_point <= 0)
  {
    set_background(1);
    switch (my->count[0])
    {
    case 0:
      add_score(20000);
      break;
    case 1:
      add_score(30000);
      break;
    case 2:
      add_score(40000);
      break;
    case 3:
      add_score(50000);
      break;
    case 4:
      add_score(60000);
      break;
    case 5:
      add_score(100000);
      break;
    default:
      fprintf(stderr, "last_boss_hit: undefined mode (%d)\n", my->count[0]);
      break;
    }
    last_boss_next(my);
    return 0;
  }

  return 0;
}

static void
last_boss_next(tenm_object *my)
{
  int n;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "last_boss_next: my is NULL\n");
    return;
  } 
  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy_shot), 0);
  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy), 0);

  /* set "was green" flag before we change the life mode */
  if (last_boss_green(my))
  {
    n = 8;
    my->count[18] = 1;
  }
  else
  {
    n = 7;
    my->count[18] = 0;
  }

  tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                               1, 5000, n, 10.0, 6));

  (my->count[0])++;
  if (my->count[0] == 1)
  {
    my->hit_point = 1000;
  }
  else if (my->count[0] == 2)
  {
    my->hit_point = 600;
  }
  else if (my->count[0] == 3)
  {
    my->hit_point = 750;
  }
  else if (my->count[0] == 4)
  {
    my->hit_point = 400;
  }
  else if (my->count[0] == 5)
  {
    my->hit_point = 1500;
  }
  else
  {
    /* don't modify my->attr or my->hit_mask here, or the player shot
     * may fly through the enemy */
    tenm_mass_delete(my->mass);
    my->mass = NULL;
  }

  my->count[1] = 0;
  my->count[2] = 0;
  my->count[17] = 0;
}

static int
last_boss_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int suffix;
  int theta;

  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  /* for deal_damage */
  my->count[3] = 0;

  /* "damaged" count down */
  if (my->count[17] > 0)
    (my->count[17])--;

  /* encounter */
  if ((my->count[0] == 0) && (my->count[2] == 210))
  {
    my->attr = ATTR_BOSS;
    my->hit_mask = ATTR_PLAYER_SHOT;
  }

  /* dead */
  if (my->count[0] == 6)
  {
    my->count[16] = 0;
    (my->count[1])++;
    if (last_boss_green(my))
      i = 8;
    else
      i = 7;

    if ((my->count[1] >= 30) && (my->count[1] <= 90)
        && (my->count[1] % 5 == 0))
    {
      theta = rand() % 360;
      tenm_table_add(explosion_new(my->x + 30.0 * tenm_cos(theta),
                                   my->y + 30.0 * tenm_sin(theta),
                                   0.0, 0.0,
                                   2, 300, i, 5.0, 8));
    }

    if (my->count[1] == 120)
    {
      tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                                   1, 15000, i, 16.0, 15));
      tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                                   2, 3000, i, 12.0, 15));
      tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                                   3, 600, i, 9.0, 15));

      tenm_table_add(stage_clear_new(200));
      return 1;
    }

    return 0;
  }

  /* the boss is immutable if the player is immutable */
  if ((get_ship() < 0) || (player->count[1] > 0))
    my->count[16] = 100;
  else if (my->count[16] > 0)
    (my->count[16])--;

  /* decoration management */
  for (i = 0; i < 4; i++)
  {
    suffix = i * 3 + 4;
    my->count[suffix + 2] += 2;
    if (my->count[suffix + 2] > 60)
    {
      if (i != 0)
      {
        my->count[suffix + 0] = (WINDOW_WIDTH / 2) - 50 + (rand() % 100);
        my->count[suffix + 1] = (WINDOW_HEIGHT / 4) - 50 + (rand() % 100);
      }
      my->count[suffix + 2] = 2;
    }
  }

  /* attack */
  if (my->count[0] == 0)
  {
    last_boss_act_0(my, player);
  }
  else if (my->count[0] == 1)
  {
    last_boss_act_1(my, player);
  }
  else if (my->count[0] == 2)
  {
    last_boss_act_2(my, player);
  }
  else if (my->count[0] == 3)
  {
    last_boss_act_3(my, player);
  }
  else if (my->count[0] == 4)
  {
    last_boss_act_4(my, player);
  }
  else if (my->count[0] == 5)
  {
    last_boss_act_5(my, player);
  }

  
  return 0;
}

static int
last_boss_act_0(tenm_object *my, const tenm_object *player)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  if (my->count[2] >= 0)
  {
    (my->count[2])++;
    if ((my->count[2] == 240) || (my->count[2] == 300)
        || (my->count[2] == 345) || (my->count[2] == 375)
        || (my->count[2] == 400) || (my->count[2] == 420))
      tenm_table_add(last_boss_star_dust_new(1));
    if ((my->count[2] == 440) || (my->count[2] == 460)
        || (my->count[2] == 480) || (my->count[2] == 500)
        || (my->count[2] == 520) || (my->count[2] == 540))
      tenm_table_add(last_boss_star_dust_new(0));
    if (my->count[2] >= 559)
    {
      my->count[1] = 0;
      my->count[2] = -1;
      return 0;
    }
    return 0;
  }

  /* self-destruction */
  (my->count[2])--;
  if (my->count[2] <= -4030)
  {
    set_background(2);
    clear_chain();
    last_boss_next(my);
    return 0;
  }

  if (my->count[1] == 0)
  {
    tenm_table_add(last_boss_star_dust_new(0));
  }

  tenm_table_add(laser_angle_new(my->x + (double) (-40 + (rand() % 81)),
                                 my->y + (double) (-40 + (rand() % 81)),
                                 8.5 + ((double) (rand() % 8)) / 4.0,
                                 rand() % 360, 
                                 25.0, 3));

  (my->count[1])++;
  if (my->count[1] >= 20)
    my->count[1] = 0;

  return 0;
}

static int
last_boss_act_1(tenm_object *my, const tenm_object *player)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  if (my->count[2] >= 0)
  {
    (my->count[2])++;
    if (my->count[2] == 30)
      tenm_table_add(last_boss_twist_circle_new(1));
    if ((my->count[2] == 300) || (my->count[2] == 435))
      tenm_table_add(last_boss_twist_circle_new(0));
    if (my->count[2] >= 524)
    {
      my->count[1] = 0;
      my->count[2] = -1;
      return 0;
    }
    return 0;
  }

  /* self-destruction */
  (my->count[2])--;
  if (my->count[2] <= -4030)
  {
    set_background(2);
    clear_chain();
    last_boss_next(my);
    return 0;
  }

  if (my->count[1] == 0)
  {
    tenm_table_add(last_boss_twist_circle_new(0));
  }

  (my->count[1])++;
  if (my->count[1] >= 90)
    my->count[1] = 0;

  return 0;
}

static int
last_boss_act_2(tenm_object *my, const tenm_object *player)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  if (my->count[2] >= 0)
  {
    (my->count[2])++;
    if (my->count[2] == 60)
      tenm_table_add(last_boss_cage_new(1));
    if (my->count[2] == 240)
      tenm_table_add(last_boss_cage_new(2));
    if ((my->count[2] == 390) || (my->count[2] == 510))
      tenm_table_add(last_boss_cage_new(0));

    if (my->count[2] == 538)
      tenm_table_add(normal_shot_point_new(my->x, my->y, 9.0,
                                           player->x, player->y, 3));
    if (my->count[2] >= 629)
    {
      my->count[1] = 0;
      my->count[2] = -1;
      return 0;
    }
    return 0;
  }

  /* self-destruction */
  (my->count[2])--;
  if (my->count[2] <= -4030)
  {
    set_background(2);
    clear_chain();
    last_boss_next(my);
    return 0;
  }

  if (my->count[1] == 0)
  {
    tenm_table_add(last_boss_cage_new(0));
  }

  if (my->count[1] % 4 == 0)
  {
    tenm_table_add(normal_shot_point_new(my->x, my->y, 9.0,
                                         player->x
                                         + (double) (-50 + (rand() % 101)), 
                                         player->y
                                         + (double) (-50 + (rand() % 101)),
                                         3));
  }

  (my->count[1])++;
  if (my->count[1] >= 120)
    my->count[1] = 0;

  return 0;
}

static int
last_boss_act_3(tenm_object *my, const tenm_object *player)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  if (my->count[2] >= 0)
  {
    (my->count[2])++;
    if (my->count[2] == 30)
      tenm_table_add(last_boss_spotlight_new(0));
    if (my->count[2] == 150)
      tenm_table_add(last_boss_spotlight_new(1));
    if (my->count[2] == 250)
    {
      tenm_table_add(last_boss_spotlight_new(0));
      tenm_table_add(last_boss_spotlight_new(2));
    }
    if (my->count[2] == 330)
    {
      tenm_table_add(last_boss_spotlight_new(1));
      tenm_table_add(last_boss_spotlight_new(3));
    }

    if (my->count[2] >= 404)
    {
      my->count[1] = 0;
      my->count[2] = -1;
      return 0;
    }
    return 0;
  }

  /* self-destruction */
  (my->count[2])--;
  if (my->count[2] <= -4030)
  {
    set_background(2);
    clear_chain();
    last_boss_next(my);
    return 0;
  }

  if (my->count[1] == 0)
  {
    tenm_table_add(last_boss_spotlight_new(0));
    tenm_table_add(last_boss_spotlight_new(2));
  }
  if (my->count[1] == 75)
  {
    tenm_table_add(last_boss_spotlight_new(1));
    tenm_table_add(last_boss_spotlight_new(3));
  }

  if (my->count[1] % 15 == 0)
    tenm_table_add(normal_shot_point_new(my->x + (double) (-40 + rand() % 81),
                                         my->y + (double) (-40 + rand() % 81),
                                         9.0, player->x, player->y, 4));

  (my->count[1])++;
  if (my->count[1] >= 150)
    my->count[1] = 0;

  return 0;
}

static int
last_boss_act_4(tenm_object *my, const tenm_object *player)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  if (my->count[2] >= 0)
  {
    (my->count[2])++;
    if (my->count[2] == 30)
      tenm_table_add(last_boss_horizon_circle_new(1));
    if (my->count[2] == 240)
      tenm_table_add(last_boss_horizon_circle_new(2));
    if (my->count[2] == 420)
      tenm_table_add(last_boss_horizon_circle_new(0));

    if (my->count[2] >= 569)
    {
      my->count[1] = 0;
      my->count[2] = -1;
      return 0;
    }
    return 0;
  }

  /* self-destruction */
  (my->count[2])--;
  if (my->count[2] <= -4030)
  {
    set_background(2);
    clear_chain();
    last_boss_next(my);
    return 0;
  }

  if (my->count[1] == 0)
  {
    tenm_table_add(last_boss_horizon_circle_new(0));
  }

  if (my->count[1] % 15 == 0)
    tenm_table_add(laser_point_new(my->x, my->y, 15.0,
                                   player->x, player->y,
                                   25.0, 0));

  (my->count[1])++;
  if (my->count[1] >= 150)
    my->count[1] = 0;

  return 0;
}

static int
last_boss_act_5(tenm_object *my, const tenm_object *player)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  if (my->count[2] >= 0)
  {
    (my->count[2])++;
    if ((my->count[2] == 30) || (my->count[2] == 110)
        || (my->count[2] == 190) || (my->count[2] == 270)
        || (my->count[2] == 350) || (my->count[2] == 430)
        || (my->count[2] == 510) || (my->count[2] == 590))
      tenm_table_add(last_boss_cross_circle_new(1));
    if ((my->count[2] == 310) || (my->count[2] == 390)
        || (my->count[2] == 470) || (my->count[2] == 550))
      tenm_table_add(last_boss_cross_circle_new(0));

    if (my->count[2] >= 629)
    {
      my->count[1] = 0;
      my->count[2] = -1;
      return 0;
    }
    return 0;
  }

  /* self-destruction */
  (my->count[2])--;
  if (my->count[2] <= -8030)
  {
    set_background(2);
    clear_chain();
    last_boss_next(my);
    return 0;
  }

  if (my->count[1] % 60 == 0)
  {
    tenm_table_add(last_boss_matrix_new(0));
  }
  if (my->count[1] % 60 == 30)
  {
    tenm_table_add(last_boss_matrix_new(1));
  }

  if (my->count[1] % 80 == 0)
  {
    tenm_table_add(last_boss_cross_circle_new(0));
  }
  if (my->count[1] % 80 == 40)
  {
    tenm_table_add(last_boss_cross_circle_new(1));
  }

  (my->count[1])++;
  if (my->count[1] >= 240)
    my->count[1] = 0;

  return 0;
}

static int
last_boss_draw(tenm_object *my, int priority)
{
  int i;
  int r;
  int width;
  int suffix;
  int status = 0;
  tenm_color color;
  char temp[32];

  /* sanity check */
  if (my == NULL)
    return 0;

  /* decoration */
  if ((priority == -1) && (my->count[0] < 6))
  {
    for (i = 0; i < 4; i++)
    {
      suffix = i * 3 + 4;

      if (my->count[suffix + 2] <= 0)
        continue;

      if (last_boss_green(my))
      {
        if (i == 0)
          color = tenm_map_color(157, 182, 123);
        else
          color = tenm_map_color(190, 206, 167);
      }
      else
      {
        if (i == 0)
          color = tenm_map_color(182, 148, 123);
        else
          color = tenm_map_color(206, 183, 167);
      }

      if (tenm_draw_circle(my->count[suffix + 0],
                           my->count[suffix + 1],
                           my->count[suffix + 2], 1, color) != 0)
        status = 1;
    }

    if ((my->count[16] > 0) && (my->count[16] < 100))
    {
      if (last_boss_green(my))
        color = tenm_map_color(222, 225, 179);
      else
        color = tenm_map_color(230, 214, 179);

      if (tenm_draw_circle((int) (my->x), (int) (my->y),
                           60 + my->count[16] * 3, 1, color) != 0)
        status = 1;
    }
  }

  /* body */
  width = 2;

  if ((my->count[0] == 0) && (my->count[2] >= 0))
  {
    if (my->count[2] < 180)
    {  
      return status;
    }
    else if (my->count[2] < 210)
    {  
      r = 60 + (210 - my->count[2]);
      width = 1;
    }
    else
    {  
      r = 60;
    }
  }
  else
  {
    r = 60;
  }

  /* dead enemy has low priority */
  if (((my->count[0] < 6) && (priority == 0))
      || ((my->count[0] >= 6) && (my->count[1] < 120) && (priority == -1)))
  {
    if (last_boss_green(my))
    {
      if (my->count[16] > 0)
        color = tenm_map_color(190, 206, 167);
      else if (my->count[17] >= 1)
        color = tenm_map_color(109, 125, 9);
      else
        color = tenm_map_color(61, 95, 13);
    }
    else
    {
      if (my->count[16] > 0)
        color = tenm_map_color(206, 183, 167);
      else if (my->count[17] >= 1)
        color = tenm_map_color(135, 89, 9);
      else
        color = tenm_map_color(95, 47, 13);
    }
    
    
    if (tenm_draw_circle((int) (my->x), (int) (my->y),
                         r, width, color) != 0)
      status = 1;

    /* hit point stat */
    if ((my->count[0] < 6) && (my->count[2] < 0))
    { 
      sprintf(temp, "%d", my->hit_point);
      if (draw_string((int) my->x, (int) my->y, temp, (int) strlen(temp)) != 0)
      {
        fprintf(stderr, "last_boss_draw: draw_string failed\n");
        status = 1;
      }
    }
  }

  return status;
}

/* return 1 (true) or 0 (false) */
static int
last_boss_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if (my->count[0] < 6)
  {
    if ((my->count[2] >= 0) && (my->count[2] < 30) && (my->count[18] != 0))
      return 1;
    if (my->count[2] >= 0)
      return 0;
    if (((my->count[0] < 5) && (my->count[2] <= -4000))
        || ((my->count[0] >= 5) && (my->count[2] <= -8000)))
      return 0;
    return 1;
  }
  if ((my->count[0] == 6) && (my->count[18] != 0))
    return 1;

  return 0;
}

static tenm_object *
last_boss_cage_new(int n)
{
  tenm_object *new = NULL;
  int *count = NULL;

  /* sanity check */
  if ((n < 0) || (n > 2))
  {
    fprintf(stderr, "last_boss_cage_new: strange n (%d)\n", n);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 13);
  if (count == NULL)
  {
    fprintf(stderr, "last_boss_cage_new: malloc(count) failed\n");
    return NULL;
  }

  /* list of count
   * [0] color (for delete_enemy_shot)
   * [1] number of circles
   * [2] radius
   * [3 -- 12] center (x, y)
   */
  count[0] = 1;
  if (n == 1)
    count[1] = 2;
  else if (n == 2)
    count[1] = 3;
  else
    count[1] = 5;
  count[2] = 5;

  if (n == 1)
  { 
    count[3] = rand() % 11 - 5 + (WINDOW_WIDTH / 2);
    count[4] = rand() % (WINDOW_HEIGHT / 4);
    count[5] = rand() % 11 - 5 + (WINDOW_WIDTH / 2);
    count[6] = rand() % (WINDOW_HEIGHT / 4) + (WINDOW_HEIGHT / 2);
    count[7] = 0.0;
    count[8] = 0.0;
    count[9] = 0.0;
    count[10] = 0.0;
    count[11] = 0.0;
    count[12] = 0.0;
  }
  else if (n == 2)
  {
    count[3] = rand() % (WINDOW_WIDTH / 3);
    count[4] = rand() % (WINDOW_HEIGHT / 3);
    count[5] = rand() % (WINDOW_WIDTH / 3) + (WINDOW_WIDTH * 2 / 3);
    count[6] = rand() % (WINDOW_HEIGHT / 3);
    count[7] = rand() % 11 - 5 + (WINDOW_WIDTH / 2);
    count[8] = rand() % (WINDOW_HEIGHT / 3) + (WINDOW_HEIGHT * 2 / 3);
    count[9] = 0.0;
    count[10] = 0.0;
    count[11] = 0.0;
    count[12] = 0.0;
  }
  else
  {
    count[3] = rand() % (WINDOW_WIDTH / 3);
    count[4] = rand() % (WINDOW_HEIGHT / 3);
    count[5] = rand() % (WINDOW_WIDTH / 3) + (WINDOW_WIDTH * 2 / 3);
    count[6] = rand() % (WINDOW_HEIGHT / 3);
    count[7] = rand() % (WINDOW_WIDTH / 3);
    count[8] = rand() % (WINDOW_HEIGHT / 3) + (WINDOW_HEIGHT * 2 / 3);
    count[9] = rand() % (WINDOW_WIDTH / 3) + (WINDOW_WIDTH * 2 / 3);
    count[10] = rand() % (WINDOW_HEIGHT / 3) + (WINDOW_HEIGHT * 2 / 3);
    count[11] = rand() % (WINDOW_WIDTH / 3) + (WINDOW_WIDTH / 3);
    count[12] = rand() % (WINDOW_HEIGHT / 3) + (WINDOW_HEIGHT / 3);
  }

  /* ATTR_ENEMY_SHOT is only to clear it when the player is killed */
  new = tenm_object_new("L cage", ATTR_ENEMY_SHOT, 0,
                        0,
                        (double) (WINDOW_WIDTH / 2),
                        (double) (WINDOW_HEIGHT / 4),
                        13, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double)) NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&last_boss_cage_act),
                        (int (*)(tenm_object *, int)) (&last_boss_cage_draw));
  if (new == NULL)
  {
    fprintf(stderr, "last_boss_cage_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
last_boss_cage_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int j;
  int suffix_i;
  int suffix_j;
  int dx;
  int dy;
  int distance_2;
  double c;
  double x1;
  double y1;
  double x2;
  double y2;

  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  my->count[2] += 5;
  if (my->count[2] > 850)
    return 1;

  if (my->count[2] % 50 != 0)
    return 0;

  for (i = 0; i < my->count[1]; i++)
    for (j = i + 1; j < my->count[1]; j++)
    {
      suffix_i = 3 + 2 * i;
      suffix_j = 3 + 2 * j;
      dx = my->count[suffix_i + 0] - my->count[suffix_j + 0];
      dy = my->count[suffix_i + 1] - my->count[suffix_j + 1];
      distance_2 = dx * dx + dy * dy;
      if (my->count[2] * my->count[2] > distance_2 / 4 + 625)
      {
        /* abuse of distance_2 */
        c = tenm_sqrt(my->count[2] * my->count[2] - distance_2 / 4)
          / tenm_sqrt(distance_2);
        x1 = ((double) (my->count[suffix_i + 0]
                        + my->count[suffix_j + 0])) / 2.0;
        y1 = ((double) (my->count[suffix_i + 1]
                        + my->count[suffix_j + 1])) / 2.0;
        x2 = ((double) -dy) * c;
        y2 = ((double) dx) * c;
        tenm_table_add(laser_point_new(x1 + x2, y1 + y2, 8.0,
                                       x1 - x2, y1 - y2,
                                       25.0, 1));
        tenm_table_add(laser_point_new(x1 - x2, y1 - y2, 8.0,
                                       x1 + x2, y1 + y2,
                                       25.0, 1));
      }
    }

  return 0;
}

static int
last_boss_cage_draw(tenm_object *my, int priority)
{
  int i;
  int suffix;
  tenm_color color;
  int status = 0;

  /* sanity check */
  if (my == NULL)
    return 0;

  if (priority != 0)
    return 0;

  color = tenm_map_color(99, 158, 138);

  for (i = 0; i < my->count[1]; i++)
  {
    suffix = 3 + 2 * i;
    if (tenm_draw_circle(my->count[suffix + 0],
                         my->count[suffix + 1],
                         my->count[2],
                         1, color) != 0)
      status = 1;
  }

  return status;
}

static tenm_object *
last_boss_star_dust_new(int n)
{
  tenm_object *new = NULL;
  int *count = NULL;
  double x;
  double y;

  /* sanity check */
  if ((n < 0) || (n > 2))
  {
    fprintf(stderr, "last_boss_star_dust_new: strange n (%d)\n", n);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 2);
  if (count == NULL)
  {
    fprintf(stderr, "last_boss_star_dust_new: malloc(count) failed\n");
    return NULL;
  }

  x = (double) ((rand() % (WINDOW_WIDTH - 200)) + 100);
  if (n == 1)
    y = (double) ((rand() % ((WINDOW_HEIGHT - 200) / 2)) + 100);
  else
    y = (double) ((rand() % (WINDOW_HEIGHT - 200)) + 100);

  /* list of count
   * [0] color (for delete_enemy_shot)
   * [1] shoot timer
   */
  count[0] = 4;
  count[1] = 2;

  /* ATTR_ENEMY_SHOT is only to clear it when the player is killed */
  new = tenm_object_new("L star dust", ATTR_ENEMY_SHOT, 0,
                        0, x, y,
                        2, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double)) NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&last_boss_star_dust_act),
                        (int (*)(tenm_object *, int))
                        (&last_boss_star_dust_draw));
  if (new == NULL)
  {
    fprintf(stderr, "last_boss_star_dust_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
last_boss_star_dust_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int n;
  int theta;
  double from_x;
  double from_y;
  double temp[2];
  double result[2];
  double speed;

  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  if (my->count[1] > 100)
  {
    n = rand() % 2;
    theta = rand() % 360;
    if (rand() % 2 == 0)
      speed = 6.0;
    else
      speed = 4.5;
    from_x = my->x + ((double) (my->count[1])) * tenm_cos(theta);
    from_y = my->y + ((double) (my->count[1])) * tenm_sin(theta);
    temp[0] = player->x - from_x;
    temp[1] = player->y - from_y;
    if (temp[0] * (my->x - from_x) + temp[1] * (my->y - from_y) < -NEAR_ZERO)
    {
      theta += 180;
      from_x = my->x + ((double) (my->count[1])) * tenm_cos(theta);
      from_y = my->y + ((double) (my->count[1])) * tenm_sin(theta);
      temp[0] = player->x - from_x;
      temp[1] = player->y - from_y;
    }

    for (i = 0; i < 12; i++)
    {
      tenm_table_add(normal_shot_point_new(from_x, from_y, speed,
                                           from_x + temp[0],
                                           from_y + temp[1],
                                           4));
      if (speed < 5.0)
        speed = 6.0;
      else
        speed = 4.5;

      if (n == 0)
        theta += 30;
      else
        theta -= 30;
      from_x = my->x + ((double) (my->count[1])) * tenm_cos(theta);
      from_y = my->y + ((double) (my->count[1])) * tenm_sin(theta);

      result[0] = 1.0;
      result[1] = 0.0;
      vector_rotate(result, temp, 30);
      temp[0] = result[0];
      temp[1] = result[1];
   }

    return 1;
  }

  (my->count[1])++;

  return 0;
}

static int
last_boss_star_dust_draw(tenm_object *my, int priority)
{
  tenm_color color;
  int status = 0;

  /* sanity check */
  if (my == NULL)
    return 0;

  if (priority != 0)
    return 0;

  color = tenm_map_color(118, 99, 158);
  if (my->count[1] > 0)
    if (tenm_draw_circle((int) (my->x), (int) (my->y),
                         my->count[1], 1, color) != 0)
      status = 1;

  color = tenm_map_color(158, 158, 158);
  if (my->count[1] > 50)
    if (tenm_draw_circle((int) (my->x), (int) (my->y),
                         (my->count[1] - 50) * 2, 1, color) != 0)
      status = 1;

  return status;
}

static tenm_object *
last_boss_spotlight_new(int n)
{
  double x;
  double y;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;

  /* sanity check */
  if ((n < 0) || (n > 3))
  {
    fprintf(stderr, "last_boss_spotlight_new: strange n (%d)\n", n);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 7);
  if (count == NULL)
  {
    fprintf(stderr, "last_boss_spotlight_new: malloc(count) failed\n");
    return NULL;
  }

  count_d = (double *) malloc(sizeof(double) * 3);
  if (count_d == NULL)
  {
    fprintf(stderr, "last_boss_spotlight_new: malloc(count_d) failed\n");
    free(count);
    return NULL;
  }

  /* list of count
   * [0] color (for delete_enemy_shot)
   * [1] move timer
   * [2] shoot timer
   * [3] shoot theta
   * [4] shoot theta delta
   * [5] shoot direction
   * [6] mode
   */
  count[0] = 6;
  count[1] = -(rand() % 25);
  count[2] = 0;
  count[3] = rand() % 360;
  if (rand() % 2 == 0)
    count[4] = 7;
  else
    count[4] = -7;
  if (rand() % 2 == 0)
    count[5] = 90;
  else
    count[5] = -90;
  count[6] = 0;

  /* list of count_d
   * [0] dx
   * [1] dy
   * [2] radius
   */
  if (n == 0)
  {
    x = 0.0;
    y = 0.0;
    count_d[0] = 4.8;
    count_d[1] = 3.6;
  }
  else if (n == 1)
  {
    x = (double) WINDOW_WIDTH;
    y = 0.0;
    count_d[0] = -4.8;
    count_d[1] = 3.6;
  }
  else if (n == 2)
  {
    x = (double) WINDOW_WIDTH;
    y = (double) WINDOW_HEIGHT;
    count_d[0] = -4.8;
    count_d[1] = -3.6;
  }
  else
  {
    x = 0.0;
    y = (double) WINDOW_HEIGHT;
    count_d[0] = 4.8;
    count_d[1] = -3.6;
  }

  count_d[2] = 2.5;

  /* ATTR_ENEMY_SHOT is only to clear it when the player is killed */
  new = tenm_object_new("L spotlight", ATTR_ENEMY_SHOT, 0,
                        0, x, y,
                        7, count, 3, count_d, 0, NULL,
                        (int (*)(tenm_object *, double)) NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&last_boss_spotlight_act),
                        (int (*)(tenm_object *, int))
                        (&last_boss_spotlight_draw));
  if (new == NULL)
  {
    fprintf(stderr, "last_boss_spotlight_new: tenm_object_new failed\n");
    if (count_d != NULL)
      free(count_d);
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
last_boss_spotlight_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int theta;
  double dx;
  double dy;

  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  /* move */
  if (my->count[6] != 1)
  {
    if (my->count[1] >= 0)
    {
      /* no need to interpolate */
      my->x += my->count_d[0];
      my->y += my->count_d[1];
    }
    if (my->count[6] == 0)
      (my->count[1])++;
    else
      (my->count[1])--;
  }

  /* shoot */
  if (my->count[6] != 0)
  {
    (my->count[2])++;
    if ((my->count[2] >= 0) && (my->count[2] % 12 == 0))
    {
      for (i = 0; i < 5; i++)
      {
        theta = my->count[3] + i * 72;
        dx = my->count_d[2] * tenm_cos(theta);
        dy = my->count_d[2] * tenm_sin(theta);
        tenm_table_add(normal_shot_angle_new(my->x + dx, my->y + dy, 6.0,
                                             theta + my->count[5], 0));
      }
      my->count[3] += my->count[4];
    }
  }

  /* mode change */
  if ((my->count[6] == 0) && (my->count[1] >= 0))
  {
    my->count_d[2] += 2.5;
    if (6.0 * ((double) (my->count[1])) > 800.0 + my->count_d[2])
      return 1;

    dx = player->x - my->x;
    dy = player->y - my->y;
    if (dx * dx + dy * dy < (my->count_d[2] * my->count_d[2]) - NEAR_ZERO)
    {
      my->count[6] = 1;
      my->count[0] = 0;
      my->count[2] = -30;
      my->count_d[0] *= -1.0;
      my->count_d[1] *= -1.0;
    }    
  }
  else if (my->count[6] == 1)
  {
    /* abuse of shoot timer*/
    if (my->count[2] >= 72)
      my->count[6] = 2;
  }
  else if (my->count[6] == 2)
  {
    my->count_d[2] -= 2.5;
    if ((my->count[1] < 0) || (my->count_d[2] < NEAR_ZERO))
      return 1;
  }

  return 0;
}

static int
last_boss_spotlight_draw(tenm_object *my, int priority)
{
  tenm_color color;
  int status = 0;

  /* sanity check */
  if (my == NULL)
    return 0;

  if (priority != 0)
    return 0;

  if (my->count[1] < 0)
    return 0;

  if (my->count[6] == 0)
    color = tenm_map_color(158, 158, 158);
  else
    color = tenm_map_color(0, 191, 47);

  if (tenm_draw_circle((int) (my->x), (int) (my->y),
                       (int) (my->count_d[2]), 1, color) != 0)
    status = 1;

  return status;
}

static tenm_object *
last_boss_horizon_circle_new(int n)
{
  int i;
  int suffix;
  tenm_object *new = NULL;
  int *count = NULL;

  /* sanity check */
  if ((n < 0) || (n > 2))
  {
    fprintf(stderr, "last_boss_horizon_circle_new: strange n (%d)\n", n);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 25);
  if (count == NULL)
  {
    fprintf(stderr, "last_boss_horizon_circle_new: malloc(count) failed\n");
    return NULL;
  }

  /* list of count
   * [0] color (for delete_enemy_shot)
   * [1] number of circles
   * [] circle data
   *   suffix + 0: radius
   *   suffix + 1: center theta
   *   suffix + 2: dr
   *   suffix + 3: dtheta
   * [] circle pair data
   *   sights in/out at the last frame
   *     1: in the window
   *     0: out of the window
   */
  count[0] = 6;

  if (n == 1)
  {
    count[1] = 2;

    count[2] = 5;
    count[3] = 65;
    count[4] = 5;
    count[5] = 2;

    count[6] = 5;
    count[7] = -85;
    count[8] = 5;
    count[9] = -1;
  }
  else if (n == 2)
  {
    count[1] = 3;
    for (i = 0; i < 3; i++)
    {
      suffix = i * 4 + 2;
      count[suffix + 0] = rand() % 100 - 110;
      count[suffix + 1] = rand() % 360;
      count[suffix + 2] = 5;
      if (i == 0)
        count[suffix + 3] = -2;
      else if (i == 1)
        count[suffix + 3] = -1;
      else if (i == 2)
        count[suffix + 3] = 1;
      else
        count[suffix + 3] = 2;
    }
  }
  else
  {
    count[1] = 4;
    for (i = 0; i < 4; i++)
    {
      suffix = i * 4 + 2;
      count[suffix + 0] = rand() % 100 - 110;
      count[suffix + 1] = rand() % 360;
      count[suffix + 2] = 5;
      if (i == 0)
        count[suffix + 3] = -2;
      else if (i == 1)
        count[suffix + 3] = -1;
      else if (i == 2)
        count[suffix + 3] = 1;
      else
        count[suffix + 3] = 2;
    }
  }

  for (i = 18; i <= 24; i++)
    count[i] = 1;

  /* ATTR_ENEMY_SHOT is only to clear it when the player is killed */
  new = tenm_object_new("L horizon circle", ATTR_ENEMY_SHOT, 0,
                        0,
                        (double) (WINDOW_WIDTH / 2),
                        (double) (WINDOW_HEIGHT / 4),
                        25, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double)) NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&last_boss_horizon_circle_act),
                        (int (*)(tenm_object *, int))
                        (&last_boss_horizon_circle_draw));
  if (new == NULL)
  {
    fprintf(stderr, "last_boss_horizon_circle_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
last_boss_horizon_circle_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int j;
  int n;
  int suffix;
  int dtheta;
  int sight_suffix;
  int sight_in;
  double o_x;
  double o_y;
  double radius;
  double result[2];
  double v1[2];
  double v2[2];

  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  n = 0;
  for (i = 0; i < my->count[1]; i++)
  {
    suffix = i * 4 + 2;
    if ((my->count[suffix + 2] > 0) || (my->count[suffix + 0] >= 5))
    {
      my->count[suffix + 0] += my->count[suffix + 2];
      my->count[suffix + 1] += my->count[suffix + 3];
      if (my->count[suffix + 0] > 450)
        my->count[suffix + 2] *= -1;
      n++;
    }
  }

  if (n <= 0)
    return 1;

  o_x = (double) WINDOW_WIDTH / 2;
  o_y = (double) WINDOW_HEIGHT / 4;

  sight_suffix = my->count[1] * 4 + 2;
  for (i = 0; i < my->count[1]; i++)
    for (j =i + 1; j < my->count[1]; j++)
    {
      result[0] = 0.0;
      result[1] = 0.0;

      suffix = i * 4 + 2;
      if (my->count[suffix + 0] < 5)
        continue;
      radius = (double) (my->count[suffix + 0]);
      v1[0] = o_x + radius * tenm_cos(my->count[suffix + 1]);
      v1[1] = o_y + radius * tenm_sin(my->count[suffix + 1]);
      dtheta = my->count[suffix + 1];

      suffix = j * 4 + 2;
      if (my->count[suffix + 0] < 5)
        continue;
      radius = (double) (my->count[suffix + 0]);
      v2[0] = o_x + radius * tenm_cos(my->count[suffix + 1]);
      v2[1] = o_y + radius * tenm_sin(my->count[suffix + 1]);
      dtheta -= my->count[suffix + 1];

      if (last_boss_horizon_circle_sights(result, v1, v2) == 0)
      {
        if ((result[0] > NEAR_ZERO)
            && (result[0] < ((double) WINDOW_WIDTH) - NEAR_ZERO)
            && (result[1] > NEAR_ZERO)
            && (result[1] < ((double) WINDOW_HEIGHT) - NEAR_ZERO))
          sight_in = 1;
        else
          sight_in = 0;

        if ((my->count[sight_suffix] != sight_in)
            && ((dtheta < -2) || (dtheta > 2))
            && ((result[0] < 50.0)
                || (result[0] > ((double) WINDOW_WIDTH) - 50.0)
                || (result[1] < 50.0)
                || (result[1] > ((double) WINDOW_HEIGHT) - 50.0)))
        {
          if ((result[0] * 3.0 >= result[1] * 4.0)
              && ((((double) WINDOW_WIDTH) - result[0]) * 3.0
                  >= result[1] * 4.0))
          {
            tenm_table_add(last_boss_horizon_new(result[0], -1.0,
                                                 0));
          }
          else if ((result[0] * 3.0
                    >= (((double) WINDOW_HEIGHT) - result[1]) * 4.0)
                   && ((((double) WINDOW_WIDTH) - result[0]) * 3.0
                       >= (((double) WINDOW_HEIGHT) - result[1]) * 4.0))
          {
            tenm_table_add(last_boss_horizon_new(result[0],
                                                 ((double) WINDOW_HEIGHT)+ 1.0,
                                                 1));
          }
          else if ((result[0] * 3.0 < result[1] * 4.0)
                   && (result[0] * 3.0
                       < (((double) WINDOW_HEIGHT) - result[1]) * 4.0))
          {
            tenm_table_add(last_boss_horizon_new(-1.0, result[1],
                                                 2));
          }
          else if (((((double) WINDOW_WIDTH) - result[0]) * 3.0
                    < result[1] * 4.0)
                   && ((((double) WINDOW_WIDTH) - result[0]) * 3.0
                       < (((double) WINDOW_HEIGHT) - result[1]) * 4.0))
          {
            tenm_table_add(last_boss_horizon_new(((double) WINDOW_WIDTH) + 1.0,
                                                 result[1],
                                                 3));
          }
        }

        my->count[sight_suffix] = sight_in;
        sight_suffix++;
      }
    }

  return 0;
}

/* set result (arg 1) to the other intersection of two circles
 * which pass the center of L
 * v1 (arg 2) and v2 (arg 3) are the centers of the circles
 * all arguments must be double[2] (you must allocate enough memory
 * before calling this function)
 * return:
 * 0 on success
 * 1 if there is only one intersection point
 * 2 on error
 */
static int
last_boss_horizon_circle_sights(double *result,
                               const double *v1, const double *v2)
{
  double dot;
  double length;
  double c;
  double o_x = (double) (WINDOW_WIDTH / 2);
  double o_y = (double) (WINDOW_HEIGHT / 4);

  /* sanity check */
  if (result == NULL)
  {
    fprintf(stderr, "last_boss_horizon_circle_sights: result is NULL\n");
    return 2;
  }
  if (v1 == NULL)
  {
    fprintf(stderr, "last_boss_horizon_circle_sights: v1 is NULL\n");
    return 2;
  }
  if (v2 == NULL)
  {
    fprintf(stderr, "last_boss_horizon_circle_sights: v2 is NULL\n");
    return 2;
  }

  dot = (o_x - v1[0]) * (v2[0] - v1[0]) + (o_y - v1[1]) * (v2[1] - v1[1]);
  length = (v2[0] - v1[0]) * (v2[0] - v1[0]) + (v2[1]-v1[1]) * (v2[1]-v1[1]);
  if (length < NEAR_ZERO)
    return 1;
  c = dot / length;
  result[0] = 2.0 * (c * (v2[0] - v1[0]) - (o_x - v1[0])) + o_x;
  result[1] = 2.0 * (c * (v2[1] - v1[1]) - (o_y - v1[1])) + o_y;

  return 0;
}

static int
last_boss_horizon_circle_draw(tenm_object *my, int priority)
{
  int i;
  int suffix;
  int x;
  int y;
  double radius;
  tenm_color color;
  int status = 0;

  /* sanity check */
  if (my == NULL)
    return 0;

  if (priority != 0)
    return 0;

  color = tenm_map_color(158, 158, 158);

  for (i = 0; i < my->count[1]; i++)
  {
    suffix = i * 4 + 2;
    if (my->count[suffix + 0] >= 5)
    {
      radius = (double) (my->count[suffix + 0]);
      x = WINDOW_WIDTH / 2 + (int) (radius * tenm_cos(my->count[suffix + 1]));
      y = WINDOW_HEIGHT / 4 + (int) (radius * tenm_sin(my->count[suffix + 1]));
      if (tenm_draw_circle(x, y, (int) radius, 1, color) != 0)
        status = 1;
    }
  }

  return status;
}

static tenm_object *
last_boss_horizon_new(double x, double y, int n)
{
  tenm_object *new = NULL;
  int *count = NULL;

  /* sanity check */
  if ((n < 0) || (n > 3))
  {
    fprintf(stderr, "last_boss_horizon_new: strange n (%d)\n", n);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 3);
  if (count == NULL)
  {
    fprintf(stderr, "last_boss_horizon_new: malloc(count) failed\n");
    return NULL;
  }

  /* list of count
   * [0] color (for delete_enemy_shot)
   * [1] n
   * [2] shoot timer
   */
  count[0] = 5;
  count[1] = n;
  count[2] = 20;

  /* ATTR_ENEMY_SHOT is only to clear it when the player is killed */
  new = tenm_object_new("L horizon", ATTR_ENEMY_SHOT, 0,
                        0, x, y,
                        3, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double)) NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&last_boss_horizon_act),
                        (int (*)(tenm_object *, int))
                        (&last_boss_horizon_draw));
  if (new == NULL)
  {
    fprintf(stderr, "last_boss_horizon_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
last_boss_horizon_act(tenm_object *my, const tenm_object *player)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  (my->count[2])--;
  if (my->count[2] <= 0)
  {
    tenm_table_add(laser_point_new(my->x, my->y, 15.0,
                                   player->x, player->y,
                                   25.0, 1));

    if (my->count[1] == 0)
    {
      tenm_table_add(laser_new(my->x, my->y, 0.0, 0.0,
                               0.0, 800.0, 5, 7, 0));
    }
    else if (my->count[1] == 1)
    {
      tenm_table_add(laser_new(my->x, my->y, 0.0, 0.0,
                               0.0, -800.0, 5, 7, 0));
    }
    else if (my->count[1] == 2)
    {
      tenm_table_add(laser_new(my->x, my->y, 0.0, 0.0,
                               800.0, 0.0, 5, 7, 0));
    }
    else if (my->count[1] == 3)
    {
      tenm_table_add(laser_new(my->x, my->y, 0.0, 0.0,
                               -800.0, 0.0, 5, 7, 0));
    }
    else
    {
      fprintf(stderr, "last_boss_horizon_act: strange my->count[1] (%d)\n",
              my->count[1]);
    }
    return 1;
  }

  return 0;
}

static int
last_boss_horizon_draw(tenm_object *my, int priority)
{
  tenm_color color;
  int status = 0;

  /* sanity check */
  if (my == NULL)
    return 0;

  if (priority != 0)
    return 0;
  
  if (my->count[2] <= 0)
    return 1;

  color = tenm_map_color(175, 0, 239);
  if (tenm_draw_circle((int) (my->x), (int) (my->y),
                       my->count[2] * 6, 1, color) != 0)
    status = 1;

  /*
  color = tenm_map_color(142, 99, 158);
  if (my->count[1] == 0)
  {
    if (tenm_draw_line((int) (my->x), (int) (my->y),
                       (int) (my->x), (int) (my->y + 800.0),
                       1, color) != 0)
      status = 1;
  }
  else if (my->count[1] == 1)
  {
    if (tenm_draw_line((int) (my->x), (int) (my->y),
                       (int) (my->x), (int) (my->y - 800.0),
                       1, color) != 0)
      status = 1;
  }
  else if (my->count[1] == 2)
  {
    if (tenm_draw_line((int) (my->x), (int) (my->y),
                       (int) (my->x + 800.0), (int) (my->y),
                       1, color) != 0)
      status = 1;
  }
  else if (my->count[1] == 3)
  {
    if (tenm_draw_line((int) (my->x), (int) (my->y),
                       (int) (my->x - 800.0), (int) (my->y),
                       1, color) != 0)
      status = 1;
  }
  else
  {
    fprintf(stderr, "last_boss_horizon_draw: strange my->count[1] (%d)\n",
            my->count[1]);
  }
  */

  return status;
}

static tenm_object *
last_boss_twist_circle_new(int n)
{
  double x;
  double y;
  tenm_object *new = NULL;
  int *count = NULL;

  /* sanity check */
  if ((n < 0) || (n > 1))
  {
    fprintf(stderr, "last_boss_twist_circle_new: strange n (%d)\n", n);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 8);
  if (count == NULL)
  {
    fprintf(stderr, "last_boss_twist_circle_new: malloc(count) failed\n");
    return NULL;
  }

  if (n == 1)
  {    
    x = (double) ((WINDOW_WIDTH / 2) - 5 + (rand() % 11));
    y = (double) ((WINDOW_HEIGHT / 2) - 5 + (rand() % 11));
  }
  else
  { 
    x = (double) ((WINDOW_WIDTH / 3) + (rand() % (WINDOW_WIDTH / 3)));
    y = (double) ((WINDOW_HEIGHT / 3) + (rand() % (WINDOW_HEIGHT / 3)));
  }

  /* list of count
   * [0] color (for delete_enemy_shot)
   * [1] dtheta total
   * [2] appear timer
   * [3] radius of the small circle
   * [4] radius ratio
   * [5] theta of the small circle
   * [6] theta of the laser
   * [7] dtheta of the small circle
   */
  count[0] = 6;
  count[1] = 0;
  count[2] = 49;
  count[3] = 50;
  count[4] = 6;
  if (n == 1)
  {
    count[5] = -95 - rand() % 11;
    count[6] = 185 + rand() % 11;
    count[7] = -3;
  }
  else
  {    
    count[5] = rand() % 360;
    count[6] = rand() % 360;
    if (rand() % 2 == 0)
      count[7] = 3;
    else
      count[7] = -3;
  }

  /* ATTR_ENEMY_SHOT is only to clear it when the player is killed */
  new = tenm_object_new("L twist circle", ATTR_ENEMY_SHOT, 0,
                        0, x, y,
                        8, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double)) NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&last_boss_twist_circle_act),
                        (int (*)(tenm_object *, int))
                        (&last_boss_twist_circle_draw));
  if (new == NULL)
  {
    fprintf(stderr, "last_boss_twist_circle_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
last_boss_twist_circle_act(tenm_object *my, const tenm_object *player)
{
  double temp;
  double x;
  double y;

  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  if (my->count[2] > 0)
  {
    (my->count[2])--;
    return 0;
  }

  my->count[5] += my->count[7];
  my->count[6] += (-1) * my->count[7] * my->count[4];

  my->count[1] += my->count[7];
  if ((my->count[1] < -720) || (my->count[1] > 720))
    return 1;

  temp = (double) (my->count[3] * (my->count[4] - 1));
  x = my->x + temp * tenm_cos(my->count[5]);
  y = my->y + temp * tenm_sin(my->count[5]);

  if ((my->count[1] < -360) || (my->count[1] > 360))
  {
    my->count[0] = 0;
    if (my->count[1] % (my->count[7] * 20) != 0)
      return 0;
    tenm_table_add(laser_point_new(x, y, 3.0,
                                   player->x, player->y,
                                   (double) (my->count[3]), 0));
  }
  else
  {
    my->count[0] = 2;
    if (my->count[1] % (my->count[7] * 2) != 0)
      return 0;
    tenm_table_add(laser_angle_new(x, y, 3.0, my->count[6],
                                   (double) (my->count[3]), 2));
  }

  return 0;
}

static int
last_boss_twist_circle_draw(tenm_object *my, int priority)
{
  int x;
  int y;
  double temp;
  tenm_color color;
  int status = 0;

  /* sanity check */
  if (my == NULL)
    return 0;

  if (priority != 0)
    return 0;

  if (my->count[2] > 0)
  {
    color = tenm_map_color(158, 158, 158);
    if (tenm_draw_circle((int) (my->x), (int) (my->y),
                         (my->count[3] - my->count[2]) * my->count[4],
                         1, color) != 0)
      status = 1;

    if ((my->count[3] - my->count[2]) * my->count[4] > my->count[3])
    {
      temp = (double) ((my->count[3] - my->count[2]) * my->count[4]
                       - my->count[3]);
      x = (int) (my->x + temp * tenm_cos(my->count[5]));
      y = (int) (my->y + temp * tenm_sin(my->count[5]));
      if (tenm_draw_circle(x, y, my->count[3], 1, color) != 0)
        status = 1;
    }
  }
  else
  {
    if ((my->count[1] < -360) || (my->count[1] > 360))
      color = tenm_map_color(99, 158, 114);
    else
      color = tenm_map_color(99, 143, 158);

    if (tenm_draw_circle((int) (my->x), (int) (my->y),
                         my->count[3] * my->count[4], 1, color) != 0)
      status = 1;

    temp = (double) (my->count[3] * (my->count[4] - 1));
    x = (int) (my->x + temp * tenm_cos(my->count[5]));
    y = (int) (my->y + temp * tenm_sin(my->count[5]));
    if (tenm_draw_circle(x, y, my->count[3], 1, color) != 0)
      status = 1;
  }
  
  return status;
}

static tenm_object *
last_boss_matrix_new(int n)
{
  double x;
  tenm_object *new = NULL;
  int *count = NULL;

  /* sanity check */
  if ((n < 0) || (n > 1))
  {
    fprintf(stderr, "last_boss_matrix_new: strange n (%d)\n", n);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 3);
  if (count == NULL)
  {
    fprintf(stderr, "last_boss_matrix_new: malloc(count) failed\n");
    return NULL;
  }

  if (n == 0)
    x = 0.0;
  else
    x = (double) (WINDOW_WIDTH);

  /* list of count
   * [0] color (for delete_enemy_shot)
   * [1] n
   * [2] shoot timer
   */
  count[0] = 3;
  count[1] = n;
  count[2] = rand() % 15;

  /* ATTR_ENEMY_SHOT is only to clear it when the player is killed */
  new = tenm_object_new("L matrix", ATTR_ENEMY_SHOT, 0,
                        0, x, 0.0,
                        3, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double)) NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&last_boss_matrix_act),
                        (int (*)(tenm_object *, int)) NULL);
  if (new == NULL)
  {
    fprintf(stderr, "last_boss_matrix_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
last_boss_matrix_act(tenm_object *my, const tenm_object *player)
{
  double speed;
  double dx;

  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  /* no need to interpolate */
  if (my->count[1] == 0)
  {
    my->x += 4.5;
    if (my->x > (double) (WINDOW_WIDTH + WINDOW_HEIGHT))
      return 1;
  }
  else
  {  
    my->x -= 4.5;
    if (my->x < (double) -WINDOW_HEIGHT)
      return 1;
  }

  (my->count[2])++;
  if (my->count[2] % 3 == 0)
  {
    
    if (my->count[2] == 3)
      speed = 6.0;
    else if (my->count[2] == 6)
      speed = 9.0;
    else if (my->count[2] == 9)
      speed = 12.0;
    else if (my->count[2] == 12)
      speed = 15.0;
    else
      speed = 18.0;

    if (my->count[1] == 0)
      dx = -speed + 4.5;
    else
      dx = speed - 4.5;

    tenm_table_add(normal_shot_new(my->x, my->y, dx, speed,
                                   3, -2,
                                   1+((int)(((double)WINDOW_HEIGHT)/speed))));
  }

  if (my->count[2] >= 15)
    my->count[2] = 0;

  return 0;
}

static tenm_object *
last_boss_cross_circle_new(int n)
{
  tenm_object *new = NULL;
  int *count = NULL;

  /* sanity check */
  if ((n < 0) || (n > 1))
  {
    fprintf(stderr, "last_boss_cross_circle_new: strange n (%d)\n", n);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 4);
  if (count == NULL)
  {
    fprintf(stderr, "last_boss_cross_circle_new: malloc(count) failed\n");
    return NULL;
  }

  /* list of count
   * [0] color (for delete_enemy_shot)
   * [1] radius
   * [2] shoot timer
   * [3] n
   */
  count[0] = 6;
  if (n == 0)
    count[1] = 500;
  else
    count[1] = 2;
  count[2] = 0;
  count[3] = n;

  /* ATTR_ENEMY_SHOT is only to clear it when the player is killed */
  new = tenm_object_new("L cross circle", ATTR_ENEMY_SHOT, 0,
                        0,
                        (double) (WINDOW_WIDTH / 2),
                        (double) (WINDOW_HEIGHT / 4),
                        4, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double)) NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&last_boss_cross_circle_act),
                        (int (*)(tenm_object *, int))
                        (&last_boss_cross_circle_draw));
  if (new == NULL)
  {
    fprintf(stderr, "last_boss_cross_circle_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
last_boss_cross_circle_act(tenm_object *my, const tenm_object *player)
{
  double distance_2;

  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  if (my->count[3] == 0)
  {
    my->count[1] -= 2;
    if (my->count[1] <= 0)
      return 1;
  }
  else
  { 
    my->count[1] += 2;
    if (my->count[1] > 500)
      return 1;
  }

  distance_2 = (player->x - my->x) * (player->x - my->x);
  distance_2 += (player->y - my->y) * (player->y - my->y);

  if (my->count[2] > 0)
  {
    (my->count[2])--;
  }
  else
  {
    if ((distance_2 > (double) ((my->count[1] - 6) * (my->count[1] - 6)))
        && (distance_2 < (double) ((my->count[1] + 6) * (my->count[1] + 6))))
    {
      tenm_table_add(last_boss_cross_new(player->x, player->y, 0));
      my->count[2] = 5;
    }
  }

  return 0;
}

static int
last_boss_cross_circle_draw(tenm_object *my, int priority)
{
  tenm_color color;
  int status = 0;

  /* sanity check */
  if (my == NULL)
    return 0;

  if (priority != 0)
    return 0;
  
  color = tenm_map_color(158, 158, 158);

  if (my->count[1] <= 0)
    return 1;

  if (tenm_draw_circle((int) (my->x), (int) (my->y),
                       my->count[1], 1, color) != 0)
    status = 1;

  return status;
}

static tenm_object *
last_boss_cross_new(double x, double y, int n)
{
  tenm_object *new = NULL;
  int *count = NULL;

  /* sanity check */
  if ((n < 0) || (n > 1))
  {
    fprintf(stderr, "last_boss_cross_new: strange n (%d)\n", n);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 2);
  if (count == NULL)
  {
    fprintf(stderr, "last_boss_cross_new: malloc(count) failed\n");
    return NULL;
  }

  /* list of count
   * [0] color (for delete_enemy_shot)
   * [1] shoot timer
   */
  count[0] = 3;
  count[1] = 12;

  /* ATTR_ENEMY_SHOT is only to clear it when the player is killed */
  new = tenm_object_new("L cross", ATTR_ENEMY_SHOT, 0,
                        0, x, y,
                        2, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double)) NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&last_boss_cross_act),
                        (int (*)(tenm_object *, int)) (&last_boss_cross_draw));
  if (new == NULL)
  {
    fprintf(stderr, "last_boss_cross_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
last_boss_cross_act(tenm_object *my, const tenm_object *player)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  my->count[1] += 12;
  if (my->count[1] > 120)
  {
    tenm_table_add(laser_point_new(my->x - ((double) my->count[1]), my->y,
                                   9.0,
                                   my->x, my->y,
                                   25.0, 2));
    tenm_table_add(laser_point_new(my->x + ((double) my->count[1]), my->y,
                                   9.0,
                                   my->x, my->y,
                                   25.0, 2));
    tenm_table_add(laser_point_new(my->x, my->y - ((double) my->count[1]),
                                   9.0,
                                   my->x, my->y,
                                   25.0, 2));
    tenm_table_add(laser_point_new(my->x, my->y + ((double) my->count[1]),
                                   9.0,
                                   my->x, my->y,
                                   25.0, 2));
    return 1;
  }

  return 0;
}

static int
last_boss_cross_draw(tenm_object *my, int priority)
{
  tenm_color color;
  int status = 0;

  /* sanity check */
  if (my == NULL)
    return 0;

  if (priority != 0)
    return 0;
  
  color = tenm_map_color(99, 143, 158);

  if (my->count[1] <= 0)
    return 1;

  if (tenm_draw_line(((int) (my->x)) -  my->count[1], (int) (my->y),
                     ((int) (my->x)) +  my->count[1], (int) (my->y),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x), ((int) (my->y)) -  my->count[1],
                     (int) (my->x), ((int) (my->y)) +  my->count[1],
                     1, color) != 0)
    status = 1;

  return status;
}
