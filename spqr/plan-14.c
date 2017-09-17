/* $Id: plan-14.c,v 1.158 2005/06/26 14:30:00 oohara Exp $ */
/* [easy] Strikers 1341 */

#include <stdio.h>
/* malloc */
#include <stdlib.h>

#include "scheduler.h"
#include "tenm_table.h"
#include "strikers.h"
#include "stage-title.h"
#include "warning.h"
#include "normal-enemy.h"
#include "const.h"

#include "plan-14.h"

static tenm_object *plan_14_more_1_new(void);
static int plan_14_more_1_act(tenm_object *my, const tenm_object *player);

int
plan_14(int t)
{
  int i;
  int s;
  double x;

  /* sanity check */
  if (t < 0)
    return SCHEDULER_ERROR;

  if (t == 30)
    tenm_table_add(stage_title_new());

  if ((t >= 160) && (t < 370))
  {
    s = t - 160;
    if (s % 14 == 0)
    {
      x = ((double) (WINDOW_WIDTH)) - 40.0 - 2.0 * ((double) s);
      tenm_table_add(normal_enemy_new(x, -19.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 2,
                                      /* move 0 */
                                      9999, -2.0, 6.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      45 - (s % 42), 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 1, 1));
    }
    if ((s % 14 == 7) && (s % 42 < 35))
    {
      x = ((double) (WINDOW_WIDTH)) - 240.0 - 2.0 * ((double) ((s / 42) * 42))
        + 2.0 * ((double) (s % 42));
      tenm_table_add(normal_enemy_new(x, -19.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 2,
                                      /* move 0 */
                                      9999, 4.0 - 2.0, 6.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      45 - (s % 42), 37, s % 37, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 37, (45 - (s % 42) + s) % 37,
                                      0, 1, 1));
    }
  }

  if (t == 470)
  {
    for (i = -1; i <= 1; i++)
      tenm_table_add(normal_enemy_new(70.0 + 40.0 * ((double) i),
                                      -59.0 - 40.0 * ((double) i),
                                      BALL_SOLDIER, 0,
                                      70, -1, 0, -1, 0, 3, 2,
                                      /* move 0 */
                                      70, 0.0, 4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      170, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 5.0, -1.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      240, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 9989 + i * 5, 0, 1, 1));
    for (i = -1; i <= 1; i++)
      tenm_table_add(normal_enemy_new(230.0 + 40.0 * ((double) i),
                                      -59.0 + 40.0 * ((double) i),
                                      BALL_SOLDIER, 0,
                                      70, -1, 0, -1, 0, 3, 2,
                                      /* move 0 */
                                      70, 0.0, 4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      200, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999,
                                      1.1, 4.0,
                                      0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      270, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 9989 + i * 5, 0, 1, 1));
    tenm_table_add(normal_enemy_new(150.0, -19.0,
                                    BALL_CAPTAIN, 0,
                                    70, -1, 0, -1, 0, 3, 1,
                                    /* move 0 */
                                    70, 0.0, 4.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    90, 0.0, 0.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 2,
                                    /* move 2 */
                                    9999, -4.0, 0.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 2,
                                    /* shoot 0 */
                                    9999, 38, 0, 0, 1, 0));


    for (i = -1; i <= 1; i++)
      tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH)
                                      - (70.0 + 40.0 * ((double) i)),
                                      -59.0 - 40.0 * ((double) i),
                                      BALL_SOLDIER, 0,
                                      70, -1, 0, -1, 0, 3, 2,
                                      /* move 0 */
                                      70, 0.0, 4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      170, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, -5.0, -1.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      240, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 9989 + i * 5, 0, 1, 1));
    for (i = -1; i <= 1; i++)
      tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH)
                                      - (230.0 + 40.0 * ((double) i)),
                                      -59.0 + 40.0 * ((double) i),
                                      BALL_SOLDIER, 0,
                                      70, -1, 0, -1, 0, 3, 2,
                                      /* move 0 */
                                      70, 0.0, 4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      200, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999,
                                      -1.1, 4.0,
                                      0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      270, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 9989 + i * 5, 0, 1, 1));
    tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) - 150.0, -19.0,
                                    BALL_CAPTAIN, 0,
                                    70, -1, 0, -1, 0, 3, 1,
                                    /* move 0 */
                                    70, 0.0, 4.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    90, 0.0, 0.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 2,
                                    /* move 2 */
                                    9999, 4.0, 0.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 2,
                                    /* shoot 0 */
                                    9999, 38, 0, 0, 1, 0));

    for (i = -1; i <= 1; i++)
      tenm_table_add(normal_enemy_new(237.0 + 40.0 * ((double) i),
                                      -210.0 - 40.0 * ((double) i),
                                      BALL_SOLDIER, 0,
                                      70, -1, 0, -1, 0, 3, 2,
                                      /* move 0 */
                                      70, 0.0, 4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      140, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 4.0, 3.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      210, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 9989 + i * 5, 0, 1, 1));
    for (i = -1; i <= 1; i++)
      tenm_table_add(normal_enemy_new(403.0 + 40.0 * ((double) i),
                                      -210.0 + 40.0 * ((double) i),
                                      BALL_SOLDIER, 0,
                                      70, -1, 0, -1, 0, 3, 2,
                                      /* move 0 */
                                      70, 0.0, 4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      140, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, -4.0, 3.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      210, 9999, 0, 0, 0, 1,
                                      /* shoot 0 */
                                      9999, 9999, 9989 + i * 5, 0, 1, 1));
    tenm_table_add(normal_enemy_new(((double) (WINDOW_WIDTH / 2)), -170.0,
                                    BALL_CAPTAIN, 0,
                                    70, -1, 0, -1, 0, 3, 1,
                                    /* move 0 */
                                    70, 0.0, 4.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    90, 0.0, 0.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 2,
                                    /* move 2 */
                                    9999, 0.0, -4.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 2,
                                    /* shoot 0 */
                                    9999, 38, 19, 0, 1, 0));
  }

  if (t == 890)
    tenm_table_add(plan_14_more_1_new());

  return SCHEDULER_SUCCESS;
}

