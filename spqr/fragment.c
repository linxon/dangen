/* $Id: fragment.c,v 1.63 2011/08/23 20:02:23 oohara Exp $ */

#include <stdio.h>
/* malloc, rand */
#include <stdlib.h>

#include "tenm_graphic.h"
#include "tenm_math.h"
#include "tenm_object.h"
#include "const.h"

#include "fragment.h"

#define NEAR_ZERO 0.0001

static int fragment_act(tenm_object *my, const tenm_object *player);
static int fragment_draw(tenm_object *my, int priority);

tenm_object *
fragment_new(double x, double y, double dx, double dy,
             double size_fragment, int number_fragment,
             int color, double speed_fragment, double speed_theta,
             int life)
{
  int i;
  int suffix;
  int theta;
  int phi_axis;
  int phi_ellipse;
  int phi_fragment;
  double temp_speed;
  double length_axis;
  double length_ellipse;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;

  /* sanity check */
  if (size_fragment < NEAR_ZERO)
    return NULL;
  if (number_fragment <= 0)
    return NULL;
  if (speed_fragment < NEAR_ZERO)
    return NULL;
  if (life <= 0)
    return NULL;

  count = (int *) malloc(sizeof(int) * 4);
  if (count == NULL)
  {
    fprintf(stderr, "fragment_new: malloc(count) failed\n");
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * (4 + 12 * number_fragment));
  if (count_d == NULL)
  {
    fprintf(stderr, "fragment_new: malloc(count_d) failed\n");
    free(count);
    return NULL;
  }
  
  /* list of count
   * [0] number of fragment
   * [1] color
   * [2] life
   * [3] time passed
   */
  count[0] = number_fragment;
  count[1] = color;
  count[2] = life;
  count[3] = 0;

  /* list of count_d
   * [0] source x
   * [1] source y
   * [2] source dx
   * [3] source dy
   * [4--] fragment (x, y, dx, dy, x0, y0,
   *                 x_r1, x_r2, y_r1, y_r2, theta, dtheta)
   *   (relative to source)
   */
  count_d[0] = x;
  count_d[1] = y;
  count_d[2] = dx;
  count_d[3] = dy;
  for (i = 0; i < number_fragment; i++)
  {
    suffix = i * 12 + 4;
    theta = rand() % 360;
    temp_speed = (double) (6 + rand() % 95) / 100.0;
    count_d[suffix + 0] = 0.0;
    count_d[suffix + 1] = 0.0;
    count_d[suffix + 2] = speed_fragment * temp_speed * tenm_cos(theta);
    count_d[suffix + 3] = speed_fragment * temp_speed * tenm_sin(theta);
    phi_axis = rand() % 180;
    phi_ellipse = rand() % 360;
    phi_fragment = 45 + rand() % 46;
    length_axis = size_fragment * tenm_cos(phi_fragment);
    length_ellipse = size_fragment * tenm_sin(phi_fragment);
    count_d[suffix + 4] = length_axis * tenm_sin(phi_axis)
      * tenm_sin(phi_ellipse);
    count_d[suffix + 5] = length_axis * tenm_sin(phi_axis)
      * (-tenm_cos(phi_ellipse));
    count_d[suffix + 6] = tenm_cos(phi_ellipse);
    count_d[suffix + 7] = -tenm_sin(phi_ellipse) * tenm_cos(phi_axis);
    count_d[suffix + 8] = tenm_sin(phi_ellipse);
    count_d[suffix + 9] = tenm_cos(phi_ellipse) * tenm_cos(phi_axis);
    count_d[suffix + 6] *= length_ellipse;
    count_d[suffix + 7] *= length_ellipse;
    count_d[suffix + 8] *= length_ellipse;
    count_d[suffix + 9] *= length_ellipse;
    count_d[suffix + 10] = 0.0;
    count_d[suffix + 11] = speed_theta - (double) (-15 + (rand() % 31));
  }

  new = tenm_object_new("fragment", 0, 0, 0, x, y,
                        4, count, 4 + 12 * number_fragment, count_d,
                        0, NULL,
                        (int (*)(tenm_object *, double)) NULL,
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&fragment_act),
                        (int (*)(tenm_object *, int)) (&fragment_draw));
  if (new == NULL)
  {
    fprintf(stderr, "fragment_new: tenm_object_new failed\n");
    if (count_d != NULL)
      free(count_d);
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
fragment_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int suffix;

  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  (my->count[3])++;
  if (my->count[3] >= my->count[2])
  {
    (my->count[0]) /= 2;
    my->count[3] = 0;
  }

  if (my->count[0] <= 0)
    return 1;

  /* fragment does not collide with anything, so let't move it here,
   * not in a separate move function
   */
  my->count_d[0] += my->count_d[2];
  my->count_d[1] += my->count_d[3];
  for (i = 0; i < my->count[0]; i++)
  {
    suffix = i * 12 + 4;
    my->count_d[suffix + 0] += my->count_d[suffix + 2];
    my->count_d[suffix + 1] += my->count_d[suffix + 3];
    my->count_d[suffix + 10] += my->count_d[suffix + 11];
  }

  return 0;
}

