/* $Id: plan-11.c,v 1.117 2004/12/15 12:13:51 oohara Exp $ */

#include <stdio.h>
/* malloc */
#include <stdlib.h>

#include "scheduler.h"
#include "tenm_table.h"
#include "theorem-weapon.h"
#include "warning.h"
#include "stage-title.h"
#include "normal-enemy.h"
#include "const.h"
#include "tenm_math.h"
#include "wall-11.h"

#include "plan-11.h"

static tenm_object *plan_11_more_1_new(int what);
static int plan_11_more_1_act(tenm_object *my, const tenm_object *player);
static tenm_object *plan_11_more_2_new(void);
static int plan_11_more_2_act(tenm_object *my, const tenm_object *player);

int
plan_11(int t)
{
  int s;
  int what;
  double x;

  /* sanity check */
  if (t < 0)
    return SCHEDULER_ERROR;

  if (t == 30)
    tenm_table_add(stage_title_new());

  if ((t >= 160) && (t <= 185))
  {
    s = t - 160;

    if (s == 0)
      tenm_table_add(wall_11_new((double) (WINDOW_WIDTH / 2), -29.0,
                                 7.0, 90, 7));
    if (s == 25)
    {
      tenm_table_add(plan_11_more_1_new(0));
      tenm_table_add(plan_11_more_1_new(1));
    }
    if (s == 10)
      tenm_table_add(normal_enemy_new((double) (WINDOW_WIDTH / 2), -23.0,
                                      BRICK, ENEMY_TYPE_OBSTACLE,
                                      0, -1, 0, -1, 0, 3, 1,
                                      /* move 0 */
                                      24, 0.0, 5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      130, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, 5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      9999, 9999, 0, 0, 1, 0));
    
  }

  if ((t >= 340) && (t <= 616))
  {
    s = t - 340;

    if (s == 0)
    { 
      tenm_table_add(normal_enemy_new((double) (WINDOW_WIDTH / 2), -23.0,
                                      BRICK, 0,
                                      0, -1, 0, -1, 0, 3, 3,
                                      /* move 0 */
                                      24, 0.0, 5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      352, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, 3.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      336, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      30, 15, 0, 0, 1, 2,
                                      /* shoot 2 */
                                      9999, 9999, 0, 0, 0, 2));
    }
    if (s == 12)
    { 
      tenm_table_add(normal_enemy_new((double) (WINDOW_WIDTH / 2) + 45.0,
                                      -24.0,
                                      SQUARE, 0,
                                      0, -1, 0, -1, 0, 3, 3,
                                      /* move 0 */
                                      12, 0.0, 5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      352, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, 3.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      168, 31, 0, 65, 0, 1,
                                      /* shoot 1 */
                                      196, 9999, 0, 65, 0, 2,
                                      /* shoot 2 */
                                      9999, 31, 0, 65, 1, 2));
      tenm_table_add(normal_enemy_new((double) (WINDOW_WIDTH / 2) - 45.0,
                                      -24.0,
                                      SQUARE, 0,
                                      0, -1, 0, -1, 0, 3, 3,
                                      /* move 0 */
                                      12, 0.0, 5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      352, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, 3.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      168, 31, 0, 115, 0, 1,
                                      /* shoot 1 */
                                      196, 9999, 0, 115, 0, 2,
                                      /* shoot 1 */
                                      9999, 31, 0, 115, 1, 2));
    }

    if (s == 150)
    { 
      tenm_table_add(normal_enemy_new(70.0 + 45.0,
                                      -24.0,
                                      SQUARE, 0,
                                      0, -1, 0, -1, 0, 3, 4,
                                      /* move 0 */
                                      36, 0.0, 5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      190, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, 3.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      66, 9999, 0, 135, 0, 1,
                                      /* shoot 1 */
                                      30, 29, 0, 135, 0, 2,
                                      /* shoot 2 */
                                      90, 29, 1, 135, 1, 3,
                                      /* shoot 3 */
                                      9999, 29, 4, 135, 0, 3));
      tenm_table_add(normal_enemy_new(70.0 - 45.0,
                                      -24.0,
                                      SQUARE, 0,
                                      0, -1, 0, -1, 0, 3, 4,
                                      /* move 0 */
                                      36, 0.0, 5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      190, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, 3.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      66, 9999, 0, 45, 0, 1,
                                      /* shoot 1 */
                                      30, 29, 0, 45, 0, 2,
                                      /* shoot 2 */
                                      90, 29, 1, 45, 1, 3,
                                      /* shoot 3 */
                                      9999, 29, 4, 45, 0, 3));

      tenm_table_add(normal_enemy_new(570.0 + 45.0,
                                      -24.0,
                                      SQUARE, 0,
                                      0, -1, 0, -1, 0, 3, 3,
                                      /* move 0 */
                                      36, 0.0, 5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      190, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, 3.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      66, 9999, 0, 135, 0, 1,
                                      /* shoot 1 */
                                      160, 29, 0, 135, 0, 2,
                                      /* shoot 2 */
                                      9999, 29, 5, 135, 1, 2));
      tenm_table_add(normal_enemy_new(570.0 - 45.0,
                                      -24.0,
                                      SQUARE, 0,
                                      0, -1, 0, -1, 0, 3, 3,
                                      /* move 0 */
                                      36, 0.0, 5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      190, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, 3.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      66, 9999, 0, 45, 0, 1,
                                      /* shoot 1 */
                                      160, 29, 0, 45, 0, 2,
                                      /* shoot 2 */
                                      9999, 29, 5, 45, 1, 2));
    }
    if (s == 174)
    { 
      tenm_table_add(normal_enemy_new(70.0,
                                      -23.0,
                                      BRICK, 0,
                                      0, -1, 0, -1, 0, 3, 3,
                                      /* move 0 */
                                      12, 0.0, 5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      190, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, 3.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      72, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      90, 23, 0, 0, 1, 2,
                                      /* shoot 2 */
                                      9999, 9999, 0, 0, 0, 2));

      tenm_table_add(normal_enemy_new(570.0,
                                      -23.0,
                                      BRICK, 0,
                                      0, -1, 0, -1, 0, 3, 3,
                                      /* move 0 */
                                      12, 0.0, 5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      190, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, 3.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      12, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      30, 15, 0, 0, 1, 2,
                                      /* shoot 2 */
                                      9999, 9999, 0, 0, 0, 2));
    }

    if (s == 176)
      tenm_table_add(wall_11_new(195.0, -29.0, 7.0, 90, 7));
    if (s == 276)
      tenm_table_add(wall_11_new(445.0, -29.0, 7.0, 90, 7));
  }

  if ((t >= 860) && (t <= 1028))
  {
    s = t - 860;

    if ((s >= 0) && (s < 168) && (s % 7 == 0))
    {
      if (s < 42)
        x = 128.0;
      else if (s < 84)
        x = 384.0;
      else if (s < 126)
        x = 256.0;
      else
        x = 512.0;

      switch (s % 42)
      {
      case 0:
        tenm_table_add(wall_11_new(x, -29.0, 7.0, 90, 7));
        break;
      case 14:
      case 21:
      case 28:
      case 35:
        if (s % 42 == 14)
        {
          what = BALL_CAPTAIN;
        }
        else
        {
          what = BALL_SOLDIER;
        }
        tenm_table_add(normal_enemy_new(x, -19.0,
                                        what, 0,
                                        0, -1, 0, -1, 0, 1, 1,
                                        /* move 0 */
                                        9999, 0.0, 7.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 0,
                                        /* shoot 0 */
                                        9999, 28, 0, 0, 1, 0));
        break;
      default:
        break;
      }
    }
  }

  if (t == 1110)
    tenm_table_add(plan_11_more_2_new());

  if ((t >= 1640) && (t <= 1830))
  {
    s = t - 1640;

    if (s == 0)
    { 
      tenm_table_add(normal_enemy_new(210.0, -42.0,
                                      TRIANGLE, ENEMY_TYPE_WEAK,
                                      0, -1, 0, -1, 0, 2, 1,
                                      /* move 0 */
                                      24, 0.0, 5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      9999, 0.0, 1.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* shoot 0 */
                                      9999, 67, 0, 0, 1, 0));
      tenm_table_add(normal_enemy_new(430.0, -42.0,
                                      TRIANGLE, ENEMY_TYPE_WEAK,
                                      0, -1, 0, -1, 0, 2, 1,
                                      /* move 0 */
                                      24, 0.0, 5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      9999, 0.0, 1.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* shoot 0 */
                                      9999, 67, 0, 0, 1, 0));
    }

    if ((s >= 160) && (s <= 190) && (s % 10 == 0))
    {
      if (s % 20 == 0)
        tenm_table_add(wall_11_new(-29.0, (double) ((s - 160) * 8),
                                   7.0, 15, 7));
      else
        tenm_table_add(wall_11_new(669.0, (double) ((s - 160) * 8),
                                   7.0, 155, 7));
    }
  }

  if (t == 2090)
    tenm_table_add(warning_new());
  if (t == 2220)
    tenm_table_add(theorem_weapon_new());

  return SCHEDULER_SUCCESS;
}

