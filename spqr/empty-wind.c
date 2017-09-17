/* $Id: empty-wind.c,v 1.214 2011/08/23 19:46:22 oohara Exp $ */
/* [very hard] Empty Wind */

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

#include "empty-wind.h"

#define NEAR_ZERO 0.0001

static int empty_wind_move(tenm_object *my, double turn_per_frame);
static int empty_wind_hit(tenm_object *my, tenm_object *your);
static int empty_wind_green(const tenm_object *my);
static void empty_wind_explode(tenm_object *my);
static int empty_wind_act(tenm_object *my, const tenm_object *player);
static void empty_wind_set_move_mode(tenm_object *my,
                                     const tenm_object *player, int mode);
static int empty_wind_draw(tenm_object *my, int priority);

static tenm_object *empty_wind_spiral_new(double x);
static int empty_wind_spiral_act(tenm_object *my, const tenm_object *player);
static int empty_wind_spiral_draw(tenm_object *my, int priority);

tenm_object *
empty_wind_new(void)
{
  int i;
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = 0.0;
  double y = 0.0;

  x = (double) (WINDOW_WIDTH / 2);
  y = -24.0;

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "empty_wind_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_circle_new(x, y, 25.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "empty_wind_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 18);
  if (count == NULL)
  {
    fprintf(stderr, "empty_wind_new: malloc(count) failed\n");
    for (i = 0; i < 2; i++)
      (p[i])->delete(p[i]);
    free(p);
    return NULL;
  }

  count_d = (double *) malloc(sizeof(double) * 7);
  if (count_d == NULL)
  {
    fprintf(stderr, "empty_wind_new: malloc(count_d) failed\n");
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
   * [2] move mode
   * [3] move timer
   * [4 -- 5] move direction
   * [6] sub attack timer
   * [7] main attack timer
   * [8 -- 9] main attack direction
   * [10] life mode
   * [11] life timer
   * [12] decoration timer
   * [13] "was green when killed" flag
   */
  count[0] = 0;
  count[1] = 0;
  count[2] = 0;
  count[3] = 0;
  count[4] = 0;
  count[5] = 0;
  count[6] = 0;
  count[7] = 0;
  count[8] = 0;
  count[9] = 0;
  count[10] = 0;
  count[11] = 0;
  count[12] = 0;
  count[13] = 0;

  /* list of count_d
   * [0] speed x
   * [1] speed y
   * [2] move center x
   * [3] move center y
   * [4] decoration dy
   * [5] normal enemy dx
   * [6] normal enemy dy
   */
  count_d[0] = 0.0;
  count_d[1] = 0.0;
  count_d[2] = 0.0;
  count_d[3] = 0.0;
  count_d[4] = 0.0;
  count_d[5] = 0.0;
  count_d[6] = 0.0;


  new = tenm_object_new("Empty Wind", ATTR_BOSS, ATTR_PLAYER_SHOT,
                        500, x, y,
                        18, count, 7, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&empty_wind_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&empty_wind_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&empty_wind_act),
                        (int (*)(tenm_object *, int))
                        (&empty_wind_draw));
  if (new == NULL)
  {
    fprintf(stderr, "empty_wind_new: tenm_object_new failed\n");
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
empty_wind_move(tenm_object *my, double turn_per_frame)
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
empty_wind_hit(tenm_object *my, tenm_object *your)
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
  if (empty_wind_green(my))
    add_chain(my, your);
  my->count[1] = 2;

  if (my->hit_point <= 0)
  {
    set_background(1);
    add_score(100000);
    empty_wind_explode(my);
    return 0;
  }

  return 0;
}

/* return 1 (true) or 0 (false) */
static int
empty_wind_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if ((my->count[10] == 1) && (my->count[11] <= 8630))
    return 1;
  if (my->count[10] == 2)
  {
    if (my->count[13] == 1)
      return 1;
    else
      return 0;
  }

  return 0;
}

static void
empty_wind_explode(tenm_object *my)
{
  int n;

  /* sanity check */
  if (my == NULL)
    return;
  /* set "was green" flag before we change the life mode */
  if (empty_wind_green(my))
  {
    my->count[13] = 1;
    n = 8;
  }
  else
  {
    my->count[13] = 0;
    n = 7;
  }

  my->count[10] = 2;
  my->count[11] = 0;
  my->count[1] = 0;

  my->count_d[0] = 0.0;
  my->count_d[1] = 0.0;

  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy_shot), 0);
  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy), 0);

  tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                               1, 5000, n, 10.0, 6));
  tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                               2, 1000, n, 6.0, 6));

  /* don't modify my->attr or my->hit_mask here, or the player shot
   * may fly through the enemy */
  tenm_mass_delete(my->mass);
  my->mass = NULL;
}

