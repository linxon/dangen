/* $Id: plan-16.c,v 1.65 2005/06/24 14:20:03 oohara Exp $ */
/* [easy] cat tail */

#include <stdio.h>
/* malloc */
#include <stdlib.h>

#include "scheduler.h"
#include "tenm_table.h"
#include "cat-tail.h"
#include "stage-title.h"
#include "warning.h"
#include "normal-enemy.h"
#include "const.h"
#include "spellbook.h"

#include "plan-16.h"

static tenm_object *plan_16_more_1_new(void);
static int plan_16_more_1_act(tenm_object *my, const tenm_object *player);

int
plan_16(int t)
{
  /* sanity check */
  if (t < 0)
    return SCHEDULER_ERROR;

  if (t == 30)
    tenm_table_add(stage_title_new());

  if (t == 160)
    tenm_table_add(plan_16_more_1_new());

  return SCHEDULER_SUCCESS;
}

static tenm_object *
plan_16_more_1_new(void)
{
  tenm_object *new = NULL;
  int *count = NULL;

  count = (int *) malloc(sizeof(int) * 4);
  if (count == NULL)
  {
    fprintf(stderr, "plan_16_more_1_new: malloc(count) failed\n");
    return NULL;
  }

  /* list of count
   * [0] mode
   * [1] total timer
   * [2] number of enemies killed
   * [3] number of enemies killed in time
   */

  count[0] = 0;
  count[1] = -1;
  count[2] = 0;
  count[3] = 0;

  new = tenm_object_new("plan 16 more 1", 0, 0,
                        1, 0.0, 0.0,
                        3, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double))
                        NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&plan_16_more_1_act),
                        (int (*)(tenm_object *, int))
                        NULL);

  if (new == NULL)
  {
    fprintf(stderr, "plan_16_more_1_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
plan_16_more_1_act(tenm_object *my, const tenm_object *player)
{
  int t_shoot;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "plan_16_more_1_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[1])++;

  switch (my->count[0])
  {
  case 0:
    if ((my->count[1] >= 0) && (my->count[1] < 360)
        && (my->count[1] % 8 == 0))
    {
      if ((my->count[1] >= 0) && (my->count[1] < 288)
          && ((my->count[1] + 0) % 72 < 40))
      {
        if ((my->count[1] + 0) % 72 == 32)
          t_shoot = 37;
        else
          t_shoot = 9999;
        tenm_table_add(normal_enemy_new(-19.0, 30.0,
                                        BALL_SOLDIER, 0,
                                        0, -1, 0, -1, 0, 1, 1,
                                        /* move 0 */
                                        9999, 6.0, 0.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 0,
                                        /* shoot 0 */
                                        9999, t_shoot, my->count[1] % t_shoot,
                                        0, 1, 0));
      }
      if ((my->count[1] >= 48) && (my->count[1] < 336)
          && ((my->count[1] + 24) % 72 < 40))
      {
        if ((my->count[1] + 24) % 72 == 32)
          t_shoot = 37;
        else
          t_shoot = 9999;
        tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) + 19.0, 130.0,
                                        BALL_SOLDIER, 0,
                                        0, -1, 0, -1, 0, 1, 1,
                                        /* move 0 */
                                        9999, -6.0, 0.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 0,
                                        /* shoot 0 */
                                        9999, t_shoot, my->count[1] % t_shoot,
                                        0, 1, 0));
      }
      if ((my->count[1] >= 24) && (my->count[1] < 312)
          && ((my->count[1] + 48) % 72 < 40))
      {
        if ((my->count[1] + 48) % 72 == 32)
          t_shoot = 37;
        else
          t_shoot = 9999;
        tenm_table_add(normal_enemy_new(-19.0, 230.0,
                                        BALL_SOLDIER, 0,
                                        0, -1, 0, -1, 0, 1, 1,
                                        /* move 0 */
                                        9999, 6.0, 0.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 0,
                                        /* shoot 0 */
                                        9999, t_shoot, my->count[1] % t_shoot,
                                        0, 1, 0));
      }
    }
    break;
  case 1:
    if (my->count[1] == 0)
      tenm_table_add(spellbook_new(my->table_index));
    break;
  case 2:
    if ((my->count[1] >= 0) && (my->count[1] < 220)
        && (my->count[1] % 11 == 0) && (my->count[1] % 55 != 44))
    {
      if (my->count[1] % 33 == 0)
        t_shoot = 43;
      else
        t_shoot = 9999;

      tenm_table_add(normal_enemy_new(-19.0, 100.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999, 13.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, t_shoot, 33, 0, 1, 0));
      tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) + 19.0, 100.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999, -13.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, t_shoot, 33, 0, 1, 0));
    }
    break;
  case 3:
    if ((my->count[1] == 0) || (my->count[1] == 208))
      tenm_table_add(spellbook_new(my->table_index));
    break;
  case 4:
    if ((my->count[1] >= 0) && (my->count[1] < 540)
        && (my->count[1] % 10 == 0) && (my->count[1] % 90 < 40))
    {
      tenm_table_add(normal_enemy_new(35.0
                                      + ((double) ((my->count[1] % 90) / 10))
                                      * 120.0
                                      + ((double) ((my->count[1] % 360) / 90))
                                      * 70.0,
                                      -24.0,
                                      BALL_CAPTAIN, 0,
                                      0, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999, 0.0, 5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, 9999, 9958, 0, 1, 0));
    }
    break;
  case 5:
    if ((my->count[1] == 0) || (my->count[1] == 138) || (my->count[1] == 276))
      tenm_table_add(spellbook_new(my->table_index));
    break;
  case 6:
    if ((my->count[1] >= 0) && (my->count[1] < 220) && (my->count[1] % 5 == 0))
    {
      if (my->count[1] % 110 < 55)
        tenm_table_add(normal_enemy_new(100.0,
                                        -19.0,
                                        BALL_SOLDIER, 0,
                                        0, -1, 0, -1, 0, 1, 1,
                                        /* move 0 */
                                        9999, 6.0, 8.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 0,
                                        /* shoot 0 */
                                        9999, 9999, 0, 0, 1, 0));
      else
        tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) - 100.0,
                                        -19.0,
                                        BALL_SOLDIER, 0,
                                        0, -1, 0, -1, 0, 1, 1,
                                        /* move 0 */
                                        9999, -6.0, 8.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 0,
                                        /* shoot 0 */
                                        9999, 9999, 0, 0, 1, 0));
    }
    break;
  case 7:
    if (my->count[1] == 30)
      tenm_table_add(warning_new());
    if (my->count[1] == 160)
    {  
      tenm_table_add(cat_tail_new());
      return 1;
    }
    break;
  default:
    fprintf(stderr, "plan_16_more_1_act: strange mode when adding enemies "
            "(%d)\n", my->count[0]);
    break;
  }

  /* mode change */
  switch (my->count[0])
  {
  case 0:
    if (my->count[1] >= 450)
    { 
      my->count[0] = 1;
      my->count[1] = -1;
      my->count[2] = 0;
      my->count[3] = 0;
    }
    break;
  case 1:
    if (my->count[2] >= 1)
    {
      if (my->count[3] >= 1)
        my->count[0] = 2;
      else
        my->count[0] = 7;
      my->count[1] = -41;
      my->count[2] = 0;
      my->count[3] = 0;
    }
    break;
  case 2:
    if (my->count[1] >= 280)
    { 
      my->count[0] = 3;
      my->count[1] = -1;
      my->count[2] = 0;
      my->count[3] = 0;
    }
    break;
  case 3:
    if (my->count[2] >= 2)
    {
      if (my->count[3] >= 2)
        my->count[0] = 4;
      else
        my->count[0] = 7;
      my->count[1] = -41;
      my->count[2] = 0;
      my->count[3] = 0;
    }
    break;
  case 4:
    if (my->count[1] >= 680)
    { 
      my->count[0] = 5;
      my->count[1] = -1;
      my->count[2] = 0;
      my->count[3] = 0;
    }
    break;
  case 5:
    if (my->count[2] >= 3)
    {
      if (my->count[3] >= 3)
        my->count[0] = 6;
      else
        my->count[0] = 7;
      my->count[1] = -41;
      my->count[2] = 0;
      my->count[3] = 0;
    }
    break;
  case 6:
    if (my->count[1] >= 300)
    { 
      my->count[0] = 7;
      my->count[1] = -1;
      my->count[2] = 0;
      my->count[3] = 0;
    }
    break;
  case 7:
    break;
  default:
    fprintf(stderr, "plan_16_more_1_act: strange mode when changing mode "
            "(%d)\n", my->count[0]);
    break;
  }

  return 0;
}

