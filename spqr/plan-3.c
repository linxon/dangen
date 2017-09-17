/* $Id: plan-3.c,v 1.87 2004/12/12 17:15:29 oohara Exp $ */
/* [hard] Seiron */

#include <stdio.h>
/* malloc */
#include <stdlib.h>

#include "scheduler.h"
#include "tenm_table.h"
#include "seiron.h"
#include "warning.h"
#include "stage-title.h"
#include "normal-enemy.h"
#include "const.h"

#include "plan-3.h"

static tenm_object *plan_3_more_1_new(void);
static int plan_3_more_1_act(tenm_object *my, const tenm_object *player);

int
plan_3(int t)
{
  int s;
  int what;
  int t_shoot;
  double x;
  double y;
  double c;

  /* sanity check */
  if (t < 0)
    return SCHEDULER_ERROR;

  if (t == 30)
    tenm_table_add(stage_title_new());

  if (t == 160)
    tenm_table_add(normal_enemy_new(128.0, -42.0,
                                    TRIANGLE, 0,
                                    0, -1, 0, -1, 0, 2, 2,
                                    /* move 0 */
                                    15, 0.0, 4.0 + 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    9999, 0.0, 0.0 + 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* shoot 0 */
                                    15, 9999, 0, 0, 0, 1,
                                    /* shoot 1 */
                                    9999, 37, 36, 0, 1, 1));

  if ((t >= 160) && (t < 240))
  {
    s = t - 160;
    if ((s % 8 == 0) && (s % 40 < 24))
    {
      if (s % 40 == 0)
      {
        what = BALL_CAPTAIN;
        t_shoot = 23;
      }
      else
      {  
        what = BALL_SOLDIER;
        t_shoot = 9999;
      }

      x = ((double) WINDOW_WIDTH) + 19.0;
      y = -50.0 + 1.0 * ((double) s);

      if (s < 30)
        tenm_table_add(normal_enemy_new(x, y, what, 0,
                                        20, -1, 0, -1, 0, 1, 3,
                                        /* move 0 */
                                        9999, -6.0, 2.5 + 1.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 0,
                                        /* shoot 0 */
                                        30 - s, t_shoot, 0, 0, 0, 1,
                                        /* shoot 1 */
                                        30, t_shoot, (30 - s) % t_shoot,
                                        0, 1, 2,
                                        /* shoot 2 */
                                        9999, t_shoot, (60 - s) % t_shoot,
                                        0, 0, 2));
      else
        tenm_table_add(normal_enemy_new(x, y, what, 0,
                                        20, -1, 0, -1, 0, 1, 2,
                                        /* move 0 */
                                        9999, -6.0, 2.5 + 1.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 0,
                                        /* shoot 0 */
                                        50 - s % 40, t_shoot, 0, 0, 0, 1,
                                        /* shoot 1 */
                                        9999, t_shoot, (50 - s % 40) % t_shoot,
                                        0, 1, 1));
    }
  }

  if (t == 310)
    tenm_table_add(normal_enemy_new(512.0, -42.0,
                                    TRIANGLE, 0,
                                    0, -1, 0, -1, 0, 2, 2,
                                    /* move 0 */
                                    15, 0.0, 4.0 + 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    9999, 0.0, 0.0 + 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* shoot 0 */
                                    15, 9999, 0, 0, 0, 1,
                                    /* shoot 1 */
                                    9999, 37, 36, 0, 1, 1));

  if ((t >= 310) && (t < 390))
  {
    s = t - 310;
    if ((s % 8 == 0) && (s % 40 < 24))
    {
      if (s % 40 == 0)
      {
        what = BALL_CAPTAIN;
        t_shoot = 23;
      }
      else
      {  
        what = BALL_SOLDIER;
        t_shoot = 9999;
      }

      x = -19.0;
      y = 153.0 + 1.0 * ((double) s);

      if (s < 30)
        tenm_table_add(normal_enemy_new(x, y, what, 0,
                                        20, -1, 0, -1, 0, 1, 3,
                                        /* move 0 */
                                        9999, 6.0, -2.5 + 1.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 0,
                                        /* shoot 0 */
                                        30 - s, t_shoot, 0, 0, 0, 1,
                                        /* shoot 1 */
                                        30, t_shoot, (30 - s) % t_shoot,
                                        0, 1, 2,
                                        /* shoot 2 */
                                        9999, t_shoot, (60 - s) % t_shoot,
                                        0, 0, 2));
      else
        tenm_table_add(normal_enemy_new(x, y, what, 0,
                                        20, -1, 0, -1, 0, 1, 2,
                                        /* move 0 */
                                        9999, 6.0, -2.5 + 1.0, 0.0, 0.0,
                                        0.0, 0.0, 0.0, 0.0, 0,
                                        /* shoot 0 */
                                        50 - s % 40, t_shoot, 0, 0, 0, 1,
                                        /* shoot 1 */
                                        9999, t_shoot, (50 - s % 40) % t_shoot,
                                        0, 1, 1));
    }
  }

  if (t == 460)
    tenm_table_add(normal_enemy_new(320.0, -42.0,
                                    TRIANGLE, 0,
                                    0, -1, 0, -1, 0, 2, 2,
                                    /* move 0 */
                                    15, 0.0, 4.0 + 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    9999, 0.0, 0.0 + 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* shoot 0 */
                                    15, 9999, 0, 0, 0, 1,
                                    /* shoot 1 */
                                    9999, 37, 36, 0, 1, 1));

  if ((t >= 460) && (t < 620))
  {
    s = t - 460;
    if ((s % 8 == 0) && (s % 40 < 24))
    {
      if (s % 40 == 0)
      {
        what = BALL_CAPTAIN;
        t_shoot = 23;
      }
      else
      {  
        what = BALL_SOLDIER;
        t_shoot = 9999;
      }

      if (s % 80 < 40)
      {        
        x = -19.0;
        y = 20.0 + 1.0 * ((double) s);

        tenm_table_add(normal_enemy_new(x, y, what, 0,
                                        73, -1, 0, -1, 0, 1, 1,
                                        /* move 0 */
                                        9999, 0.0, 0.0 + 1.0, 0.0, 0.0,
                                        41.0, 123.9230 + 1.0 * ((double) s),
                                        0.0, 0.25, 0,
                                        /* shoot 0 */
                                        9999, t_shoot, 0, 0, 1, 0));
      }
      else
      {        
        x = ((double) WINDOW_WIDTH) + 19.0;
        y = -100.0 + 1.0 * ((double) s);

        tenm_table_add(normal_enemy_new(x, y, what, 0,
                                        73, -1, 0, -1, 0, 1, 1,
                                        /* move 0 */
                                        9999, 0.0, 0.0 + 1.0, 0.0, 0.0,
                                        ((double) WINDOW_WIDTH) - 41.0,
                                        3.9230 + 1.0 * ((double) s),
                                        0.0, -0.25, 0,
                                        /* shoot 0 */
                                        9999, t_shoot, 0, 0, 1, 0));
      }
    }
  }

  if ((t >= 630) && (t < 710))
  {
    s = t - 630;
    if (s % 8 == 0)
    {
      x = -19.0;
      y = 250.0 + 1.0 * ((double) s);

      tenm_table_add(normal_enemy_new(x, y, BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 2,
                                      /* move 0 */
                                      9999, 6.0, -2.5 + 1.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      85 - s, 9999, 9999 - (85 - s), 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 1, 1));
    }
  }

  if (t == 705)
    tenm_table_add(normal_enemy_new(210.0, -42.0,
                                    TRIANGLE, 0,
                                    0, -1, 0, -1, 0, 2, 2,
                                    /* move 0 */
                                    15, 0.0, 4.0 + 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    9999, 0.0, 0.0 + 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* shoot 0 */
                                    15, 9999, 0, 0, 0, 1,
                                    /* shoot 1 */
                                    9999, 37, 36, 0, 1, 1));
  if (t == 796)
    tenm_table_add(normal_enemy_new(420.0, -42.0,
                                    TRIANGLE, 0,
                                    0, -1, 0, -1, 0, 2, 2,
                                    /* move 0 */
                                    15, 0.0, 4.0 + 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    9999, 0.0, 0.0 + 1.0, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* shoot 0 */
                                    15, 9999, 0, 0, 0, 1,
                                    /* shoot 1 */
                                    9999, 37, 36, 0, 1, 1));

  if ((t >= 756) && (t < 836))
  {
    s = t - 756;
    if (s % 8 == 0)
    {
      x = -19.0;
      y = 69 + 1.0 * ((double) s);

      t_shoot = 43;

      tenm_table_add(normal_enemy_new(x, y, BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 2,
                                      /* move 0 */
                                      9999, 6.0, -2.5 + 1.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      30, t_shoot, s % t_shoot, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, t_shoot, (s + 30) % t_shoot,
                                      0, 1, 1));
    }
  }

  if ((t >= 1020) && (t < 1380))
  {
    s = t - 1020;
    if ((s % 9 == 0) && ((s >= 90) || (s % 45 < 36)))
    {
      if (s < 90)
      {
        if (s < 45)
        {
          x = -19.0;
          c = -1.0;
        }
        else
        {
          x = ((double) WINDOW_WIDTH) + 19.0;
          c = 1.0;
        }

        if (s % 45 == 0)
          t_shoot = 13;
        else
          t_shoot = 9999;
      }
      else
      {        
        if (s % 18 == 0)
        {
          x = -19.0;
          c = -1.0;
        }
        else
        {
          x = ((double) WINDOW_WIDTH) + 19.0;
          c = 1.0;
        }

        t_shoot = 43;
      }

      if (s % 45 == 0)
        what = BALL_CAPTAIN;
      else
        what = BALL_SOLDIER;
      y = 209.8075;

      tenm_table_add(normal_enemy_new(x, y, what, 0,
                                      0, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999, 0.0, 0.0, 0.0, 0.0,
                                      320.0 + 189.0 * c, -50.0,
                                      0.0, 0.16 * c, 0,
                                      /* shoot 0 */
                                      9999, t_shoot, s % t_shoot, 0, 1, 0));
    }
  }

  if (t == 1480)
    tenm_table_add(plan_3_more_1_new());

  return SCHEDULER_SUCCESS;
}

