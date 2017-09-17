/* $Id: net-can-howl.c,v 1.431 2004/10/13 23:56:06 oohara Exp $ */
/* [very hard] Senators */

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
#include "stage-clear.h"

#include "net-can-howl.h"

#define NEAR_ZERO 0.0001

static int net_can_howl_core_move(tenm_object *my, double turn_per_frame);
static int net_can_howl_core_hit(tenm_object *my, tenm_object *your);
static void net_can_howl_core_second_form(tenm_object *my);
static void net_can_howl_core_explode(tenm_object *my);
static int net_can_howl_core_act(tenm_object *my, const tenm_object *player);
static int net_can_howl_core_cannon(tenm_object *my,
                                    const tenm_object *player);
static int net_can_howl_core_draw(tenm_object *my, int priority);
static int net_can_howl_core_draw_wraith(double x, double y, double length,
                                         int theta, int theta_z, int green);
static int net_can_howl_core_green(const tenm_object *my);

static tenm_object *net_can_howl_black_hole_new(double x, double y);
static int net_can_howl_black_hole_act(tenm_object *my,
                                       const tenm_object *player);
static int net_can_howl_black_hole_draw(tenm_object *my, int priority);

static tenm_object *net_can_howl_seeker_new(int t, int n);
static int net_can_howl_seeker_act(tenm_object *my,
                                   const tenm_object *player);
static int net_can_howl_seeker_draw(tenm_object *my, int priority);

static tenm_object *senator_new(int table_index, int what);
static int senator_move(tenm_object *my, double turn_per_frame);
static int senator_hit(tenm_object *my, tenm_object *your);
static void senator_explode(tenm_object *my);
static int senator_act(tenm_object *my, const tenm_object *player);
static int senator_draw(tenm_object *my, int priority);
static int senator_green(const tenm_object *my);

static int core_signal_second_form(tenm_object *my, int n);
static int core_signal_kill_senator(tenm_object *my, int n);
static int senator_signal(tenm_object *my, int n);

static int net_can_howl_position(double *result, int what, int t_0, int t_1);

static tenm_object *voter_new(int what, int t);
static int voter_move(tenm_object *my, double turn_per_frame);
static int voter_hit(tenm_object *my, tenm_object *your);
static int voter_act(tenm_object *my, const tenm_object *player);
static int voter_draw(tenm_object *my, int priority);
static int voter_green(const tenm_object *my);
static double voter_parabola_formation(double a_1, double b_1,
                                       double a_2, double b_2,
                                       double s, double x);

static tenm_object *net_can_howl_more_1_new(void);
static int net_can_howl_more_1_act(tenm_object *my, const tenm_object *player);

tenm_object *
net_can_howl_core_new(void)
{
  int i;
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = 0.0;
  double y = 0.0;
  double hand_x = 0.0;
  double hand_y = 0.0;

  x = (double) (WINDOW_WIDTH / 2);
  y = (double) (WINDOW_HEIGHT / 2);
  hand_x = 30.0;
  hand_y = 0.0;

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "net_can_howl_core_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + hand_x - hand_y * 0.5,
                                             y + hand_y + hand_x * 0.5,
                                             x - hand_x - hand_y * 0.5,
                                             y - hand_y + hand_x * 0.5,
                                             x - hand_x + hand_y * 0.5,
                                             y - hand_y - hand_x * 0.5,
                                             x + hand_x + hand_y * 0.5,
                                             y + hand_y - hand_x * 0.5);
  if (p[0] == NULL)
  {
    fprintf(stderr, "net_can_howl_core_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 18);
  if (count == NULL)
  {
    fprintf(stderr, "net_can_howl_core_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  count_d = (double *) malloc(sizeof(double) * 6);
  if (count_d == NULL)
  {
    fprintf(stderr, "net_can_howl_core_new: malloc(count_d) failed\n");
    if (count != NULL)
      free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] for deal_damage
   * [1] "damaged" timer
   * [2] move timer
   * [3] main attack mode
   * [4] main attack timer
   * [5] sub attack mode
   * [6] sub attack timer
   * [7] nway attack timer
   * [8] life mode
   * [9] number of senators dead
   * [10 -- 16] senators index
   * [17] "was green when killed" flag
   */
  count[0] = 0;
  count[1] = 0;
  count[2] = -1;
  count[3] = 1;
  count[4] = 0;
  count[5] = 0;
  count[6] = 0;
  count[7] = 0;
  count[8] = 0;
  count[9] = 0;
  for (i = 10; i <= 16; i++)
    count[i] = -1;
  count[17] = 0;

  /* list of count_d
   * [0] speed x
   * [1] speed y
   * [2 -- 5] sub attack manager
   */
  count_d[0] = (((double) (WINDOW_WIDTH / 2)) - x) / 360.0;
  count_d[1] = (((double) (WINDOW_HEIGHT / 4)) - y) / 360.0;
  for (i = 2; i <= 5; i++)
    count_d[i] = 0.0;

  new = tenm_object_new("net can howl core", 0, 0,
                        100, x, y,
                        18, count, 6, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&net_can_howl_core_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&net_can_howl_core_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&net_can_howl_core_act),
                        (int (*)(tenm_object *, int))
                        (&net_can_howl_core_draw));
  if (new == NULL)
  {
    fprintf(stderr, "new_can_howl_core_new: tenm_object_new failed\n");
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
net_can_howl_core_move(tenm_object *my, double turn_per_frame)
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
net_can_howl_core_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (your == NULL)
    return 0;

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;
  if ((my->count[8] != 1) && (my->count[8] != 3))
    return 0;

  deal_damage(my, your, 0);
  if (net_can_howl_core_green(my))
    add_chain(my, your);
  my->count[1] = 2;

  if (my->hit_point <= 0)
  {
    set_background(1);
    if (my->count[8] == 1)
    {
      add_score(20000);
      net_can_howl_core_second_form(my);
      return 0;
    }

    add_score(80000);
    net_can_howl_core_explode(my);

    return 0;
  }

  return 0;
}

static void
net_can_howl_core_second_form(tenm_object *my)
{
  int i;
  int n;

  /* sanity check */
  if (my == NULL)
    return;

  if (my->count[8] != 1)
    return;

  my->count_d[0] = 0.0;
  my->count_d[1] = 0.0;

  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy_shot), 0);
  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy), 0);

  if (net_can_howl_core_green(my))
  {
    my->count[17] = 1;
    n = 8;
  }
  else
  {  
    my->count[17] = 0;
    n = 7;
  }

  tenm_table_add(fragment_new(my->x, my->y, 0.0, 0.0,
                              50.0, 20, n, 10.0, 0.0, 20));
  tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                               1, 1000, n, 10.0, 6));

  my->hit_point = 2012;
  my->count[8] = 2;
  my->count[2] = -1;
  my->count[1] = 0;

  for (i = 0; i < 7; i++)
    if (my->count[10 + i] >= 0)
      tenm_table_apply(my->count[10 + i],
                       (int (*)(tenm_object *, int))
                       (&core_signal_second_form),
                       0);
}

static void
net_can_howl_core_explode(tenm_object *my)
{
  int i;
  int n;

  /* sanity check */
  if (my == NULL)
    return;

  if (my->count[8] != 3)
    return;

  my->count_d[0] = 0.0;
  my->count_d[1] = 0.0;

  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy_shot), 0);
  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy), 0);

  if (net_can_howl_core_green(my))
  {
    my->count[17] = 1;
    n = 8;
  }
  else
  {  
    my->count[17] = 0;
    n = 7;
  }

  tenm_table_add(fragment_new(my->x, my->y, 0.0, 0.0,
                              20.0, 200, n, 6.0, 0.0, 20));
  tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                               1, 5000, n, 10.0, 6));

  for (i = 0; i < 7; i++)
    if (my->count[10 + i] >= 0)
      tenm_table_apply(my->count[10 + i],
                       (int (*)(tenm_object *, int))
                       core_signal_kill_senator,
                       0);
  my->count[8] = 4;
  my->count[2] = -1;
  my->count[1] = 0;

  /* don't modify my->attr or my->hit_mask here, or the player shot
   * may fly through the enemy */
  tenm_mass_delete(my->mass);
  my->mass = NULL;
}

