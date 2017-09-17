/* $Id: plan-19.c,v 1.30 2011/08/23 20:48:47 oohara Exp $ */
/* [very easy] Gosanpachi */

#include <stdio.h>
/* malloc */
#include <stdlib.h>

#include "scheduler.h"
#include "tenm_table.h"
#include "gosanpachi.h"
#include "stage-title.h"
#include "warning.h"
#include "normal-enemy.h"
#include "const.h"
#include "tenm_graphic.h"
#include "ship.h"

#include "plan-19.h"

static tenm_object *plan_19_more_1_new(void);
static int plan_19_more_1_act(tenm_object *my, const tenm_object *player);
static int plan_19_more_1_draw(tenm_object *my, int priority);

int
plan_19(int t)
{
  /* sanity check */
  if (t < 0)
    return SCHEDULER_ERROR;

  if (t == 30)
    tenm_table_add(stage_title_new());

  if (t == 160)
    tenm_table_add(plan_19_more_1_new());

  return SCHEDULER_SUCCESS;
}

static tenm_object *
plan_19_more_1_new(void)
{
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;

  count = (int *) malloc(sizeof(int) * 4);
  if (count == NULL)
  {
    fprintf(stderr, "plan_19_more_1_new: malloc(count) failed\n");
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 3);
  if (count_d == NULL)
  {
    fprintf(stderr, "plan_19_more_1_new: malloc(count_d) failed\n");
    free(count);
    return NULL;
  }

  /* list of count
   * [0] number of tries
   * [1] mode
   * [2] timer
   * [3] captain timer
   */
  /* list of count_d
   * [0] target x
   * [1] speed
   * [2] distance
   */
  count[0] = 0;
  count[1] = 0;
  count[2] = -1;
  count[3] = -1;

  count_d[0] = 0.0;
  count_d[1] = 3.0;
  count_d[2] = 73.0;
  new = tenm_object_new("plan 19 more 1", 0, 0,
                        1, 0.0, 0.0,
                        4, count, 3, count_d, 0, NULL,
                        (int (*)(tenm_object *, double))
                        NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&plan_19_more_1_act),
                        (int (*)(tenm_object *, int))
                        (&plan_19_more_1_draw));

  if (new == NULL)
  {
    fprintf(stderr, "plan_19_more_1_new: tenm_object_new failed\n");
    if (count_d != NULL)
      free(count_d);
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
plan_19_more_1_act(tenm_object *my, const tenm_object *player)
{
  int what;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "plan_19_more_1_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[2])++;

  switch (my->count[1])
  {
  case 0:
    /* don't lock-on if the player is immutable */
    if ((my->count[2] >= 0) && (get_ship() >= 0) &&(player->count[1] <= 0))
    {  
      my->count_d[0] = player->x;
      my->count_d[1] = 3.0;
      my->count_d[2] = 48.0;
      my->count[1] = 1;
      my->count[2] = -30;
      my->count[3] = -1;
    }
    break;
  case 1:
    /* don't lock-on if the player is immutable */
    if ((my->count[2] >= 30)
        && ((player->x < my->count_d[0] - 20.0)
            || (player->x > my->count_d[0] + 20.0)
            || (get_ship() < 0)
            || (player->count[1] > 0)
            || (my->count[2] >= 500)))
    {
      (my->count[0])++;
      if (my->count[0] >= 2)
      {
        my->count[1] = 2;
        my->count[2] = -1;
      }
      else
      {  
        my->count[1] = 0;
        my->count[2] = -100;
      }
      return 0;
    }

    if (my->count[2] < 0)
      return 0;

    (my->count[3])++;
    my->count_d[1] += 0.05;
    my->count_d[2] += my->count_d[1];
    if (my->count_d[2] > 76.0)
    {
      if (my->count[3] >= 0)
      {
        my->count[3] = -25;
        what = BALL_CAPTAIN;
      }
      else
      {
        what = BALL_SOLDIER;
      }
      
      my->count_d[2] -= 75.0;
      tenm_table_add(normal_enemy_new(my->count_d[0], -19.0,
                                      what, 0,
                                      0, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999, 0.0, my->count_d[1], 0.0, 0.05,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, 9999, 0, 0, 1, 0));
    }
    break;
  case 2:
    if (my->count[2] == 100)
      tenm_table_add(warning_new());
    if (my->count[2] == 230)
    {
      tenm_table_add(gosanpachi_new());
      return 1;
    }
    break;
  default:
    fprintf(stderr, "plan_19_more_1_act: undefined mode (%d)\n",
            my->count[1]);
    break;
  }
  
  return 0;
}

static int
plan_19_more_1_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "plan_19_more_1_draw: my is NULL\n");
    return 0;
  }

  if (priority != 0)
    return 0;

  color = tenm_map_color(158, 158, 158);
  if (my->count[1] == 1)
  {
    if (tenm_draw_line((int) (my->count_d[0]), 0,
                       (int) (my->count_d[0]), WINDOW_HEIGHT,
                       1, color) != 0)
    status = 1;
  }

  return status;
}
