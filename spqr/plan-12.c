/* $Id: plan-12.c,v 1.4 2004/12/12 13:30:10 oohara Exp $ */

#include <stdio.h>

#include "scheduler.h"
#include "tenm_table.h"
#include "w-ko.h"
#include "warning.h"
#include "stage-title.h"
#include "normal-enemy.h"
#include "const.h"
#include "tenm_math.h"

#include "plan-12.h"

int
plan_12(int t)
{
  int i;
  int s;
  double x;
  double y;
  int theta;
  int temp;

  /* sanity check */
  if (t < 0)
    return SCHEDULER_ERROR;

  if (t == 30)
    tenm_table_add(stage_title_new());

  if ((t >= 160) && (t < 880))
  {
    s = t - 160;
    if (s % 13 == 0)
    {
      for (i = 0; i < 2; i++)
      {
        if ((s < 180) && (i != 0))
          continue;

        theta = -(s + i * 180);
        if (s % 26 == 0)
          temp = 37;
        else
          temp = 9999;
        x = ((double) (WINDOW_WIDTH / 2))
          + 424.0 * tenm_cos(theta)
          + 100.0 * tenm_cos(theta - 90);
        y = ((double) (WINDOW_HEIGHT / 2))
          + 424.0 * tenm_sin(theta)
          + 100.0 * tenm_sin(theta - 90);
        tenm_table_add(normal_enemy_new(x, y,
                                        BALL_SOLDIER, 0,
                                        140, -1, 0, -1, 0, 1, 1,
                                        /* move 0 */
                                        9999,
                                        -7.0 * tenm_cos(theta),
                                        -7.0 * tenm_sin(theta),
                                        0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 0,
                                        /* shoot 0 */
                                        9999, temp, s % temp, 0, 1, 0));
      }
    }
  }

  if (t == 1070)
    tenm_table_add(warning_new());
  if (t == 1200)
    tenm_table_add(w_ko_new());

  return SCHEDULER_SUCCESS;
}