static int
net_can_howl_core_act(tenm_object *my, const tenm_object *player)
{
  int i;
  double x;
  double y;
  double hand_x;
  double hand_y;
  double v[2];
  double result[2];
  int theta;

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

  /* move/add-enemy timer */
  (my->count[2])++;

  /* encounter*/
  if (my->count[8] == 0)
  {
    if (my->count[2] == 360)
    {
      my->attr = ATTR_BOSS | ATTR_OPAQUE;
      my->hit_mask = ATTR_PLAYER_SHOT;
    }

    if ((my->count[2] >= 30) && (my->count[2] <= 210)
        && (my->count[2] % 30 == 0))
    {
      i = (my->count[2] / 30) - 1;
      my->count[10 + i] = tenm_table_add(senator_new(my->table_index,
                                                     i + 1));
      if (my->count[10 + i] < 0)
      {
        fprintf(stderr, "net_can_howl_core_act: cannnot create senator "
                "(%d)\n", i);
      }
    }

    if (my->count[2] >= 370)
    {
      (my->count[4])++;
      net_can_howl_core_cannon(my, player);
      if ((my->count[2] >= 530) && (my->count[4] > 160))
      {
        my->count[3] = 5;
        my->count[4] = 0;

        my->count[8] = 1;
        my->count[2] = -1;
      }
    }
  }

  /* dead */
  if (my->count[8] == 4)
  {
    if (my->count[17] != 0)
      i = 8;
    else
      i = 7;
    if ((my->count[2] >= 30) && (my->count[2] <= 75)
        && (my->count[2] % 15 == 0))
    {
      theta = rand() % 360;
      x = my->x + ((double) (my->count[2] * 3)) * tenm_cos(theta);
      y = my->y + ((double) (my->count[2] * 3)) * tenm_sin(theta);
      tenm_table_add(fragment_new(x, y, 0.0, 0.0,
                                  30.0, 50, i, 10.0, 0.0, 20));
    }
    if (my->count[2] >= 120)
    {
      tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                                   1, 5000, i, 10.0, 9));
      tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                                   2, 1000, i, 7.5, 12));

      tenm_table_add(stage_clear_new(100));
      return 1;
    }
  }
  
  /* self-destruction */
  if ((my->count[8] == 1) && (my->count[2] >= 2760))
  {
    set_background(2);
    clear_chain();
    net_can_howl_core_second_form(my);
    return 0;
  }
  if ((my->count[8] == 3) && (my->count[2] >= 8030))
  {
    set_background(2);
    clear_chain();
    net_can_howl_core_explode(my);
    return 0;
  }

  /* speed change */
  if (my->count[8] == 0)
  {
    if (my->count[2] == 360)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
  }
  else if (my->count[8] == 1)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
  }
  else if (my->count[8] == 2)
  {
    if (my->count[2] == 30)
    {
      my->attr = 0;
      my->hit_mask = 0;
      my->count[17] = 0;

      result[0] = 0.0;
      result[1] = 0.0;
      net_can_howl_position(result, 0, 0, 0);
      my->count_d[0] = (result[0] - my->x) / 120.0;
      my->count_d[1] = (result[1] - my->y) / 120.0;
    }
    if (my->count[2] == 150)
    {
      my->attr = ATTR_BOSS | ATTR_OPAQUE;
      my->hit_mask = ATTR_PLAYER_SHOT;

      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
  }
  else if (my->count[8] == 3)
  {
    result[0] = 0.0;
    result[1] = 0.0;
    net_can_howl_position(result, 0, 0, my->count[2] + 1);
    my->count_d[0] = result[0] - my->x;
    my->count_d[1] = result[1] - my->y;
  }
  else if (my->count[8] == 4)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
  }

  /* shape change */
  if (my->count[8] == 3)
  {
    x = result[0];
    y = result[1];

    result[0] = 0.0;
    result[1] = 0.0;
    net_can_howl_position(result, 8, 0, my->count[2] + 1);
    hand_x = result[0] - x;
    hand_y = result[1] - y;
  }
  else
  {
    x = my->x;
    y = my->y;
    hand_x = 30.0;
    hand_y = 0.0;
  }

  if (my->mass != NULL)
  {
    (((tenm_polygon *)(my->mass->p[0]))->v[0])->x = x + hand_x - hand_y * 0.5;
    (((tenm_polygon *)(my->mass->p[0]))->v[0])->y = y + hand_y + hand_x * 0.5;
    (((tenm_polygon *)(my->mass->p[0]))->v[1])->x = x - hand_x - hand_y * 0.5;
    (((tenm_polygon *)(my->mass->p[0]))->v[1])->y = y - hand_y + hand_x * 0.5;
    (((tenm_polygon *)(my->mass->p[0]))->v[2])->x = x - hand_x + hand_y * 0.5;
    (((tenm_polygon *)(my->mass->p[0]))->v[2])->y = y - hand_y - hand_x * 0.5;
    (((tenm_polygon *)(my->mass->p[0]))->v[3])->x = x + hand_x + hand_y * 0.5;
    (((tenm_polygon *)(my->mass->p[0]))->v[3])->y = y + hand_y - hand_x * 0.5;
  }

  if ((my->count[8] == 2) && (my->count[2] >= 180))
  {
    my->count[8] = 3;
    my->count[2] = -1;
    my->count[17] = 0;
    return 0;
  }

  /* add normal enemy */
  if ((my->count[8] == 1) && (my->count[2] >= 200) && (my->count[2] < 2000))
  {
    if ((my->count[2] % 200 == 0) || (my->count[2] % 200 == 10))
    {
      tenm_table_add(voter_new(10, my->count[2] % 200));
      tenm_table_add(voter_new(11, my->count[2] % 200));
    }
    if ((my->count[2] % 200 == 80) || (my->count[2] % 200 == 90))
    {
      tenm_table_add(voter_new(9, my->count[2] % 200 - 80));
    }

    if ((my->count[2] % 200 == 100) || (my->count[2] % 200 == 110))
    {
      tenm_table_add(voter_new(13, my->count[2] % 200 - 100));
      tenm_table_add(voter_new(14, my->count[2] % 200 - 100));
    }
    if ((my->count[2] % 200 == 180) || (my->count[2] % 200 == 190))
    {
      tenm_table_add(voter_new(12, my->count[2] % 200 - 180));
    }
  }

  if ((my->count[8] == 0) && (my->count[2] == 0))
    tenm_table_add(net_can_howl_more_1_new());
  if ((my->count[8] == 1) && (my->count[2] == 2160))
    tenm_table_add(net_can_howl_more_1_new());

  if (my->count[8] == 3)
  {
    if (((my->count[2] >= 0) && (my->count[2] < 1000))
        || ((my->count[2] >= 4000) && (my->count[2] < 5000)))
    {
      if ((my->count[2] % 200 == 0) || (my->count[2] % 200 == 10))
        tenm_table_add(voter_new(15, my->count[2]));
      else if ((my->count[2] % 200 == 40) || (my->count[2] % 200 == 50))
        tenm_table_add(voter_new(16, my->count[2]));
      else if ((my->count[2] % 200 == 120) || (my->count[2] % 200 == 130))
        tenm_table_add(voter_new(18, my->count[2]));
      else if ((my->count[2] % 200 == 160) || (my->count[2] % 200 == 170))
        tenm_table_add(voter_new(17, my->count[2]));
    }
    else if (((my->count[2] >= 1800) && (my->count[2] < 2800))
             || ((my->count[2] >= 5800) && (my->count[2] < 6800)))
    {
      if ((my->count[2] % 200 == 0) || (my->count[2] % 200 == 10))
        tenm_table_add(voter_new(19, my->count[2]));
      else if ((my->count[2] % 200 == 50) || (my->count[2] % 200 == 60))
        tenm_table_add(voter_new(20, my->count[2]));
      else if ((my->count[2] % 200 == 100) || (my->count[2] % 200 == 110))
        tenm_table_add(voter_new(21, my->count[2]));
      else if ((my->count[2] % 200 == 150) || (my->count[2] % 200 == 160))
        tenm_table_add(voter_new(22, my->count[2]));
    }
  }
  
  /* no attack if it is not 2nd form */
  if (my->count[8] != 3)
    return 0;

  /* main attack */
  (my->count[4])++;
  switch(my->count[3])
  {
  case 1:
    if ((my->count[9] == 3) || (my->count[9] == 7))
    {
      if (my->mass == NULL)
      {
        my->count[4] = 0;
        break;
      }
      if ((my->count[4] >= 50) && (my->count[4] <= 58))
      {
        v[0] = (((tenm_polygon *)(my->mass->p[0]))->v[1])->x
          - (((tenm_polygon *)(my->mass->p[0]))->v[3])->x;
        v[1] = (((tenm_polygon *)(my->mass->p[0]))->v[1])->y
          - (((tenm_polygon *)(my->mass->p[0]))->v[3])->y;
        v[0] *= 6.0;
        v[1] *= 6.0;
        result[0] = v[0];
        result[1] = v[1];
        theta = (my->count[4] - 50) * (my->count[4] - 50) * 3;
        vector_rotate(result, v, -theta);
        tenm_table_add(laser_new(my->x, my->y,
                                 my->count_d[0], my->count_d[1],
                                 result[0], result[1],
                                 3, 1, 0));
      }
    }
    if (my->count[4] >= 75)
    {
      my->count[3] = 3;
      my->count[4] = 0;
    }
    break;
  case 3:
    if ((my->count[9] == 4) || (my->count[9] == 7))
    {
      if (my->mass == NULL)
      {
        my->count[4] = 0;
        break;
      }
      net_can_howl_core_cannon(my, player);
    }
    if (my->count[4] >= 159)
    {
      my->count[3] = 5;
      my->count[4] = 0;
    }
    break;
  case 5:
    if (my->count[9] >= 6)
    {
      if (my->count[4] >= 150)
      {
        tenm_table_add(net_can_howl_seeker_new(my->count[2], 5));
      }
    }
    if (my->count[4] >= 150)
    {
      my->count[3] = 1;
      my->count[4] = 0;
    }
    break;
  default:
    break;
  }

  /* sub attack */
  (my->count[6])++;
  switch(my->count[5])
  {
  case 0:
  case 1:
    if (((my->count[9] >= 2) && (my->count[5] == 0))
        || ((my->count[9] >= 3) && (my->count[5] == 1)))
    {
      if (my->count[6] == 50)
      {
        my->count_d[2] = player->x;
        my->count_d[3] = player->y;
      }
      else if (my->count[6] >= 70)
      {
        my->count_d[4] = my->x;
        my->count_d[5] = my->y;
        tenm_table_add(laser_point_new(my->x, my->y, 25.0,
                                       my->count_d[2], my->count_d[3],
                                       50.0, 3));
      }
    }
    if (my->count[6] >= 70)
    {
        (my->count[5])++;
        if (my->count[5] > 5)
          my->count[5] = 0;
        my->count[6] = 0;
    }    
    break;
  case 2:
  case 3:
  case 5:
    if (my->count[9] >= 7)
    {
      if (my->count[6] >= 100)
      {
        x = (double) (rand() % WINDOW_WIDTH);
        y = (double) (rand() % WINDOW_HEIGHT);
        tenm_table_add(net_can_howl_black_hole_new(x, y));
      }
    }
    if (my->count[6] >= 100)
    {
      (my->count[5])++;
      if (my->count[5] > 5)
        my->count[5] = 0;
      my->count[6] = 0;
    }
    break;
  case 4:
    if (my->count[9] >= 5)
    {
      if (my->count[6] >= 70)
      {
        tenm_table_add(laser_point_new(my->count_d[4], my->count_d[5], 25.0,
                                       my->count_d[2], my->count_d[3],
                                       50.0, 5));
      }
    }
    if (my->count[6] >= 70)
    {
      (my->count[5])++;
      if (my->count[5] > 5)
        my->count[5] = 0;
      my->count[6] = 0;
    }
    break;
  default:
    break;
  }

  /* nway attack */
  (my->count[7])++;
  if ((((my->count[9] == 1) || (my->count[9] == 6))
       && (my->count[7] == 60))
      || ((my->count[9] == 2) && (my->count[7] == 75))
      || ((my->count[9] == 3) && (my->count[7] == 90))
      || ((my->count[9] == 4) && (my->count[7] == 105))
      || (((my->count[9] == 5) || (my->count[9] == 6))
          && (my->count[7] == 120))
      || ((my->count[9] >= 7) &&
          (my->count[7] >= 60) && (my->count[7] <= 120)
          && (my->count[7] % 15 == 0)))
  {
    v[0] = player->x - my->x;
    v[1] = player->y - my->y;
    result[0] = v[0];
    result[1] = v[1];
    theta = -10 - 10 * ((my->count[7] - 60) / 15);
    vector_rotate(result, v, -theta);
    for (i = 0; i < 2 * ((my->count[7] - 60) / 15) + 3; i++)
    {
      vector_rotate(result, v, theta + 10 * i);
      tenm_table_add(laser_point_new(my->x, my->y, 7.0,
                                     my->x + result[0],
                                     my->y + result[1],
                                     25.0, 5));
    }
  }
  if (my->count[7] >= 120)
    my->count[7] = 0;

  return 0;
}

