/* $Id: negation-engine.c,v 1.122 2004/09/30 17:11:04 oohara Exp $ */

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
#include "laser.h"
#include "tenm_math.h"
#include "normal-enemy.h"

#include "negation-engine.h"

#define NEAR_ZERO 0.0001

static int negation_engine_act(tenm_object *my, const tenm_object *player);

static tenm_object *negation_engine_triangle_new(double x, double y,
                                                 double size, int theta_body);
static int negation_engine_triangle_move(tenm_object *my,
                                         double turn_per_frame);
static int negation_engine_triangle_act(tenm_object *my,
                                        const tenm_object *player);
static int negation_engine_triangle_draw(tenm_object *my, int priority);

tenm_object *
negation_engine_new(void)
{
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;

  count = (int *) malloc(sizeof(int) * 5);
  if (count == NULL)
  {
    fprintf(stderr, "negation_engine_new: malloc(count) failed\n");
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 4);
  if (count_d == NULL)
  {
    fprintf(stderr, "negation_engine_new: malloc(count_d) failed\n");
    free(count);
    return NULL;
  }

  /* list of count
   * [0] timer
   * [1] more 1 counter
   * [2] more 2 counter
   * [2] more 3 counter
   */

  /* list of count_d
   */

  count[0] = -1;
  count[1] = 1;
  count[2] = 0;
  count[3] = 0;

  new = tenm_object_new("negation engine",
                        0, 0,
                        1, 0.0, 0.0,
                        5, count, 4, count_d, 0, NULL,
                        (int (*)(tenm_object *, double))
                        NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&negation_engine_act),
                        (int (*)(tenm_object *, int))
                        NULL);
  if (new == NULL)
  {
    fprintf(stderr, "negation_engine_new: tenm_object_new failed\n");
    if (count_d != NULL)
      free(count_d);
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
negation_engine_act(tenm_object *my, const tenm_object *player)
{
  int temp;
  int time_shoot;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "negation_engine_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[0])++;

  if (my->count[0] == 0)
  {
    tenm_table_add(negation_engine_triangle_new(220.0, -490.0,
                                                212.0, 15));
    tenm_table_add(negation_engine_triangle_new(420.0, -150.0,
                                                212.0, 15));
    tenm_table_add(negation_engine_triangle_new(420.0, -820.0,
                                                212.0, 15));
  }

  if ((my->count[0] >= 100) && (my->count[0] <= 3000)
      && (my->count[0] % 25 == 0))
  {
    tenm_table_add(normal_enemy_new(659.0,
                                    71.4730
                                    + 0.1 * ((double) (my->count[0] - 50)),
                                    BALL_SOLDIER, 0,
                                    0, my->table_index, 1, -1, 0, 2, 2,
                                    /* move 0 */
                                    182,
                                    -1.9319 + 0.0,
                                    0.5176 + 0.1,
                                    0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    9999,
                                    0.5176 + 0.0,
                                    -1.9319 + 0.1,
                                    0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* shoot 0 */
                                    210, 120, 0, 50, 0, 1,
                                    /* shoot 1 */
                                    9999, 120, 30, 50, 1, 0));
  }

  if ((my->count[0] >= 1685) && (my->count[0] <= 3885)
      && (my->count[0] % 25 == 10)
      && (my->count[1] >= 2)
      && ((my->count[0] <= 2385) || (my->count[0] % 150 >= 85)))
  {
    my->count[1] -= 2;
    tenm_table_add(normal_enemy_new(107.5703
                                    + 0.1 * ((double) (my->count[0] - 1635))
                                    * 0.26795,
                                    -19.0703,
                                    BALL_SOLDIER, 0,
                                    0, -1, 0, -1, 0, 1, 2,
                                    /* move 0 */
                                    9999,
                                    -0.5176 + 0.0,
                                    1.9319 + 0.1,
                                    0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    90, 110, 0, 50, 0, 1,
                                    /* shoot 1 */
                                    9999, 110, 0, 50, 1, 0));
  }
  if ((my->count[0] > 2385) && (my->count[0] <= 6335)
      && (my->count[0] % 25 == 10)
      && ((my->count[0] % 150 < 85) || (my->count[0] >= 3910)))
  {
    temp = (int) (0.1 * ((double) (my->count[0] - 1635)) / 1.9319);
    if (temp < 1)
      temp = 1;
    if (my->count[0] < 3910)
      time_shoot = 110;
    else if (my->count[0] < 5600)
      time_shoot = 105;
    else
      time_shoot = 100;
    tenm_table_add(normal_enemy_new(107.5703
                                    + 0.1 * ((double) (my->count[0] - 1635))
                                    * 0.26795,
                                    -19.0703,
                                    BALL_SOLDIER, 0,
                                    0, my->table_index, 2, -1, 0, 2, 2,
                                    /* move 0 */
                                    temp,
                                    -0.5176 + 0.0,
                                    1.9319 + 0.1,
                                    0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    9999,
                                    1.9319 + 0.0,
                                    -0.5176 + 0.1,
                                    0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* shoot 0 */
                                    temp + 10, time_shoot, 0, 50, 0, 1,
                                    /* shoot 1 */
                                    9999, time_shoot, (temp + 10) % time_shoot,
                                    50, 1, 0));
  }
  if ((my->count[0] >= 4100) && (my->count[0] < 5600)
      && (my->count[0] % 25 == 0)
      && (my->count[2] >= 2))
  {
    my->count[2] -= 2;
    temp = (int) ((499.0 - 301.4178
                  - 0.1 * ((double) (my->count[0] - 4100))) / 1.9319);
    if (temp < 1)
      temp = 1;
    tenm_table_add(normal_enemy_new(325.9060
                                    + 0.1 * ((double) (my->count[0] - 4100))
                                    * 0.26795,
                                    499.0,
                                    BALL_SOLDIER, 0,
                                    0, -1, 0, -1, 0, 2, 1,
                                    /* move 0 */
                                    temp,
                                    0.5176 + 0.0,
                                    -1.9319 + 0.1,
                                    0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    9999,
                                    1.4142 + 0.0,
                                    1.4142 + 0.1,
                                    0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* shoot 0 */
                                    9999, 20, 0, 50, 1, 0));
  }

  if ((my->count[0] >= 6100) && (my->count[0] <= 9800)
      && (my->count[0] % 25 == 0))
  {
    temp = 60 + (my->count[0] - 6100) / 50;
    if (temp < 1)
      temp = 1;
    tenm_table_add(normal_enemy_new(659.0,
                                    6.4730
                                    + 0.1 * ((double) (my->count[0] - 6100)),
                                    BALL_SOLDIER, 0,
                                    0, my->table_index, 3, -1, 0, 2, 3,
                                    /* move 0 */
                                    182,
                                    -1.9319 + 0.0,
                                    0.5176 + 0.1,
                                    0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* move 1 */
                                    9999,
                                    0.5176 + 0.0,
                                    -1.9319 + 0.1,
                                    0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 1,
                                    /* shoot 0 */
                                    temp, 60, 0, 50, 0, 1,
                                    /* shoot 1 */
                                    100, 60, temp % 60, 50, 1, 2,
                                    /* shoot 2 */
                                    9999, 100, (temp + 100) % 60, 50, 0, 2));
  }
  if ((my->count[0] >= 6460) && (my->count[0] <= 9999)
      && (my->count[0] % 25 == 10)
      && (my->count[3] >= 1))
  {
    (my->count[3])--;
    tenm_table_add(normal_enemy_new(107.5703
                                    + 0.1 * ((double) (my->count[0] - 1635))
                                    * 0.26795,
                                    -19.0703,
                                    BALL_SOLDIER, 0,
                                    0, -1, 0, -1, 0, 1, 2,
                                    /* move 0 */
                                    9999,
                                    -0.5176 + 0.0,
                                    1.9319 + 0.1,
                                    0.0, 0.0,
                                    0.0, 0.0, 0.0, 0.0, 0,
                                    /* shoot 0 */
                                    100, 60, 0, 50, 0, 1,
                                    /* shoot 1 */
                                    9999, 60, 40, 50, 1, 0));
  }

  return 0;
}

static tenm_object *
negation_engine_triangle_new(double x, double y, double size, int theta_body)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;

  /* sanity check */
  if (size < NEAR_ZERO)
  {
    fprintf(stderr, "negation_engine_triangle_new: size is non-potisive "
            "(%f)\n", size);
    return NULL;
  }

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "negation_engine_triangle_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *)
    tenm_polygon_new(3,
                     x + 0.5 * size * tenm_cos(theta_body),
                     y + 0.5 * size * tenm_sin(theta_body),
                     x + 0.5 * size * tenm_cos(theta_body + 120),
                     y + 0.5 * size * tenm_sin(theta_body + 120),
                     x + 0.5 * size * tenm_cos(theta_body + 240),
                     y + 0.5 * size * tenm_sin(theta_body + 240));
  if (p[0] == NULL)
  {
    fprintf(stderr, "negation_engine_triangle_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 5);
  if (count == NULL)
  {
    fprintf(stderr, "negation_engine_triangle_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 4);
  if (count_d == NULL)
  {
    fprintf(stderr, "negation_engine_triangle_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] timer
   * [1] body theta
   * [2] hand theta
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   * [2] size
   */

  count[0] = 0;
  count[1] = theta_body;
  count[2] = 0;

  count_d[0] = 0.0;
  count_d[1] = 4.0;
  count_d[2] = size;

  new = tenm_object_new("negation engine triangle",
                        ATTR_ENEMY | ATTR_OPAQUE, 0,
                        1, x, y,
                        5, count, 4, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&negation_engine_triangle_move),
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&negation_engine_triangle_act),
                        (int (*)(tenm_object *, int))
                        (&negation_engine_triangle_draw));
  if (new == NULL)
  {
    fprintf(stderr, "negation_engine_triangle_new: tenm_object_new failed\n");
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
negation_engine_triangle_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "negation_engine_triangle_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "negation_engine_triangle_move: strange turn_per_frame "
            "(%f)\n", turn_per_frame);
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
negation_engine_triangle_act(tenm_object *my, const tenm_object *player)
{
  int t;
  int i;
  int theta;
  int phi;
  double length;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "negation_engine_triangle_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[0])++;

  if (my->count[0] >= 10790)
    return 1;

  /* speed change */
  if (my->count[0] == 50)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.1;
  }
  if (my->count[0] == 10490)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 2.0;
  }

  /* shoot */
  t = my->count[0] - 50;
  if (t < 0)
    return 0;

  (my->count[2])++;

  if ((t >= 121) && (t % 11 == 0))
  {
    for (i = 0; i < 360; i += 120)
    {
      theta = my->count[1] + i;
      phi = my->count[1] + i - 150 + my->count[2] % 120;
      length = my->count_d[2] * tenm_sqrt(3);
      length *= tenm_sin(120 - my->count[2] % 120) / tenm_sin(60);
      tenm_table_add(laser_angle_new(my->x + my->count_d[2] * tenm_cos(theta)
                                     + length * tenm_cos(phi),
                                     my->y + my->count_d[2] * tenm_sin(theta)
                                     + length * tenm_sin(phi), 5.0,
                                     t * 2,
                                     25.0, 5));
    }
  }
  if (t % 13 == 0)
  {
    for (i = 0; i < 360; i += 120)
    {
      tenm_table_add(laser_angle_new(my->x, my->y, 5.0,
                                     t * (-1) + i,
                                     25.0, 4));
    }
  }

  return 0;
}

static int
negation_engine_triangle_draw(tenm_object *my, int priority)
{
  int i;
  int theta;
  int phi;
  double length;
  int status = 0;
  tenm_color color;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "negation_engine_triangle_draw: my is NULL\n");
    return 0;
  }

  if (priority != 0)
    return 0;

  color = tenm_map_color(182, 123, 162);
  /* decoration */
  for (i = 0; i < 360; i += 120)
  {
    theta = my->count[1] + i;
    if (tenm_draw_line((int) (my->x + my->count_d[2] * tenm_cos(theta) * 0.75),
                       (int) (my->y + my->count_d[2] * tenm_sin(theta) * 0.75),
                       (int) (my->x),
                       (int) (my->y),
                       1, color) != 0)
      status = 1;
  }

  /* hand */
  color = tenm_map_color(182, 123, 162);
  for (i = 0; i < 360; i += 120)
  {
    theta = my->count[1] + i;
    if (tenm_draw_line((int) (my->x + my->count_d[2] * tenm_cos(theta)),
                       (int) (my->y + my->count_d[2] * tenm_sin(theta)),
                       (int) (my->x + my->count_d[2] * tenm_cos(theta + 120)),
                       (int) (my->y + my->count_d[2] * tenm_sin(theta + 120)),
                       1, color) != 0)
      status = 1;
  }

  for (i = 0; i < 360; i += 120)
  {
    theta = my->count[1] + i;
    phi = my->count[1] + i - 150 + my->count[2] % 120;
    length = my->count_d[2] * tenm_sqrt(3);
    length *= tenm_sin(120 - my->count[2] % 120) / tenm_sin(60);
    color = tenm_map_color(95, 13, 68);
    if (tenm_draw_line((int) (my->x + my->count_d[2] * tenm_cos(theta)
                              + length * tenm_cos(phi)),
                       (int) (my->y + my->count_d[2] * tenm_sin(theta)
                              + length * tenm_sin(phi)),
                       (int) (my->x + my->count_d[2] * tenm_cos(theta + 120)
                              + length * tenm_cos(phi + 120)),
                       (int) (my->y + my->count_d[2] * tenm_sin(theta + 120)
                              + length * tenm_sin(phi + 120)),
                       1, color) != 0)
      status = 1;

    color = tenm_map_color(175, 0, 239);
    if (tenm_draw_circle((int) (my->x + my->count_d[2] * tenm_cos(theta)
                                + length * tenm_cos(phi)),
                         (int) (my->y + my->count_d[2] * tenm_sin(theta)
                              + length * tenm_sin(phi)),
                         5, 1, color) != 0)
      status = 1;
  }

  color = tenm_map_color(95, 13, 68);
  /* body */
  for (i = 0; i < 360; i += 120)
  {
    theta = my->count[1] + i;
    if (tenm_draw_line((int) (my->x
                              + 0.5 * my->count_d[2] * tenm_cos(theta)),
                       (int) (my->y
                              + 0.5 * my->count_d[2] * tenm_sin(theta)),
                       (int) (my->x
                              + 0.5 * my->count_d[2] * tenm_cos(theta + 120)),
                       (int) (my->y
                              + 0.5 * my->count_d[2] * tenm_sin(theta + 120)),
                       5, color) != 0)
      status = 1;
  }

  return status;
}
