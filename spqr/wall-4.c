/* $Id: wall-4.c,v 1.11 2011/08/23 20:52:51 oohara Exp $ */

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

#include "wall-4.h"

static int wall_4_move(tenm_object *my, double turn_per_frame);
static int wall_4_act(tenm_object *my, const tenm_object *player);
static int wall_4_draw(tenm_object *my, int priority);

tenm_object *
wall_4_new(double x, double y, double speed_slide, 
           int t_slide, int t_slide_initial)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "wall_4_new: malloc(p) failed\n");
    return NULL;
  }
  p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + 30.0, y - 30.0,
                                             x + 30.0, y + 30.0,
                                             x - 30.0, y + 30.0,
                                             x - 30.0, y - 30.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "wall_4_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 2);
  if (count == NULL)
  {
    fprintf(stderr, "wall_4_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "wall_4_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] slide timer
   * [1] t_slide
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */

  count[0] = t_slide_initial;
  count[1] = t_slide;

  count_d[0] = speed_slide;
  count_d[1] = 1.0;

  new = tenm_object_new("wall 4", ATTR_OBSTACLE | ATTR_OPAQUE, 0,
                        1, x, y,
                        2, count, 2, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&wall_4_move),
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&wall_4_act),
                        (int (*)(tenm_object *, int))
                        (&wall_4_draw));

  if (new == NULL)
  {
    fprintf(stderr, "wall_4_new: tenm_object_new failed\n");
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
wall_4_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "wall_4_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "wall_4_move: strange turn_per_frame (%f)\n",
            turn_per_frame);
    return 0;
  }

  dx_temp = my->count_d[0] / turn_per_frame;
  dy_temp = my->count_d[1] / turn_per_frame;
  if ((my->count[1] > 0) && (my->count[0] < 0))
    dx_temp = 0.0;
  my->x += dx_temp;
  my->y += dy_temp;
  if (my->mass != NULL)
    tenm_move_mass(my->mass, dx_temp, dy_temp);

  if (my->y > ((double) WINDOW_HEIGHT) + 31.0)
    return 1;

  return 0;
}

static int
wall_4_act(tenm_object *my, const tenm_object *player)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "wall_13_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[0])++;
  if ((my->count[1] > 0) && (my->count[0] >= my->count[1]))
  {
    my->count[0] = -20;
    my->count_d[0] *= -1.0;
  }

  return 0;
}

static int
wall_4_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  double c;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "wall_4_draw: my is NULL\n");
    return 0;
  }

  if (priority != 0)
    return 0;

  /* decoration */
  color = tenm_map_color(182, 123, 162);
  if ((my->count[1] > 0) && (my->count[0] < 0))
  {
    c = 30.0 + ((double) (my->count[0]));

    if (tenm_draw_line(((int) (my->x + c)),
                       ((int) (my->y - c)),
                       ((int) (my->x + c)),
                       ((int) (my->y + c)),
                       1, color) != 0)
      status = 1;
    if (tenm_draw_line(((int) (my->x + c)),
                       ((int) (my->y + c)),
                       ((int) (my->x - c)),
                       ((int) (my->y + c)),
                       1, color) != 0)
      status = 1;
    if (tenm_draw_line(((int) (my->x - c)),
                       ((int) (my->y + c)),
                       ((int) (my->x - c)),
                       ((int) (my->y - c)),
                       1, color) != 0)
      status = 1;
    if (tenm_draw_line(((int) (my->x - c)),
                       ((int) (my->y - c)),
                       ((int) (my->x + c)),
                       ((int) (my->y - c)),
                       1, color) != 0)
      status = 1;
  }

  /* body */
  color = tenm_map_color(95, 13, 68);

  if (my->count[1] > 0)
  {
    if (tenm_draw_line(((int) (my->x + 10.0)),
                       ((int) (my->y - 10.0)),
                       ((int) (my->x + 10.0)),
                       ((int) (my->y + 10.0)),
                       1, color) != 0)
      status = 1;
    if (tenm_draw_line(((int) (my->x + 10.0)),
                       ((int) (my->y + 10.0)),
                       ((int) (my->x - 10.0)),
                       ((int) (my->y + 10.0)),
                       1, color) != 0)
      status = 1;
    if (tenm_draw_line(((int) (my->x - 10.0)),
                       ((int) (my->y + 10.0)),
                       ((int) (my->x - 10.0)),
                       ((int) (my->y - 10.0)),
                       1, color) != 0)
      status = 1;
    if (tenm_draw_line(((int) (my->x - 10.0)),
                       ((int) (my->y - 10.0)),
                       ((int) (my->x + 10.0)),
                       ((int) (my->y - 10.0)),
                       1, color) != 0)
      status = 1;
  }
  else
  {  
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
  }

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
