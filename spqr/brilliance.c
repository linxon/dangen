/* $Id: brilliance.c,v 1.31 2005/04/18 23:25:17 oohara Exp $ */

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
#include "player-shot.h"
#include "tenm_table.h"
#include "background.h"
#include "chain.h"
#include "laser.h"
#include "normal-shot.h"
#include "tenm_math.h"
#include "fragment.h"
#include "explosion.h"
#include "normal-enemy.h"
#include "score.h"

#include "brilliance.h"

static int brilliance_move(tenm_object *my, double turn_per_frame);
static int brilliance_hit(tenm_object *my, tenm_object *your);
static int brilliance_signal(tenm_object *my, int n);
static int brilliance_act(tenm_object *my, const tenm_object *player);
static int brilliance_draw(tenm_object *my, int priority);
static int brilliance_green(const tenm_object *my);

tenm_object *
brilliance_new(double x, double y, int time_stay, int table_index)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;

  /* sanity check */
  if (time_stay <= 0)
  {
    fprintf(stderr, "brilliance_new: time_stay is non-positive (%d)\n",
            time_stay);
    return NULL;
  }
  if (table_index < 0)
  {
    fprintf(stderr, "brilliance_new: strange table_index (%d)\n",
            table_index);
    return NULL;
  }

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "brilliance_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_polygon_new(5,
                                             x + 38.0, y - 30.0,
                                             x + 62.0, y - 20.0,
                                             x, y + 42.0,
                                             x - 62.0, y - 20.0,
                                             x - 38.0, y - 30.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "brilliance_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 6);
  if (count == NULL)
  {
    fprintf(stderr, "brilliance_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 3);
  if (count_d == NULL)
  {
    fprintf(stderr, "brilliance_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] for deal_damage
   * [1] "damaged" timer
   * [2] life mode
   * [3] life timer
   * [4] time_stay
   * [5] table_index
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */

  count[0] = 0;
  count[1] = 0;
  count[2] = 0;
  count[3] = 0;
  count[4] = time_stay;
  count[5] = table_index;

  count_d[0] = -1.0 + 1.0;
  count_d[1] = 5.0 + 0.2;

  new = tenm_object_new("Brilliance", ATTR_ENEMY, ATTR_PLAYER_SHOT,
                        270, x, y,
                        6, count, 3, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&brilliance_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&brilliance_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&brilliance_act),
                        (int (*)(tenm_object *, int))
                        (&brilliance_draw));

  if (new == NULL)
  {
    fprintf(stderr, "brilliance_new: tenm_object_new failed\n");
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
brilliance_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "brilliance_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "brilliance_move: strange turn_per_frame (%f)\n",
            turn_per_frame);
    return 0;
  }

  dx_temp = my->count_d[0] / turn_per_frame;
  dy_temp = my->count_d[1] / turn_per_frame;
  my->x += dx_temp;
  my->y += dy_temp;
  if (my->mass != NULL)
    tenm_move_mass(my->mass, dx_temp, dy_temp);

  if ((my->count[2] == 2) && (!in_window_object(my)))
    return 1;

  return 0;
}

static int
brilliance_hit(tenm_object *my, tenm_object *your)
{
  int n;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "brilliance_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "brilliance_hit: your is NULL\n");
    return 0;
  }

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;

  deal_damage(my, your, 0);
  if (brilliance_green(my))
    add_chain(my, your);
  my->count[1] = 41;

  if (my->hit_point <= 0)
  {
    add_score(75);

    if (brilliance_green(my))
      n = 8;
    else
      n = 7;
    tenm_table_add(explosion_new(my->x, my->y,
                                 my->count_d[0] * 0.5,
                                 my->count_d[1] * 0.5,
                                 1, 1000, n, 8.0, 6));
    tenm_table_add(fragment_new(my->x, my->y,
                                my->count_d[0] * 0.5, my->count_d[1] * 0.5,
                                30.0, 30, n, 5.0, 0.0, 20));

    if (my->count[2] <= 1)
      tenm_table_apply(my->count[5],
                       (int (*)(tenm_object *, int))
                       (&brilliance_signal),
                       0);
    return 1;
  }

  return 0;
}

static int
brilliance_signal(tenm_object *my, int n)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (strcmp(my->name, "plan 15 more 1") != 0)
    return 0;

  (my->count[2])++;

  return 0;
}

