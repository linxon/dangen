/* $Id: plan-20.c,v 1.30 2005/06/26 18:54:32 oohara Exp $ */
/* [easiest] P-can */

#include <stdio.h>
/* malloc */
#include <stdlib.h>

#include "scheduler.h"
#include "tenm_table.h"
#include "p-can.h"
#include "stage-title.h"
#include "warning.h"
#include "normal-enemy.h"
#include "const.h"

#include "plan-20.h"

static tenm_object *plan_20_more_1_new(void);
static int plan_20_more_1_act(tenm_object *my, const tenm_object *player);

int
plan_20(int t)
{
  /* sanity check */
  if (t < 0)
    return SCHEDULER_ERROR;

  if (t == 30)
    tenm_table_add(stage_title_new());

  if (t == 160)
    tenm_table_add(plan_20_more_1_new());

  /*
  if (t == 160)
    tenm_table_add(warning_new());

  if (t == 290)
    tenm_table_add(p_can_new());
  */

  return SCHEDULER_SUCCESS;
}

static tenm_object *
plan_20_more_1_new(void)
{
  tenm_object *new = NULL;
  int *count = NULL;

  count = (int *) malloc(sizeof(int) * 6);
  if (count == NULL)
  {
    fprintf(stderr, "plan_20_more_1_new: malloc(count) failed\n");
    return NULL;
  }

  /* list of count
   * [0] total timer
   * [1] mode
   * [2] timer
   * [3] number of enemies killed/escaped
   */
  count[0] = -1;
  count[1] = 0;
  count[2] = -1;
  count[3] = 0;

  new = tenm_object_new("plan 20 more 1", 0, 0,
                        1, 0.0, 0.0,
                        6, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double))
                        NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&plan_20_more_1_act),
                        (int (*)(tenm_object *, int))
                        NULL);
  if (new == NULL)
  {
    fprintf(stderr, "plan_20_more_1_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
plan_20_more_1_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int t_shoot;
  double x;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "plan_20_more_1_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[0])++;
  (my->count[2])++;

  switch(my->count[1])
  {
  case 0:
  case 2:
  case 4:
    if (my->count[2] == 0)
    {
      if (my->count[1] == 0)
        x = 240.0;
      else
        x = 50.0;
      tenm_table_add(normal_enemy_new(x, -24.0,
                                      BALL_CAPTAIN, 0,
                                      0, my->table_index, 3,
                                      my->table_index, 3,
                                      3, 1,
                                      /* move 0 */
                                      20, 0.0, 4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      150, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, -4.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      9999, 9999, 0, 0, 1, 0));
    }
    if (my->count[3] >= 1)
    {
      (my->count[1])++;
      my->count[2] = -1;
      my->count[3] = 0;
    }
    break;
  case 1:
  case 3:
  case 5:
    if (my->count[2] == 0)
    {
      if (my->count[1] == 1)
        x = ((double) WINDOW_WIDTH) - 240.0;
      else
        x = ((double) WINDOW_WIDTH) - 50.0;
      tenm_table_add(normal_enemy_new(x, -24.0,
                                      BALL_CAPTAIN, 0,
                                      0, my->table_index, 3,
                                      my->table_index, 3,
                                      3, 1,
                                      /* move 0 */
                                      20, 0.0, 4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      150, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 4.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      9999, 9999, 0, 0, 1, 0));
    }
    if (my->count[3] >= 1)
    {
      if ((my->count[1] == 5) && (my->count[0] >= 700))
        my->count[1] = 7;
      else
        (my->count[1])++;
      my->count[2] = -1;
      my->count[3] = 0;
    }
    break;
  case 6:
    if (my->count[0] >= 701)
    {
      my->count[1] = 7;
      my->count[2] = -150;
      my->count[3] = 0;
    }
    if (my->count[2] % 12 == 0)
    {
      if (my->count[2] < 84)
      {  
        tenm_table_add(normal_enemy_new((double) (WINDOW_WIDTH / 2), -19.0,
                                        BALL_SOLDIER, 0,
                                        0, -1, 0, -1, 0, 1, 1,
                                        /* move 0 */
                                        9999, 0.0, 4.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 0,
                                        /* shoot 0 */
                                        9999, 9999, 0, 0, 1, 0));
      }
      else if (my->count[2] < 154)
      {
        /* do nothing */
        ;
      }
      else if (my->count[2] < 204)
      {  
        tenm_table_add(normal_enemy_new((double) (WINDOW_WIDTH / 2), -19.0,
                                        BALL_SOLDIER, 0,
                                        0, -1, 0, -1, 0, 1, 2,
                                        /* move 0 */
                                        9999, 0.0, 4.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 0,
                                        /* shoot 0 */
                                        80, 9999, 0, 0, 0, 1,
                                        /* shoot 0 */
                                        9999, 9999, 0, 0, 1, 1));
      }
      else if (my->count[2] < 252)
      {
        /* do nothing */
        ;
      }
      else if (!((my->count[2] >= 360) && (my->count[2] < 396)))
      {
        if (my->count[2] % 24 == 0)
          t_shoot = 40;
        else
          t_shoot = 9999;
        x = ((double) (WINDOW_WIDTH / 2));
        if (my->count[2] >= 360)
        {  
          x = ((double) ((my->count[2] % 36) / 12)) * 40.0;
          x += ((double) (WINDOW_WIDTH / 2)) - 40.0;
        }
        tenm_table_add(normal_enemy_new(x, -19.0,
                                        BALL_SOLDIER, 0,
                                        0, -1, 0, -1, 0, 1, 2,
                                        /* move 0 */
                                        9999, 0.0, 4.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 0,
                                        /* shoot 0 */
                                        40, t_shoot, my->count[2] % t_shoot,
                                        0, 0, 1,
                                        /* shoot 0 */
                                        9999, 9999, 0, 0, 1, 1));
      }
    }
    break;
  case 7:
    if (my->count[2] == 30)
    {
      tenm_table_add(normal_enemy_new((double) (WINDOW_WIDTH / 2), -23.0,
                                      BRICK, 0,
                                      0, my->table_index, 3,
                                      my->table_index, 3,
                                      3, 1,
                                      /* move 0 */
                                      20, 0.0, 4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      300, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, -4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      9999, 73, 0, 0, 1, 0));
      tenm_table_add(normal_enemy_new((double) (WINDOW_WIDTH / 2) + 150.0,
                                      -24.0,
                                      SQUARE, 0,
                                      0, my->table_index, 3,
                                      my->table_index, 3,
                                      3, 1,
                                      /* move 0 */
                                      20, 0.0, 4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      300, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, -4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      9999, 73, 20, 157, 1, 0));
      tenm_table_add(normal_enemy_new((double) (WINDOW_WIDTH / 2) - 150.0,
                                      -24.0,
                                      SQUARE, 0,
                                      0, my->table_index, 3,
                                      my->table_index, 3,
                                      3, 1,
                                      /* move 0 */
                                      20, 0.0, 4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      300, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, -4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      9999, 73, 20, 23, 1, 0));
    }

    if (my->count[3] >= 3)
    {
      if (my->count[2] <= 130)
        my->count[1] = 8;
      else
        my->count[1] = 9;
      my->count[2] = -1;
      my->count[3] = 0;
    }
    break;
  case 8:
    if (my->count[2] == 30)
    {
      for (i = 0; i < 4; i++)
      {
        x = 50.0 + 180.0 * ((double) i);
        tenm_table_add(normal_enemy_new(x, -24.0,
                                        SQUARE, 0,
                                        0, my->table_index, 3,
                                        my->table_index, 3,
                                        3, 1,
                                        /* move 0 */
                                        20, 0.0, 4.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 1,
                                        /* move 1 */
                                        300, 0.0, 0.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 2,
                                        /* move 2 */
                                        9999, 0.0, -4.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 2,
                                        /* shoot 0 */
                                        9999, 73, 0, 90, 1, 0));
      }
    }
    if (my->count[3] >= 4)
    {
      my->count[1] = 9;
      my->count[2] = -1;
      my->count[3] = 0;
    }
    break;
  case 9:
    if (my->count[2] == 100)
      tenm_table_add(warning_new());
    if (my->count[2] == 230)
    {  
      tenm_table_add(p_can_new());
      return 1;
    }
    break;
  default:
    fprintf(stderr, "plan_20_more_1_act: undefined mode (%d)\n", my->count[1]);
    break;
  }
  
  return 0;
}
