/* $Id: plan-13.c,v 1.105 2005/01/16 12:03:37 oohara Exp $ */

#include <stdio.h>

#include "scheduler.h"
#include "tenm_table.h"
#include "insane-hand.h"
#include "warning.h"
#include "stage-title.h"
#include "normal-enemy.h"
#include "const.h"
#include "wall-13.h"

#include "plan-13.h"

void plan_13_formation(double x, double y, double v, int t_move, int what);

int
plan_13(int t)
{
  int i;
  int s;
  int t_shoot;
  double x;
  double y;
  double dx;
  double dy;

  /* sanity check */
  if (t < 0)
    return SCHEDULER_ERROR;

  if (t == 30)
    tenm_table_add(stage_title_new());

  if (t >= 160)
  {
    s = t - 160;

    if ((((s >= 0) && (s < 32))
         || ((s >= 64) && (s < 96))
         || ((s >= 128) && (s < 160)))
        && (s % 8 == 0))
    {
      if (s % 32 == 0)
        t_shoot = 43;
      else
        t_shoot = 9999;
      tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) + 19.0, 90.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999, -5.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, t_shoot, 0, 0, 1, 0));
    }
    if ((((s >= 32) && (s < 64))
         || ((s >= 160) && (s < 192))
         || ((s >= 224) && (s < 256)))
        && (s % 8 == 0))
    {
      if (s % 32 == 0)
        t_shoot = 43;
      else
        t_shoot = 9999;
      tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) + 19.0, 240.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999, -5.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, t_shoot, 0, 0, 1, 0));
    }
    if ((((s >= 96) && (s < 128))
         || ((s >= 192) && (s < 224))
         || ((s >= 256) && (s < 288)))
        && (s % 8 == 0))
    {
      if (s % 32 == 0)
        t_shoot = 43;
      else
        t_shoot = 9999;
      tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) + 19.0, 390.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999, -5.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, t_shoot, 0, 0, 1, 0));
    }

    if (s == 0)
    {
      plan_13_formation(729.0, 90.0, 5.2, 25, 3);
      plan_13_formation(969.0, 390.0, 5.2, 25, 3);
      for (i = 0; i < 5; i++)
        tenm_table_add(wall_13_new(1100.0 + ((double) i) * 145.0, 30.0, t));
      tenm_table_add(wall_13_new(1100.0, 450.0, t));
      tenm_table_add(wall_13_new(1680.0, 450.0, t));
    }

    if ((s >= 581) && (s < 1016) && ((s - 581) % 145 == 0))
      tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) + 24.0, 450.0,
                                      SQUARE, 0,
                                      0, -1, 0, -1, 0, 3, 2,
                                      /* move 0 */
                                      1230 - t, -1.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      270, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, 4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      1500 - t, 43, 0, 225, 1, 1,
                                      /* shoot 2 */
                                      9999, 9999, 0, 225, 0, 1));

    if ((s >= 600) && (s < 960) && ((s - 600) % 30 == 0))
      tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) - 67.0
                                      - ((double) (s - 600))
                                      + ((double) (((s - 600) % 120) / 30))
                                      * 145.0,
                                      -24.0,
                                      BALL_CAPTAIN, 0,
                                      0, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999, -1.0, 0.0, 0.0, 0.1,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, 17, 0, 0, 1, 0));
    if ((s >= 1100) && (s < 1220) && ((s - 1100) % 30 == 0))
      tenm_table_add(normal_enemy_new(102.0
                                      + ((double) (((s - 1100) % 120) / 30))
                                      * 145.0,
                                      -24.0,
                                      BALL_CAPTAIN, 0,
                                      0, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999, 0.0, 0.0, 0.0, 0.1,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, 17, 0, 0, 1, 0));
  }

  if (t >= 1530)
  {
    s = t - 1530;
    if ((s >= 0) && (s < 80) && (s % 4 == 0))
    {
      if (s % 16 < 8)
        dx = 0.75;
      else
        dx = -0.75;
      dy = -5.0 - ((double) (s / 8)) * 0.25;
      if (s % 8 == 0)
        tenm_table_add(normal_enemy_new(234.0, ((double) WINDOW_HEIGHT) + 19.0,
                                        BALL_SOLDIER, 0,
                                        0, -1, 0, -1, 0, 1, 1,
                                        /* move 0 */
                                        9999, dx, dy, 0.0, 0.1,
                                        0.0, 0.0, 0.0, 0.0, 0,
                                        /* shoot 0 */
                                        9999, 9999, 0, 0, 1, 0));
      else
        tenm_table_add(normal_enemy_new(406.0, ((double) WINDOW_HEIGHT) + 19.0,
                                        BALL_SOLDIER, 0,
                                        0, -1, 0, -1, 0, 1, 1,
                                        /* move 0 */
                                        9999, dx, dy, 0.0, 0.1,
                                        0.0, 0.0, 0.0, 0.0, 0,
                                        /* shoot 0 */
                                        9999, 9999, 0, 0, 1, 0));
    }
  }

  if ((t >= 1921) && (t <= 2821) && ((t - 1921) % 180 == 0))
  {
    tenm_table_add(wall_13_new(30.0, ((double) WINDOW_HEIGHT) + 29.0, t));
    tenm_table_add(wall_13_new(((double) WINDOW_WIDTH) - 30.0,
                               ((double) WINDOW_HEIGHT) + 29.0, t));
  }
  
  if (t == 1800)
  {
    for (i = 0; i < 5; i++)
    {
      if (i < 3)
        dy = -2.0 * ((double) i);
      else
        dy = -3.0 + 2.0 * ((double) (i - 3));
      tenm_table_add(normal_enemy_new(82.0 + 10.0 * ((double) i),
                                      520.0 - dy * 43.0,
                                      BALL_SOLDIER, 0,
                                      250, -1, 0, -1, 0, 3, 2,
                                      /* move 0 */
                                      380, 0.0, -1.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      43, (double) i, dy - 1.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, -1.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      350, 85, 17 * i, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 85, (350 + 17 * i) % 85, 0, 1, 1));
      tenm_table_add(normal_enemy_new(559.0 - 10.0 * ((double) i),
                                      520.0 - dy * 43.0,
                                      BALL_SOLDIER, 0,
                                      250, -1, 0, -1, 0, 3, 2,
                                      /* move 0 */
                                      380, 0.0, -1.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      43, -((double) i), dy - 1.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, -1.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      350, 85, 17 * i, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 85, (350 + 17 * i) % 85, 0, 1, 1));

      tenm_table_add(normal_enemy_new(82.0 + 10.0 * ((double) i),
                                      740.0 - dy * 43.0,
                                      BALL_SOLDIER, 0,
                                      480, -1, 0, -1, 0, 3, 2,
                                      /* move 0 */
                                      380, 0.0, -1.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      43, (double) i, dy - 1.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, -1.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      350, 85, 17 * i, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 85, (350 + 17 * i) % 85, 0, 1, 1));
      tenm_table_add(normal_enemy_new(559.0 - 10.0 * ((double) i),
                                      740.0 - dy * 43.0,
                                      BALL_SOLDIER, 0,
                                      480, -1, 0, -1, 0, 3, 2,
                                      /* move 0 */
                                      380, 0.0, -1.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      43, -((double) i), dy - 1.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      9999, 0.0, -1.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      350, 85, 17 * i, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 85, (350 + 17 * i) % 85, 0, 1, 1));
    }
  }
  
  if (t == 2281)
  {
    for (i = 0; i < 4; i++)
      tenm_table_add(wall_13_new(120.0 + ((double) i) * 90.0,
                              ((double) WINDOW_HEIGHT) + 29.0, t));
    plan_13_formation(280.0, ((double) WINDOW_HEIGHT) + 259.0, 3.0, 80, 0);
    plan_13_formation(360.0, ((double) WINDOW_HEIGHT) + 259.0, 3.0, 80, 0);
    plan_13_formation(280.0, ((double) WINDOW_HEIGHT) + 339.0, 3.0, 80, 0);
    plan_13_formation(360.0, ((double) WINDOW_HEIGHT) + 339.0, 3.0, 80, 0);
    plan_13_formation(320.0, ((double) WINDOW_HEIGHT) + 299.0, 5.0, 48, 1);
  }
  if (t == 2556)
  {
    tenm_table_add(normal_enemy_new(30.0, ((double) WINDOW_HEIGHT) + 24.0,
                                    SQUARE, 0,
                                    0, -1, 0, -1, 0, 1, 1,
                                    /* move 0 */
                                    9999, 0.0, -1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    9999, 43, 0, 350, 1, 0));
    tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) - 30.0,
                                    ((double) WINDOW_HEIGHT) + 24.0,
                                    SQUARE, 0,
                                    0, -1, 0, -1, 0, 1, 1,
                                    /* move 0 */
                                    9999, 0.0, -1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    9999, 43, 0, 170, 1, 0));
  }
  if (t == 2821)
    for (i = 0; i < 4; i++)
      tenm_table_add(wall_13_new(520.0 - ((double) i) * 90.0,
                                 ((double) WINDOW_HEIGHT) + 29.0, t));
  if ((t >= 2981) && (t < 3191))
  {
    s = t - 2981;
    if (s % 30 == 0)
    {
      if (s % 60 == 30)
      {
        x = -31.0;
        dx = 5.0;
      }
      else
      {
        x = ((double) WINDOW_WIDTH) + 31.0;
        dx = -5.0;
      }
      y = ((double) WINDOW_HEIGHT) - 24.0 - ((double) (s - 90)) * 1.0;
      if (s >= 90)
        y -= ((double) (((s - 90) / 30) * 48));
      else
        y += ((double) ((s / 30) * 48)) - 144.0;
      tenm_table_add(normal_enemy_new(x, y,
                                      BRICK, 0,
                                      0, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999, dx, -1.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, 37, 27, 0, 1, 0));
    }
  }

  if (t == 3380)
    tenm_table_add(warning_new());

  if (t == 3510)
    tenm_table_add(insane_hand_new());

  return SCHEDULER_SUCCESS;
}

