/* $Id: wall-13.c,v 1.8 2011/08/23 20:53:22 oohara Exp $ */

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
#include "tenm_math.h"

#include "wall-13.h"

static int wall_13_move(tenm_object *my, double turn_per_frame);
static int wall_13_act(tenm_object *my, const tenm_object *player);
static double wall_13_speed(int axis, int t);
static int wall_13_draw(tenm_object *my, int priority);

tenm_object *
wall_13_new(double x, double y, int t)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  int i;

  /* sanity check */
  if (t < 0)
  {
    fprintf(stderr, "wall_13_new: t is negative (%d)\n", t);
    return NULL;
  }

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "wall_13_new: malloc(p) failed\n");
    return NULL;
  }
  p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + 30.0, y - 30.0,
                                             x + 30.0, y + 30.0,
                                             x - 30.0, y + 30.0,
                                             x - 30.0, y - 30.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "wall_13_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 1);
  if (count == NULL)
  {
    fprintf(stderr, "wall_13_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "wall_13_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] move timer
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */

  count[0] = t;

  for (i = 0; i <= 1; i++)
    count_d[i] = wall_13_speed(i, t);

  new = tenm_object_new("wall 13", ATTR_OBSTACLE | ATTR_OPAQUE, 0,
                        1, x, y,
                        1, count, 2, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&wall_13_move),
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&wall_13_act),
                        (int (*)(tenm_object *, int))
                        (&wall_13_draw));
  if (new == NULL)
  {
    fprintf(stderr, "wall_13_new: tenm_object_new failed\n");
    if (count_d != NULL)
      free(count_d);
    if (count != NULL)
      free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  return new;
}

static int
wall_13_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "wall_13_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "wall_13_move: strange turn_per_frame (%f)\n",
            turn_per_frame);
    return 0;
  }

  dx_temp = my->count_d[0] / turn_per_frame;
  dy_temp = my->count_d[1] / turn_per_frame;
  my->x += dx_temp;
  my->y += dy_temp;
  if (my->mass != NULL)
    tenm_move_mass(my->mass, dx_temp, dy_temp);

  if (my->y < -31.0)
    return 1;

  return 0;
}

static int
wall_13_act(tenm_object *my, const tenm_object *player)
{
  int i;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "wall_13_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[0])++;
  /* speed change */
  for (i = 0; i <= 1; i++)
    my->count_d[i] = wall_13_speed(i, my->count[0]);

  return 0;
}

static double
wall_13_speed(int axis, int t)
{
  /* sanity check */
  if ((axis < 0) || (axis > 1))
  {
    fprintf(stderr, "wall_13_speed: strange axis (%d)\n", axis);
    return 0.0;
  }

  if (axis == 0)
  {
    if (t < 1230)
      return -1.0;
    return 0.0;
  }
  else
  {
    if (t < 1800)
      return 0.0;
    return -1.0;
  }

  return 0.0;
}

static int
wall_13_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "wall_13_draw: my is NULL\n");
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