static int
empty_wind_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int n;
  int theta;
  double dx;
  double dy;
  double length;
  int what;
  int time_shoot;

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

  /* decoration */
  my->count[12] += 5;
  if (my->count[12] >= 360)
    my->count[12] = 0;
  my->count_d[4] -= my->count_d[1] * 0.5;
  if (my->count_d[4] < NEAR_ZERO)
    my->count_d[4] = 0.0;
  else if (my->count_d[4] > 50.0)
    my->count_d[4] = 50.0;

  /* encounter */
  if (my->count[10] == 0)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 4.5;
    (my->count[11])++;
    if (my->count[11] > 40)
    {
      my->count[10] = 1;
      my->count[11] = 0;
      empty_wind_set_move_mode(my, player, 3);
      return 0;
    }
    return 0;
  }

  /* dead */
  if (my->count[10] == 2)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] += 0.3;
    (my->count[11])++;
    if (my->count[11] > 60)
    {
      if (empty_wind_green(my))
        n = 8;
      else
        n = 7;
      tenm_table_add(explosion_new(my->x, ((double) (WINDOW_HEIGHT)) + 25.0,
                                   0.0, -6.0,
                                   1, 5000, n, 10.0, 6));
      tenm_table_add(explosion_new(my->x, ((double) (WINDOW_HEIGHT)) + 25.0,
                                   0.0, -4.5,
                                   2, 1000, n, 6.0, 6));

      tenm_table_add(stage_clear_new(100));
      return 1;
    }
    return 0;
  }

  /* self-destruction */
  if ((my->count[10] == 1) && (my->count[11] > 8660))
  {
    set_background(2);
    clear_chain();
    empty_wind_explode(my);
    return 0;
  }

  /* jump */
  if (my->y > player->y + 24.0)
  {
    my->count_d[1] = -4.5;
  }
  else
  {
    my->count_d[1] += 0.3;
  }

  /* horizontal move */
  my->count_d[0] = 0.0;
  switch (my->count[2])
  {
  case 0:
    if (my->count[3] < 0)
    {
      (my->count[3])++;
    }
    else if (0 != 0)
    {
      empty_wind_set_move_mode(my, player, 3);
    }
    else if ((my->x > player->x - 120.0) && (my->x < player->x + 120.0))
    {
      if (rand() % 2 == 0)
      {
        empty_wind_set_move_mode(my, player, 1);
      }
      else
      {
        empty_wind_set_move_mode(my, player, 2);
      }
    }
    else if ((my->x < player->x - 300) || (my->x > player->x + 300.0)
             || ((my->x > 200.0) && (player->x < 100.0))
             || ((my->x < ((double) WINDOW_WIDTH) - 200.0)
                 && (player->x > ((double) WINDOW_WIDTH) - 100.0))
             )
    {
      empty_wind_set_move_mode(my, player, 1);
      /* chase the player */
      if (my->x > player->x)
        my->count[4] = -1;
      else
        my->count[4] = 1;
    }
    break;
  case 1:
    my->count_d[0] = 3.0 * ((double) (my->count[4]));
    (my->count[3])++;
    if (my->count[3] < 20)
    {
      my->count_d[0] = 0.3 * ((double) (my->count[4] * (my->count[3] - 10)));
      my->count_d[1] = 0.0;
    }
    else if (my->count[3] >= 80)
    {
      empty_wind_set_move_mode(my, player, 0);
      my->count[3] = -(rand() % 10);
    }
    break;
  case 2:
    theta = my->count[5] + my->count[3] * my->count[4];
    my->count_d[0] = 80.0 * tenm_cos(theta);
    my->count_d[1] = 80.0 * tenm_sin(theta);
    my->count_d[0] += my->count_d[2];
    my->count_d[1] += my->count_d[3];
    my->count_d[0] -= my->x;
    my->count_d[1] -= my->y;
    my->count[3] += 5;
    if (my->count[3] >= 180)
    {
      empty_wind_set_move_mode(my, player, 0);
      my->count[3] = -(rand() % 10);
    }
    break;
  case 3:
    if (my->count[3] < 0)
    {
      dx = my->count_d[2] - my->x;
      dy = my->count_d[3] - my->y;
      length = tenm_sqrt((int) (dx * dx + dy * dy));
      if (length < NEAR_ZERO)
        length = 1.0;
      if (length < 4.5)
      {
        my->count_d[0] = dx;
        my->count_d[1] = dy;
        my->count[3] = 0;
      }
      else
      {
        my->count_d[0] = 4.5 * dx / length;
        my->count_d[1] = 4.5 * dy / length;
      }
    }
    else
    {
      if ((my->count[3] < 25) || (my->count[3] >= 65))
      {  
        my->count[5] += 3 * my->count[4];
        length = 300.0;
        dy = -30;
      }
      else
      {  
        my->count[5] += 9 * my->count[4];
        length = 100.0;
        dy = 170;
      }
      my->count_d[0] = ((double) (WINDOW_WIDTH / 2))
        + length * tenm_cos(my->count[5]);
      my->count_d[1] = dy + length * tenm_sin(my->count[5]);
      my->count_d[0] -= my->x;
      my->count_d[1] -= my->y;
      (my->count[3])++;
      if (my->count[3] >= 90)
      {
        empty_wind_set_move_mode(my, player, 0);
        my->count[3] = -(rand() % 10);
      }
    }
    break;
  default:
    break;
  }

  /* horizontal move override */
  if ((my->count[11] == 1950) || (my->count[11] == 4000)
      || (my->count[11] == 6000))
    empty_wind_set_move_mode(my, player, 3);

  /* boundary check */
  if (my->x < NEAR_ZERO)
  {
    if (my->count_d[0] < 0.0)
      my->count_d[0] = 0.0;
  }
  if (my->x > ((double) WINDOW_WIDTH) - NEAR_ZERO)
  {
    if (my->count_d[0] > 0.0)
      my->count_d[0] = 0.0;
  }

  if (my->count[10] != 1)
    return 0;

  /* add normal enemy */
  (my->count[11])++;
  if ((my->count[11] > 272) && (my->count[11] <= 1904)
      && (my->count[11] % 136 >= 96) && (my->count[11] % 8 == 0))
  {
    if (my->count[11] % 136 == 96)
    { 
      if (player->x < (double) (WINDOW_WIDTH / 2))
        my->count_d[5] = -1.0;
      else
        my->count_d[5] = 1.0;
      if (player->y < (double) (WINDOW_HEIGHT / 2))
        my->count_d[6] = -1.0;
      else
        my->count_d[6] = 1.0;
    }
    dx = my->count_d[5];
    dy = my->count_d[6];
    if (my->count[11] % 136 == 96)
    {
      what = BALL_CAPTAIN;
      time_shoot = 40;
    }
    else
    {  
      what = BALL_SOLDIER;
      time_shoot = 1000;
    }

    tenm_table_add(normal_enemy_new(((double) (WINDOW_WIDTH / 2))
                                    - dx * (((double) (WINDOW_WIDTH/2)) +14.0),
                                    ((double) (WINDOW_HEIGHT / 2))
                                    - dy * (((double) (WINDOW_HEIGHT/2))+14.0),
                                    what, 0,
                                    0, -1, 0, -1, 0, 3, 3,
                                    /* move 0 */
                                    80, 5.6569 * dx, 5.6569 * dy, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    /* ./rotate.py 200 201.4480 41.4480
                                       121.4480 121.4480 0 -0.4 */
                                    108, 0.0, 0.0, 0.0, 0.0,
                                    320.0 + dx * 198.5520,
                                    240.0 + dy * 118.5520,
                                    0.0, -0.4 * dx * dy, 2,
                                    /* move 2 */
                                    1000, 0.0, -8.0 * dy, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 2,
                                    /* shoot 0 */
                                    80, time_shoot, 0, 0, 0, 1,
                                    /* shoot 1 */
                                    108, time_shoot, 0, 0, 1, 2,
                                    /* shoot 2 */
                                    1000, time_shoot, 8, 0, 0, 2));
  }
  if ((my->count[11] > 2312) && (my->count[11] <= 3944)
      && (my->count[11] % 136 >= 96) && (my->count[11] % 8 == 0))
  {
    if (my->count[11] % 136 == 96)
    { 
      if (player->x < (double) (WINDOW_WIDTH / 2))
        my->count_d[5] = -1.0;
      else
        my->count_d[5] = 1.0;
      if (player->y < (double) (WINDOW_HEIGHT / 2))
        my->count_d[6] = -1.0;
      else
        my->count_d[6] = 1.0;
    }
    dx = my->count_d[5];
    dy = my->count_d[6];
    if (my->count[11] % 136 == 96)
    {
      what = BALL_CAPTAIN;
      time_shoot = 40;
    }
    else
    {  
      what = BALL_SOLDIER;
      time_shoot = 1000;
    }

    tenm_table_add(normal_enemy_new(((double) (WINDOW_WIDTH / 2))
                                    - dx * (((double) (WINDOW_WIDTH/2)) +14.0),
                                    ((double) (WINDOW_HEIGHT / 2))
                                    - dy * (((double) (WINDOW_HEIGHT/2))+14.0),
                                    what, 0,
                                    0, -1, 0, -1, 0, 3, 4,
                                    /* move 0 */
                                    130, 3.0 * dx, 12.0 * dy, 0.0, -0.15 * dy,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    30, 7.5 * dx, 0.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 2,
                                    /* move 2 */
                                    1000, -3.0 * dx, -6.0 * dy, 0.0, 0.15 * dy,
                                    0.0, 0.0, 0.0, 0.0, 2,
                                    /* shoot 0 */
                                    40 + 136 - (my->count[11] % 136),
                                    time_shoot, 0, 0, 0, 1,
                                    /* shoot 1 */
                                    50, time_shoot, 0, 0, 1, 2,
                                    /* shoot 2 */
                                    70, time_shoot, 10, 0, 0, 3,
                                    /* shoot 3 */
                                    1000, time_shoot, 0, 0, 1, 3));
  }
  if ((my->count[11] > 4352) && (my->count[11] <= 5984)
      && (my->count[11] % 136 >= 96) && (my->count[11] % 8 == 0))
  {
    if (my->count[11] % 136 == 96)
    { 
      if (player->x < (double) (WINDOW_WIDTH / 2))
        my->count_d[5] = -1.0;
      else
        my->count_d[5] = 1.0;
      if (player->y < (double) (WINDOW_HEIGHT / 2))
        my->count_d[6] = -1.0;
      else
        my->count_d[6] = 1.0;
    }
    dx = my->count_d[5];
    dy = my->count_d[6];
    if (my->count[11] % 136 == 96)
    {
      what = BALL_CAPTAIN;
      time_shoot = 40;
    }
    else
    {  
      what = BALL_SOLDIER;
      time_shoot = 1000;
    }

    tenm_table_add(normal_enemy_new(((double) (WINDOW_WIDTH / 2))
                                    - dx * (((double) (WINDOW_WIDTH/2)) +19.0),
                                    ((double) (WINDOW_HEIGHT / 2))
                                    - dy * 157.0,
                                    what, 0,
                                    0, -1, 0, -1, 0, 4, 3,
                                    /* move 0 */
                                    62, 8.0 * dx, 0.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    /*
                                      ./rotate.py 200 163 83 163 240 0 -0.25
                                    */
                                    94, 0.0, 0.0, 0.0, 0.0,
                                    ((double) (WINDOW_WIDTH / 2))
                                    + 157.0 * dx,
                                    ((double) (WINDOW_HEIGHT / 2)),
                                    0.0, 0.25 * dx * dy, 2,
                                    /* move 2 */
                                    /*
                                      ./rotate.py 200 320 240 399 240 0 0.5
                                     */
                                    57, 0.0, 0.0, 0.0, 0.0,
                                    ((double) (WINDOW_WIDTH / 2))
                                    - 79.0 * dx,
                                    ((double) (WINDOW_HEIGHT / 2)),
                                    0.0, -0.5 * dx * dy, 3,
                                    /* move 3 */
                                    1000, 4.0 * dx, -6.9282 * dy, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 3,
                                    /* shoot 0 */
                                    156, time_shoot, 0, 0, 0, 1,
                                    /* shoot 1 */
                                    57, time_shoot, 36, 0, 1, 2,
                                    /* shoot 2 */
                                    1000, time_shoot, 13, 0, 0, 2));
  }
  if ((my->count[11] >= 6392) && (my->count[11] < 7832)
      && (my->count[11] % 8 == 0))
  {
    if (player->x < (double) (WINDOW_WIDTH / 2))
      dx = -1.0;
    else
      dx = 1.0;
    if (player->y < (double) (WINDOW_HEIGHT / 2))
      dy = -1.0;
    else
      dy = 1.0;

    tenm_table_add(normal_enemy_new(((double) (WINDOW_WIDTH / 2))
                                    - dx * (((double) (WINDOW_WIDTH/2))+19.0),
                                    ((double) (WINDOW_HEIGHT / 2))
                                    - dy * (((double) (WINDOW_HEIGHT/2))-20.0),
                                    BALL_SOLDIER, 0,
                                    0, -1, 0, -1, 0, 1, 1,
                                    /* move 0 */
                                    1000, 7.0 * dx, 0.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    1000, 1000, 0, 0, 1, 0));
  }

  /* sub attack */
  if ((my->count[11] > 200) && (my->count[11] <= 8630))
  {
    (my->count[6])++;
    if ((my->count[6] >= 45) && (my->count[6] % 5 == 0))
    {
      if (my->count[6] % 10 >= 5)
      {
        dx = 0.0;
        dy = 0.0;
        n = 4;
      }
      else
      {
        dx = player->count_d[0] * 15.0;
        dy = player->count_d[1] * 15.0;
        n = 5;
      }
      tenm_table_add(laser_point_new(my->x, my->y, 12.0,
                                     player->x + dx, player->y + dy,
                                     30.0, n));
    }
    if (my->count[6] >= 70)
    {
      if ((my->count[11] >= 2050) && (my->count[11] < 2370))
        my->count[6] = my->count[11] - 2370;
      else if ((my->count[11] >= 2370) && (my->count[11] < 4000))
        my->count[6] = -170;
      else if ((my->count[11] >= 4000) && (my->count[11] < 4450))
        my->count[6] = my->count[11] - 4450;
      else if ((my->count[11] >= 4450) && (my->count[11] < 5950))
        my->count[6] = -170;
      else if ((my->count[11] >= 5950) && (my->count[11] < 6250))
        my->count[6] = my->count[11] - 6250;
      else
        my->count[6] = 0;
    }
  }
  
  /* main attack */
  if ((my->count[11] > 2200) && (my->count[11] <= 8630))
  {
    (my->count[7])++;
    if (my->count[11] >= 4300)
    {
      if (my->count[7] == 36)
      {
        tenm_table_add(empty_wind_spiral_new(player->x));
      }
    }
    if ((my->count[11] < 4300) || (my->count[11] >= 6250))
    {  
      if (my->count[7] == 36)
      {
        if (rand() % 2 == 0)
          my->count[8] = 1;
        else
          my->count[8] = -1;
        my->count[9] = rand() % 360;
      }
      if ((my->count[7] >= 90) && (my->count[7] % 10 == 0))
      {
        theta = my->count[9] + 10 * my->count[8] * (my->count[7] - 90);
        length = 50.0 + ((double) (my->count[7] - 90));
        for (i = 0; i < 360; i += 30)
        {
          dx = length * tenm_cos(theta + i);
          dy = 0.5 * length * tenm_sin(theta + i);
          dy += 50.0;
          dy -= (double) ((my->count[7] - 90) * 5);
          tenm_table_add(laser_angle_new(my->x + dx, my->y + dy, 9.0,
                                         theta + i + 150 * my->count[8],
                                         25.0, 0));
        }
      }
    }

    if (my->count[7] >= 170)
    {
      if (my->count[11] < 4000)
        my->count[7] = -70;
      else if ((my->count[11] >= 4000) && (my->count[11] < 4300))
        my->count[7] = my->count[11] - 4300;
      else if ((my->count[11] >= 4300) && (my->count[11] < 5950))
        my->count[7] = -70;
      else if ((my->count[11] >= 5950) && (my->count[11] < 6250))
        my->count[7] = my->count[11] - 6250;
      else
        my->count[7] = 0;
    }
    
  }
  
  return 0;  
}

