/* $Id: plan-7.c,v 1.96 2004/12/01 12:55:27 oohara Exp $ */
/* [hard] cat tail (grep mix) */

#include <stdio.h>
/* malloc */
#include <stdlib.h>

#include "scheduler.h"
#include "tenm_table.h"
#include "cat-tail-grep.h"
#include "warning.h"
#include "stage-title.h"
#include "normal-enemy.h"
#include "const.h"
#include "tenm_math.h"

#include "plan-7.h"

static tenm_object *plan_7_more_1_new(void);
static int plan_7_more_1_act(tenm_object *my, const tenm_object *player);

int
plan_7(int t)
{
  int i;
  int s;
  double x;
  double c;

  /* sanity check */
  if (t < 0)
    return SCHEDULER_ERROR;

  if (t == 30)
    tenm_table_add(stage_title_new());

  if (t == 160)
  {
    for (i = 0; i < 5; i++)
      tenm_table_add(normal_enemy_new(120.0 + 100.0 * ((double) i), -23.0,
                                      BRICK, 0,
                                      0, -1, 0, -1, 0, 3, 1,
                                      /* move 0 */
                                      34, 0.0, 6.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      96, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, 6.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 1 */
                                      9999, 10, (i * 6) % 10, 50, 1, 0));
  }

  if ((t >= 250) && (t < 850))
  {
    s = t - 250;
    if ((s % 10 == 0) && (s % 60 < 30))
    {
      if (s % 120 < 60)
      {
        x = ((double) (WINDOW_WIDTH)) + 13.0;
        c = 1.0;
      }
      else
      {
        x = -13.0;
        c = -1.0;
      }
      tenm_table_add(normal_enemy_new(x, -13.0,
                                      BALL_SOLDIER, 0,
                                      150, -1, 0, -1, 0, 3, 3,
                                      /* move 0 */
                                      30, -0.8 * c, 5.6, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      60, -5.6 * c, -0.8, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      60, -0.8 * c, 5.6, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* shoot 0 */
                                      29, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      60, 30, 29, 0, 1, 2,
                                      /* shoot 2 */
                                      60, 30, 29, 0, 0, 1));
      tenm_table_add(normal_enemy_new(x, -13.0,
                                      BALL_SOLDIER, 0,
                                      150, -1, 0, -1, 0, 3, 3,
                                      /* move 0 */
                                      30, -5.6 * c, -0.8, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      60, -0.8 * c, 5.6, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      60, -5.6 * c, -0.8, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* shoot 0 */
                                      29, 9999, 0, 0, 1, 1,
                                      /* shoot 1 */
                                      60, 30, 29, 0, 0, 2,
                                      /* shoot 2 */
                                      60, 30, 29, 0, 1, 1));
    }
  }
  
  if (t == 1050)
    tenm_table_add(plan_7_more_1_new());

  return SCHEDULER_SUCCESS;
}