static int
brilliance_act(tenm_object *my, const tenm_object *player)
{
  int i;
  double result[2];
  double v[2];

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "brilliance_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  /* for deal_damage */
  my->count[0] = 0;

  /* "damaged" count down */
  if (my->count[1] > 0)
    (my->count[1])--;

  (my->count[3])++;

  /* shoot */
  if ((my->count[2] == 1) && (my->count[3] % 37 == 0))
  {
    if (my->count[3] % 74 == 0)
    {  
      v[0] = 0.0;
      v[1] = 26.0;
    }
    else
    {  
      v[0] = -24.0;
      v[1] = 10.0;
    }

    for (i = 1; i <= 3; i++)
    {
      result[0] = v[0];
      result[1] = v[1];
      vector_rotate(result, v, -12 - 7 * i);
      tenm_table_add(laser_point_new(my->x + v[1], my->y - v[0],
                                     2.5 + 1.0 * ((double) i),
                                     my->x + v[1] + result[0],
                                     my->y - v[0] + result[1],
                                     25.0, 1));
      if (i != 2)
      {
        result[0] = v[0];
        result[1] = v[1];
        vector_rotate(result, v, -12 + 7 * i);
        tenm_table_add(laser_point_new(my->x + v[1], my->y - v[0],
                                       2.5 + 1.0 * ((double) i),
                                       my->x + v[1] + result[0],
                                       my->y - v[0] + result[1],
                                       25.0, 1));
        result[0] = v[0];
        result[1] = v[1];
        vector_rotate(result, v, 12 - 7 * i);
        tenm_table_add(laser_point_new(my->x - v[1], my->y + v[0],
                                       2.5 + 1.0 * ((double) i),
                                       my->x - v[1] + result[0],
                                       my->y + v[0] + result[1],
                                       25.0, 1));
      }
      result[0] = v[0];
      result[1] = v[1];
      vector_rotate(result, v, 12 + 7 * i);
      tenm_table_add(laser_point_new(my->x - v[1], my->y + v[0],
                                     2.5 + 1.0 * ((double) i),
                                     my->x - v[1] + result[0],
                                     my->y + v[0] + result[1],
                                     25.0, 1));
    }
  }

  /* speed change */
  if ((my->count[2] == 0) && (my->count[3] >= 48))
  {
    my->count[2] = 1;
    my->count[3] = 0;

    my->count_d[0] = 0.0 + 1.0;
    my->count_d[1] = 0.0 + 0.2;
  }
  else if ((my->count[2] == 1) && (my->count[3] >= my->count[4]))
  {
    my->count[2] = 2;
    my->count[3] = 0;

    my->count_d[0] = 1.0 + 1.0;
    my->count_d[1] = -5.0 + 0.2;
  }

  return 0;
}

static int
brilliance_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  char temp[32];

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "brilliance_draw: my is NULL\n");
    return 0;
  }

  if (priority != 0)
    return 0;

  /* body */
  if (brilliance_green(my))
  {
    if (my->count[1] >= 40)
      color = tenm_map_color(109, 125, 9);
    else
      color = tenm_map_color(61, 95, 13);
  }
  else
  {
    if (my->count[1] >= 40)
      color = tenm_map_color(135, 89, 9);
    else
      color = tenm_map_color(95, 47, 13);
  }

  if (tenm_draw_line((int) (my->x - 62.0), (int) (my->y - 20.0),
                     (int) (my->x + 62.0), (int) (my->y - 20.0),
                     1, color) != 0)
    status = 1;

  if (tenm_draw_line((int) (my->x + 38.0), (int) (my->y - 30.0),
                     (int) (my->x + 62.0), (int) (my->y - 20.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x + 62.0), (int) (my->y - 20.0),
                     (int) (my->x), (int) (my->y + 42.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x), (int) (my->y + 42.0),
                     (int) (my->x - 62.0), (int) (my->y - 20.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 62.0), (int) (my->y - 20.0),
                     (int) (my->x - 38.0), (int) (my->y - 30.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 38.0), (int) (my->y - 30.0),
                     (int) (my->x + 38.0), (int) (my->y - 30.0),
                     2, color) != 0)
    status = 1;

  /* hit point stat */
  if ((priority == 0) && (my->count[1] >= 1))
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 10, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "brilliance_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

/* return 1 (true) or 0 (false) */
static int
brilliance_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if (my->count[2] <= 1)
    return 1;

  return 0;
}
