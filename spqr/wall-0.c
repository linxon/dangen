/* $Id: wall-0.c,v 1.5 2011/08/23 20:52:23 oohara Exp $ */

#include <stdio.h>
/* malloc, rand */
#include <stdlib.h>

#include "const.h"
#include "tenm_object.h"
#include "tenm_graphic.h"
#include "tenm_primitive.h"
#include "util.h"
#include "tenm_table.h"
#include "tenm_math.h"

#include "wall-0.h"

static int wall_0_move(tenm_object *my, double turn_per_frame);
static int wall_0_draw(tenm_object *my, int priority);

tenm_object *
wall_0_new(double x)
{
  double y = -29.0;
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  double *count_d = NULL;

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "wall_0_new: malloc(p) failed\n");
    return NULL;
  }
  p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + 30.0, y - 30.0,
                                             x + 30.0, y + 30.0,
                                             x - 30.0, y + 30.0,
                                             x - 30.0, y - 30.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "wall_0_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "wall_0_new: malloc(count_d) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count_d
   * [0] speed x
   * [1] speed y
   */
  count_d[0] = 0.0;
  count_d[1] = 4.0;

  new = tenm_object_new("wall 0", ATTR_OBSTACLE | ATTR_OPAQUE, 0,
                        1, x, y,
                        0, NULL, 2, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&wall_0_move),
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, int))
                        (&wall_0_draw));


  if (new == NULL)
  {
    fprintf(stderr, "wall_0_new: tenm_object_new failed\n");
    if (count_d != NULL)
      free(count_d);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  return new;
}

static int
wall_0_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "wall_0_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "wall_0_move: strange turn_per_frame (%f)\n",
            turn_per_frame);
    return 0;
  }

  dx_temp = my->count_d[0] / turn_per_frame;
  dy_temp = my->count_d[1] / turn_per_frame;

  my->x += dx_temp;
  my->y += dy_temp;
  if (my->mass != NULL)
    tenm_move_mass(my->mass, dx_temp, dy_temp);

  if (my->y > ((double) WINDOW_HEIGHT) + 31.0)
    return 1;

  return 0;
}

static int
wall_0_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "wall_0_draw: my is NULL\n");
    return 0;
  }

  if (priority != 0)
    return 0;

  /* body */
  color = tenm_map_color(95, 13, 68);

  if (tenm_draw_line(((int) (my->x - 30.0)),
                     ((int) (my->y - 30.0)),
                     ((int) (my->x + 30.0)),
                     ((int) (my->y + 30.0)),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x + 30.0)),
                     ((int) (my->y - 30.0)),
                     ((int) (my->x - 30.0)),
                     ((int) (my->y + 30.0)),
                     1, color) != 0)
    status = 1;

  if (tenm_draw_line(((int) (my->x + 30.0)),
                     ((int) (my->y - 30.0)),
                     ((int) (my->x + 30.0)),
                     ((int) (my->y + 30.0)),
                     2, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x + 30.0)),
                     ((int) (my->y + 30.0)),
                     ((int) (my->x - 30.0)),
                     ((int) (my->y + 30.0)),
                     2, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x - 30.0)),
                     ((int) (my->y + 30.0)),
                     ((int) (my->x - 30.0)),
                     ((int) (my->y - 30.0)),
                     2, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x - 30.0)),
                     ((int) (my->y - 30.0)),
                     ((int) (my->x + 30.0)),
                     ((int) (my->y - 30.0)),
                     2, color) != 0)
    status = 1;

  return status;
}
