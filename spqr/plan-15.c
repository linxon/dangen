/* $Id: plan-15.c,v 1.70 2005/05/03 16:36:29 oohara Exp $ */
/* [easy] Watcher Below */

#include <stdio.h>
/* malloc */
#include <stdlib.h>

#include "scheduler.h"
#include "tenm_table.h"
#include "watcher-below.h"
#include "warning.h"
#include "stage-title.h"
#include "const.h"
#include "normal-enemy.h"
#include "brilliance.h"

#include "plan-15.h"

static tenm_object *plan_15_more_1_new(void);
static int plan_15_more_1_act(tenm_object *my, const tenm_object *player);

int
plan_15(int t)
{
  int s;
  int t_shoot;
  double x;

  /* sanity check */
  if (t < 0)
    return SCHEDULER_ERROR;

  if (t == 30)
    tenm_table_add(stage_title_new());

  if ((t >= 160) && (t <= 702))
  {
    if (t < 220)
    {  
      s = t - 160;
      x = 280.0;
    }
    else if (t < 290)
    {  
      s = t - 220;
      x = 510.0;
    }
    else if (t < 360)
    {  
      s = t - 290;
      x = 340.0;
    }
    else if (t < 430)
    {  
      s = t - 360;
      x = 170.0;
    }
    else if (t < 480)
    {  
      s = t - 430;
      x = 230.0;
    }
    else if (t < 550)
    {  
      s = t - 480;
      x = 370.0;
    }
    else if (t < 600)
    {  
      s = t - 550;
      x = 300.0;
    }
    else if (t < 670)
    {  
      s = t - 600;
      x = 440.0;
    }
    else
    {  
      s = t - 670;
      x = 510.0;
    }

    if (s == 0)
      tenm_table_add(normal_enemy_new(x, -23.0,
                                      BRICK, 0,
                                      0, -1, 0, -1, 0, 1, 2,
                                      /* move 0 */
                                      9999, 0.0, 2.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      60, 9999, 9939, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 1, 1));

    if ((s >= 0) && (s < 32) && (s % 8 == 0))
    {
      if (s == 0)
        t_shoot = 43;
      else
        t_shoot = 9999;
      tenm_table_add(normal_enemy_new(x, -133.0 + ((double) s) * 2.0,
                                      BALL_SOLDIER, 0,
                                      103, -1, 0, -1, 0, 1, 2,
                                      /* move 0 */
                                      9999, 0.0, 2.0, 0.0, 0.0,
                                      x, -23.0 + ((double) s) * 2.0,
                                      0.0, 0.3, 0,
                                      /* shoot 0 */
                                      90 - s, t_shoot, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, t_shoot, (90 - s) % t_shoot,
                                      0, 1, 1));
    }
  }

  if (t == 900)
    tenm_table_add(plan_15_more_1_new());
  
  /*
  if (t == 160)
    tenm_table_add(warning_new());
  if (t == 290)
    tenm_table_add(watcher_below_new());
  */

  return SCHEDULER_SUCCESS;
}