static tenm_object *
plan_7_more_1_new(void)
{
  tenm_object *new = NULL;
  int *count = NULL;

  count = (int *) malloc(sizeof(int) * 3);
  if (count == NULL)
  {
    fprintf(stderr, "plan_7_more_1_new: malloc(count) failed\n");
    return NULL;
  }

  /* list of count
   * [0] mode
   * [1] timer
   * [2] number of enemies killed / escaped
   */
  count[0] = 0;
  count[1] = -1;
  count[2] = 0;

  new = tenm_object_new("plan 7 more 1", 0, 0,
                        1, 0.0, 0.0,
                        3, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double))
                        NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&plan_7_more_1_act),
                        (int (*)(tenm_object *, int))
                        NULL);

  if (new == NULL)
  {
    fprintf(stderr, "plan_7_more_1_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
plan_7_more_1_act(tenm_object *my, const tenm_object *player)
{
  int i;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "plan_7_more_1_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[1])++;
  if ((my->count[0] == 0) && (my->count[1] == 0))
  {
    for (i = -9; i <= 9; i += 2)
    {
      if ((i + 9) % 4 < 2)
        tenm_table_add(normal_enemy_new(32.0, -23.0,
                                        BRICK, 0,
                                        0, my->table_index, 2,
                                        my->table_index, 2, 6, 10,
                                        /* move 0 */
                                        48,
                                        0.0, 5.5 + 0.5 * ((double) i),
                                        0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 1,
                                        /* move 1 */
                                        144,
                                        4.0, 0.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 2,
                                        /* move 2 */
                                        48,
                                        0.0, -1.0 - 1.0 * ((double) i),
                                        0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 3,
                                        /* move 3 */
                                        96,
                                        -6.0, 0.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 4,
                                        /* move 4 */
                                        48,
                                        0.0, 1.0 + 1.0 * ((double) i),
                                        0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 5,
                                        /* move 5 */
                                        9999,
                                        6.0, 0.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 5,
                                        /* shoot 0 */
                                        48, 100, 5 * i, 0, 0, 1,
                                        /* shoot 1 */
                                        46, 100, (5 * i + 48) % 100,
                                        0, 1, 2,
                                        /* shoot 2 */
                                        24, 100, (5 * i + 94) % 100,
                                        0, 0, 3,
                                        /* shoot 3 */
                                        74, 100, (5 * i + 118) % 100,
                                        0, 1, 4,
                                        /* shoot 4 */
                                        48, 100, (5 * i + 192) % 100,
                                        0, 0, 5,
                                        /* shoot 5 */
                                        20, 100, (5 * i + 240) % 100,
                                        0, 1, 6,
                                        /* shoot 6 */
                                        16, 100, (5 * i + 260) % 100,
                                        0, 0, 7,
                                        /* shoot 7 */
                                        60, 100, (5 * i + 276) % 100,
                                        0, 1, 8,
                                        /* shoot 8 */
                                        48, 100, (5 * i + 336) % 100,
                                        0, 0, 9,
                                        /* shoot 9 */
                                        9999, 100, (5 * i + 384) % 100,
                                        0, 1, 9));
      else
        tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) - 32.0,
                                        -23.0,
                                        BRICK, 0,
                                        0, my->table_index, 2,
                                        my->table_index, 2, 6, 10,
                                        /* move 0 */
                                        48,
                                        0.0, 5.5 + 0.5 * ((double) i),
                                        0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 1,
                                        /* move 1 */
                                        96,
                                        -6.0, 0.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 2,
                                        /* move 2 */
                                        48,
                                        0.0, 1.0 - 1.0 * ((double) i),
                                        0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 3,
                                        /* move 3 */
                                        144,
                                        4.0, 0.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 4,
                                        /* move 4 */
                                        48,
                                        0.0, -1.0 + 1.0 * ((double) i),
                                        0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 5,
                                        /* move 5 */
                                        9999,
                                        -6.0, 0.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 5,
                                        /* shoot 0 */
                                        48, 100, 5 * i, 0, 0, 1,
                                        /* shoot 1 */
                                        50, 100, (5 * i + 48) % 100,
                                        0, 1, 2,
                                        /* shoot 2 */
                                        16, 100, (5 * i + 98) % 100,
                                        0, 0, 3,
                                        /* shoot 3 */
                                        30, 100, (5 * i + 114) % 100,
                                        0, 1, 4,
                                        /* shoot 4 */
                                        48, 100, (5 * i + 144) % 100,
                                        0, 0, 5,
                                        /* shoot 5 */
                                        64, 100, (5 * i + 192) % 100,
                                        0, 1, 6,
                                        /* shoot 6 */
                                        24, 100, (5 * i + 256) % 100,
                                        0, 0, 7,
                                        /* shoot 7 */
                                        56, 100, (5 * i + 280) % 100,
                                        0, 1, 8,
                                        /* shoot 8 */
                                        48, 100, (5 * i + 336) % 100,
                                        0, 0, 9,
                                        /* shoot 9 */
                                        9999, 100, (5 * i + 384) % 100,
                                        0, 1, 9));
    }
  }

  if ((my->count[0] == 0) && ((my->count[2] >= 10) || (my->count[1] >= 9999)))
  {
    my->count[0] = 1;
    my->count[1] = 0;
  }
  if (my->count[0] == 1)
  {
    if (my->count[1] == 100)
      tenm_table_add(warning_new());

    if (my->count[1] == 230)
    { 
      tenm_table_add(cat_tail_grep_new());
      return 1;
    }
  }

  return 0;
}
