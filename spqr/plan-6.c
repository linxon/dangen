/* $Id: plan-6.c,v 1.74 2004/10/03 21:56:48 oohara Exp $ */
/* [very hard] Empty Wind */

#include <stdio.h>
/* malloc */
#include <stdlib.h>

#include "scheduler.h"
#include "tenm_table.h"
#include "empty-wind.h"
#include "warning.h"
#include "stage-title.h"
#include "normal-enemy.h"
#include "tenm_math.h"
#include "const.h"
#include "respiration.h"
#include "tenm_object.h"

#include "plan-6.h"

static tenm_object *plan_6_more_1_new(void);
static int plan_6_more_1_act(tenm_object *my, const tenm_object *player);

int
plan_6(int t)
{
  double x;
  double y;
  double dx;
  double dy;
  double ddx;
  double ddy;
  int n;

  /* sanity check */
  if (t < 0)
    return SCHEDULER_ERROR;

  if (t == 30)
    tenm_table_add(stage_title_new());

  if ((((t >= 160) && (t < 864))
       || ((t >= 952) && (t < 1654)))
      && ((t - 160) % 11 == 0))
  {
    if ((t - 160) % 176 < 44)
      n = 0;
    else if ((t - 160) % 176 < 88)
      n = 1;
    else if ((t - 160) % 176 < 132)
      n = 2;
    else
      n = 3;
    if (t >= 952)
      n = (8 - n) % 4;

    switch (n)
    {
    case 0:
      x = -19.0;
      y = 120.0;
      dx = 8.0;
      dy = 6.0;
      ddx = -0.06;
      ddy = -0.06;
      break;
    case 1:
      x = ((double) (WINDOW_WIDTH)) + 19.0;
      y = 120.0;
      dx = -8.0;
      dy = 6.0;
      ddx = 0.06;
      ddy = -0.06;
      break;
    case 2:
      x = ((double) (WINDOW_WIDTH)) + 19.0;
      y = ((double) (WINDOW_HEIGHT)) - 120.0;
      dx = -8.0;
      dy = -6.0;
      ddx = 0.06;
      ddy = 0.06;
      break;
    case 3:
      x = -19.0;
      y = ((double) (WINDOW_HEIGHT)) - 120.0;
      dx = 8.0;
      dy = -6.0;
      ddx = -0.06;
      ddy = 0.06;
      break;
    default:
      fprintf(stderr, "plan_6: undefined n (%d) (t = %d)\n", n, t);
      x = -19.0;
      y = 120.0;
      dx = 8.0;
      dy = 6.0;
      ddx = -0.06;
      ddy = -0.06;
      break;
    }

    tenm_table_add(normal_enemy_new(x, y,
                                    BALL_SOLDIER, 0,
                                    0, -1, 0, -1, 0, 1, 2,
                                    /* move 0 */
                                    9999,
                                    dx, dy, ddx, ddy,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    150, 120, 120 - t % 120, 0, 0, 1,
                                    /* shoot 1 */
                                    9999, 120, (270 - t  % 120) % 120,
                                    0, 1, 1));
  }

  if (t == 1840)
    tenm_table_add(plan_6_more_1_new());

  return SCHEDULER_SUCCESS;
}

static tenm_object *
plan_6_more_1_new(void)
{
  tenm_object *new = NULL;
  int *count = NULL;

  count = (int *) malloc(sizeof(int) * 2);
  if (count == NULL)
  {
    fprintf(stderr, "plan_6_more_1_new: malloc(count) failed\n");
    return NULL;
  }

  /* list of count
   * [0] timer
   * [1] "Respiration is dead" flag
   */
  count[0] = -1;
  count[1] = -1;

  new = tenm_object_new("plan 6 more 1", 0, 0,
                        1, 0.0, 0.0,
                        2, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double))
                        NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&plan_6_more_1_act),
                        (int (*)(tenm_object *, int))
                        NULL);

  if (new == NULL)
  {
    fprintf(stderr, "plan_6_more_1_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
plan_6_more_1_act(tenm_object *my, const tenm_object *player)
{
  int t;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "plan_6_more_1_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[0])++;
  if (my->count[0] == 0)
  {
    tenm_table_add(respiration_new(my->table_index));
  }

  if ((my->count[1] >= 0) && (my->count[0] < 1080))
  {
    t = my->count[0] - my->count[1];
    if ((t > 0) && (t % 11 == 0))
      tenm_table_add(normal_enemy_new(-8.0, -8.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 3, 2,
                                      /* move 0 */
                                      18,
                                      6.0, 6.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      30,
                                      0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999,
                                      6.0, 6.0,
                                      0.1 * tenm_cos(t * 2),
                                      0.1 * tenm_sin(t * 2),
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      76, 25, -48 - t % 25, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 25, (3 + 25 - t % 25) % 25,
                                      0, 1, 1));
  }

  if (((my->count[1] >= 0) && (my->count[0] == 1300))
      || ((my->count[1] < 0) && (my->count[0] == 1400)))
    tenm_table_add(warning_new());
  if (((my->count[1] >= 0) && (my->count[0] == 1430))
      || ((my->count[1] < 0) && (my->count[0] == 1530)))
    tenm_table_add(empty_wind_new());

  return 0;
}
