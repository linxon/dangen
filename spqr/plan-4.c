/* $Id: plan-4.c,v 1.58 2005/06/22 16:10:12 oohara Exp $ */
/* [easy] Seiron Fake */

#include <stdio.h>

#include "scheduler.h"
#include "tenm_table.h"
#include "seiron-fake.h"
#include "warning.h"
#include "stage-title.h"
#include "wall-4.h"
#include "const.h"
#include "normal-enemy.h"

#include "plan-4.h"

int
plan_4(int t)
{
  int i;
  int t_shoot;

  /* sanity check */
  if (t < 0)
    return SCHEDULER_ERROR;

  if (t == 30)
    tenm_table_add(stage_title_new());

  if (t == 160)
  {
    for (i = 0; i < 3; i++)
      tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) - 140.0
                                      - 120.0 * ((double) i),
                                      -24.0,
                                      BALL_CAPTAIN, 0,
                                      0, -1, 0, -1, 0, 3, 1,
                                      /* move 0 */
                                      208 + 30 * i, 0.0, 1.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      30, 0.0, -5.0 + 1.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, 1.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      9999, 51, i * 17, 0, 1, 0));
  }
  if (t == 165)
  {
    for (i = 0; i < 4; i++)
      tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) - 80.0
                                      - 120.0 * ((double) i),
                                      -19.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 2,
                                      /* move 0 */
                                      9999, 0.0, 1.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      208, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 1, 1));
  }
  if (t == 225)
  {
    tenm_table_add(wall_4_new(150.0, -29.0, 4.0, 100, 0));
    tenm_table_add(wall_4_new(90.0, -29.0, 0.0, -1, 0));
    tenm_table_add(wall_4_new(((double) WINDOW_WIDTH) - 30.0,
                              -29.0, 0.0, -1, 0));
  }

  if ((t >= 360) && (t < 408) && ((t - 360) % 8 == 0))
  {
    if ((t - 360) % 24 == 0)
      t_shoot = 41;
    else
      t_shoot = 9999;
    tenm_table_add(normal_enemy_new(322.0,
                                    -481.0 + ((double) (t - 225)) * 1.0,
                                    BALL_SOLDIER, 0,
                                    97, -1, 0, -1, 0, 2, 2,
                                    /* move 0 */
                                    73, -4.0, 4.0 + 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    9999, 0.0, 5.657 + 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* shoot 0 */
                                    73, t_shoot, 0, 0, 0, 1,
                                    /* shoot 1 */
                                    9999, t_shoot, 73 % t_shoot, 0, 1, 1));
  }
  if ((t >= 535) && (t < 583) && ((t - 535) % 8 == 0))
  {
    if ((t - 535) % 24 == 0)
      t_shoot = 9958;
    else
      t_shoot = 9999;
    tenm_table_add(normal_enemy_new(318.0,
                                    -481.0 + ((double) (t - 225)) * 1.0,
                                    BALL_SOLDIER, ENEMY_TYPE_WEAK,
                                    97, -1, 0, -1, 0, 2, 2,
                                    /* move 0 */
                                    73, 4.0, 4.0 + 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    9999, 0.0, 5.657 + 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* shoot 0 */
                                    73, 9999, t_shoot, 0, 0, 1,
                                    /* shoot 1 */
                                    9999, 9999, 0, 0, 1, 1));
  }
  if (t == 545)
  {
    tenm_table_add(wall_4_new(90.0, -29.0, 4.0, 100, 0));
    tenm_table_add(wall_4_new(30.0, -29.0, 0.0, -1, 0));
    tenm_table_add(wall_4_new(((double) WINDOW_WIDTH) - 90.0,
                              -29.0, 0.0, -1, 0));
  }

  if (t == 640)
  {
    tenm_table_add(normal_enemy_new(200.0, -24.0,
                                    BALL_CAPTAIN, 0,
                                    0, -1, 0, -1, 0, 1, 2,
                                    /* move 0 */
                                    9999, 0.0, 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    340, 88, 22, 0, 0, 1,
                                    /* shoot 1 */
                                    9999, 88, (340 + 22) % 88, 0, 1, 1));
    tenm_table_add(normal_enemy_new(320.0, -24.0,
                                    BALL_CAPTAIN, 0,
                                    0, -1, 0, -1, 0, 1, 2,
                                    /* move 0 */
                                    9999, 0.0, 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    340, 88, 11, 0, 0, 1,
                                    /* shoot 1 */
                                    9999, 88, (340 + 11) % 88, 0, 1, 1));
    tenm_table_add(normal_enemy_new(440.0, -24.0,
                                    BALL_CAPTAIN, 0,
                                    0, -1, 0, -1, 0, 1, 2,
                                    /* move 0 */
                                    9999, 0.0, 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    340, 88, 0, 0, 0, 1,
                                    /* shoot 1 */
                                    9999, 88, 340 % 88, 0, 1, 1));
  }
  if (t == 760)
  {
    tenm_table_add(normal_enemy_new(200.0, -24.0,
                                    BALL_CAPTAIN, 0,
                                    0, -1, 0, -1, 0, 1, 2,
                                    /* move 0 */
                                    9999, 0.0, 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    190, 88, (33 + 120) % 88, 0, 0, 1,
                                    /* shoot 1 */
                                    9999, 88, (190 + 33 + 120) % 88, 0, 1, 1));
    tenm_table_add(normal_enemy_new(440.0, -24.0,
                                    BALL_CAPTAIN, 0,
                                    0, -1, 0, -1, 0, 1, 2,
                                    /* move 0 */
                                    9999, 0.0, 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    190, 88, (77 + 120) % 88, 0, 0, 1,
                                    /* shoot 1 */
                                    9999, 88, (190 + 77 + 120) % 88, 0, 1, 1));
  }
  if (t == 880)
  {
    tenm_table_add(normal_enemy_new(200.0, -24.0,
                                    BALL_CAPTAIN, 0,
                                    0, -1, 0, -1, 0, 1, 2,
                                    /* move 0 */
                                    9999, 0.0, 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    100, 88, (44 + 240) % 88, 0, 1, 1,
                                    /* shoot 1 */
                                    9999, 88, (100 + 44 + 240) % 88, 0, 0, 1));
    tenm_table_add(normal_enemy_new(320.0, -24.0,
                                    BALL_CAPTAIN, 0,
                                    0, -1, 0, -1, 0, 1, 2,
                                    /* move 0 */
                                    9999, 0.0, 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    100, 88, (55 + 240) % 88, 0, 1, 1,
                                    /* shoot 1 */
                                    9999, 88, (100 + 55 + 240) % 88, 0, 0, 1));
    tenm_table_add(normal_enemy_new(440.0, -24.0,
                                    BALL_CAPTAIN, 0,
                                    0, -1, 0, -1, 0, 1, 2,
                                    /* move 0 */
                                    9999, 0.0, 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    100, 88, (66 + 240) % 88, 0, 1, 1,
                                    /* shoot 1 */
                                    9999, 88, (100 + 66 + 240) % 88, 0, 0, 1));
  }

  if (t == 770)
    tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) - 30.0, -24.0,
                                    SQUARE, 0,
                                    0, -1, 0, -1, 0, 1, 2,
                                    /* move 0 */
                                    9999, 0.0, 4.0 + 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    79, 23, 13, 157, 1, 1,
                                    /* shoot 1 */
                                    9999, 9999, 0, 157, 1, 1));

  if (t == 975)
  {
    tenm_table_add(wall_4_new(30.0, -29.0, 4.0, 145, 0));
  }
  if (t == 1035)
  {
    tenm_table_add(wall_4_new(30.0, -29.0, 4.0, 65, 0));
    tenm_table_add(wall_4_new(((double) WINDOW_WIDTH) - 30.0, -29.0,
                              -4.0, 65, 0));
  }
  if ((t >= 1045) && (t < 1173) && ((t - 1045) % 8 == 0))
  {
    if ((t - 1045) % 32 == 0)
      t_shoot = 19;
    else
      t_shoot = 9999;

    tenm_table_add(normal_enemy_new(-19.0,
                                    -29.0 + ((double) (t - 975)) * 1.0,
                                    BALL_SOLDIER, ENEMY_TYPE_WEAK,
                                    0, -1, 0, -1, 0, 1, 2,
                                    /* move 0 */
                                    9999, 6.0, 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    30, t_shoot, 0, 0, 0, 1,
                                    /* shoot 1 */
                                    9999, t_shoot, 30 % t_shoot, 0, 1, 1));
  }
  if ((t >= 1213) && (t < 1341) && ((t - 1213) % 8 == 0))
  {
    if ((t - 1213) % 32 == 0)
      t_shoot = 19;
    else
      t_shoot = 9999;

    tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) + 19.0,
                                    -29.0 + ((double) (t - 975)) * 1.0,
                                    BALL_SOLDIER, ENEMY_TYPE_WEAK,
                                    0, -1, 0, -1, 0, 1, 2,
                                    /* move 0 */
                                    9999, -6.0, 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    30, t_shoot, 0, 0, 0, 1,
                                    /* shoot 1 */
                                    9999, t_shoot, 30 % t_shoot, 0, 1, 1));
  }

  if (t == 1600)
    tenm_table_add(warning_new());

  if (t == 1730)
    tenm_table_add(seiron_fake_new());

  return SCHEDULER_SUCCESS;
}