static int
fragment_draw(tenm_object *my, int priority)
{
  int i;
  int suffix;
  int status = 0;
  double a_x;
  double a_y;
  double b_x;
  double b_y;
  tenm_color color;

  /* sanity check */
  if (my == NULL)
    return 0;
  if (priority != 0)
    return 0;

  switch (my->count[1])
  {
    /* enemy shot */
  case 0:
    color = tenm_map_color(99, 158, 79);
    color = tenm_map_color(99, 158, 114);
    break;
  case 1:
    color = tenm_map_color(99, 158, 114);
    color = tenm_map_color(99, 158, 138);
    break;
  case 2:
    color = tenm_map_color(79, 138, 158);
    color = tenm_map_color(99, 143, 158);
    break;
  case 3:
    color = tenm_map_color(79, 118, 158);
    color = tenm_map_color(99, 128, 158);
    break;
  case 4:
    color = tenm_map_color(118, 99, 158);
    break;
  case 5:
    color = tenm_map_color(142, 99, 158);
    break;
    /* dummy enemy shot */
  case 6:
    color = tenm_map_color(158, 158, 158);
    break;
    /* enemy */
  case 7:
    color = tenm_map_color(95, 47, 13);
    break;
  case 8:
    color = tenm_map_color(61, 95, 13);
    break;
  case 9:
    color = tenm_map_color(95, 13, 68);
    break;
    /* player */
  case 10:
    color = tenm_map_color(10, 75, 139);
    break;
  default:
    color = tenm_map_color(0, 0, 0);
    break;
  }

  for (i = 0; i < my->count[0]; i++)
  {
    suffix = i * 12 + 4;
    a_x = my->count_d[0] + my->count_d[suffix + 0];
    a_y = my->count_d[1] + my->count_d[suffix + 1];
    b_x = my->count_d[suffix + 6] * tenm_cos((int) (my->count_d[suffix + 10]))
      + my->count_d[suffix + 7] * tenm_sin((int) (my->count_d[suffix + 10]));
    b_y = my->count_d[suffix + 8] * tenm_cos((int) (my->count_d[suffix + 10]))
      + my->count_d[suffix + 9] * tenm_sin((int) (my->count_d[suffix + 10]));
    b_x += my->count_d[suffix + 4];
    b_y += my->count_d[suffix + 5];
    b_x *= 0.5;
    b_y *= 0.5;
    if (tenm_draw_line((int) (a_x - b_x), (int) (a_y - b_y),
                       (int) (a_x + b_x), (int) (a_y + b_y),
                       1, color) != 0)
      status = 1;
  }
  return status;
}