/* return 1 if shooting is done, 0 if not */
static int
net_can_howl_core_cannon(tenm_object *my,
                         const tenm_object *player)
{
  double a_x;
  double a_y;
  double b_x;
  double b_y;
  double v[2];
  int i;
  double s;
  double x;
  double y;
  double result[2];
  int theta;

  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  if (my->mass == NULL)
  {
    return 0;
  }
  if ((my->count[4] >= 120) && (my->count[4] < 140)
      && (my->count[4] % 2 == 0))
  {
    a_x = (((tenm_polygon *)(my->mass->p[0]))->v[0])->x;
    a_y = (((tenm_polygon *)(my->mass->p[0]))->v[0])->y;
    b_x = (((tenm_polygon *)(my->mass->p[0]))->v[1])->x;
    b_y = (((tenm_polygon *)(my->mass->p[0]))->v[1])->y;

    v[0] = (((tenm_polygon *)(my->mass->p[0]))->v[0])->x
      - (((tenm_polygon *)(my->mass->p[0]))->v[3])->x;
    v[1] = (((tenm_polygon *)(my->mass->p[0]))->v[0])->y
      - (((tenm_polygon *)(my->mass->p[0]))->v[3])->y;
    for (i = 0; i < 2; i++)
    {
      if (i == 0)
        s = (double) (my->count[4] % 8) / 2;
      else
        s = 7 - (double) (my->count[4] % 8) / 2;
      x = (a_x * (7.0 - s) + b_x * s) / 7.0;
      y = (a_y * (7.0 - s) + b_y * s) / 7.0;

      result[0] = v[0];
      result[1] = v[1];

      theta = 15 - (my->count[4] - 120);
      if (theta < -15)
        theta = -15;
      if (i != 0)
        theta *= -1;

      vector_rotate(result, v, -theta);
      tenm_table_add(laser_point_new(x, y, 25.0,
                                     x + result[0], y + result[1],
                                     30.0, 2));
    }
  }

  if (my->count[4] >= 159)
  {
    return 1;
  }

  return 0;
}

