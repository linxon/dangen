/* $Id: stage-title.c,v 1.7 2011/08/23 20:49:38 oohara Exp $ */

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

#include "stage-title.h"

static int stage_title_act(tenm_object *my, const tenm_object *player);
static int stage_title_draw(tenm_object *my, int priority);

tenm_object *
stage_title_new(void)
{
  tenm_object *new = NULL;
  int *count = NULL;

  count = (int *) malloc(sizeof(int) * 1);
  if (count == NULL)
  {
    fprintf(stderr, "stage_title_new: malloc(count) failed\n");
    return NULL;
  }

  /* list of count
   * [0] timer
   */
  count[0] = 0;

  new = tenm_object_new("stage title", 0, 0,
                        1, 0.0, 0.0,
                        1, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double))
                        NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&stage_title_act),
                        (int (*)(tenm_object *, int))
                        (&stage_title_draw));

  if (new == NULL)
  {
    fprintf(stderr, "stage_title_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
stage_title_act(tenm_object *my, const tenm_object *player)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "stage_title_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[0])++;
  if (my->count[0] >= 100)
    return 1;

  return 0;
}

static int
stage_title_draw(tenm_object *my, int priority)
{
  int status = 0;
  char temp[32];

  /* sanity check */
  if ((get_stage_number() < 1) || (get_stage_number() > 5))
    return 0;
  if (get_stage_name(get_stage_number()) == NULL)
    return 0;

  if (priority != 1)
    return 0;

  if (get_stage_number() <= 4)
    sprintf(temp, "stage %d", get_stage_number());
  else
    sprintf(temp, "final stage");
  if (draw_string(WINDOW_WIDTH / 2 - ((int) strlen(temp)) * 9 / 2, 190,
                  temp, (int) strlen(temp)) != 0)
  {
    fprintf(stderr, "stage_title_draw: draw_string (stage number) failed\n");
    status = 1;
  }

  sprintf(temp, "%.20s", get_stage_name(get_stage_number()));
  if (draw_string(WINDOW_WIDTH / 2 - ((int) strlen(temp)) * 9 / 2, 220,
                  temp, (int) strlen(temp)) != 0)
  {
    fprintf(stderr, "stage_title_draw: draw_string (stage name) failed\n");
    status = 1;
  }

  return status;
}
