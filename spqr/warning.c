/* $Id: warning.c,v 1.7 2004/08/13 15:31:00 oohara Exp $ */

#include <stdio.h>
/* malloc, rand */
#include <stdlib.h>
/* strlen, strcmp */
#include <string.h>

#include "const.h"
#include "tenm_object.h"
#include "tenm_graphic.h"
#include "tenm_primitive.h"
#include "util.h"
#include "tenm_table.h"
#include "background.h"
#include "tenm_math.h"

#include "warning.h"

static int warning_act(tenm_object *my, const tenm_object *player);
static int warning_draw(tenm_object *my, int priority);

tenm_object *
warning_new(void)
{
  tenm_object *new = NULL;
  int *count = NULL;

  count = (int *) malloc(sizeof(int) * 5);
  if (count == NULL)
  {
    fprintf(stderr, "warning_new: malloc(count) failed\n");
    return NULL;
  }

  /* list of count
   * [0] timer
   */

  count[0] = 0;

  new = tenm_object_new("warning", 0, 0,
                        1, 0.0, 0.0,
                        1, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double))
                        NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&warning_act),
                        (int (*)(tenm_object *, int))
                        (&warning_draw));
  if (new == NULL)
  {
    fprintf(stderr, "warning_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
warning_act(tenm_object *my, const tenm_object *player)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "warning_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  if ((my->count[0] == 0) || (my->count[0] == 50))
    set_background(3);

  (my->count[0])++;
  if (my->count[0] >= 100)
    return 1;

  return 0;
}

static int
warning_draw(tenm_object *my, int priority)
{
  int status = 0;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "warning_draw: my is NULL\n");
    return 0;
  }

  if (priority != 1)
    return 0;

  if (draw_string(WINDOW_WIDTH - 1 - my->count[0] * 10, 300,
                  "warning: boss approaching", 25) != 0)
  {
    fprintf(stderr, "warning_draw: draw_string failed\n");
    status = 1;
  }

  return status;
}