static int
net_can_howl_core_draw(tenm_object *my, int priority)
{
  int i;
  double a_x;
  double a_y;
  double b_x;
  double b_y;
  double dx;
  double dy;
  double length;
  double v[2];
  double result[2];
  int theta;
  tenm_color color;
  char temp[32];
  int status = 0;

  /* sanity check */
  if (my == NULL)
    return 0;

  /* wraith form */
  if ((my->count[8] == 0)
      && (my->count[2] >= 0) && (my->count[2] < 360)
      && (priority == 0))
  {
    length = 30.0 + ((double) ((360 - my->count[2]) * 2));
    net_can_howl_core_draw_wraith(my->x, my->y, length,
                                  my->count[2] * (-1),
                                  90 + my->count[2] * (-1), 0);
  }
  if ((my->count[8] == 2)
      && (my->count[2] >= 30) && (my->count[2] < 150)
      && (priority == 0))
  {
    if (my->count[2] < 90)
      length = 30.0 + ((double) ((my->count[2] - 30) * 3));
    else
      length = 30.0 + ((double) ((150 - my->count[2]) * 3));
    net_can_howl_core_draw_wraith(my->x, my->y, length,
                                  (my->count[2] - 30) * 3,
                                  90 + (my->count[2] - 30) * 3, 0);
  }
  if ((my->count[8] == 4)
      && (my->count[2] >= 0) && (my->count[2] < 120)
      && (priority == -1))
  {
    if (my->count[2] <= 90)
      length = 30.0 + ((double) ((my->count[2]) * 4));
    else
      length = ((double) ((120 - my->count[2]) * 13));
    if (length < NEAR_ZERO)
      length = 1.0;
    net_can_howl_core_draw_wraith(my->x, my->y, length,
                                  my->count[2] * (-6),
                                  90 + my->count[2] * (-3), my->count[17]);
  }

  /* decoration */
  if (priority == 0)
  {
    /* arrow */
    if ((my->count[8] == 3)
        && ((my->count[5] == 0) || (my->count[5] == 1))
        && (my->count[6] >= 50) && (my->count[6] < 70)
        && (((my->count[9] >= 2) && (my->count[5] == 0))
            || ((my->count[9] >= 3) && (my->count[5] == 1))))
    {
      color = tenm_map_color(99, 128, 158);
      if (tenm_draw_circle((int) (my->x), (int) (my->y),
                           15 + (my->count[6] - 50), 1,
                           color) != 0)
        status = 1;
      dx = my->count_d[2] - my->x;
      dy = my->count_d[3] - my->y;
      length = tenm_sqrt((int) (dx * dx + dy * dy));
      if (length < NEAR_ZERO)
        length = 1.0;
      if (tenm_draw_line((int) (my->x), (int) (my->y),
                         (int) (my->x + 50.0 * dx / length),
                         (int) (my->y + 50.0 * dy / length),
                         1, color))
        status = 1;
    }
    /* warp arrow */
    if ((my->count[8] == 3)
        && (my->count[5] == 4)
        && (my->count[6] >= 50) && (my->count[6] < 70)
        && (my->count[9] >= 5))
    {
      color = tenm_map_color(142, 99, 158);
      if (tenm_draw_circle((int) (my->count_d[4]), (int) (my->count_d[5]),
                           15 + (my->count[6] - 50), 1,
                           color) != 0)
        status = 1;
      dx = my->count_d[2] - my->count_d[4];
      dy = my->count_d[3] - my->count_d[5];
      length = tenm_sqrt((int) (dx * dx + dy * dy));
      if (length < NEAR_ZERO)
        length = 1.0;
      if (tenm_draw_line((int) (my->count_d[4]), (int) (my->count_d[5]),
                         (int) (my->count_d[4] + 50.0 * dx / length),
                         (int) (my->count_d[5] + 50.0 * dy / length),
                         1, color))
        status = 1;
    }
    /* blade */
    if ((my->count[8] == 3)
        && (my->mass != NULL) && (my->count[3] == 1)
        && (my->count[4] >= 1) && (my->count[4] < 45)
        && ((my->count[9] == 3) || (my->count[9] == 7)))
    {
      v[0] = (((tenm_polygon *)(my->mass->p[0]))->v[1])->x
        - (((tenm_polygon *)(my->mass->p[0]))->v[3])->x;
      v[1] = (((tenm_polygon *)(my->mass->p[0]))->v[1])->y
        - (((tenm_polygon *)(my->mass->p[0]))->v[3])->y;
      v[0] *= 6.0;
      v[1] *= 6.0;

      for (i = 0; i <= my->count[4] / 5; i++)
      {
        if (i == my->count[4] / 5)
          color = tenm_map_color(99, 128, 158);
        else
          color = tenm_map_color(158, 158, 158);

        result[0] = v[0];
        result[1] = v[1];
        theta = i * i * 3;
        vector_rotate(result, v, -theta);

        if (tenm_draw_line((int) (my->x), (int) (my->y),
                           (int) (my->x + result[0]),
                           (int) (my->y + result[1]),
                           1, color))
          status = 1;
      }
    }
    /* main cannon */
    if (((my->count[8] == 0) || (my->count[8] == 3))
        && (my->mass != NULL)
        && (((my->count[3] == 3)
             && ((my->count[9] == 4) || (my->count[9] == 7)))
            || (my->count[8] == 0))
        && (my->count[4] >= 1) && (my->count[4] < 140))
    {
      v[0] = (((tenm_polygon *)(my->mass->p[0]))->v[0])->x
        - (((tenm_polygon *)(my->mass->p[0]))->v[3])->x;
      v[1] = (((tenm_polygon *)(my->mass->p[0]))->v[0])->y
        - (((tenm_polygon *)(my->mass->p[0]))->v[3])->y;
      v[0] *= 27.0;
      v[1] *= 27.0;
      a_x = (((tenm_polygon *)(my->mass->p[0]))->v[0])->x;
      a_y = (((tenm_polygon *)(my->mass->p[0]))->v[0])->y;
      b_x = (((tenm_polygon *)(my->mass->p[0]))->v[1])->x;
      b_y = (((tenm_polygon *)(my->mass->p[0]))->v[1])->y;

      if (my->count[4] < 35)
        theta = 15 + 90;
      else if (my->count[4] < 80)
        theta = 15 + (80 - my->count[4]) * 2;
      else
        theta = 15;

      if (my->count[4] < 80)
        color = tenm_map_color(158, 158, 158);
      else
        color = tenm_map_color(99, 143, 158);

      result[0] = v[0];
      result[1] = v[1];
      vector_rotate(result, v, -theta);
      if (tenm_draw_line((int) (a_x), (int) (a_y),
                         (int) (a_x + result[0]),
                         (int) (a_y + result[1]),
                         1, color))
        status = 1;

      vector_rotate(result, v, theta);
      if (tenm_draw_line((int) (b_x), (int) (b_y),
                         (int) (b_x + result[0]),
                         (int) (b_y + result[1]),
                         1, color))
        status = 1;
    }
  }
  
  /* body */
  if (priority == 0)
  {    
    if ((my->count[8] == 1) || (my->count[8] == 3)
        || ((my->count[8] == 0)
            && (my->count[2] > 360))
        || ((my->count[8] == 2)
            && ((my->count[2] < 30) || (my->count[2] > 150))))
    {
      if (net_can_howl_core_green(my))
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
  
      if (my->mass != NULL)
      {
        for (i = 0; i < 4; i++)
        {
          a_x = (int) ((((tenm_polygon *)(my->mass->p[0]))->v[i % 4])->x);
          a_y = (int) ((((tenm_polygon *)(my->mass->p[0]))->v[i % 4])->y);
          b_x = (int) ((((tenm_polygon *)(my->mass->p[0]))->v[(i+1) % 4])->x);
          b_y = (int) ((((tenm_polygon *)(my->mass->p[0]))->v[(i+1) % 4])->y);
          if (tenm_draw_line(a_x, a_y, b_x, b_y, 3, color) != 0)
            status = 1;
        }
      }
    }
  }
  
  /* hit point stat */
  if (((my->count[8] == 1) || (my->count[8] == 3))
      && (priority == 0))
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 5, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "net_can_howl_core_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

static int
net_can_howl_core_draw_wraith(double x, double y, double length,
                              int theta, int theta_z, int green)
{
  int i;
  double a_x;
  double a_y;
  double b_x;
  double b_y;
  tenm_color color;
  int status = 0;

  /* sanity check */
  if (length < NEAR_ZERO)
    return 0;

  if (green != 0)
          color = tenm_map_color(157, 182, 123);
  else
    color = tenm_map_color(182, 147, 123);

  for (i = 0; i < 360; i += 90)
  {
    a_x = tenm_cos(theta + i) - tenm_sin(theta + i);
    a_y = (tenm_sin(theta + i) + tenm_cos(theta + i)) * tenm_cos(theta_z)
      - 0.5 * tenm_sin(theta_z);
    b_x = tenm_cos(theta + i + 90) - tenm_sin(theta+ i + 90);
    b_y = (tenm_sin(theta + i + 90) + tenm_cos(theta + i + 90))
      * tenm_cos(theta_z)
      - 0.5 * tenm_sin(theta_z);
    a_x *= length;
    a_y *= length;
    b_x *= length;
    b_y *= length;
    a_x += x;
    a_y += y;
    b_x += x;
    b_y += y;
    if (tenm_draw_line((int) a_x, (int) a_y,
                       (int) b_x, (int) b_y, 1, color) != 0)
      status = 1;
  }
  for (i = 0; i < 360; i += 90)
  {
    a_x = tenm_cos(-(theta + i)) - tenm_sin(-(theta + i));
    a_y = (tenm_sin(-(theta + i)) + tenm_cos(-(theta + i)))
      * tenm_cos(theta_z)
      + 0.5 * tenm_sin(theta_z);
    b_x = tenm_cos(-(theta + i + 90)) - tenm_sin(-(theta+ i + 90));
    b_y = (tenm_sin(-(theta + i + 90)) + tenm_cos(-(theta + i + 90)))
      * tenm_cos(theta_z)
      + 0.5 * tenm_sin(theta_z);
    a_x *= length;
    a_y *= length;
    b_x *= length;
    b_y *= length;
    a_x += x;
    a_y += y;
    b_x += x;
    b_y += y;
    if (tenm_draw_line((int) a_x, (int) a_y,
                       (int) b_x, (int) b_y, 1, color) != 0)
      status = 1;
  }
  for (i = 0; i < 360; i += 90)
  {
    a_x = tenm_cos(theta + i) - tenm_sin(theta + i);
    a_y = (tenm_sin(theta + i) + tenm_cos(theta + i))
      * tenm_cos(theta_z)
      - 0.5 * tenm_sin(theta_z);
    b_x = tenm_cos(-(theta + i + 90)) - tenm_sin(-(theta+ i + 90));
    b_y = (tenm_sin(-(theta + i + 90)) + tenm_cos(-(theta + i + 90)))
      * tenm_cos(theta_z)
      + 0.5 * tenm_sin(theta_z);
    a_x *= length;
    a_y *= length;
    b_x *= length;
    b_y *= length;
    a_x += x;
    a_y += y;
    b_x += x;
    b_y += y;
    if (tenm_draw_line((int) a_x, (int) a_y,
                       (int) b_x, (int) b_y, 1, color) != 0)
      status = 1;
  }

  return status;
}

/* return 1 (true) or 0 (false) */
static int
net_can_howl_core_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if (my->count[8] == 1)
  {
    if ((my->count[2] >= 2630) && (my->count[2] < 2730))
      return 1;
  }
  else if (my->count[8] == 2)
  {
    if (my->count[17] != 0)
      return 1;
  }
  else if (my->count[8] == 3)
  {
    if ((my->count[9] >= 7) && (my->count[2] < 8000))
      return 1;
  }

  return 0;
}