void
plan_13_formation(double x, double y, double v, int t_move, int what)
{
  int w;
  int t_no_escape;
  int t_shoot;
  double d;

  /* sanity check */
  if (t_move <= 0)
  {
    fprintf(stderr, "plan_13_formation: t_move is non-positive (%d)\n",
            t_move);
    return;
  }
  if ((what < 0) || (what > 3))
  {
    fprintf(stderr, "plan_13_formation: strange what (%d)\n", what);
    return;
  }

  d = v * ((double) t_move) / 2.0;
  t_no_escape = ((int) ((x + d) / 1.0)) + 26;

  switch (what)
  {
  case 0:
  case 1:
    t_no_escape = ((int) ((y + d) / 1.0)) + 26;
    if (what == 1)
    {
      w = BALL_CAPTAIN;
      t_shoot = 36;
    }
    else
    {
      w = BALL_SOLDIER;
      t_shoot = 9999;
    }
    tenm_table_add(normal_enemy_new(x + d, y - d, w, 0,
                                    t_no_escape, -1, 0, -1, 0, 2, 1,
                                    /* move 0 */
                                    t_move, -v, v - 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    t_move, v, -v - 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    9999, t_shoot, 0, 0, 1, 0));
    tenm_table_add(normal_enemy_new(x + d, y + d, w, 0,
                                    t_no_escape, -1, 0, -1, 0, 2, 1,
                                    /* move 0 */
                                    t_move, -v, -v - 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    t_move, v, v - 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    9999, t_shoot, 9, 0, 1, 0));
    tenm_table_add(normal_enemy_new(x - d, y + d, w, 0,
                                    t_no_escape, -1, 0, -1, 0, 2, 1,
                                    /* move 0 */
                                    t_move, v, -v - 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    t_move, -v, v - 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    9999, t_shoot, 18, 0, 1, 0));
    tenm_table_add(normal_enemy_new(x - d, y - d, w, 0,
                                    t_no_escape, -1, 0, -1, 0, 2, 1,
                                    /* move 0 */
                                    t_move, v, v - 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    t_move, -v, -v - 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    9999, t_shoot, 27, 0, 1, 0));
    break;
  case 2:
  case 3:
    t_no_escape = ((int) ((x + d) / 1.0)) + 26;
    if (what == 3)
    {
      w = BALL_CAPTAIN;
      t_shoot = 36;
    }
    else
    {
      w = BALL_SOLDIER;
      t_shoot = 9999;
    }
    tenm_table_add(normal_enemy_new(x + d, y - d, w, 0,
                                    t_no_escape, -1, 0, -1, 0, 4, 2,
                                    /* move 0 */
                                    t_move, -v - 1.0, v, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    t_move, 0.0 - 1.0, 0.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 2,
                                    /* move 2 */
                                    t_move, v - 1.0, -v, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 3,
                                    /* move 3 */
                                    t_move, 0.0 - 1.0, 0.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    300, t_shoot, 27, 0, 0, 1,
                                    /* shoot 1 */
                                    9999, t_shoot, 327 % t_shoot, 0, 1, 1));
    tenm_table_add(normal_enemy_new(x + d, y + d, w, 0,
                                    t_no_escape, -1, 0, -1, 0, 4, 2,
                                    /* move 0 */
                                    t_move, 0.0 - 1.0, 0.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    t_move, -v - 1.0, -v, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 2,
                                    /* move 2 */
                                    t_move, 0.0 - 1.0, 0.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 3,
                                    /* move 3 */
                                    t_move, v - 1.0, v, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    300, t_shoot, 18, 0, 0, 1,
                                    /* shoot 1 */
                                    9999, t_shoot, 318 % t_shoot, 0, 1, 1));
    tenm_table_add(normal_enemy_new(x - d, y + d, w, 0,
                                    t_no_escape, -1, 0, -1, 0, 4, 2,
                                    /* move 0 */
                                    t_move, v - 1.0, -v, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    t_move, 0.0 - 1.0, 0.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 2,
                                    /* move 2 */
                                    t_move, -v - 1.0, v, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 3,
                                    /* move 3 */
                                    t_move, 0.0 - 1.0, 0.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    300, t_shoot, 9, 0, 0, 1,
                                    /* shoot 1 */
                                    9999, t_shoot, 309 % t_shoot, 0, 1, 1));
    tenm_table_add(normal_enemy_new(x - d, y - d, w, 0,
                                    t_no_escape, -1, 0, -1, 0, 4, 2,
                                    /* move 0 */
                                    t_move, 0.0 - 1.0, 0.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    t_move, v - 1.0, v, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 2,
                                    /* move 2 */
                                    t_move, 0.0 - 1.0, 0.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 3,
                                    /* move 3 */
                                    t_move, -v - 1.0, -v, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    300, t_shoot, 0, 0, 0, 1,
                                    /* shoot 1 */
                                    9999, t_shoot, 300 % t_shoot, 0, 1, 1));
    break;
  default:
    fprintf(stderr, "plan_13_formation: undefined what (%d)\n", what);
    break;
  }
  
}