static void
empty_wind_set_move_mode(tenm_object *my, const tenm_object *player, int mode)
{
  int theta;

  /* sanity check */
  if (my == NULL)
    return;
  if (player == NULL)
    return;
  if ((mode < 0) || (mode > 3))
  {
    fprintf(stderr, "empty_wind_set_move_mode: strange mode (%d)\n", mode);
    return;
  }

  my->count[2] = mode;
  switch (mode)
  {
  case 0:
    my->count[3] = 0;
    my->count_d[1] = 0.0;
    break;
  case 1:
    my->count[3] = 0;
    if (my->x > (double) (WINDOW_WIDTH / 2))
      my->count[4] = -1;
    else
      my->count[4] = 1;
    break;
  case 2:
    my->count[3] = 0;
    if (my->x < 200.0)
      my->count[4] = -1;
    else if (my->x > ((double) WINDOW_WIDTH) - 200.0)
      my->count[4] = 1;
    else if (my->x > player->x)
      my->count[4] = 1;
    else
      my->count[4] = -1;
    if (my->count[4] > 0)
      my->count[5] = 0;
    else
      my->count[5] = 180;
    my->count_d[2] = my->x - 80.0 * ((double) (my->count[4]));
    my->count_d[3] = my->y;
    /* default: lower half circle
     * upper half circle if one of the following happens:
     * 1) we are near the lower side of the window
     * 2) we are far from the upper side of the window
     *    and the player is below us
     */
    if ((my->y > ((double) WINDOW_HEIGHT) - 100.0)
        || ((my->y > 100.0) && (my->y < player->y)))
    {
      my->count[4] *= -1;
    }
    break;
  case 3:
    my->count[3] = -1;
    /*
    if (my->x < (double) (WINDOW_WIDTH / 2))
    */
    if (my->x < player->x)
    {  
      theta = 165;
      my->count[4] = -1;
    }
    else
    {  
      theta = 15;
      my->count[4] = 1;
    }
    my->count[5] = theta;
    my->count_d[2] = ((double) (WINDOW_WIDTH / 2)) + 300.0 * tenm_cos(theta);
    my->count_d[3] = -30.0 + 300.0 * tenm_sin(theta);
    break;
  default:
    fprintf(stderr, "empty_wind_set_move_mode: undefined mode (%d)\n", mode);
    break;
  }
    
}

