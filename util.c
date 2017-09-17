/* $Id: util.c,v 1.57 2005/07/11 21:23:12 oohara Exp $ */

/* NOT_HAVE_POSIX */
#include <config.h>

#include <stdio.h>
/* malloc, strtol */
#include <stdlib.h>
/* errno */
#include <errno.h>
/* INT_MIN, INT_MAX */
#include <limits.h>

#include "tenm_graphic.h"
#include "tenm_table.h"
#include "tenm_primitive.h"
#include "tenm_collision.h"
#include "tenm_object.h"
#include "tenm_math.h"
#include "const.h"
#include "explosion.h"

#include "util.h"

#define NEAR_ZERO 0.0001

#ifdef NOT_HAVE_POSIX
#define FONTDIR "spqr/image/"
#else /* not NOT_HAVE_POSIX */
#define FONTDIR DATADIR "/games/dangen/image/"
#endif

static tenm_image *font = NULL;
static tenm_primitive *window = NULL;
static double window_width = 0.0;
static double window_height = 0.0;
static char *buffer = NULL;
static int buffer_size = 0;

static int in_window_point(double x, double y);

/* return 0 on success, 1 on error */
int
util_init(int width, int height)
{
  font = tenm_load_image(1, FONTDIR "/" "font.png",
                         1, 95,
                         tenm_map_color(255, 255, 255));

  window= (tenm_primitive *) tenm_polygon_new(4,
                                              0.0, 0.0,
                                              0.0, (double) height,
                                              (double) width, 0.0,
                                              (double) width, (double) height);
  if (window == NULL)
  {
    tenm_image_delete(font);
    fprintf(stderr, "util_init: tenm_polygon_new failed\n");
    return 1;
  }
  window_width = width;
  window_height = height;

  return 0;
}

void
util_quit(void)
{
  if (font != NULL)
  {
    tenm_image_delete(font);
    font = NULL;
  }
  if (window != NULL)
  {
    (window->delete)(window);
    window = NULL;
  }
  window_width = 0.0;
  window_height = 0.0;
  if (buffer != NULL)
  {
    free(buffer);
    buffer = NULL;
  }
  buffer_size = 0;
}

int
draw_string(int x, int y, const char *string, int length)
{
  /* sanity check */
  if (font == NULL)
    return 0;
  if (string == NULL)
    return 0;
  if (length <= 0)
    return 0;

  if (tenm_draw_string(x, y, font, string, length) != 0)
  {
    fprintf(stderr, "draw_string: tenm_draw_string failed\n");
    return 1;
  }
  return 0;
}

int
draw_string_int(int x, int y, const int *string, int length)
{
  int i;
  char *buffer_temp;

  /* sanity check */
  if (font == NULL)
    return 0;
  if (string == NULL)
    return 0;
  if (length <= 0)
    return 0;

  /* don't substitute buffer directly, or you will be in a trouble
   * if realloc fails (you don't need data in buffer, but you still
   * need to free buffer in any case) */
  if ((buffer == NULL) || (length + 1 > buffer_size))
  {
    if (buffer == NULL)
      buffer_temp = (char *) malloc(sizeof(char) * (length + 1));
    else
      buffer_temp = (char *) realloc(buffer, sizeof(char) * (length + 1));
    if (buffer_temp == NULL)
    {
      fprintf(stderr, "draw_string_int: memory allocation to buffer failed\n");
      return 1;
    }
    buffer = buffer_temp;
    buffer_size = length + 1;
  }

  /* stupid way to get a pointer to char
   * (char * and int * are incompatible)
   */
  for (i = 0; i < length; i++)
    buffer[i] = (char) string[i];
  buffer[length] = '\0';

  if (tenm_draw_string(x, y, font, buffer, length) != 0)
  {
    fprintf(stderr, "draw_string_int: tenm_draw_string failed\n");
    return 1;
  }
  return 0;
}

/* return 1 (true) or 0 (false) */
int
in_window_object(const tenm_object *p)
{
  int i;

  if (window == NULL)
  {
    fprintf(stderr, "in_window_primitive: window is NULL\n");
    return 0;
  }
  if (p == NULL)
  {
    fprintf(stderr, "in_window_primitive: p is NULL\n");
    return 0;
  }
  if (p->mass == NULL)
  {
    fprintf(stderr, "in_window_primitive: p->mass is NULL\n");
    return 0;
  }
  for (i = 0; i < p->mass->n; i++)
    if (in_window_primitive(p->mass->p[i]))
      return 1;
  return 0;
}

/* optimized under the assumption that p is usually in the window
 * return 1 (true) or 0 (false) */