static tenm_object *
net_can_howl_black_hole_new(double x, double y)
{
  tenm_object *new = NULL;
  int *count = NULL;

  count = (int *) malloc(sizeof(int) * 3);
  if (count == NULL)
  {
    fprintf(stderr, "net_can_howl_black_hole_new: malloc(count) failed\n");
    return NULL;
  }

  /* list of count
   * [0] color (for delete_enemy_shot)
   * [1] shoot timer
   * [2] shoot theta
   */
  count[0] = 6;
  count[1] = 1;
  count[2] = 0;

  /* ATTR_ENEMY_SHOT is only to clear it when the player is killed */
  new = tenm_object_new("net can howl black-hole", ATTR_ENEMY_SHOT, 0,
                        0, x, y,
                        3, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double)) NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&net_can_howl_black_hole_act),
                        (int (*)(tenm_object *, int))
                        (&net_can_howl_black_hole_draw));
  if (new == NULL)
  {
    fprintf(stderr, "net_can_howl_black_hole_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
net_can_howl_black_hole_act(tenm_object *my, const tenm_object *player)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  (my->count[1])++;
  if ((my->count[1] >= 30) && (my->count[1] <= 75))
  {
    if (my->count[1] % 5 == 0)
    {
      my->count[2] = rand() % 360;
      tenm_table_add(normal_shot_new(my->x + 816.0 * tenm_cos(my->count[2]),
                                     my->y + 816.0 * tenm_sin(my->count[2]),
                                     -8.0 * tenm_cos(my->count[2]),
                                     -8.0 * tenm_sin(my->count[2]),
                                     4, 102, 102));
    }
  }

  if (my->count[1] >= 80)
    return 1;

  return 0;
}

static int
net_can_howl_black_hole_draw(tenm_object *my, int priority)
{
  int r;
  double r_d;
  tenm_color color;
  int status = 0;

  /* sanity check */
  if (my == NULL)
    return 0;

  if (priority != 0)
    return 0;

  color = tenm_map_color(158, 158, 158);
  if (my->count[1] <= 20)
    r = my->count[1] * 5;
  else if (my->count[1] <= 30)
    r = 100;
  else
    r = 100 - (my->count[1] - 30) * 2;
  if (r < 1)
    r = 1;

  if (tenm_draw_circle((int) (my->x), (int) (my->y),
                       r, 1, color) != 0)
      status = 1;

  if ((my->count[1] >= 30) && (my->count[1] <= 75))
  {
    r_d = (double) ((r / 2) + 50 * (4 - (my->count[1] % 5)));
    if (r_d < 1.0)
      r_d = 1.0;
    if (tenm_draw_line((int) (my->x + r_d * tenm_cos(my->count[2])),
                       (int) (my->y + r_d * tenm_sin(my->count[2])),
                       (int) (my->x + (r_d + 150.0) * tenm_cos(my->count[2])),
                       (int) (my->y + (r_d + 150.0) * tenm_sin(my->count[2])),
                       1, color) != 0)
      status = 1;
  }
  
  return status;
}

static tenm_object *
net_can_howl_seeker_new(int t, int n)
{
  int i;
  int suffix;
  int suffix_d;
  double x;
  double y;
  double result[2];
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;

  result[0] = 0.0;
  result[1] = 0.0;
  net_can_howl_position(result, 0, 0, t);
  x = result[0];
  y = result[1];

  count = (int *) malloc(sizeof(int) * (6 + n));
  if (count == NULL)
  {
    fprintf(stderr, "net_can_howl_seeker_new: malloc(count) failed\n");
    return NULL;
  }

  count_d = (double *) malloc(sizeof(double) * (n * 2));
  if (count_d == NULL)
  {
    fprintf(stderr, "net_can_howl_seeker_new: malloc(count_d) failed\n");
    free(count);
    return NULL;
  }

  /* list of count
   * [0] color (for delete_enemy_shot)
   * [1] shoot timer
   * [2] total number of markers
   * [3] number of laser fired
   * [4] activated marker
   * [5] core tracking timer
   * [6 -- ] "marker used" flag
   */
  count[0] = 6;
  count[1] = 30;
  count[2] = n;
  count[3] = 0;
  count[4] = -1;
  count[5] = t;
  for (i = 0; i < n; i++)
  {
    suffix = i + 6;
    count[suffix + 0] = 0;
  }

  /* list of count_d
   * [0 --] marker (x, y)
   */
  for (i = 0; i < n; i++)
  {
    suffix_d = i * 2;
    count_d[suffix_d + 0] = (double) (rand() % WINDOW_WIDTH);
    count_d[suffix_d + 1] = (double) (rand() % WINDOW_HEIGHT);
  }

  /* ATTR_ENEMY_SHOT is only to clear it when the player is killed */
  new = tenm_object_new("net can howl seeker", ATTR_ENEMY_SHOT, 0,
                        0, x, y,
                        6 + n, count, n * 2, count_d, 0, NULL,
                        (int (*)(tenm_object *, double)) NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&net_can_howl_seeker_act),
                        (int (*)(tenm_object *, int))
                        (&net_can_howl_seeker_draw));
  if (new == NULL)
  {
    fprintf(stderr, "net_can_howl_seeker_new: tenm_object_new failed\n");
    if (count_d != NULL)
      free(count_d);
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
net_can_howl_seeker_act(tenm_object *my, const tenm_object *player)
{
  int i;
  double x;
  double y;
  double source_x;
  double source_y;
  double target_x;
  double target_y;
  double dx;
  double dy;
  double length;
  double length_max;
  int target_n;
  int suffix;
  int suffix_d;
  int life;
  double result[2];

  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  (my->count[5])++;
  /* to please delete_enemy_shot) */
  result[0] = 0.0;
  result[1] = 0.0;
  net_can_howl_position(result, 0, 0, my->count[5]);
  my->x = result[0];
  my->y = result[1];

  (my->count[1])--;
  if (my->count[1] <= 0)
  {
    if ((my->count[3] == 0) || (my->count[4] < 0))
    {
      source_x = my->x;
      source_y = my->y;
    }
    else
    {
      suffix_d = my->count[4] * 2;
      source_x = my->count_d[suffix_d + 0];
      source_y = my->count_d[suffix_d + 1];
    }

    target_x = player->x;
    target_y = player->y;
    target_n = -1;

    if (my->count[3] >= my->count[2])
    {
      tenm_table_add(laser_point_new(source_x, source_y, 25.0,
                                     player->x, player->y, 25.0, 3));
      return 1;
    }

    length_max = -1.0;
    for (i = 0; i < my->count[2]; i++)
    {
      suffix = i + 6;
      suffix_d = i * 2;
      if (my->count[suffix + 0] != 0)
        continue;
      x = my->count_d[suffix_d + 0];
      y = my->count_d[suffix_d + 1];
      dx = x - source_x;
      dy = y - source_y;
      length = dx * dx + dy * dy;
      if (length > length_max)
      {
        target_x = x;
        target_y = y;
        target_n = i;
        length_max = length;
      }
    }  

    dx = target_x - source_x;
    dy = target_y - source_y;
    length = tenm_sqrt((int) (dx * dx + dy * dy));
    if (length < NEAR_ZERO)
      length = 1.0;
    life = (int) (length / 25.0);
    if (life <= 0)
      life = 1;

    tenm_table_add(laser_new(source_x, source_y,
                             25.0 * dx / length,
                             25.0 * dy / length,
                             25.0 * dx / length,
                             25.0 * dy / length,
                             3, life, 0));

    my->count[1] = life + 10;
    (my->count[3])++;
    my->count[4] = target_n;
    if (target_n >= 0)
    {
      suffix = target_n + 6;
      my->count[suffix + 0] = 1;
    }

    if (my->count[3] > my->count[2])
      return 1;
  }

  return 0;
}

static int
net_can_howl_seeker_draw(tenm_object *my, int priority)
{
  int i;
  int suffix;
  int suffix_d;
  int theta;
  int x;
  int y;
  int dx;
  tenm_color color;
  int status = 0;

  /* sanity check */
  if (my == NULL)
    return 0;

  if (priority != 0)
    return 0;

  for (i = 0; i < my->count[2]; i++)
  {
    suffix = i + 6;
    suffix_d = i * 2;
    if ((my->count[suffix + 0] != 0) && (my->count[4] != i))
      continue;
    if ((my->count[4] == i) && (my->count[1] < 10))
      color = tenm_map_color(0, 111, 223);
    else
      color = tenm_map_color(158, 158, 158);

    theta = (my->count[5] * 3) % 360 + i * 360 / my->count[2];
    x = (int) (my->count_d[suffix_d + 0]);
    y = (int) (my->count_d[suffix_d + 1]);
    dx = (int) (25.0 * tenm_cos(theta));
    if (tenm_draw_line(x, y + 25, x + dx, y, 1, color) != 0)
      status = 1;
    if (tenm_draw_line(x, y - 25, x + dx, y, 1, color) != 0)
      status = 1;
    if (tenm_draw_line(x, y + 25, x - dx, y, 1, color) != 0)
      status = 1;
    if (tenm_draw_line(x, y - 25, x - dx, y, 1, color) != 0)
      status = 1;
  }
  
  return status;
}

static tenm_object *
senator_new(int table_index, int what)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = 0.0;
  double y = 0.0;
  double target_x;
  double target_y;

  /* sanity check */
  if (table_index < 0)
  {
    fprintf(stderr, "senator_new: strange table_index (%d)\n", table_index);
    return NULL;
  }
  if ((what < 1) || (what > 7))
  {
    fprintf(stderr, "senator_new: strange what (%d)\n", what);
    return NULL;
  }

  if (what % 2 == 0)
  {
    x = -29.0;
    y = (double) (WINDOW_HEIGHT / 4);
  }
  else
  {
    x = ((double) (WINDOW_WIDTH)) + 29.0;
    y = (double) (WINDOW_HEIGHT / 4);
  }
  target_x = ((double) (WINDOW_WIDTH / 2))
    + ((double) (WINDOW_WIDTH / 2)) * tenm_cos(-90 + 360 * (what - 1) / 7);
  target_y = ((double) (WINDOW_HEIGHT / 4))
    + ((double) (WINDOW_HEIGHT / 4)) * tenm_sin(-90 + 360 * (what - 1) / 7);

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "senator_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_circle_new(x, y, 30.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "senator_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 8);
  if (count == NULL)
  {
    fprintf(stderr, "senator_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  count_d = (double *) malloc(sizeof(double) * 6);
  if (count_d == NULL)
  {
    fprintf(stderr, "senator_new: malloc(count_d) failed\n");
    if (count != NULL)
      free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] for deal_damage
   * [1] "damaged" timer
   * [2] move timer
   * [3] what
   * [4] shoot timer
   * [5] shoot manager
   * [6] core index
   * [7] life mode
   */
  count[0] = 0;
  count[1] = 0;
  count[2] = 330 - what * 30;
  count[3] = what;
  count[4] = -100;
  if (what == 7)
    count[5] = 400;
  else
    count[5] = 0;
  count[6] = table_index;
  count[7] = 0;

  /* list of count_d
   * [0] speed x
   * [1] speed y
   * [2 -- 3] shoot manager
   * [4 -- 5] decoration manager
   */
  count_d[0] = (target_x - x) / ((double) (count[2]));
  count_d[1] = (target_y - y) / ((double) (count[2]));
  count_d[2] = 0.0;
  count_d[3] = 0.0;
  count_d[4] = 0.0;
  count_d[5] = 0.0;

  new = tenm_object_new("Senator", ATTR_BOSS,
                        ATTR_PLAYER_SHOT,
                        400, x, y,
                        8, count, 6, count_d, 1, p,
                        (int (*)(tenm_object *, double)) (&senator_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&senator_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&senator_act),
                        (int (*)(tenm_object *, int)) (&senator_draw));
  if (new == NULL)
  {
    fprintf(stderr, "senator_new: tenm_object_new failed\n");
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
senator_move(tenm_object *my, double turn_per_frame)
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
senator_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (your == NULL)
    return 0;

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;
  if ((my->count[7] != 1) && (my->count[7] != 3))
    return 0;

  deal_damage(my, your, 0);
  if (senator_green(my))
    add_chain(my, your);
  my->count[1] = 41;

  if (my->hit_point <= 0)
  {
    senator_explode(my);
    add_score(15000);
    tenm_table_apply(my->count[6],
                     (int (*)(tenm_object *, int)) (&senator_signal),
                     my->table_index);
    return 1;
  }

  return 0;
}

static void
senator_explode(tenm_object *my)
{
  int n;
  /* sanity check */
  if (my == NULL)
    return;

  if (senator_green(my))
    n = 8;
  else
    n = 7;

  tenm_table_add(explosion_new(my->x, my->y,
                               my->count_d[0] * 0.5,
                               my->count_d[1] * 0.5,
                               1, 1000, n, 8.0, 6));
  tenm_table_add(explosion_new(my->x, my->y,
                               my->count_d[0] * 0.5,
                               my->count_d[1] * 0.5,
                               2, 300, n, 5.0, 8));
  /* no signal here --- this can be called at the core self-destruction */
}

static int
senator_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int theta;
  double temp[2];
  double result[2];

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
  my->count_d[4] = player->x;
  my->count_d[5] = player->y;

  /* speed change */
  if (my->count[7] == 0)
    (my->count[2])--;
  else
    (my->count[2])++;

  if (my->count[7] == 0)
  {
    if (my->count[2] == 0)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[2] < -200)
    {
      my->count[2] = -1;
      my->count[7] = 1;
    }
  }
  else if ((my->count[7] == 1) || (my->count[7] == 3))
  {
    if (my->count[7] == 3)
    {
      result[0] = 0.0;
      result[1] = 0.0;
      net_can_howl_position(result, my->count[3], 0, my->count[2] + 1);
    }
    else
    {
      theta = -90 + 360 * (my->count[3] - 1) / 7
        + (8 - my->count[3]) * (my->count[2] + 1) / 2;
      result[0] = ((double) (WINDOW_WIDTH / 2))
        + ((double) (WINDOW_WIDTH / 2)) * tenm_cos(theta);
      result[1] = ((double) (WINDOW_HEIGHT / 4))
        + ((double) (WINDOW_HEIGHT / 4)) * tenm_sin(theta);
    }
    my->count_d[0] = result[0] - my->x;
    my->count_d[1] = result[1] - my->y;
  }
  else if (my->count[7] == 2)
  {
    if (my->count[2] == 0)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    else if (my->count[2] == 30)
    {
      result[0] = 0.0;
      result[1] = 0.0;
      net_can_howl_position(result, my->count[3], 0, 0);
      my->count_d[0] = (result[0] - my->x) / 120.0;
      my->count_d[1] = (result[1] - my->y) / 120.0;
    }
    else if (my->count[2] == 150)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
  }

  if ((my->count[7] == 2) && (my->count[2] >= 180))
  {
    my->count[7] = 3;
    my->count[2] = -1;

    my->count[4] = -100;

    return 0;
  }

  if ((my->count[7] != 1) && (my->count[7] != 3))
    return 0;

  /* shoot */
  (my->count[4])++;
  switch(my->count[3])
  {
  case 1:
    if (my->count[4] == 50)
    {
      my->count_d[2] = player->x;
      my->count_d[3] = player->y;
      tenm_table_add(normal_shot_point_new(my->x, my->y, 9.0,
                                           my->count_d[2], my->count_d[3], 1));
    }
    else if (my->count[4] == 55)
    {
      temp[0] = my->count_d[2] - my->x;
      temp[1] = my->count_d[3] - my->y;
      result[0] = 0.0;
      result[1] = 0.0;
      vector_rotate(result, temp, 5);
      tenm_table_add(normal_shot_point_new(my->x, my->y, 9.0,
                                           my->x + result[0],
                                           my->y + result[1], 1));
      vector_rotate(result, temp, -5);
      tenm_table_add(normal_shot_point_new(my->x, my->y, 9.0,
                                           my->x + result[0],
                                           my->y + result[1], 1));
    }
    else if (my->count[4] >= 60)
    {
      tenm_table_add(normal_shot_point_new(my->x, my->y, 9.0,
                                           my->count_d[2], my->count_d[3], 1));
      my->count[4] = 0;
    }
    break;
  case 2:
    if (my->count[4] == 48)
    {
      my->count[5] = rand() % 360;
      for (i = 0; i < 360; i += 24)
        tenm_table_add(normal_shot_angle_new(my->x, my->y, 5.0,
                                             my->count[5] + i, 0));
    }
    else if (my->count[4] >= 96)
    {
      for (i = 0; i < 360; i += 24)
        tenm_table_add(normal_shot_angle_new(my->x, my->y, 5.0,
                                             my->count[5] + 12 + i, 0));
      my->count[4] = 0;
    }
    break;
  case 3:
    if (my->count[4] >= 20)
    {
      tenm_table_add(normal_shot_point_new(my->x, my->y,
                                           4.0 + ((double) (rand() % 8)) / 4.0,
                                           player->x
                                           + (double) (-50 + rand() % 101),
                                           player->y
                                           + (double) (-50 + rand() % 101),
                                           1));
      my->count[4] = 0;
    }
    break;
  case 4:
    if (my->count[4] == 64)
    {
      my->count[5] = rand() % 360;
      for (i = 0; i < 360; i += 60)
      {
        tenm_table_add(normal_shot_angle_new(my->x, my->y, 5.5,
                                             my->count[5] + i, 0));
        tenm_table_add(normal_shot_angle_new(my->x, my->y, 5.5,
                                             my->count[5] + 10 + i, 0));
      }
    }
    else if (my->count[4] >= 74)
    {
      for (i = 0; i < 360; i += 60)
      {
        tenm_table_add(normal_shot_angle_new(my->x, my->y, 6.5,
                                             my->count[5] + 30 + i, 0));
        tenm_table_add(normal_shot_angle_new(my->x, my->y, 6.5,
                                             my->count[5] + 30 + 10 + i, 0));
      }
      
      my->count[4] = 0;
    }
    break;
  case 5:
    if (my->count[4] == 55)
    {
      my->count_d[2] = player->x;
      my->count_d[3] = player->y;

      temp[0] = my->count_d[2] - my->x;
      temp[1] = my->count_d[3] - my->y;
      result[0] = 0.0;
      result[1] = 0.0;
      vector_rotate(result, temp, 21);
      tenm_table_add(normal_shot_point_new(my->x, my->y, 9.0,
                                           my->x + result[0],
                                           my->y + result[1], 1));
      vector_rotate(result, temp, -21);
      tenm_table_add(normal_shot_point_new(my->x, my->y, 9.0,
                                           my->x + result[0],
                                           my->y + result[1], 1));
    }
    else if (my->count[4] == 60)
    {
      temp[0] = my->count_d[2] - my->x;
      temp[1] = my->count_d[3] - my->y;
      result[0] = 0.0;
      result[1] = 0.0;
      vector_rotate(result, temp, 13);
      tenm_table_add(normal_shot_point_new(my->x, my->y, 8.0,
                                           my->x + result[0],
                                           my->y + result[1], 1));
      vector_rotate(result, temp, -13);
      tenm_table_add(normal_shot_point_new(my->x, my->y, 8.0,
                                           my->x + result[0],
                                           my->y + result[1], 1));
    }
    else if (my->count[4] >= 65)
    {
      temp[0] = my->count_d[2] - my->x;
      temp[1] = my->count_d[3] - my->y;
      result[0] = 0.0;
      result[1] = 0.0;
      vector_rotate(result, temp, 5);
      tenm_table_add(normal_shot_point_new(my->x, my->y, 7.0,
                                           my->x + result[0],
                                           my->y + result[1], 1));
      vector_rotate(result, temp, -5);
      tenm_table_add(normal_shot_point_new(my->x, my->y, 7.0,
                                           my->x + result[0],
                                           my->y + result[1], 1));
      my->count[4] = 0;
    }
    break;
  case 6:
    if (my->count[4] >= 7)
    {
      tenm_table_add(normal_shot_angle_new(my->x, my->y,
                                           3.0 + ((double) (rand() % 8)) / 4.0,
                                           rand() % 360, 0));
      my->count[4] = 0;
    }
    break;
  case 7:
    if (my->count[5] > my->hit_point)
    {
      tenm_table_add(normal_shot_point_new(my->x, my->y,
                                           6.0 + ((double)(rand() % 48)) /16.0,
                                           player->x
                                           + (double) (-25 + rand() % 51),
                                           player->y
                                           + (double) (-25 + rand() % 51),
                                           5));
      (my->count[5]) -= 5;
    }
    break;
  default:
    break;
  }
  
  return 0;
}

static int
senator_draw(tenm_object *my, int priority)
{
  double dx;
  double dy;
  double length;
  tenm_color color;
  char temp[32];
  int status = 0;

  /* sanity check */
  if (my == NULL)
    return 0;

  if (priority != 0)
    return 0;

  /* decoration */
  if (senator_green(my))
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
  
  dx = my->count_d[4] - my->x;
  dy = my->count_d[5] - my->y;
  length = tenm_sqrt((int) (dx * dx + dy * dy));
  if (length < NEAR_ZERO)
    length = 1.0;
  if (tenm_draw_circle((int) (my->x + 10.0 * dx / length),
                       (int) (my->y + 10.0 * dy / length),
                       15, 1, color) != 0)
    status = 1;

  /* body */
  if (senator_green(my))
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
  
  if (tenm_draw_circle((int) (my->x), (int) (my->y),
                       30, 3, color) != 0)
    status = 1;

  /* hit point stat */
  if (my->count[1] > 0)
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string((int) my->x, (int) my->y, temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "senator_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

/* return 1 (true) or 0 (false) */
static int
senator_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if (my->count[7] == 1)
  {
    if (my->count[2] < 2730)
      return 1;
  }
  if (my->count[7] == 3)
  {
    if (my->count[2] < 8000)
      return 1;
  }

  return 0;
}


static int
core_signal_second_form(tenm_object *my, int n)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (strcmp(my->name, "Senator") != 0)
    return 0;

  my->count[7] = 2;
  my->count[2] = -1;

  my->count[4] = -100;

  my->count_d[0] = 0.0;
  my->count_d[1] = 0.0;

  return 0;
}

static int
core_signal_kill_senator(tenm_object *my, int n)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (strcmp(my->name, "Senator") != 0)
    return 0;

  /* the explosion should be brown because this is self-destruction */
  my->count[7] = 4;
  senator_explode(my);

  return 1;
}

