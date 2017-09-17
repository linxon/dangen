/* $Id: plan-17.c,v 1.113 2005/06/26 10:17:35 oohara Exp $ */
/* [very easy] Afterdeath */

#include <stdio.h>

#include "scheduler.h"
#include "tenm_table.h"
#include "afterdeath.h"
#include "stage-title.h"
#include "warning.h"
#include "normal-enemy.h"
#include "const.h"
#include "tenm_math.h"

#include "plan-17.h"

static void plan_17_circle_party(double x, double dx, double dy);

int
plan_17(int t)
{
  int s;
  int theta;
  double x;
  double ddy;
  int t_shoot;

  /* sanity check */
  if (t < 0)
    return SCHEDULER_ERROR;

  if (t == 30)
    tenm_table_add(stage_title_new());

  if (t == 160)
    plan_17_circle_party(200.0, 4.0, 3.0);

  if (t == 350)
    plan_17_circle_party(440.0, -3.5, 0.5);
  if (t == 400)
    plan_17_circle_party(520.0, -3.3, 4.4);

  if ((t == 590) || (t == 630) || (t == 670))
  {
    if (t == 590)
      x = 200.0;
    else if (t == 630)
      x = 100.0;
    else
      x = 300.0;

    tenm_table_add(normal_enemy_new(x, -24.0,
                                    BRICK, 0,
                                    0, -1, 0, -1, 0, 1, 2,
                                    /* move 0 */
                                    9999, 0.0, 4.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    15 + (t - 590), 37, 30, 0, 0, 1,
                                    /* shoot 1 */
                                    9999, 37, (45 + (t - 590)) % 37, 0, 1, 1));
  }

  if ((t >= 860) && (t < 1294))
  {
    s = t - 860;
    if ((s % 62 == 0) || (s % 62 == 26))
    {
      x = 60.0;
      theta = 25 + s % 26;
      if (s % 62 != 0)
      {
        x = 500.0;
        theta = 180 - theta;
      }
      plan_17_circle_party(x, 2.7 * tenm_cos(theta), 3.6 * tenm_sin(theta));
    }
  }

  if ((t >= 1490) && (t < 1610))
  {
    s = t - 1490;
    if ((s % 8 == 0) && (s % 40 < 32))
    {
      if (s < 40)
      {  
        x = 20.0;
        ddy = -0.06;
      }
      else if (s < 80)
      {
        x = 120.0;
        ddy = -0.08;
      }
      else
      {
        x = 220.0;
        ddy = -0.1;
      }

      if (s % 40 == 0)
        t_shoot = 40;
      else
        t_shoot = 9999;

      tenm_table_add(normal_enemy_new(x, -19.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 2,
                                      /* move 0 */
                                      9999, 2.0, ddy * (-90.0), 0.0, ddy,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      90, t_shoot, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 1, 1));
    }
  }
  if ((t >= 1800) && (t < 1960))
  {
    s = t - 1800;
    if ((s % 8 == 0) && (s % 40 < 32))
    {
      if (s < 40)
      {  
        x = ((double) WINDOW_WIDTH) - 10.0;
        ddy = -0.04;
      }
      else if (s < 80)
      {
        x = ((double) WINDOW_WIDTH) - 100.0;
        ddy = -0.06;
      }
      else if (s < 120)
      {
        x = ((double) WINDOW_WIDTH) - 190.0;
        ddy = -0.08;
      }
      else
      {
        x = ((double) WINDOW_WIDTH) - 280.0;
        ddy = -0.1;
      }

      if (s % 40 == 0)
        t_shoot = 25;
      else
        t_shoot = 9999;

      tenm_table_add(normal_enemy_new(x, -19.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 2,
                                      /* move 0 */
                                      9999, -1.5, ddy * (-90.0), 0.0, ddy,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      90, t_shoot, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 1, 1));
    }
  }

  if (t == 2200)
    tenm_table_add(warning_new());

  if (t == 2330)
    tenm_table_add(afterdeath_new());

  return SCHEDULER_SUCCESS;
}

static void
plan_17_circle_party(double x, double dx, double dy)
{
  int i;
  int t_shoot;
  double speed_theta;
  int theta;

  if (dx > 0.0)
  {  
    speed_theta = 0.4;
    theta = -180;
  }
  else
  {  
    speed_theta = -0.4;
    theta = 180;
  }

  for (i = 0; i < 4; i++)
  {
    if (i == 0)
      t_shoot = 9912;
    else
      t_shoot = 0;

    tenm_table_add(normal_enemy_new(x, -19.0 - ((double) i) * 25.0,
                                    BALL_SOLDIER, 0,
                                    50, -1, 0, -1, 0, 5, 2,
                                    /* move 0 */
                                    50, 0.0, 3.0 + ((double) i) * 0.5,
                                    0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    5, 0.0, 0.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 2,
                                    /* move 2 */
                                    10,
                                    3.0 * tenm_cos(i * 25),
                                    3.0 * tenm_sin(i * 25),
                                    0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 3,
                                    /* move 3 */
                                    100, dx, dy, 0.0, 0.0,
                                    x, 150.0 - 19.0, 1.0, speed_theta, 4,
                                    /* move 4 */
                                    9999,
                                    11.0 * tenm_cos(i * 25 + theta),
                                    11.0 * tenm_sin(i * 25 + theta),
                                    0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 4,
                                    /* shoot 0 */
                                    97, 9999, t_shoot, 0, 0, 1,
                                    /* shoot 1 */
                                    9999, 9999, 0, 0, 1, 1));
  }
}
