/* $Id: plan-8.c,v 1.224 2005/01/17 12:38:58 oohara Exp $ */
/* [hard] Silver Chimera */

#include <stdio.h>
/* malloc */
#include <stdlib.h>

#include "scheduler.h"
#include "tenm_table.h"
#include "stage-title.h"
#include "normal-enemy.h"
#include "wall-8.h"
#include "const.h"
#include "tenm_math.h"
#include "tenm_object.h"
#include "mankanshoku.h"

#include "plan-8.h"

static tenm_object *plan_8_more_1_new(void);
static int plan_8_more_1_act(tenm_object *my, const tenm_object *player);

int
plan_8(int t)
{
  int s;
  int what;
  int temp1;
  int temp2;
  int t_shoot;
  int i;
  double x;

  /* sanity check */
  if (t < 0)
    return SCHEDULER_ERROR;

  if (t == 30)
    tenm_table_add(stage_title_new());

  if ((t >= 160) && (t < 2458) && ((t - 160) % 24 == 0))
  {
    tenm_table_add(wall_8_new(30.0, -29.0, 0, t));
    tenm_table_add(wall_8_new(((double) WINDOW_WIDTH) - 30.0, -29.0, 0, t));
  }
  if (t == 2458)
  {
    for (i = 0; i < 4; i++)
    {
      tenm_table_add(wall_8_new(30.0, -74.0 - ((double) i) * 180.0, 0, t));
      tenm_table_add(wall_8_new(((double) WINDOW_WIDTH) - 30.0,
                                -74.0 - ((double) i) * 180.0,
                                0, t));
    }
  }  

  if (((t >= 224) && (t <= 236))
      || ((t >= 249) && (t <= 261))
      || ((t >= 274) && (t <= 286))
      || ((t >= 299) && (t <= 311))
      || ((t >= 324) && (t <= 336)))
  {
    if (t <= 236)
    {
      s = t - 224;
      x = 135.0;
    }
    else if (t <= 261)
    {
      s = t - 249;
      x = 320.0;
    }
    else if (t <= 286)
    {
      s = t - 274;
      x = 505.0;
    }
    else if (t <= 311)
    {
      s = t - 299;
      x = 228.0;
    }
    else
    {
      s = t - 324;
      x = 412.0;
    }

    if ((s == 0) || (s == 12))
    {
      tenm_table_add(normal_enemy_new(x - 45.0, -19.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999,
                                      0.0, 7.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, 9999, 0, 0, 1, 0));
      tenm_table_add(normal_enemy_new(x + 45.0, -19.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999,
                                      0.0, 7.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, 9999, 0, 0, 1, 0));
    }
    if (s == 6)
    {
      tenm_table_add(normal_enemy_new(x, -19.0,
                                      BALL_CAPTAIN, 0,
                                      0, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999,
                                      0.0, 7.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, 9999, 0, 0, 1, 0));
    }
  }

  if (t == 408)
    tenm_table_add(plan_8_more_1_new());

  if ((t >= 584) && (t < 744))
  {
    s = t - 584;
    if ((s % 8 == 0) && (s % 40 != 32))
    {
      if ((s == 0) || (s == 40) || (s == 80) || (s == 120))
      {
        what = BRICK;
        t_shoot = 31;
      }
      else
      {  
        what = BALL_SOLDIER;
        t_shoot = 9999;
      }

      if (s < 40)
      {
        temp1 = 29;
        temp2 = 35;
      }
      else if (s < 80)
      {
        temp1 = 79;
        temp2 = 9999;
      }
      else if (s < 120)
      {
        temp1 = 179;
        temp2 = 9999;
      }
      else
      {
        temp1 = 129;
        temp2 = 35;
      }

      tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) - 472.8,
                                      -19.6153,
                                      what, 0,
                                      0, -1, 0, -1, 0, 3, 3,
                                      /* move 0 */
                                      29,
                                      7.2, 3.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      /* ./rotate.py 400 376 67.3847 
                                       * 299.0769 252.0 0 0.195
                                       */
                                      191,
                                      0.0, 0.0, 0.0, 0.0,
                                      299.0769, 252.0, 0.0, 0.195, 2,
                                      /* move 2 */
                                      9999,
                                      0.0, 7.8, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      temp1, t_shoot,
                                      (62 - s % 31) % 31,
                                      0, 0, 1,
                                      /* shoot 1 */
                                      temp2, t_shoot,
                                      (temp1 + 62 - s % 31) % 31,
                                      0, 1, 2,
                                      /* shoot 2 */
                                      9999, 9999, 0,
                                      0, 0, 2));
    }
  }

  if (t == 680)
  {
    tenm_table_add(normal_enemy_new(60.0, -29.0,
                                    BALL_CAPTAIN, 0,
                                    2, -1, 0, -1, 0, 1, 1,
                                    /* move 0 */
                                    9999,
                                    0.0, 7.5, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    9999, 7, 0, 0, 1, 0));
  }
  if (t == 776)
  {
    tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) - 60.0, -29.0,
                                    BALL_CAPTAIN, 0,
                                    2, -1, 0, -1, 0, 1, 1,
                                    /* move 0 */
                                    9999,
                                    0.0, 7.5, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    9999, 7, 0, 0, 1, 0));
  }
  
  if ((t >= 1000) && (t < 1750))
  {
    s = t - 1000;
    if ((s % 75 == 0) && (s < 750))
    {
      temp1 = rand() % 2;
      tenm_table_add(normal_enemy_new(164.0, -19.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 2,
                                      /* move 0 */
                                      9999,
                                      0.0, 7.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      22 + (temp1 % 2) * 21, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 1, 1));
      tenm_table_add(normal_enemy_new(190.0, -19.0,
                                      BALL_CAPTAIN, 0,
                                      0, -1, 0, -1, 0, 1, 2,
                                      /* move 0 */
                                      9999,
                                      0.0, 7.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      22 + (temp1 % 2) * 21,
                                      9999, 9967 * (temp1 % 2), 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 1, 1));
      tenm_table_add(normal_enemy_new(320.0, -19.0,
                                      BALL_CAPTAIN, 0,
                                      0, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999,
                                      0.0, 7.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, 9999, 0, 0, 1, 0));
      tenm_table_add(normal_enemy_new(450.0, -19.0,
                                      BALL_CAPTAIN, 0,
                                      0, -1, 0, -1, 0, 1, 2,
                                      /* move 0 */
                                      9999,
                                      0.0, 7.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      22 + ((temp1 + 1) % 2) * 21,
                                      9999, 9967 * ((temp1 + 1) % 2), 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 1, 1));
      tenm_table_add(normal_enemy_new(476.0, -19.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 2,
                                      /* move 0 */
                                      9999,
                                      0.0, 7.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      22 + ((temp1 + 1) % 2) * 21,
                                      9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 1, 1));
    }

    if (s == 150)
      tenm_table_add(normal_enemy_new(112.0, -24.0,
                                      BALL_CAPTAIN, 0,
                                      0, -1, 0, -1, 0, 2, 2,
                                      /* move 0 */
                                      600,
                                      0.0, 0.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      9999,
                                      0.0, 0.5, 0.0, 0.2,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* shoot 0 */
                                      600, 41, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 41, 26, 0, 1, 1));
    if (s == 300)
      tenm_table_add(normal_enemy_new(528.0, -24.0,
                                      BALL_CAPTAIN, 0,
                                      0, -1, 0, -1, 0, 2, 2,
                                      /* move 0 */
                                      475,
                                      0.0, 0.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      9999,
                                      0.0, 0.5, 0.0, 0.2,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* shoot 0 */
                                      475, 41, 6, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 41, 30, 0, 1, 1));
  }

  if ((t >= 1904) && (t < 2192))
  {
    s = t - 1904;

    if (s < 288)
    { 
      if (s % 24 == 0)
      {
        what = 1 + rand() % 2;
        if (s < 96)
        {  
          temp1 = rand() % 300;
          for (i = -2; i <= 4; i++)
            tenm_table_add(wall_8_new((double) (i * 300 + temp1), -29.0,
                                      what, t));
        }
        else if (s < 192)
        {  
          temp1 = rand() % 240;
          for (i = -3; i <= 4; i++)
            tenm_table_add(wall_8_new((double) (i * 240 + temp1), -29.0,
                                      what, t));
        }
        else
        {  
          temp1 = rand() % 180;
          for (i = -4; i <= 5; i++)
            tenm_table_add(wall_8_new((double) (i * 180 + temp1), -29.0,
                                      what, t));
        }
      }

      if (s % 24 == 8)
      {  
        tenm_table_add(wall_8_new(210.0, -29.0, 0, t));
        /*
        tenm_table_add(wall_8_new(90.0 + ((double) (rand() % 2)) * 60.0, -29.0,
                                  0, t));
        */
      }

      if (s % 24 == 16)
      {  
        tenm_table_add(wall_8_new(210.0, -29.0, 0, t));
        for (i = 0; i < 3; i++)
        {
          if (i == 1)
            temp1 = 17;
          else
            temp1 = 9999;
          tenm_table_add(normal_enemy_new(80.0 + ((double) i) * 40.0, -29.0,
                                          BALL_SOLDIER, 0,
                                          2, -1, 0, -1, 0, 1, 1,
                                          /* move 0 */
                                          9999,
                                          0.0, 7.5, 0.0, 0.0,
                                          0.0, 0.0, 0.0, 0.0, 0,
                                          /* shoot 0 */
                                          9999, temp1, 11, 0, 1, 0));
        }
        
      }
    }
  }

  if ((t >= 2248) && (t < 2398))
  {
    s = t - 2248;

    if ((s % 32 == 0) && (s < 96))
      tenm_table_add(normal_enemy_new(300.0 + ((double) (s / 32)) * 100.0,
                                      -23.0,
                                      BRICK, 0,
                                      0, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999,
                                      0.0, 8.0 + ((double) (s / 32)) * 1.0,
                                      0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, 11, 0, 0, 1, 0));

    if (s == 96)
    {
      for (i = 0; i < 5; i++)
        tenm_table_add(wall_8_new(90.0 + ((double) i) * 60.0, -29.0, 3, t));
    }

    if ((s % 12 == 0) && (s >= 108) && (s <= 132))
      tenm_table_add(wall_8_new(330.0, -29.0, 3, t));

    if ((s % 3 == 0) && (s >= 105) && (s < 150))
    {
      tenm_table_add(normal_enemy_new(90.0, -19.0,
                                      BALL_SOLDIER, ENEMY_TYPE_WEAK,
                                      0, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999,
                                      0.0, 15.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, 9999, 0, 0, 1, 0));
    }
  }

  if (t == 2458)
    tenm_table_add(mankanshoku_new());

  if (t >= 2543)
  {
    s = t - 2543;
    if (s == 0)
    {
      tenm_table_add(normal_enemy_new(210.0, -24.0,
                                      SQUARE, 0,
                                      0, -1, 0, -1, 0, 4, 2,
                                      /* move 0 */
                                      14,
                                      0.0, 4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      933,
                                      0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      75,
                                      0.0, 0.0, 0.0, 0.1,
                                      0.0, 0.0, 0.0, 0.0, 3,
                                      /* move 3 */
                                      9999,
                                      0.0, 7.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 3,
                                      /* shoot 0 */
                                      947, 19, 0, 35, 1, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 35, 0, 1));
      tenm_table_add(normal_enemy_new(430.0, -24.0,
                                      SQUARE, 0,
                                      0, -1, 0, -1, 0, 4, 2,
                                      /* move 0 */
                                      14,
                                      0.0, 4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      933,
                                      0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      75,
                                      0.0, 0.0, 0.0, 0.1,
                                      0.0, 0.0, 0.0, 0.0, 3,
                                      /* move 3 */
                                      9999,
                                      0.0, 7.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 3,
                                      /* shoot 0 */
                                      947, 19, 0, 145, 1, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 145, 0, 1));
      tenm_table_add(normal_enemy_new(-24.0, 150.0,
                                      SQUARE, 0,
                                      0, -1, 0, -1, 0, 4, 2,
                                      /* move 0 */
                                      14,
                                      4.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      933,
                                      0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      75,
                                      0.0, 0.0, 0.0, 0.1,
                                      0.0, 0.0, 0.0, 0.0, 3,
                                      /* move 3 */
                                      9999,
                                      0.0, 7.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 3,
                                      /* shoot 0 */
                                      947, 19, 0, 57, 1, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 57, 0, 1));
      tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) + 24.0, 150.0,
                                      SQUARE, 0,
                                      0, -1, 0, -1, 0, 4, 2,
                                      /* move 0 */
                                      14,
                                      -4.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      933,
                                      0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      75,
                                      0.0, 0.0, 0.0, 0.1,
                                      0.0, 0.0, 0.0, 0.0, 3,
                                      /* move 3 */
                                      9999,
                                      0.0, 7.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 3,
                                      /* shoot 0 */
                                      947, 19, 0, 123, 1, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 123, 0, 1));
    }

    if ((s == 36) || (s == 48))
    {
      temp1 = 83 - (s - 36);
      if (temp1 < 1)
        temp1 = 1;
      tenm_table_add(normal_enemy_new(555.0, -19.0,
                                      BALL_SOLDIER, ENEMY_TYPE_WEAK,
                                      0, -1, 0, -1, 0, 2, 2,
                                      /* move 0 */
                                      58,
                                      0.0, 4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      9999,
                                      -6.0, -4.0, 0.0, 0.1,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* shoot 0 */
                                      temp1, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 11, 0, 0, 1, 1));
    }
    if ((s == 98) || (s == 110))
    {
      temp1 = 91 - (s - 98);
      if (temp1 < 1)
        temp1 = 1;
      tenm_table_add(normal_enemy_new(85.0, -19.0,
                                      BALL_SOLDIER, ENEMY_TYPE_WEAK,
                                      0, -1, 0, -1, 0, 2, 2,
                                      /* move 0 */
                                      58,
                                      0.0, 4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      9999,
                                      5.0, -4.0, 0.0, 0.1,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* shoot 0 */
                                      temp1, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 11, 0, 0, 1, 1));
    }
    if ((s == 156) || (s == 168))
    {
      temp1 = 103 - (s - 156);
      if (temp1 < 1)
        temp1 = 1;
      tenm_table_add(normal_enemy_new(555.0, -19.0,
                                      BALL_SOLDIER, ENEMY_TYPE_WEAK,
                                      0, -1, 0, -1, 0, 2, 2,
                                      /* move 0 */
                                      58,
                                      0.0, 4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      9999,
                                      -4.0, -4.0, 0.0, 0.1,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* shoot 0 */
                                      temp1, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 11, 0, 0, 1, 1));
    }
    if ((s == 206) || (s == 218))
    {
      temp1 = 123 - (s - 206);
      if (temp1 < 1)
        temp1 = 1;
      tenm_table_add(normal_enemy_new(85.0, -19.0,
                                      BALL_SOLDIER, ENEMY_TYPE_WEAK,
                                      0, -1, 0, -1, 0, 2, 2,
                                      /* move 0 */
                                      58,
                                      0.0, 4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      9999,
                                      3.0, -4.0, 0.0, 0.1,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* shoot 0 */
                                      temp1, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 11, 0, 0, 1, 1));
    }
    if ((s == 236) || (s == 248))
    {
      temp1 = 163 - (s - 236);
      if (temp1 < 1)
        temp1 = 1;
      tenm_table_add(normal_enemy_new(555.0, -19.0,
                                      BALL_SOLDIER, ENEMY_TYPE_WEAK,
                                      0, -1, 0, -1, 0, 2, 2,
                                      /* move 0 */
                                      58,
                                      0.0, 4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      9999,
                                      -2.0, -4.0, 0.0, 0.1,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* shoot 0 */
                                      temp1, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 11, 0, 0, 1, 1));
    }

    if ((s == 450) || (s == 458) || (s == 466) || (s == 474))
    {
      if (s == 474)
      {  
        temp1 = BALL_CAPTAIN;
        temp2 = 23;
      }
      else
      {  
        temp1 = BALL_SOLDIER;
        temp2 = 9999;
      }
      tenm_table_add(normal_enemy_new(95.0, -19.0,
                                      temp1, 0,
                                      0, -1, 0, -1, 0, 3, 2,
                                      /* move 0 */
                                      75 - (s - 450),
                                      0.0, 6.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      /* ./rotate.py 300 95.0 287.0
                                       * 283.0 287.0 0.0 -0.16
                                       */
                                      245,
                                      0.0, 0.0, 0.0, 0.0,
                                      139.0 + ((double) (s - 450)) * 6.0,
                                      431.0 - ((double) (s - 450)) * 6.0,
                                      0.0, -0.16, 2,
                                      /* move 2 */
                                      9999,
                                      6.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      296, temp2, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, temp2, 296 % temp2, 0, 1, 1));
    }
    if ((s == 520) || (s == 528) || (s == 536) || (s == 544))
    {
      if (s == 544)
      {  
        temp1 = BALL_CAPTAIN;
        temp2 = 23;
      }
      else
      {
        temp1 = BALL_SOLDIER;
        temp2 = 9999;
      }
      tenm_table_add(normal_enemy_new(545.0, -19.0,
                                      temp1, 0,
                                      0, -1, 0, -1, 0, 3, 2,
                                      /* move 0 */
                                      75 - (s - 520),
                                      0.0, 6.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      245,
                                      0.0, 0.0, 0.0, 0.0,
                                      501.0 - ((double) (s - 520)) * 6.0,
                                      431.0 - ((double) (s - 520)) * 6.0,
                                      0.0, 0.16, 2,
                                      /* move 2 */
                                      9999,
                                      -6.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* shoot 0 */
                                      296, temp2, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, temp2, 296 % temp2, 0, 1, 1));
    }

  }
  
  return SCHEDULER_SUCCESS;
}