static int
empty_wind_draw(tenm_object *my, int priority)
{
  tenm_color color;
  char temp[32];
  int status = 0;
  int i;
  int theta;
  double length;
  double dx;
  double dy;

  /* sanity check */
  if (my == NULL)
    return 0;

  /* decoration */
  if ((priority == -1) && (my->count[10] != 2))
  {
    /* vortex */
    if (empty_wind_green(my))
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

    for (i = 0; i < 4; i++)
    {
      theta = my->count[12] + i * 90;
      if (i % 2 == 1)
        theta = 210 - theta;
      dy = my->count_d[4];
      length = 80.0 - dy * 0.75;
      if (i % 2 == 1)
        dy += 15.0;
      if (tenm_draw_line((int) (my->x + length * tenm_cos(theta)),
                         (int) (my->y + length * 0.5 * tenm_sin(theta) + dy),
                         (int) (my->x + length * tenm_cos(theta- 30)),
                         (int) (my->y + length * 0.5*tenm_sin(theta-30) + dy),
                         1, color) != 0)
        status = 1;
    }

    /* main attack */
    if ((my->count[7] >= 36)
        && ((my->count[11] < 4300) || (my->count[11] >= 6250)))
    {
      color = tenm_map_color(158, 158, 158);
      for (i = 0; i < 360; i += 180)
      {
        if (my->count[7] < 90)
          theta = 10 * my->count[8] * (my->count[7] - 36) + i;
        else
          theta = i;
        if (tenm_draw_line((int) (my->x + 50.0 * tenm_cos(theta)),
                           (int) (my->y + 25.0 * tenm_sin(theta) + 50.0),
                           (int) (my->x + 130.0 * tenm_cos(theta)),
                           (int) (my->y + 65.0 * tenm_sin(theta) - 350.0),
                           1, color) != 0)
          status = 1;
      }
      if (my->count[7] >= 90)
      {
        color = tenm_map_color(99, 158, 114);
        for (i = 0; i < 2; i++)
        {
          theta = my->count[9] + 10 * my->count[8] * (my->count[7] - 90)
            + 90 * i;
          length = 50.0 + ((double) (my->count[7] - 90));
          dx = length * tenm_cos(theta);
          dy = 0.5 * length * tenm_sin(theta);
          if (tenm_draw_line((int) (my->x + dx),
                             (int) (my->y + 50.0
                                    - (double) ((my->count[7] - 90) * 5) + dy),
                             (int) (my->x - dx),
                             (int) (my->y + 50.0
                                    - (double) ((my->count[7] - 90) * 5) - dy),
                             1, color) != 0)
            status = 1;
        }
      }
    }
  }

  if (((my->count[10] != 2) && (priority == 0))
      || (priority == -1))
  {  
    /* body */
    if (empty_wind_green(my))
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
    
    if (tenm_draw_circle((int) (my->x), (int) (my->y),
                         25, 3, color) != 0)
      status = 1;
  }
  
  /* hit point stat */
  if (my->count[10] == 1)
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) (my->x)) - 5, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "empty_wind_draw: draw_string failed\n");
      status = 1;
    }
  }
  
  return status;
}

