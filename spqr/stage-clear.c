/* $Id: stage-clear.c,v 1.9 2005/07/10 04:32:37 oohara Exp $ */

#include <stdio.h>
/* malloc */
#include <stdlib.h>
/* strlen, strcmp */
#include <string.h>

#include "const.h"
#include "tenm_object.h"
#include "tenm_graphic.h"
#include "util.h"
#include "tenm_table.h"
#include "stage.h"
#include "score.h"
#include "scheduler.h"
#include "ship.h"

#include "stage-clear.h"

static int stage_clear_act(tenm_object *my, const tenm_object *player);
static int stage_clear_draw(tenm_object *my, int priority);

tenm_object *
stage_clear_new(int t)
{
  tenm_object *new = NULL;
  int *count = NULL;

  /* sanity check */
  if (t < 0)
  {
    fprintf(stderr, "stage_clear_new: t is negative (%d)\n", t);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 2);
  if (count == NULL)
  {
    fprintf(stderr, "stage_clear_new: malloc(count) failed\n");
    return NULL;
  }

  /* list of count
   * [0] timer
   * [1] message delay
   */
  count[0] = -1;
  count[1] = t;

  new = tenm_object_new("stage clear", 0, 0,
                        1, 0.0, 0.0,
                        2, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double))
                        NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&stage_clear_act),
                        (int (*)(tenm_object *, int))
                        (&stage_clear_draw));

  if (new == NULL)
  {
    fprintf(stderr, "stage_clear_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
stage_clear_act(tenm_object *my, const tenm_object *player)
{
  int stage;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "stage_clear_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  if (get_ship() < 0)
    return 1;

  (my->count[0])++;
  if (my->count[0] == 0)
  {
    stage = get_stage_number();
    if ((stage >= 1) && (stage <= 5))
      set_stage_cleared(stage, 1);
  }
  if (my->count[0] >= my->count[1] + 100)
  {
    set_this_stage_cleared(1);
    return 1;
  }
  
  return 0;
}

static int
stage_clear_draw(tenm_object *my, int priority)
{
  int status = 0;
  char temp[32];

  /* sanity check */
  if ((get_stage_number() < 1) || (get_stage_number() > 5))
    return 0;

  if (priority != 1)
    return 0;
  if (my->count[0] < my->count[1])
    return 0;
  if (get_ship() < 0)
    return 0;

  if (get_stage_number() <= 4)
    sprintf(temp, "stage %d cleared", get_stage_number());
  else
    sprintf(temp, "final stage cleared");
  if (draw_string(WINDOW_WIDTH / 2 - ((int) strlen(temp)) * 9 / 2, 190,
                  temp, (int) strlen(temp)) != 0)
  {
    fprintf(stderr, "stage_clear_draw: draw_string (stage number) failed\n");
    status = 1;
  }

  sprintf(temp, "stage score: %8d", get_stage_score(get_stage_number()));
  if (draw_string(WINDOW_WIDTH / 2 - ((int) strlen(temp)) * 9 / 2, 220,
                  temp, (int) strlen(temp)) != 0)
  {
    fprintf(stderr, "stage_title_draw: draw_string (stage score) failed\n");
    status = 1;
  }

  return status;
}