static tenm_object *
plan_8_more_1_new(void)
{
  tenm_object *new = NULL;
  int *count = NULL;

  count = (int *) malloc(sizeof(int) * 6);
  if (count == NULL)
  {
    fprintf(stderr, "plan_8_more_1_new: malloc(count) failed\n");
    return NULL;
  }
  /* list of count
   * [0] total timer
   * [1] number of normal enemies killed
   */
  count[0] = -1;
  count[1] = 0;

  new = tenm_object_new("plan 8 more 1", 0, 0,
                        1, 0.0, 0.0,
                        6, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double))
                        NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&plan_8_more_1_act),
                        (int (*)(tenm_object *, int))
                        NULL);

  if (new == NULL)
  {
    fprintf(stderr, "plan_8_more_1_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
plan_8_more_1_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int j;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "plan_8_more_1_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[0])++;
  if (my->count[0] == 0)
  {
    for (i = 15; i < 360; i += 30)
    {
      if ((i % 180 >= 75) && (i % 180 <= 105))
        continue;
      for (j = 0; j < 360; j += 180)
        tenm_table_add(normal_enemy_new(320.0 + 130.0 * tenm_cos(35 + j)
                                        + 65.0 * tenm_cos(i),
                                        -225.0 + 130.0 * tenm_sin(35 + j)
                                        + 65.0 * tenm_sin(i),
                                        BALL_SOLDIER, 0,
                                        56, my->table_index, 1, -1, 0, 1, 1,
                                        /* move 0 */
                                        9999,
                                        0.0, 7.5, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 0,
                                        /* shoot 0 */
                                        9999, 9999, 0, 0, 1, 0));
    }
    for (j = 0; j < 360; j += 90)
      tenm_table_add(normal_enemy_new(320.0 + 130.0 * tenm_cos(15 + j),
                                      -225.0 + 130.0 * tenm_sin(15 + j),
                                      BALL_SOLDIER, 0,
                                      56, my->table_index, 1, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999,
                                      0.0, 7.5, 0.0, 0.0,
                                      320.0, -225.0, 0.0, -0.6, 0,
                                      /* shoot 0 */
                                      9999, 11, 0, 0, 1, 0));

  }

  if ((my->count[0] == 120) && (my->count[1] < 20))
    return 1;

  if (my->count[0] == 120)
    tenm_table_add(normal_enemy_new((double) (WINDOW_WIDTH / 2), -19.0,
                                    BALL_SOLDIER, 0,
                                    0, -1, 0, -1, 0, 1, 1,
                                    /* move 0 */
                                    9999,
                                    0.0, 7.5, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    9999, 9999, 0, 0, 1, 0));

  if ((my->count[0] == 132) || (my->count[0] == 138) || (my->count[0] == 144))
  {
    tenm_table_add(normal_enemy_new(((double) (WINDOW_WIDTH / 2)) - 20.0,
                                    -19.0,
                                    BALL_SOLDIER, 0,
                                    0, -1, 0, -1, 0, 1, 1,
                                    /* move 0 */
                                    9999,
                                    0.0, 7.5, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    9999, 9999, 0, 0, 1, 0));
    tenm_table_add(normal_enemy_new(((double) (WINDOW_WIDTH / 2)) + 20.0,
                                    -19.0,
                                    BALL_SOLDIER, 0,
                                    0, -1, 0, -1, 0, 1, 1,
                                    /* move 0 */
                                    9999,
                                    0.0, 7.5, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    9999, 9999, 0, 0, 1, 0));
  }

  if (my->count[0] >= 144)
    return 1;

  return 0;
}
