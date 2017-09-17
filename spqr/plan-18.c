/* $Id: plan-18.c,v 1.80 2005/06/26 16:16:50 oohara Exp $ */
/* [very easy] Hugin */

#include <stdio.h>

#include "scheduler.h"
#include "tenm_table.h"
#include "hugin.h"
#include "stage-title.h"
#include "warning.h"
#include "normal-enemy.h"
#include "const.h"

#include "plan-18.h"

int
plan_18(int t)
{
  int i;
  int s;
  int what;
  double x;
  double y;
  double dx;
  int t_shoot;

  /* sanity check */
  if (t < 0)
    return SCHEDULER_ERROR;

  if (t == 30)
    tenm_table_add(stage_title_new());

  if ((t == 160) || (t == 240))
  {
    for (i = 0; i < 4; i++)
    {
      if (i == 0)
        what = BALL_CAPTAIN;
      else
        what = BALL_SOLDIER;
      x = -19.0;
      dx = 6.0 - ((double) i) * 0.6;
      if (t == 240)
      {
        x = ((double) WINDOW_WIDTH) - x;
        dx *= -1.0;
      }
      tenm_table_add(normal_enemy_new(x, 0.0,
                                      what, 0,
                                      0, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999, dx, 4.5 - ((double) i) * 0.45,
                                      0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* shoot 0 */
                                      9999, 9999, 0, 0, 1, 0));
    }
  }

  if (t == 320)
  {
    tenm_table_add(normal_enemy_new(((double) (WINDOW_WIDTH / 2)), -23.0,
                                    BRICK, 0,
                                    0, -1, 0, -1, 0, 3, 3,
                                    /* move 0 */
                                    10, 0.0, 6.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    445, 0.0, 0.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 2,
                                    /* move 2 */
                                    9999, 0.0, -6.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 2,
                                    /* shoot 0 */
                                    400, 46, 0, 0, 0, 1,
                                    /* shoot 1 */
                                    55, 9999, 0, 0, 1, 2,
                                    /* shoot 2 */
                                    9999, 9999, 0, 0, 0, 2));
    tenm_table_add(normal_enemy_new(((double) (WINDOW_WIDTH / 2)) + 60.0,
                                    -23.0,
                                    BALL_CAPTAIN, 0,
                                    0, -1, 0, -1, 0, 3, 3,
                                    /* move 0 */
                                    10, 0.0, 8.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    445, 0.0, 0.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 2,
                                    /* move 2 */
                                    9999, 0.0, -8.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 2,
                                    /* shoot 0 */
                                    400, 46, 23, 0, 0, 1,
                                    /* shoot1 */
                                    55, 9999, 0, 0, 1, 2,
                                    /* shoot 2 */
                                    9999, 9999, 0, 0, 0, 2));
    tenm_table_add(normal_enemy_new(((double) (WINDOW_WIDTH / 2)) - 60.0,
                                    -23.0,
                                    BALL_CAPTAIN, 0,
                                    0, -1, 0, -1, 0, 3, 3,
                                    /* move 0 */
                                    10, 0.0, 8.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    445, 0.0, 0.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 2,
                                    /* move 2 */
                                    9999, 0.0, -8.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 2,
                                    /* shoot 0 */
                                    400, 46, 23, 0, 0, 1,
                                    /* shoot1 */
                                    55, 9999, 0, 0, 1, 2,
                                    /* shoot 2 */
                                    9999, 9999, 0, 0, 0, 2));
  }
  if ((t >= 320) && (t < 720))
  {
    s = t - 160;
    if (s % 40 == 0)
    {
      for (i = 0; i < 4; i++)
      {
        if (i == 0)
          what = BALL_CAPTAIN;
        else
          what = BALL_SOLDIER;
        x = -19.0;
        dx = 12.0 - ((double) i) * 1.2;
        if (s % 80 >= 40)
        {
          x = ((double) WINDOW_WIDTH) - x;
          dx *= -1.0;
        }
        tenm_table_add(normal_enemy_new(x, 0.0,
                                        what, 0,
                                        0, -1, 0, -1, 0, 1, 1,
                                        /* move 0 */
                                        9999, dx, 9.0 - ((double) i) * 0.9,
                                        0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 1,
                                        /* shoot 0 */
                                        9999, 9999, 0, 0, 1, 0));
      }
    }
  }

  if ((t >= 820) && (t < 1130))
  {
    s = t - 820;
    if ((s % 10 == 0) && (s != 210))
    {
      y = -19.0 - 400.0 + ((double) ((s + 90) % 100)) * 4.0;
      if (s % 30 == 0)
        t_shoot = 9969;
      else
        t_shoot = 0;
      tenm_table_add(normal_enemy_new(400.0, y,
                                      BALL_SOLDIER, 0,
                                      300, -1, 0, -1, 0, 2, 3,
                                      /* move 0 */
                                      100, 8.0, 2.0, -0.16, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      100, -8.0, 2.0, 0.16, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      20, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      60, 9999, t_shoot, 0, 1, 2,
                                      /* shoot 2 */
                                      40, 9999, 0, 0, 0, 1));
    }
  }

  if (t == 1570)
    tenm_table_add(warning_new());

  if (t == 1700)
    tenm_table_add(hugin_new());

  return SCHEDULER_SUCCESS;
}