int
in_window_primitive(const tenm_primitive *p)
{
  double temp_x;
  double temp_y;
  double temp_r;
  int i;

  if (window == NULL)
  {
    fprintf(stderr, "in_window_primitive: window is NULL\n");
    return 0;
  }
  if (p == NULL)
  {
    fprintf(stderr, "in_window_primitive: p is NULL\n");
    return 0;
  }
  
  switch (p->klass)
  {
  case TENM_POINT:
    return in_window_point(((const tenm_point *) p)->x,
                           ((const tenm_point *) p)->y);
    break;
  case TENM_CIRCLE:
    temp_x = ((const tenm_circle *) p)->center->x;
    temp_y = ((const tenm_circle *) p)->center->y;
    temp_r = ((const tenm_circle *) p)->r;
    if ((temp_x + temp_r >= 0.0) && (temp_x - temp_r < window_width)
        && (temp_y >= 0.0) && (temp_y < window_height))
      return 1;
    if ((temp_y + temp_r >= 0.0) && (temp_y - temp_r < window_height)
        && (temp_x >= 0.0) && (temp_x < window_width))
      return 1;
    return tenm_collided_primitive(p, window);
    break;
  case TENM_SEGMENT:
    temp_x = ((const tenm_segment *) p)->a->x;
    temp_y = ((const tenm_segment *) p)->a->y;
    if (in_window_point(temp_x, temp_y))
      return 1;
    temp_x = ((const tenm_segment *) p)->b->x;
    temp_y = ((const tenm_segment *) p)->b->y;
    if (in_window_point(temp_x, temp_y))
      return 1;
    return tenm_collided_primitive(p, window);
    break;
  case TENM_POLYGON:
    for (i = 0; i < ((const tenm_polygon *) p)->n; i++)
    {
      temp_x = ((const tenm_polygon *) p)->v[i]->x;
      temp_y = ((const tenm_polygon *) p)->v[i]->y;
      if (in_window_point(temp_x, temp_y))
        return 1;
    }
    return tenm_collided_primitive(p, window);
    break;
  default:
    fprintf(stderr, "primitive_in_window: strange primitive found (%d)\n",
            p->klass);
    return 0;
    break;
  }
  /* should not reach here */
  return 0;
}

/* return 1 (true) or 0 (false) */
static int
in_window_point(double x, double y)
{
  /* sanity check */
  if (window == NULL)
  {
    fprintf(stderr, "point_in_window: window is NULL\n");
    return 0;
  }

  if ((x < 0) || (x >= window_width))
    return 0;
  if ((y < 0) || (y >= window_height))
    return 0;
  return 1;
}

/* rotate the vector v (arg 2) by theta (arg 3) degree
 * result (arg 1) and v (arg 2) must be double[2] (you must allocate enough
 * memory before calling this function)
 * the result is undefined if result (arg 1) and v (arg 2) overlap
 */
void
vector_rotate(double *result, const double *v, int theta)
{
  /* sanity check */
  if (result == NULL)
  {
    fprintf(stderr, "vector_rotate: result is NULL\n");
    return;
  }
  if (v == NULL)
  {
    fprintf(stderr, "vector_rotate: v is NULL\n");
    return;
  }

  result[0] = tenm_cos(theta) * v[0] - tenm_sin(theta) * v[1];
  result[1] = tenm_sin(theta) * v[0] + tenm_cos(theta) * v[1];
}

/* rotate the vector v (arg 2) to the vector a (arg 4)
 * by at most theta (arg 3) degree
 * result (arg 1), v (arg 2) and a(arg 3) must be double[2]
 * (you must allocate enough memory before calling this function)
 * the result is undefined if any pair of result (arg 1), v (arg 2)
 * and a (arg 3) overlap
 */
void
vector_rotate_bounded(double *result, const double *v,
                      const double *a, int theta)
{
  double length_v;
  double length_a;
  double dot;
  double c;
  double r1[2];
  double r2[2];
  double dot_r1;
  double dot_r2;

  /* sanity check */
  if (result == NULL)
  {
    fprintf(stderr, "vector_rotate_bounded: result is NULL\n");
    return;
  }
  if (v == NULL)
  {
    fprintf(stderr, "vector_rotate_bounded: v is NULL\n");
    return;
  }
  if (a == NULL)
  {
    fprintf(stderr, "vector_rotate_bounded: a is NULL\n");
    return;
  }

  if (theta <= 0)
  {
    result[0] = v[0];
    result[1] = v[1];
    return;
  }

  length_v = tenm_sqrt((int) (v[0] * v[0] + v[1] * v[1]));
  length_a = tenm_sqrt((int) (a[0] * a[0] + a[1] * a[1]));
  if (length_v < NEAR_ZERO)
    length_v = 1.0;
  if (length_a < NEAR_ZERO)
    length_a = 1.0;

  if (theta >= 180)
  {
    result[0] = a[0] * length_v / length_a;
    result[1] = a[1] * length_v / length_a;
    return;
  }

  dot = v[0] * a[0] + v[1] * a[1];
  c = dot / (length_v * length_a);

  if (c > tenm_cos(theta))
  {
    result[0] = a[0] * length_v / length_a;
    result[1] = a[1] * length_v / length_a;
    return;
  }

  r1[0] = 0.0;
  r1[1] = 0.0;
  vector_rotate(r1, v, theta);
  r2[0] = 0.0;
  r2[1] = 0.0;
  vector_rotate(r2, v, -theta);
  dot_r1 = r1[0] * a[0] + r1[1] * a[1];
  dot_r2 = r2[0] * a[0] + r2[1] * a[1];

  if (dot_r1 >= dot_r2)
  {
    result[0] = r1[0];
    result[1] = r1[1];
    return;
  }

  result[0] = r2[0];
  result[1] = r2[1];
}

int
delete_enemy_shot(tenm_object *my, int n)
{
  if (my == NULL)
    return 0;
  if (!(my->attr & ATTR_ENEMY_SHOT))
    return 0;

  tenm_table_add(explosion_new(my->x, my->y,
                               0.0, 0.0,
                               1, 20, my->count[0], 2.0, 8));

  return 1;
}

int
delete_enemy(tenm_object *my, int n)
{
  if (my == NULL)
    return 0;
  if (my->attr & ATTR_BOSS)
    return 0;
  if (!(my->attr & (ATTR_ENEMY | ATTR_OBSTACLE | ATTR_OPAQUE)))
    return 0;

  return 1;
}