static tenm_object *
empty_wind_spiral_new(double x)
{
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;

  count = (int *) malloc(sizeof(int) * 18);
  if (count == NULL)
  {
    fprintf(stderr, "empty_wind_spiral_new: malloc(count) failed\n");
    return NULL;
  }

  count_d = (double *) malloc(sizeof(double) * 6);
  if (count_d == NULL)
  {
    fprintf(stderr, "empty_wind_spiral_new: malloc(count_d) failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  /* list of count
   * [0] color (for delete_enemy_shot)
   * [1] shoot timer
   * [2] theta base
   * [3] theta direction
   */
  count[0] = 1;
  count[1] = 0;
  count[2] = rand() % 360;
  if (rand() % 2 == 0)
    count[3] = 1;
  else
    count[3] = -1;
  count[4] = 0;
  count[5] = 0;
  count[6] = 0;
  count[7] = 0;
  count[8] = 0;
  count[9] = 0;

  /* list of count_d
   */
  count_d[0] = 0.0;
  count_d[1] = 0.0;
  count_d[2] = 0.0;
  count_d[3] = 0.0;

  /* ATTR_ENEMY_SHOT is only to clear it when the player is killed */
  new = tenm_object_new("Empty Wind spiral", ATTR_ENEMY_SHOT, 0,
                        0, x, (double) (WINDOW_HEIGHT / 2),
                        18, count, 6, count_d, 0, NULL,
                        (int (*)(tenm_object *, double)) NULL,
                        (int (*)(tenm_object *, tenm_object *)) NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&empty_wind_spiral_act),
                        (int (*)(tenm_object *, int))
                        (&empty_wind_spiral_draw));
  if (new == NULL)
  {
    fprintf(stderr, "empty_wind_spiral_new: tenm_object_new failed\n");
    free(count_d);
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
empty_wind_spiral_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int theta;
  double x;
  double y;
  
  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  (my->count[1])++;
  if (my->count[1] > 72)
  {
    for (i = 0; i < 72; i++)
    {
      theta = my->count[2] + my->count[3] * 30 * i;
      x = my->x + 50.0 * tenm_cos(theta);
      y = ((double) WINDOW_HEIGHT) + 10.0 - (double) (i * 9)
        + 50.0 * tenm_sin(theta);

      tenm_table_add(laser_angle_new(x, y, 9.0, theta + my->count[3] * 150,
                                     25.0, 1));
    }
    return 1;
  }
  
  return 0;
}

static int
empty_wind_spiral_draw(tenm_object *my, int priority)
{
  int i;
  int theta;
  tenm_color color;
  int status = 0;
  double x;
  double y;
  double dx;
  double dy;

  /* sanity check */
  if (my == NULL)
    return 0;
  if (my->count[1] < 0)
    return 0;

  if (priority != 0)
    return 0;

  color = tenm_map_color(99, 158, 138);
  for (i = 0; i < my->count[1]; i++)
  {
    theta = my->count[2] + my->count[3] * 30 * i;
    x = my->x + 50.0 * tenm_cos(theta);
    y = ((double) WINDOW_HEIGHT) + 10.0 - (double) (i * 9)
      + 50.0 * tenm_sin(theta);
    dx = 25.0 * tenm_cos(theta + my->count[3] * 150);
    dy = 25.0 * tenm_sin(theta + my->count[3] * 150);
    if (tenm_draw_line((int) (x),
                       (int) (y),
                       (int) (x + dx),
                       (int) (y + dy),
                       1, color) != 0)
      status = 1;
  }

  theta = my->count[2] + my->count[3] * 30 * my->count[1];
  x = my->x + 50.0 * tenm_cos(theta);
  y = ((double) WINDOW_HEIGHT) + 10.0 - (double) (my->count[1] * 9)
    + 50.0 * tenm_sin(theta);
  if (tenm_draw_circle((int) (x), (int) (y),
                       5, 1, color) != 0)
    status = 1;


  return status;
}