static tenm_object *
plan_11_more_1_new(int what)
{
  tenm_object *new = NULL;
  int *count = NULL;

  /* sanity check */
  if ((what < 0) || (what > 1))
  {
    fprintf(stderr, "plan_11_more_1_new: strange what (%d)\n", what);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 4);
  if (count == NULL)
  {
    fprintf(stderr, "plan_11_more_1_new: malloc(count) failed\n");
    return NULL;
  }

  /* list of count
   * [0] mode
   * [1] timer
   * [2] what
   * [3] number of enemies killed
   */
  count[0] = 0;
  count[1] = -1;
  count[2] = what;
  count[3] = 0;

  new = tenm_object_new("plan 11 more 1", 0, 0,
                        1, 0.0, 0.0,
                        4, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double))
                        NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&plan_11_more_1_act),
                        (int (*)(tenm_object *, int))
                        NULL);
  if (new == NULL)
  {
    fprintf(stderr, "plan_11_more_1_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
plan_11_more_1_act(tenm_object *my, const tenm_object *player)
{
  double x;
  double dx;
  int t_shoot;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "plan_11_more_1_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[1])++;

  if (my->count[0] == 0)
  {
    if ((my->count[1] >= 0) && (my->count[1] <= 40)
        && (my->count[1] % 20 == 0))
    {
      x = ((double) WINDOW_WIDTH) + 19.0;
      dx = -5.0;
      if (my->count[2] != 0)
      {
        x = ((double) WINDOW_WIDTH) - x;
        dx *= -1.0;
      }
      tenm_table_add(normal_enemy_new(x, 97.0,
                                      BALL_SOLDIER, ENEMY_TYPE_WEAK,
                                      0, my->table_index, 3, -1, 0, 1, 2,
                                      /* move 0 */
                                      9999, dx, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      60 - my->count[1], 17, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 17, (60 - my->count[1]) % 17,
                                      0, 1, 1));
    }

    if ((my->count[1] >= 100) && (my->count[1] <= 125)
        && ((my->count[1] - 100) % 5 == 0))
    {
      if ((my->count[1] - 100) % 10 == 0)
        x = 105.0;
      else
        x = 3.0;
      if (my->count[2] != 0)
        x = ((double) WINDOW_WIDTH) - x;
      if ((my->count[1] - 100 == 5) || (my->count[1] - 100 == 20))
        t_shoot = 23;
      else
        t_shoot = 9999;
      tenm_table_add(normal_enemy_new(x, -19.0,
                                      BALL_SOLDIER, 0,
                                      0, my->table_index, 3, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999, 0.0, 5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, t_shoot, 0, 0, 1, 0));
    }

    if ((my->count[1] <= 230) && (my->count[3] >= 9))
    {
      my->count[0] = 1;
      my->count[1] = -1;
    }
    else if (my->count[1] > 230)
    { 
      return 1;
    }
  }
  else  if (my->count[0] == 1)
  {
    if ((my->count[1] >= 50) && (my->count[1] <= 70)
        && (my->count[1] % 5 == 0))
    {
      x = 44.0;
      if (my->count[2] != 0)
        x = ((double) WINDOW_WIDTH) - x;
      tenm_table_add(normal_enemy_new(x, -19.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999, 0.0, 10.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, 9999, 0, 0, 1, 0));
    }

    if (my->count[1] > 70)
      return 1;
  }

  return 0;
}

static tenm_object *
plan_11_more_2_new(void)
{
  tenm_object *new = NULL;
  int *count = NULL;

  count = (int *) malloc(sizeof(int) * 4);
  if (count == NULL)
  {
    fprintf(stderr, "plan_11_more_2_new: malloc(count) failed\n");
    return NULL;
  }

  /* list of count
   * [0] mode
   * [1] timer
   * [2] number of enemies killed
   * [3] total timer
   */
  count[0] = 0;
  count[1] = -1;
  count[2] = 0;
  count[3] = -1;

  new = tenm_object_new("plan 11 more 2", 0, 0,
                        1, 0.0, 0.0,
                        4, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double))
                        NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&plan_11_more_2_act),
                        (int (*)(tenm_object *, int))
                        NULL);
  if (new == NULL)
  {
    fprintf(stderr, "plan_11_more_2_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
plan_11_more_2_act(tenm_object *my, const tenm_object *player)
{
  double dx;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "plan_11_more_2_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[1])++;
  (my->count[3])++;

  if (my->count[3] > 430)
    return 1;

  if ((my->count[0] == 0) || (my->count[0] == 1))
  {
    if ((my->count[1] == 0) && (my->count[3] <= 349))
    {
      tenm_table_add(normal_enemy_new((double) (WINDOW_WIDTH / 2), -42.0,
                                      TRIANGLE, 0,
                                      0, my->table_index, 2, -1, 0, 3, 3,
                                      /* move 0 */
                                      31, 0.0, 5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      359 - my->count[3], 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, -5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      31, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      359 - my->count[3], 43, 31, 0, 1, 2,
                                      /* shoot 2 */
                                      9999, 9999, 0, 0, 0, 2));

      tenm_table_add(normal_enemy_new(25.0, -24.0,
                                      BALL_CAPTAIN, 0,
                                      0, my->table_index, 2, -1, 0, 3, 2,
                                      /* move 0 */
                                      31, 0.0, 5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      359 - my->count[3], 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, -5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      390 - my->count[3], 17, 0, 0, 1, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 0, 1));
      tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) - 25.0, -24.0,
                                      BALL_CAPTAIN, 0,
                                      0, my->table_index, 2, -1, 0, 3, 2,
                                      /* move 0 */
                                      31, 0.0, 5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      359 - my->count[3], 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, -5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      390 - my->count[3], 17, 0, 0, 1, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 0, 1));
    }

    if (my->count[2] >= 3)
    {
      if (my->count[0] == 0)
      {
        if (my->count[3] < 349)
        { 
          my->count[0] = 1;
          my->count[1] = -31;
          my->count[2] = 0;
        }
      }
      else if (my->count[0] == 1)
      {
        if (my->count[3] < 390)
        {
          if (my->count[3] < 285)
            my->count[0] = 2;
          else if (my->count[3] < 300)
            my->count[0] = 3;
          else
            my->count[0] = 4;
          my->count[1] = -1;
          my->count[2] = 0;
        }
      }
    }
  }
  else if (my->count[0] == 2)
  {
    if ((my->count[3] < 390) && (my->count[1] % 5 == 0))
    {
      if (my->count[1] % 50 < 25)
        dx = 8.0;
      else
        dx = -8.0;
      tenm_table_add(normal_enemy_new((double) (WINDOW_WIDTH / 2), -19.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 3, 2,
                                      /* move 0 */
                                      24, 0.0, 5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      10, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, dx, 0.8, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      34, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 1, 1));
    }
    if (my->count[3] == 290)
      tenm_table_add(wall_11_new(669.0, 260.0, 7.0, 180, 7));
    if (my->count[3] == 320)
      tenm_table_add(wall_11_new(-29.0, 360.0, 7.0, 0, 7));
  }
  else if (my->count[0] == 3)
  {
    if ((my->count[3] < 390) && (my->count[1] % 7 == 0))
    {
      if (my->count[1] % 56 < 28)
        dx = 6.0;
      else
        dx = -6.0;
      tenm_table_add(normal_enemy_new((double) (WINDOW_WIDTH / 2), -19.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 3, 2,
                                      /* move 0 */
                                      24, 0.0, 5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      10, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, dx, 0.6, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      34, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 1, 1));
    }
    if (my->count[3] == 320)
      tenm_table_add(wall_11_new(-29.0, 260.0, 7.0, 0, 7));
  }
  else if (my->count[0] == 4)
  {
    if ((my->count[3] < 390) && (my->count[1] % 8 == 0)
        && (my->count[1] % 32 < 24))
    {
      if (my->count[1] % 64 < 32)
        dx = 5.0;
      else
        dx = -5.0;
      tenm_table_add(normal_enemy_new((double) (WINDOW_WIDTH / 2), -19.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 3, 2,
                                      /* move 0 */
                                      24, 0.0, 5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      10, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, dx, 0.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      34, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 1, 1));
    }
  }

  return 0;
}
