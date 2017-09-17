/* $Id: plan-9.c,v 1.49 2004/11/28 07:36:00 oohara Exp $ */
/* [hard] 0x82da3104 */

#include <stdio.h>
/* malloc */
#include <stdlib.h>

#include "scheduler.h"
#include "tenm_table.h"
#include "hatsuda.h"
#include "warning.h"
#include "stage-title.h"
#include "tenmado.h"
#include "const.h"
#include "tenm_object.h"
#include "normal-enemy.h"

#include "plan-9.h"

static tenm_object *plan_9_more_1_new(void);
static int plan_9_more_1_act(tenm_object *my, const tenm_object *player);

int
plan_9(int t)
{
  /* sanity check */
  if (t < 0)
    return SCHEDULER_ERROR;

  if (t == 30)
    tenm_table_add(stage_title_new());

  if (t == 160)
    tenm_table_add(plan_9_more_1_new());

  return SCHEDULER_SUCCESS;
}

static tenm_object *
plan_9_more_1_new(void)
{
  tenm_object *new = NULL;
  int *count = NULL;

  count = (int *) malloc(sizeof(int) * 6);
  if (count == NULL)
  {
    fprintf(stderr, "plan_9_more_1_new: malloc(count) failed\n");
    return NULL;
  }

  /* list of count
   * [0] total timer
   * [1] number of tenmado created
   * [2] number of tenmado killed
   * [3] new enemy timer
   * [4] new enemy direction
   * [5] more end time
   */
  count[0] = -1;
  count[1] = 0;
  count[2] = 0;
  count[3] = -1;
  count[4] = -1;
  count[5] = 1000;

  new = tenm_object_new("plan 9 more 1", 0, 0,
                        1, 0.0, 0.0,
                        6, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double))
                        NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&plan_9_more_1_act),
                        (int (*)(tenm_object *, int))
                        NULL);

  if (new == NULL)
  {
    fprintf(stderr, "plan_9_more_1_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
plan_9_more_1_act(tenm_object *my, const tenm_object *player)
{
  double x;
  double dx;
  int t_shoot;
  int temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "plan_9_more_1_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[0])++;

  if ((my->count[4] < 0) && (my->count[0] < my->count[5] - 100)
      && (my->count[0] < 9999)
      && (((my->count[2] < 3) && (my->count[1] < my->count[2] + 1))
          || ((my->count[2] >= 3) && (my->count[1] < my->count[2] + 2))))
  {
    (my->count[1])++;
    my->count[3] = -1;
    if (player->x < ((double) (WINDOW_WIDTH / 2)))
      my->count[4] = 0;
    else
      my->count[4] = 1;

    if (my->count[1] <= 1)
    {
      temp = 0;
    }
    else
    {
      if (my->count[1] <= 4)
        temp = 68;
      else
        temp = 68 - (my->count[1] - 4) * 2;
      if (temp < 49)
        temp = 49;
    }
    my->count[5] += temp;
  }

  if (my->count[4] >= 0)
  {
    (my->count[3])++;

    if ((my->count[1] > 1)  && (my->count[1] != 5)
        && (my->count[3] == 0))
    {
      if (my->count[4] == 0)
      {
        x = ((double) WINDOW_WIDTH) - 44.0;
      }
      else
      {
        x = 44.0;
      }
      tenm_table_add(normal_enemy_new(x, -24.0,
                                      BALL_CAPTAIN, 0,
                                      0, -1, 0, -1, 0, 2, 2,
                                      /* move 0 */
                                      66,
                                      0.0, 4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      9999,
                                      0.0, 7.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* shoot 0 */
                                      51, 17, 0, 0, 1, 1,
                                      /* shoot 1 */
                                      9999, 9, 0, 0, 1, 1));
    }
    if ((my->count[1] > 1) && (my->count[1] != 5)
        && ((my->count[3] == 22) || (my->count[3] == 36)
            || (my->count[3] == 43) || (my->count[3] == 50)))
    {
      if (my->count[4] == 0)
      {
        x = ((double) WINDOW_WIDTH) - 50.0;
      }
      else
      {
        x = 50.0;
      }
      tenm_table_add(normal_enemy_new(x, -19.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 2, 2,
                                      /* move 0 */
                                      66 - my->count[3],
                                      0.0, 7.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 0 */
                                      9999,
                                      0.0, 10.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* shoot 0 */
                                      64 - my->count[3], 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 1, 1));
    }
  
    if (((my->count[1] <= 1) && (my->count[3] == 0))
        || ((my->count[1] > 1) && (my->count[3] == 50)))
    {
      if (my->count[4] == 0)
      {
        x = 0.0;
        dx = 6.0;
      }
      else
      {
        x = (double) WINDOW_WIDTH;
        dx = -6.0;
      }

      if (my->count[1] <= 4)
        t_shoot = 13;
      else
        t_shoot = 13 - (my->count[1] - 4);
      if (t_shoot < 7)
        t_shoot = 7;
      tenm_table_add(tenmado_new(x, -29.0, my->count[1] % 2, dx,
                                 my->count[5] - my->count[0],
                                 my->table_index, t_shoot));
      my->count[3] = -1;
      my->count[4] = -1;
    }
  }

  if (my->count[0] == my->count[5] + 130)
  {
    tenm_table_add(warning_new());
  }

  if (my->count[0] == my->count[5] + 260)
  {
    tenm_table_add(hatsuda_new());
    return 1;
  }
  
  return 0;
}
