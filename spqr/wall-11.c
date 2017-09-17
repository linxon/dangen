/* $Id: wall-11.c,v 1.8 2004/12/12 16:13:15 oohara Exp $ */

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
#include "normal-shot.h"

#include "wall-11.h"

#define NEAR_ZERO 0.0001

static int wall_11_move(tenm_object *my, double turn_per_frame);
static int wall_11_act(tenm_object *my, const tenm_object *player);
static int wall_11_draw(tenm_object *my, int priority);

tenm_object *
wall_11_new(double x, double y, double speed, int theta, int t_shoot)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  int i;
  double result[2];
  double v[2];
  double c[10];
  double d[10];

  /* sanity check */
  if (speed < NEAR_ZERO)
  {
    fprintf(stderr, "wall_11_new: speed is too small (%f)\n", speed);
    return NULL;
  }
  if (t_shoot <= 0)
  {
    fprintf(stderr, "wall_11_new: t_shoot is non-positive (%d)\n", t_shoot);
    return NULL;
  }

  c[0] = 30.0;
  c[1] = 0.0;
  c[2] = 20.0;
  c[3] = 10.0;
  c[4] = -10.0;
  c[5] = 10.0;
  c[6] = -10.0;
  c[7] = -10.0;
  c[8] = 20.0;
  c[9] = -10.0;

  for (i = 0; i < 5; i++)
  {
    v[0] = c[i * 2 + 0];
    v[1] = c[i * 2 + 1];
    result[0] = v[0];
    result[1] = v[1];
    vector_rotate(result, v, theta);
    d[i * 2 + 0] = result[0];
    d[i * 2 + 1] = result[1];
  }

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "wall_11_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_polygon_new(5,
                                             x + d[0], y + d[1],
                                             x + d[2], y + d[3],
                                             x + d[4], y + d[5],
                                             x + d[6], y + d[7],
                                             x + d[8], y + d[9]);
  if (p[0] == NULL)
  {
    fprintf(stderr, "wall_11_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 4);
  if (count == NULL)
  {
    fprintf(stderr, "wall_11_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 12);
  if (count_d == NULL)
  {
    fprintf(stderr, "wall_11_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] timer
   * [1] escape time
   * [2] shoot time
   * [3] theta
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   * [2 -- 11] mass vertexes
   */

  count[0] = 0;
  count[1] = ((int) (900.0 / speed)) + 1;
  if (count[1] < 1)
    count[1] = 1;
  count[2] = t_shoot;
  count[3] = theta;

  count_d[0] = speed * tenm_cos(theta);
  count_d[1] = speed * tenm_sin(theta);
  for (i = 0; i < 10; i++)
    count_d[2 + i] = d[i];

  new = tenm_object_new("wall 11", ATTR_OBSTACLE, 0,
                        1, x, y,
                        4, count, 12, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&wall_11_move),
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&wall_11_act),
                        (int (*)(tenm_object *, int))
                        (&wall_11_draw));

  if (new == NULL)
  {
    fprintf(stderr, "wall_11_new: tenm_object_new failed\n");
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
wall_11_move(tenm_object *my, double turn_per_frame)
{  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "wall_11_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "wall_11_move: strange turn_per_frame (%f)\n",
            turn_per_frame);
    return 0;
  }

  dx_temp = my->count_d[0] / turn_per_frame;
  dy_temp = my->count_d[1] / turn_per_frame;
  my->x += dx_temp;
  my->y += dy_temp;
  if (my->mass != NULL)
    tenm_move_mass(my->mass, dx_temp, dy_temp);

  return 0;
}

static int
wall_11_act(tenm_object *my, const tenm_object *player)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "wall_8_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[0])++;
  if (my->count[0] >= my->count[1])
    return 1;

  if (my->count[0] % my->count[2] == 0)
  {    
    tenm_table_add(normal_shot_angle_new(my->x, my->y, 4.5,
                                         my->count[3] + 150, 4));
    tenm_table_add(normal_shot_angle_new(my->x, my->y, 4.5,
                                         my->count[3] - 150, 4));
  }

  return 0;
}

static int
wall_11_draw(tenm_object *my, int priority)
{
  int i;
  int status = 0;
  tenm_color color;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "wall_11_draw: my is NULL\n");
    return 0;
  }

  if (priority != 0)
    return 0;

  /* body */
  color = tenm_map_color(95, 13, 68);

  for (i = 0; i < 5; i++)
  {
    if (tenm_draw_line(((int) (my->x + my->count_d[2 + (i * 2 + 0) % 10])),
                       ((int) (my->y + my->count_d[2 + (i * 2 + 1) % 10])),
                       ((int) (my->x + my->count_d[2 + (i * 2 + 2) % 10])),
                       ((int) (my->y + my->count_d[2 + (i * 2 + 3) % 10])),
                       3, color) != 0)
      status = 1;
  }

  return status;
}