static int
senator_signal(tenm_object *my, int n)
{
  int i;

  /* sanity check */
  if (my == NULL)
    return 0;
  if (strcmp(my->name, "net can howl core") != 0)
    return 0;

  /* reset attack timer */
  my->count[4] = 0;
  my->count[6] = 0;
  my->count[7] = 0;
  /* notify death of senator */
  (my->count[9])++;
  for (i = 0; i < 7; i++)
    if (my->count[10 + i] == n)
      my->count[10 + i] = -1;

  return 0;
}

/* set result (arg 1) to the position of what (arg 2)
 * result (arg 1) must be double[2] (you must allocate enough memory
 * before calling this function)
 * return 0 on success, 1 on error
 */
/* list of what
 * 0: core origin
 * 1 -- 7: senator
 * 8: core left hand
 * 9 -- 14: voter (1st form)
 * 15 -- 22: voter (2nd form)
 */
static int
net_can_howl_position(double *result, int what, int t_0, int t_1)
{
  double origin_x;
  double origin_y;
  double core_x;
  double core_y;
  double x;
  double y;
  double temp[2];
  double temp_r[2];
  int theta;
  int t;

  /* sanity check */
  if (result == NULL)
  {
    fprintf(stderr, "net_can_howl_position: result is NULL\n");
    return 1;
  }
  if ((what < 0) || (what > 22))
  {
    fprintf(stderr, "net_can_howl_position: strange what (%d)\n", what);
    return 1;
  }
  if (t_0 < 0)
  {
    fprintf(stderr, "net_can_howl_position: t_0 is negative (%d)\n", t_0);
    return 1;
  }
  if (t_1 < 0)
  {
    fprintf(stderr, "net_can_howl_position: t_1 is negative (%d)\n", t_1);
    return 1;
  }

  t = t_1 - t_0;
  if (t < 0)
  {
    fprintf(stderr, "net_can_howl_position: t is negative (%d)\n", t);
    return 1;
  }

  origin_x = (double) (WINDOW_WIDTH / 2);
  origin_y = (double) (WINDOW_HEIGHT / 2);

  core_x = (double) (WINDOW_WIDTH / 2);
  core_y = 19.0;

  if ((t >= 1100) && (t < 1400))
    core_y += (double) (t - 1100);
  else if ((t >= 1400) && (t < 1700))
    core_y += (double) (600 - (t - 1100));

  if ((t >= 2900) && (t < 3121))
    core_y += (double) (t - 2900);
  else if ((t >= 3121) && (t < 3700))
    core_y += 221.0;
  else if ((t >= 3700) && (t < 3921))
    core_y += (double) (221 - (t - 3700));

  if ((t >= 5100) && (t < 5400))
    core_y += (double) (t - 5100);
  else if ((t >= 5400) && (t < 5700))
    core_y += (double) (600 - (t - 5100));

  if ((t >= 6900) && (t < 7121))
    core_y += (double) (t - 6900);
  else if ((t >= 7121) && (t < 7700))
    core_y += 221.0;
  else if ((t >= 7700) && (t < 7921))
    core_y += (double) (221 - (t - 7700));

  if (what == 0)
  {
    x = core_x;
    y = core_y;
  }
  else if ((what >= 1) && (what <= 6))
  {
    theta = -90 + 60 * (what - 1);
    if (what % 2 == 0)
    {
      theta -= 56;
      theta += t / 2;
    }
    else
    {
      theta -= t / 2;
    }
    x = core_x + 120.0 * tenm_cos(-150 + what * 60) + 240.0 * tenm_cos(theta);
    y = core_y + 120.0 * tenm_sin(-150 + what * 60) + 240.0 * tenm_sin(theta);
  }
  else if (what == 7)
  {
    theta = -90 + t / 2;
    x = core_x + 90.0 * tenm_cos(theta);
    y = core_y + 90.0 * tenm_sin(theta);
  }
  else if (what == 8)
  {
    x = core_x + 30.0;
    y = core_y;
  }
  else if ((what == 9) || (what == 12))
  {
    x = ((double) WINDOW_WIDTH) + 20.0 - ((double) t) * 4.0;
    y = voter_parabola_formation((double) (WINDOW_WIDTH / 2),
                                 (double) (WINDOW_HEIGHT / 3),
                                 (double) (WINDOW_WIDTH * 3 / 4),
                                 (double) (WINDOW_HEIGHT * 2 / 3),
                                 -1.0125, x);
    if (what == 12)
      x = ((double) (WINDOW_WIDTH)) - x;
  }
  else if ((what == 10) || (what == 13))
  {
    x = -20.0 + ((double) t) * 4.0;
    y = voter_parabola_formation((double) (WINDOW_WIDTH / 2),
                                 (double) (WINDOW_HEIGHT / 3),
                                 (double) (WINDOW_WIDTH * 3 / 4),
                                 (double) (WINDOW_HEIGHT * 2 / 3),
                                 -1.0025, x);
    if (what == 13)
      x = ((double) (WINDOW_WIDTH)) - x;
  }
  else if ((what == 11) || (what == 14))
  {
    x = -20.0 + ((double) t) * 4.0;
    y = voter_parabola_formation((double) (WINDOW_WIDTH / 2),
                                 (double) (WINDOW_HEIGHT / 3),
                                 (double) (WINDOW_WIDTH * 3 / 4),
                                 (double) (WINDOW_HEIGHT * 2 / 3),
                                 -0.9975, x);
    if (what == 14)
      x = ((double) (WINDOW_WIDTH)) - x;
  }
  else if ((what == 15) || (what == 16)
           || (what == 17) || (what == 18))
  {
    if (t < 96)
    {
      x = (double) (WINDOW_WIDTH / 2) - 60.0;
      y = -160.0 - 20.0 + ((double) t) * 5.0;
    }
    else if (t < 156)
    {
      x = (double) (WINDOW_WIDTH / 2) - 60.0 + ((double) (t - 96)) * 4.0;
      y = 300.0 - ((double) (t - 96)) * 3.0;
    }
    else
    {
      x = 500.0;
      y = 120.0 + ((double) (t - 156)) * 5.0;
    }

    if ((what == 17) || (what == 18))
      x -= 60.0;
    if ((what == 16) || (what == 18))
      x = ((double) (WINDOW_WIDTH)) - x;
  }
  else if ((what >= 19) && (what <= 22))
  {
    x = -420.0 + ((double) t) * 4.0;
    y = 60.0 - x * x / 240.0;

    if ((what == 19) || (what == 21))
    {
      theta = 30;
    }
    else
    {
      x *= -1.0;
      theta = 180 + 30;
    }
    if ((what == 21) || (what == 22))
    {
      x *= -1.0;
      theta *= -1;
    }
    
    temp[0] = x;
    temp[1] = y;
    temp_r[0] = temp[0];
    temp_r[1] = temp[1];
    vector_rotate(temp_r, temp, theta);

    x = temp_r[0] + 320.0;
    y = temp_r[1] + 240.0;
  }
  else
  {
    fprintf(stderr, "net_can_howl_position: should not reach here\n");
    x = 0.0;
    y = 0.0;
  }
  

  temp[0] = x - origin_x;
  temp[1] = y - origin_y;
  temp_r[0] = temp[0];
  temp_r[1] = temp[1];

  if ((what < 9) || (what > 14))
  {
    if (t_1 > 4200)
      vector_rotate(temp_r, temp, (t_1 - 4200) / 2);
  }
  
  x = temp_r[0];
  y = temp_r[1];

  result[0] = origin_x + x;
  result[1] = origin_y + y;

  return 0;
}

