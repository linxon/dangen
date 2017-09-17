/* $Id: plan-2.c,v 1.42 2004/09/24 13:39:45 oohara Exp $ */
/* [very hard] Senators */

#include <stdio.h>

#include "scheduler.h"
#include "tenm_table.h"
#include "net-can-howl.h"
#include "warning.h"
#include "stage-title.h"
#include "normal-enemy.h"
#include "const.h"

#include "plan-2.h"

int
plan_2(int t)
{
  /* sanity check */
  if (t < 0)
    return SCHEDULER_ERROR;

  if (t == 30)
    tenm_table_add(stage_title_new());

  if ((t >= 160) && (t <= 508) && ((t - 160) % 29 == 0))
  {
    if ((t - 160) % 58 < 29)
      tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) - 120.0, -24.0,
                                      SQUARE, 0,
                                      0, -1, 0, -1, 0, 1, 2,
                                      /* move 0 */
                                      9999,
                                      -4.8, 3.6, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      50, 13, 0, 45, 0, 1,
                                      /* shoot 0 */
                                      9999, 13, 11, 45, 1, 1));
    else
      tenm_table_add(normal_enemy_new(120.0, -24.0,
                                      SQUARE, 0,
                                      0, -1, 0, -1, 0, 1, 2,
                                      /* move 0 */
                                      9999,
                                      4.8, 3.6, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      50, 13, (26 - t % 13) % 13,
                                      135, 0, 1,
                                      /* shoot 1 */
                                      9999, 13, (76 - t % 13) % 13,
                                      135, 1, 1));
#if 0
    tenm_table_add(normal_enemy_new(520.0, -24.0,
                                    SQUARE, 0,
                                    0, -1, 0, -1, 0, 3, 1,
                                    /* move 0 */
                                    44,
                                    0.0, 6.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    209,
                                    0.0, 0.0, 0.0, 0.0,
                                    320.0, 240.0, 0.0, 0.15, 2,
                                    /* move 2 */
                                    9999,
                                    0.0, 6.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 2,
                                    /* shoot 0 */
                                    9999, 17, 0, (t * 3) % 360, 1, 0));
#endif /* 0 */
  }

  if ((t >= 190) && (t < 566))
  {
    switch ((t - 160) % 58)
    {
    case 30:
      tenm_table_add(normal_enemy_new(40.0, -14.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999,
                                      0.0, 5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, 50, (100 - t % 50) % 50, 0, 1, 0));
      break;
    case 38:
    case 46:
    case 54:
      tenm_table_add(normal_enemy_new(40.0, -14.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 2,
                                      /* move 0 */
                                      9999,
                                      0.0, 5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      50, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 1, 1));
      break;
    case 1:
      tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) - 40.0, -14.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999,
                                      0.0, 5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, 50, (100 - t % 50) % 50, 0, 1, 0));
      break;
    case 9:
    case 17:
    case 25:
      tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) - 40.0, -14.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 2,
                                      /* move 0 */
                                      9999,
                                      0.0, 5.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      50, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 1, 1));
      break;
    default:
      break;
    }
  }

  if (t == 740)
    tenm_table_add(warning_new());

  if (t == 870)
    tenm_table_add(net_can_howl_core_new());

  return SCHEDULER_SUCCESS;
}