static tenm_object *
plan_15_more_1_new(void)
{
  tenm_object *new = NULL;
  int *count = NULL;

  count = (int *) malloc(sizeof(int) * 4);
  if (count == NULL)
  {
    fprintf(stderr, "plan_15_more_1_new: malloc(count) failed\n");
    return NULL;
  }

  /* list of count
   * [0] mode
   * [1] total timer
   * [2] number of enemies killed
   * [3] mode timer
   */

  count[0] = 0;
  count[1] = -1;
  count[2] = 0;
  count[3] = -1;

  new = tenm_object_new("plan 15 more 1", 0, 0,
                        1, 0.0, 0.0,
                        3, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double))
                        NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&plan_15_more_1_act),
                        (int (*)(tenm_object *, int))
                        NULL);
  if (new == NULL)
  {
    fprintf(stderr, "plan_15_more_1_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
plan_15_more_1_act(tenm_object *my, const tenm_object *player)
{
  int t;
  int t_shoot;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "plan_15_more_1_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[1])++;
  t = my->count[1];

  if ((t >= 0) && (t <= 504))
  {
    if ((t % 8 == 0) && (t % 40 < 32))
    {
      if (t % 16 == 0)
        t_shoot = 43;
      else
        t_shoot = 9999;

      tenm_table_add(normal_enemy_new(((double) t) * 1.0,
                                      -240.0 + ((double) t) * 0.2,
                                      BALL_SOLDIER, 0,
                                      206, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999, 1.0, 0.2, 0.0, 0.0,
                                      ((double) t) * 1.0,
                                      ((double) t) * 0.2,
                                      0.0, 0.15, 0,
                                      /* shoot 0 */
                                      9999, t_shoot, t % t_shoot, 0, 1, 0));
    }
  }

  (my->count[3])++;
  if ((t < 450)
      && (((my->count[0] == 0) && (my->count[3] == 50))
          || ((my->count[0] == 1) && (my->count[3] == 10))))
    tenm_table_add(normal_enemy_new(48.0 + ((double) t) * 1.0,
                                    -240.0 + ((double) t) * 0.2,
                                    BRICK, 0,
                                    48, my->table_index, 2, -1, 0, 3, 3,
                                    /* move 0 */
                                    48, -1.0 + 1.0, 5.0 + 0.2, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    500 - t, 0.0 + 1.0, 0.0 + 0.2, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 2,
                                    /* move 2 */
                                    9999, 1.0 + 1.0, -5.0 + 0.2, 0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 2,
                                    /* shoot 0 */
                                    48, 9999, 0, 0, 1, 1,
                                    /* shoot 1 */
                                    500 - t, 43, 0, 0, 1, 2,
                                    /* shoot 2 */
                                    9999, 9999, 0, 0, 0, 2));
  

  if ((my->count[0] == 2) && (t < 450) && (my->count[3] == 10))
    tenm_table_add(brilliance_new(48.0 + ((double) t) * 1.0,
                                  -240.0 + ((double) t) * 0.2,
                                  500 - t, my->table_index));

  if (my->count[0] == 3)
  {
    if ((my->count[3] >= 0) && (my->count[3] < 16)
        && ((my->count[3] - 0) % 4 == 0))
      tenm_table_add(normal_enemy_new(480.0, -19.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 2,
                                      /* move 0 */
                                      9999, 0.0, 1.5, 0.0, 0.0,
                                      320.0, -19.0, 0.0, 0.3, 0,
                                      /* shoot 0 */
                                      13 - (my->count[3] - 0),
                                      9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 1, 1));
    if ((my->count[3] >= 30) && (my->count[3] < 46)
        && ((my->count[3] - 30) % 4 == 0))
      tenm_table_add(normal_enemy_new(80.0, -19.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 2,
                                      /* move 0 */
                                      9999, 0.0, 1.5, 0.0, 0.0,
                                      320.0, -19.0, 0.0, -0.225, 0,
                                      /* shoot 0 */
                                      19 - (my->count[3] - 30),
                                      9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 9999, 0, 0, 1, 1));
    if ((my->count[3] >= 60) && (my->count[3] < 76)
        && ((my->count[3] - 60) % 4 == 0))
    {
      tenm_table_add(normal_enemy_new(640.0, -19.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 3,
                                      /* move 0 */
                                      9999, 0.0, 1.5, 0.0, 0.0,
                                      320.0, -19.0, 0.0, 0.15, 0,
                                      /* shoot 0 */
                                      55 - (my->count[3] - 60),
                                      9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      15, 9999, 0, 0, 1, 2,
                                      /* shoot 2 */
                                      9999, 9999, 0, 0, 0, 2));
    }
  }

  if (my->count[0] == 4)
  {
    if (my->count[3] == 30)
      tenm_table_add(warning_new());
    if (my->count[3] == 160)
    {  
      tenm_table_add(watcher_below_new());
      return 1;
    }
  }
  
  /* mode change */
  if (t < 450)
  {
    if ((my->count[0] <= 1) && (my->count[2] >= 1))
    { 
      (my->count[0])++;
      my->count[2] = 0;
      my->count[3] = -1;
    }
  }
  else if ((t >= 720) && (my->count[0] <= 2))
  {
    if ((my->count[0] == 2) && (my->count[2] >= 1))
      my->count[0] = 3;
    else
      my->count[0] = 4;
    my->count[2] = 0;
    my->count[3] = -1;
  }

  if ((my->count[0] == 3) && (my->count[3] >= 200))
  {
    my->count[0] = 4;
    my->count[2] = 0;
    my->count[3] = -1;
  }

  return 0;
}