static tenm_object *
voter_new(int what, int t)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = 0.0;
  double y = 0.0;
  double result[2];

  result[0] = 0.0;
  result[1] = 0.0;
  if ((what >= 9) && (what <= 14))
    net_can_howl_position(result, what, 0, 0);
  else
    net_can_howl_position(result, what, t, t);
  x = result[0];
  y = result[1];

  /*
  x = ((double) (WINDOW_WIDTH / 2))
    + ((double) (WINDOW_WIDTH / 2)) * tenm_cos(-90 + 360 * (what - 1) / 7);
  y = ((double) (WINDOW_HEIGHT / 4))
    + ((double) (WINDOW_HEIGHT / 4)) * tenm_sin(-90 + 360 * (what - 1) / 7);
  */

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "voter_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_circle_new(x, y, 20.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "voter_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 6);
  if (count == NULL)
  {
    fprintf(stderr, "voter_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "voter_new: malloc(count_d) failed\n");
    if (count != NULL)
      free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] for deal_damage
   * [1] "damaged" timer
   * [2] move timer
   * [3] what
   * [4] shoot timer
   * [5] t_0 (for net_can_howl_position())
   */
  count[0] = 0;
  count[1] = 0;
  count[2] = -1;
  count[3] = what;
  if ((what >= 9) && (what <= 14))
  {
    count[4] = t;
    count[5] = 0;
  }
  else
  {
    count[4] = 0;
    count[5] = t;
  }

  /* list of count_d
   * [0] speed x
   * [1] speed y
   */
  count_d[0] = 0.0;
  count_d[1] = 0.0;

  new = tenm_object_new("Voter", ATTR_ENEMY,
                        ATTR_PLAYER_SHOT,
                        1, x, y,
                        6, count, 2, count_d, 1, p,
                        (int (*)(tenm_object *, double)) (&voter_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&voter_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&voter_act),
                        (int (*)(tenm_object *, int)) (&voter_draw));
  if (new == NULL)
  {
    fprintf(stderr, "voter_new: tenm_object_new failed\n");
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
voter_move(tenm_object *my, double turn_per_frame)
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
voter_hit(tenm_object *my, tenm_object *your)
{
  int n;

  /* sanity check */
  if (my == NULL)
    return 0;
  if (your == NULL)
    return 0;

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;

  deal_damage(my, your, 0);
  if (voter_green(my))
    add_chain(my, your);
  my->count[1] = 41;

  if (my->hit_point <= 0)
  {
    if (voter_green(my))
      n = 8;
    else
      n = 7;

    tenm_table_add(explosion_new(my->x, my->y,
                                 my->count_d[0] / 2.0, my->count_d[1]/ 2.0,
                                 2, 20, n, 3.0, 8));
    add_score(2);
    return 1;
  }

  return 0;
}

static int
voter_act(tenm_object *my, const tenm_object *player)
{
  double result[2];

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

  /* speed change */
  (my->count[2])++;

  result[0] = 0.0;
  result[1] = 0.0;
  net_can_howl_position(result, my->count[3],
                        my->count[5],
                        my->count[5] + my->count[2] + 1);
  my->count_d[0] = result[0] - my->x;
  my->count_d[1] = result[1] - my->y;

  /* escape */
  if ((my->count[3] == 9) || (my->count[3] == 12))
  {
    if (my->count[2] > 97)
      return 1;
  }
  else if ((my->count[3] == 10) || (my->count[3] == 13)
           || (my->count[3] == 11) || (my->count[3] == 14))
  {
    if (my->count[2] > 170)
      return 1;
  }
  else if ((my->count[3] >= 15) && (my->count[3] <= 18))
  {
    if (my->count[2] > 264)
      return 1;
  }
  else if ((my->count[3] >= 19) && (my->count[3] <= 22))
  {
    if (my->count[2] > 210)
      return 1;
  }

  /* shoot */
  (my->count[4])++;
  if ((((my->count[3] == 11) || (my->count[3] == 14))
       && (my->count[4] == 120))
      || (((my->count[3] >= 15) && (my->count[3] <= 22))
          && (my->count[4] % 60 == 0)))
    tenm_table_add(normal_shot_point_new(my->x, my->y, 4.0,
                                         player->x,
                                         player->y,
                                         3));

  return 0;
}

static int
voter_draw(tenm_object *my, int priority)
{
  tenm_color color;
  char temp[32];
  int status = 0;

  /* sanity check */
  if (my == NULL)
    return 0;

  if (priority != 0)
    return 0;

  /* body */
  if (voter_green(my))
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

  if (tenm_draw_circle((int) (my->x), (int) (my->y),
                       20, 3, color) != 0)
  {
    fprintf(stderr, "voter_draw: tenm_draw_circle failed\n");
    status = 1;
  }
  if (tenm_draw_line(((int) (my->x)) + 20, (int) (my->y),
                     (int) (my->x), ((int) (my->y) + 20),
                     3, color) != 0)
  {
    fprintf(stderr, "voter_draw: tenm_draw_line failed\n");
    status = 1;
  }
  
  /* hit point stat */
  if (my->count[1] > 0)
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string((int) my->x, (int) my->y, temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "voter_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

/* return 1 (true) or 0 (false) */
static int
voter_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (my->count == NULL)
    return 0;

  if ((my->count[3] == 9) || (my->count[3] == 12))
  {
    if ((my->count[4] >= 40) && (my->count[4] <= 60))
    {  
      return 1;
    }
  }
  else if (((my->count[3] == 10) || (my->count[3] == 11)
            || (my->count[3] == 13) || (my->count[3] == 14)))
  {
    if ((my->count[4] >= 120) && (my->count[4] <= 140))
    {
      return 1;
    }
  }
  else if ((my->count[3] >= 15) && (my->count[3] <= 18))
  {
    if (my->count[2] > 76)
      return 1;
  }  
  else if ((my->count[3] >= 19) && (my->count[3] <= 22))
  {
    if (my->count[2] > 75)
      return 1;
  } 

  return 0;
}

/* return y coordinate of the point (x, y) that is on the parabola
 * that passes (a_1, b_1) and (a_2, b_2)
 */
static double
voter_parabola_formation(double a_1, double b_1,
                                double a_2, double b_2,
                                double s, double x)
{
  double y = 0.0;

  /* sanity check */
  if ((s - (-1.0) > -NEAR_ZERO) && (s - (-1.0) < NEAR_ZERO))
    return 0.0;
  if ((a_1 - a_2 > -NEAR_ZERO) && (a_1 - a_2 < NEAR_ZERO))
    return 0.0;

  y = (s + 1.0) * (x - a_1) * (x - a_2);
  y += x * (b_2 - b_1) / (a_2 - a_1);
  y += (a_2 * b_1 - a_1 * b_2) / (a_2 - a_1);

  return y;
}

static tenm_object *
net_can_howl_more_1_new(void)
{
  tenm_object *new = NULL;
  int *count = NULL;

  count = (int *) malloc(sizeof(int) * 3);
  if (count == NULL)
  {
    fprintf(stderr, "net_can_howl_more_1_new: malloc(count) failed\n");
    return NULL;
  }

  /* list of count
   * [0] mode
   * [1] timer
   * [2] number of ball dead
   */
  count[0] = 0;
  count[1] = 0;
  count[2] = 0;

  /* ATTR_ENEMY is here only to delete it when the boss is dead */
  new = tenm_object_new("net can howl more 1", ATTR_ENEMY, 0,
                        1, 0.0, 0.0,
                        3, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double)) NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&net_can_howl_more_1_act),
                        (int (*)(tenm_object *, int)) NULL);
  if (new == NULL)
  {
    fprintf(stderr, "net_can_howl_more_1_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
net_can_howl_more_1_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int j;
  double x;
  double y;
  double dx;
  double dy;

  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  if (my->count[0] == 0)
  {
    if ((my->count[1] >= 80) && (my->count[1] < 140)
        && (my->count[1] % 5 == 0))
    {
      x = (double) (40 * ((my->count[1] - 70) / 5));
      dx = 0.5;
      for (i = 0; i < 2; i++)
      {
        tenm_table_add(normal_enemy_new(x, -19.0,
                                        BALL_SOLDIER, 0,
                                        0, my->table_index, 2, -1, 0,
                                        1, 1,
                                        /* move 0 */
                                        1000, dx, 4.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 0,
                                        /* shoot 0 */
                                        1000, 1000, 0, 0, 1, 0));
        x = ((double) (WINDOW_WIDTH)) - x;
        dx *= -1.0;
      }
    }

    if ((my->count[1] < 270) && (my->count[2] >= 24))
    {
      my->count[0] = 1;
    }

    if (my->count[1] > 350)
      return 1;
  }
  else if (my->count[0] == 1)
  {
    if (my->count[1] == 300)
    {
      x = -19.0;
      y = 80.0;
      dx = 4.0;
      dy = 2.0;
      for (i = 0; i < 2; i++)
      {
        for (j = 0; j < 2; j++)
        {
          tenm_table_add(normal_enemy_new(x, y,
                                          BALL_SOLDIER, 0,
                                          0, -1, 0, -1, 0, 1, 3,
                                          /* move 0 */
                                          1000, dx, dy, 0.0, 0.0,
                                          0.0, 0.0, 0.0, 0.0, 0,
                                          /* shoot 0 */
                                          80, 1000, 0, 0, 0, 1,
                                          /* shoot 1 */
                                          20, 1000, 0, 0, 1, 2,
                                          /* shoot 2 */
                                          1000, 1000, 0, 0, 0, 2));
          x = ((double) (WINDOW_WIDTH)) - x;
          dx *= -1.0;
        }
        y = ((double) (WINDOW_HEIGHT)) - y;
        dy *= -1.0;
      }
    }

    if (my->count[1] == 330)
    {
      x = -19.0;
      y = 80.0;
      dx = 6.0;
      dy = 3.0;
      for (i = 0; i < 2; i++)
      {
        for (j = 0; j < 2; j++)
        {
          tenm_table_add(normal_enemy_new(x, y,
                                          BALL_SOLDIER, 0,
                                          0, -1, 0, -1, 0, 1, 3,
                                          /* move 0 */
                                          1000, dx, dy, 0.0, 0.0,
                                          0.0, 0.0, 0.0, 0.0, 0,
                                          /* shoot 0 */
                                          50, 1000, 0, 0, 0, 1,
                                          /* shoot 1 */
                                          20, 1000, 0, 0, 1, 2,
                                          /* shoot 2 */
                                          1000, 1000, 0, 0, 0, 2));
          x = ((double) (WINDOW_WIDTH)) - x;
          dx *= -1.0;
        }
        y = ((double) (WINDOW_HEIGHT)) - y;
        dy *= -1.0;
      }
    }

    if (my->count[1] == 345)
    {
      x = -19.0;
      y = 80.0;
      dx = 8.0;
      dy = 4.0;
      for (i = 0; i < 2; i++)
      {
        for (j = 0; j < 2; j++)
        {
          tenm_table_add(normal_enemy_new(x, y,
                                          BALL_SOLDIER, 0,
                                          0, -1, 0, -1, 0, 1, 3,
                                          /* move 0 */
                                          1000, dx, dy, 0.0, 0.0,
                                          0.0, 0.0, 0.0, 0.0, 0,
                                          /* shoot 0 */
                                          35, 1000, 0, 0, 0, 1,
                                          /* shoot 1 */
                                          20, 1000, 0, 0, 1, 2,
                                          /* shoot 2 */
                                          1000, 1000, 0, 0, 0, 2));
          x = ((double) (WINDOW_WIDTH)) - x;
          dx *= -1.0;
        }
        y = ((double) (WINDOW_HEIGHT)) - y;
        dy *= -1.0;
      }
    }

    if (my->count[1] > 360)
      return 1;
  }

  (my->count[1])++;

  return 0;
}