static tenm_object *
plan_3_more_1_new(void)
{
  tenm_object *new = NULL;
  int *count = NULL;

  count = (int *) malloc(sizeof(int) * 3);
  if (count == NULL)
  {
    fprintf(stderr, "plan_3_more_1_new: malloc(count) failed\n");
    return NULL;
  }

  /* list of count
   * [0] mode
   * [1] timer
   * [2] number of enemies killed
   */
  count[0] = 0;
  count[1] = -1;
  count[2] = 0;

  new = tenm_object_new("plan 3 more 1", 0, 0,
                        1, 0.0, 0.0,
                        3, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double))
                        NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&plan_3_more_1_act),
                        (int (*)(tenm_object *, int))
                        NULL);

  if (new == NULL)
  {
    fprintf(stderr, "plan_3_more_1_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
plan_3_more_1_act(tenm_object *my, const tenm_object *player)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "plan_3_more_1_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[1])++;
  if (my->count[0] == 0)
  {
    if (my->count[1] == 0)
    {
      tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) + 49.0, 300.0,
                                      TRIANGLE, 0,
                                      0, my->table_index, 2, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999, -3.6, -1.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, 17, 0, 0, 1, 0));
    }
    if (my->count[1] == 30)
    {
      tenm_table_add(normal_enemy_new(-49.0, 300.0,
                                      TRIANGLE, 0,
                                      0, my->table_index, 2, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999, 3.6, -1.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, 17, 0, 0, 1, 0));
    }

    if ((my->count[1] <= 160) && (my->count[2] >= 2))
    {
      my->count[0] = 1;
      my->count[1] = -1;
    }
    else if (my->count[1] >= 370)
    {
      my->count[0] = 2;
      my->count[1] = -1;
    }
  }
  else if (my->count[0] == 1)
  {
    if ((my->count[1] >= 30) && (my->count[1] < 110)
        && (my->count[1] % 8 == 0))
    {
      tenm_table_add(normal_enemy_new(((double) WINDOW_WIDTH) + 19.0, 300.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999, -6.0, -2.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, 9999, 0, 0, 1, 0));
      tenm_table_add(normal_enemy_new(-19.0, 300.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999, 6.0, -2.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, 9999, 0, 0, 1, 0));
    }

    if (my->count[1] >= 260)
    { 
      my->count[0] = 2;
      my->count[1] = -1;
    }
  }
  else if (my->count[0] == 2)
  {
    if (my->count[1] == 0)
      tenm_table_add(warning_new());
    if (my->count[1] == 130)
    { 
      tenm_table_add(seiron_new());
      return 1;
    }
  }

  return 0;
}