static tenm_object *
plan_14_more_1_new(void)
{
  tenm_object *new = NULL;
  int *count = NULL;

  count = (int *) malloc(sizeof(int) * 6);
  if (count == NULL)
  {
    fprintf(stderr, "plan_14_more_1_new: malloc(count) failed\n");
    return NULL;
  }

  /* list of count
   * [0] mode
   * [1 -- 2]  timer
   * [3] number of enemies killed
   * [4] number of right captains created
   * [5] number of right captains killed
   */
  count[0] = 0;
  count[1] = -1;
  count[2] = 0;
  count[3] = 0;
  count[4] = 0;
  count[5] = 0;

  new = tenm_object_new("plan 14 more 1", 0, 0,
                        1, 0.0, 0.0,
                        6, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double))
                        NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&plan_14_more_1_act),
                        (int (*)(tenm_object *, int))
                        NULL);
  if (new == NULL)
  {
    fprintf(stderr, "plan_14_more_1_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
plan_14_more_1_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int t_shoot;
  double dx;
  double dy;

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
    if (my->count[1] % 82 == 0)
    {
      if (my->count[1] < 328)
        tenm_table_add(normal_enemy_new(75.0, -24.0,
                                        BALL_CAPTAIN, 0,
                                        0, my->table_index, 3, -1, 0, 1, 1,
                                        /* move 0 */
                                        9999, 0.0, 5.2, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 0,
                                        /* shoot 0 */
                                        9999, 37, my->count[0] % 37, 0, 1, 0));
      if (my->count[1] < 492)
        tenm_table_add(normal_enemy_new(-24.0, 240.0,
                                        BALL_CAPTAIN, 0,
                                        0, my->table_index, 3, -1, 0, 1, 1,
                                        /* move 0 */
                                        9999, 4.8, 2.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 0,
                                        /* shoot 0 */
                                        9999, 37, my->count[0] % 37, 0, 1, 0));
    }

    if ((my->count[4] < 5) && (my->count[1] < 482)
        && (my->count[4] <= my->count[5]))
    {
      (my->count[2])++;
      if (my->count[2] >= 35)
      { 
        (my->count[4])++;
        my->count[2] = 0;

        tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) - 75.0, -24.0,
                                        BALL_CAPTAIN, 0,
                                        0, my->table_index, 5, -1, 0, 3, 2,
                                        /* move 0 */
                                        9, 0.0, 6.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 1,
                                        /* move 1 */
                                        627 - my->count[1], 0.0, 0.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 2,
                                        /* move 2 */
                                        9999, 0.0, -6.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 2,
                                        /* shoot 0 */
                                        636 - my->count[1],
                                        37, my->count[0] % 37, 0, 1, 1,
                                        /* shoot 1 */
                                        9999, 9999, 0, 0, 0, 1));
      }
    }

    if ((my->count[1] >= 328) && (my->count[1] < 376) && (my->count[5] >= 2)
        && ((my->count[1] - 328) % 12 == 0))
    {
      tenm_table_add(normal_enemy_new(-19.0, 0.0,
                                      BALL_SOLDIER, 0,
                                      0, my->table_index, 3, -1, 0, 1, 2,
                                      /* move 0 */
                                      9999, 3.6, 1.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      121 - (my->count[1] - 328),
                                      9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 1, 1));
      tenm_table_add(normal_enemy_new(-19.0, 170.0,
                                      BALL_SOLDIER, 0,
                                      0, my->table_index, 3, -1, 0, 1, 2,
                                      /* move 0 */
                                      9999, 3.6, 1.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      121 - (my->count[1] - 328),
                                      9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 1, 1));
    }
    if ((my->count[1] >= 384) && (my->count[1] < 420) && (my->count[5] >= 4)
        && ((my->count[1] - 384) % 6 == 0))
    {
      tenm_table_add(normal_enemy_new(-19.0, 85.0,
                                      BALL_SOLDIER, 0,
                                      0, my->table_index, 3, -1, 0, 1, 3,
                                      /* move 0 */
                                      9999, 7.2, 3.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      30, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      15, 9999, 0, 0, 1, 2,
                                      /* shoot 2 */
                                      9999, 9999, 0, 0, 0, 2));
    }
  }
  else if (my->count[0] == 1)
  {
    if (my->count[1] == 0)
    {
      tenm_table_add(normal_enemy_new(160.0, -23.0,
                                      BRICK, 0,
                                      0, -1, 0, -1, 0, 3, 2,
                                      /* move 0 */
                                      14, 0.0, 6.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      500, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, -6.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      514, 74, 37, 0, 1, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 0, 1));
      tenm_table_add(normal_enemy_new(320.0, -23.0,
                                      BRICK, 0,
                                      0, -1, 0, -1, 0, 3, 2,
                                      /* move 0 */
                                      14, 0.0, 6.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      500, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, -6.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      514, 74, 0, 0, 1, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 0, 1));
      tenm_table_add(normal_enemy_new(480.0, -23.0,
                                      BRICK, 0,
                                      0, -1, 0, -1, 0, 3, 2,
                                      /* move 0 */
                                      14, 0.0, 6.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      500, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, -6.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      514, 74, 37, 0, 1, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 0, 1));
    }

    if ((my->count[1] >= 0) && (my->count[1] < 144)
        && ((my->count[1] - 0) % 12 == 0))
    {
      tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) + 19.0, 0.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 3, 2,
                                      /* move 0 */
                                      176, -3.6, 1.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      44, 0.0, -6.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 3.6, 1.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      320 - my->count[1], 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 1, 1));
      tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) + 19.0, 170.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 3, 2,
                                      /* move 0 */
                                      176, -3.6, 1.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      44, 0.0, -6.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 3.6, 1.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      320 - my->count[1], 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 1, 1));
    }

    if ((my->count[1] >= 56) && (my->count[1] < 92)
        && ((my->count[1] - 56) % 6 == 0))
    {
      if (my->count[1] == 86)
        t_shoot = 15;
      else
        t_shoot = 9999;

      tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) + 19.0, 85.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 3,
                                      /* move 0 */
                                      9999, -7.2, 3.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      30, t_shoot, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      15, 9999, 0, 0, 1, 2,
                                      /* shoot 2 */
                                      9999, 9999, 0, 0, 0, 2));
    }

    if ((my->count[1] >= 260) && (my->count[1] < 296)
        && ((my->count[1] - 260) % 6 == 0))
    {
      if (my->count[1] == 290)
        t_shoot = 15;
      else
        t_shoot = 9999;

      tenm_table_add(normal_enemy_new(-17.6, 75.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 3,
                                      /* move 0 */
                                      9999, 7.2, 3.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      30, t_shoot, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      15, 9999, 0, 0, 1, 2,
                                      /* shoot 2 */
                                      9999, 9999, 0, 0, 0, 2));
    }
  }
  else if (my->count[0] == 2)
  {
    if (my->count[1] == 30)
    {      
      for (i = 6; i < 9; i++)
      {
        if (i % 2 == 0)
          t_shoot = 9959;
        else
          t_shoot = 0;

        tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) + 16.0
                                        + 35.0 * ((double) i),
                                        300.0 - 35.0 * ((double) i),
                                        BALL_SOLDIER, 0,
                                        106, -1, 0, -1, 0, 3, 3,
                                        /* move 0 */
                                        106, -6.0, 0.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 1,
                                        /* move 1 */
                                        136, 0.0, 0.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 2,
                                        /* move 2 */
                                        9999, 0.0 + 1.2, 3.0 + 0.5, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 2,
                                        /* shoot 0 */
                                        72 - i * 8, 9999, 0, 0, 0, 1,
                                        /* shoot 1 */
                                        48, 9999, t_shoot, 0, 1, 2,
                                        /* shoot 2 */
                                        24, 9999, 0, 0, 0, 1));
      }
    
      for (i = 0; i < 6; i++)
      {
        if (i % 2 == 0)
          t_shoot = 9959;
        else
          t_shoot = 0;

        if (i < 3)
        {
          dx = 3.0;
          dy = 0.0;
        }
        else
        {
          dx = 1.5;
          dy = 1.5;
        }
      
        dx += 1.2;
        dy += 0.5;

        tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) + 16.0
                                        + 35.0 * ((double) i),
                                        300.0 - 35.0 * ((double) i),
                                        BALL_SOLDIER, 0,
                                        106, -1, 0, -1, 0, 3, 3,
                                        /* move 0 */
                                        106, -6.0, 0.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 1,
                                        /* move 1 */
                                        136, 0.0, 0.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 2,
                                        /* move 2 */
                                        9999, dx, dy, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 2,
                                        /* shoot 0 */
                                        48 - i * 8, 9999, t_shoot + i * 8,
                                        0, 1, 1,
                                        /* shoot 1 */
                                        24, 9999, 0, 0, 0, 2,
                                        /* shoot 2 */
                                        48, 9999, t_shoot, 0, 1, 1));
      }
    
      for (i = 6; i < 9; i++)
      {
        if (i % 2 == 0)
          t_shoot = 9959;
        else
          t_shoot = 0;

        tenm_table_add(normal_enemy_new(-16.0 - 35.0 * ((double) i),
                                        300.0 - 35.0 * ((double) i),
                                        BALL_SOLDIER, 0,
                                        106, -1, 0, -1, 0, 3, 3,
                                        /* move 0 */
                                        106, 6.0, 0.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 1,
                                        /* move 1 */
                                        136, 0.0, 0.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 2,
                                        /* move 2 */
                                        9999, 0.0 - 1.2, 3.0 + 0.5, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 2,
                                        /* shoot 0 */
                                        72 - i * 8, 9999, 0, 0, 0, 1,
                                        /* shoot 1 */
                                        48, 9999, t_shoot, 0, 1, 2,
                                        /* shoot 2 */
                                        24, 9999, 0, 0, 0, 1));
      }
    
      for (i = 0; i < 6; i++)
      {
        if (i % 2 == 0)
          t_shoot = 9959;
        else
          t_shoot = 0;

        if (i < 3)
        {
          dx = -3.0;
          dy = 0.0;
        }
        else
        {
          dx = -1.5;
          dy = 1.5;
        }

        dx += -1.2;
        dy += 0.5;

        tenm_table_add(normal_enemy_new(-16.0 - 35.0 * ((double) i),
                                        300.0 - 35.0 * ((double) i),
                                        BALL_SOLDIER, 0,
                                        106, -1, 0, -1, 0, 3, 3,
                                        /* move 0 */
                                        106, 6.0, 0.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 1,
                                        /* move 1 */
                                        136, 0.0, 0.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 2,
                                        /* move 2 */
                                        9999, dx, dy, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 2,
                                        /* shoot 0 */
                                        48 - i * 8, 9999, t_shoot + i * 8,
                                        0, 1, 1,
                                        /* shoot 1 */
                                        24, 9999, 0, 0, 0, 2,
                                        /* shoot 2 */
                                        48, 9999, t_shoot, 0, 1, 1));
      }
    }
  }
  else if (my->count[0] == 3)
  {
    if (my->count[1] == 0)
      tenm_table_add(warning_new());

    if (my->count[1] == 130)
    { 
      tenm_table_add(strikers_new());
      return 1;
    }
  }
  
  if (my->count[0] == 0)
  {
    if (my->count[1] >= 642)
    {
      if ((my->count[3] >= 24) && (my->count[5] >= 5))
        my->count[0] = 1;
      else
        my->count[0] = 2;

      my->count[1] = -1;
      my->count[2] = 0;
      my->count[3] = 0;
      my->count[4] = 0;
      my->count[5] = 0;
    }
  }
  else if (my->count[0] == 1)
  {
    if (my->count[1] >= 544)
    {
      my->count[0] = 2;

      my->count[1] = -1;
      my->count[2] = 0;
      my->count[3] = 0;
      my->count[4] = 0;
      my->count[5] = 0;
    }
  }
  else if (my->count[0] == 2)
  {
    if (my->count[1] >= 522)
    {
      my->count[0] = 3;

      my->count[1] = -1;
      my->count[2] = 0;
      my->count[3] = 0;
      my->count[4] = 0;
      my->count[5] = 0;
    }
  }

  return 0;
}
