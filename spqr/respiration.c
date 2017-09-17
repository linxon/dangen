/* $Id: respiration.c,v 1.118 2004/09/06 20:25:34 oohara Exp $ */

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
#include "score.h"

#include "respiration.h"

#define NEAR_ZERO 0.0001

static int respiration_move(tenm_object *my, double turn_per_frame);
static int respiration_hit(tenm_object *my, tenm_object *your);
static int respiration_act(tenm_object *my, const tenm_object *player);
static int respiration_draw(tenm_object *my, int priority);
static int respiration_green(const tenm_object *my);
static int respiration_signal(tenm_object *my, int n);

tenm_object *
respiration_new(int table_index)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = (double) (WINDOW_WIDTH / 2);
  double y = -120.0;

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "respiration_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_circle_new(x, y, 45.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "respiration_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 4);
  if (count == NULL)
  {
    fprintf(stderr, "respiration_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "respiration_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] for deal_damage
   * [1] "damaged" timer
   * [2] life timer
   * [3] more index
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */

  count[0] = 0;
  count[1] = 0;
  count[2] = -40;
  count[3] = table_index;

  count_d[0] = 0.0;
  count_d[1] = 6.0;

  new = tenm_object_new("Respiration", ATTR_ENEMY, ATTR_PLAYER_SHOT,
                        350, x, y,
                        4, count, 2, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&respiration_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&respiration_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&respiration_act),
                        (int (*)(tenm_object *, int))
                        (&respiration_draw));

  if (new == NULL)
  {
    fprintf(stderr, "respiration_new: tenm_object_new failed\n");
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
respiration_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "respiration_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "respiration_move: strange turn_per_frame (%f)\n",
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
respiration_hit(tenm_object *my, tenm_object *your)
{
  int n;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "respiration_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "respiration_hit: your is NULL\n");
    return 0;
  }

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;

  deal_damage(my, your, 0);
  if (respiration_green(my))
    add_chain(my, your);
  my->count[1] = 41;

  if (my->hit_point <= 0)
  {
    add_score(150);
    tenm_table_apply(my->count[3],
                     (int (*)(tenm_object *, int)) (&respiration_signal),
                     0);
    if (respiration_green(my))
      n = 8;
    else
      n = 7;

    tenm_table_add(explosion_new(my->x, my->y,
                                 my->count_d[0] * 0.5,
                                 my->count_d[1] * 0.5,
                                 1, 1000, n, 8.0, 6));
    tenm_table_add(explosion_new(my->x, my->y,
                                 my->count_d[0] * 0.5,
                                 my->count_d[1] * 0.5,
                                 2, 300, n, 5.0, 8));
    return 1;
  }

  return 0;
}

static int
respiration_act(tenm_object *my, const tenm_object *player)
{
  int i;
  double x;
  double dx;
  double dy;
  double length;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "respiration_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  /* for deal_damage */
  my->count[0] = 0;

  /* "damaged" count down */
  if (my->count[1] > 0)
    (my->count[1])--;

  (my->count[2])++;

  /* encounter */
  if (my->count[2] < 0)
    return 0;

  /* escape */
  if (my->count[2] >= 1200)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = -6.0;

    if (my->count[2] >= 1240)
      return 1;

    return 0;
  }

  /* speed change */
  if (my->count[2] % 200 < 40)
  {
    my->count_d[0] = -6.0;
  }
  else if (my->count[2] % 200 < 60)
  {
    my->count_d[0] = 0.0;
  }
  else if (my->count[2] % 200 < 140)
  {
    my->count_d[0] = 6.0;
  }
  else if (my->count[2] % 200 < 160)
  {
    my->count_d[0] = 0.0;
  }
  else
  {
    my->count_d[0] = -6.0;
  }
  my->count_d[1] = 0.0;

  /* shoot */
  if (my->count[2] % 13 == 0)
  {
    dx = player->x - my->x;
    dy = player->y - my->y;
    length = tenm_sqrt((int) (dx * dx + dy * dy));
    if (length < NEAR_ZERO)
      length = 1.0;

    tenm_table_add(laser_point_new(my->x, my->y, 6.0,
                                   player->x, player->y,
                                   25.0, 4));
    tenm_table_add(laser_point_new(my->x, my->y, 7.5,
                                   player->x + dx * 0.25,
                                   player->y - dy * 0.25,
                                   25.0, 4));
    tenm_table_add(laser_point_new(my->x, my->y, 9.0,
                                   player->x + dx * 0.5,
                                   player->y - dy * 0.5,
                                   25.0, 4));
  }

  if (my->count[2] % 46 == 0)
  {
    for (i = -19; i <= 37; i += 4)
    {
      if (player->x > my->x)
      {
        x = my->x - 40.0;
        dx = 4.0 + ((double) i) * 0.1;
      }
      else
      {
        x = my->x + 40.0;
        dx = -(4.0 + ((double) i) * 0.1);
      }
      if (player->y > my->y)
        dy = 4.0 - ((double) (i * i)) / 200.0;
      else
        dy = 1.0 - ((double) (i * i)) / 200.0;
      tenm_table_add(normal_shot_new(x, my->y,
                                     dx, dy,
                                     2, -2, 0));
    }
  }
  if (my->count[2] % 46 == 23)
  {
    for (i = -19; i <= 37; i += 4)
    {
      if (player->x > my->x)
      {
        x = my->x - 40.0;
        dx = 3.0 + ((double) i) * 0.1;
      }
      else
      {
        x = my->x + 40.0;
        dx = -(3.0 + ((double) i) * 0.1);
      }
      if (player->y > my->y)
        dy = 1.0 + ((double) (i * i)) / 200.0;
      else
        dy = -1.0 + ((double) (i * i)) / 200.0;
      tenm_table_add(normal_shot_new(x, my->y,
                                     dx, dy,
                                     2, -2, 0));
    }
  }

  return 0;
}

static int
respiration_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  char temp[32];

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "respiration_draw: my is NULL\n");
    return 0;
  }

  if (priority != 0)
    return 0;

  /* decoration */
  if (respiration_green(my))
  {
    if (my->count[1] >= 40)
      color = tenm_map_color(181, 190, 92);
    else
      color = tenm_map_color(157, 182, 123);
  }
  else
  {
    if (my->count[1] >= 40)
      color = tenm_map_color(200, 164, 92);
    else
      color = tenm_map_color(182, 147, 123);
  }

  if (tenm_draw_line(((int) (my->x + 5.0002)),
                     ((int) (my->y - 62.4167)),
                     ((int) (my->x + 48.3334)),
                     ((int) (my->y - 50.8056)),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x + 48.3334)),
                     ((int) (my->y - 50.8056)),
                     ((int) (my->x + 95.9487)),
                     ((int) (my->y + 31.6667)),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x + 95.9487)),
                     ((int) (my->y + 31.6667)),
                     ((int) (my->x + 84.3376)),
                     ((int) (my->y + 75.0)),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x + 84.3376)),
                     ((int) (my->y + 75.0)),
                     ((int) (my->x + 5.0002)),
                     ((int) (my->y - 62.4167)),
                     1, color) != 0)
    status = 1;

  if (tenm_draw_line(((int) (my->x - 5.0002)),
                     ((int) (my->y - 62.4167)),
                     ((int) (my->x - 48.3334)),
                     ((int) (my->y - 50.8056)),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x - 48.3334)),
                     ((int) (my->y - 50.8056)),
                     ((int) (my->x - 95.9487)),
                     ((int) (my->y + 31.6667)),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x - 95.9487)),
                     ((int) (my->y + 31.6667)),
                     ((int) (my->x - 84.3376)),
                     ((int) (my->y + 75.0)),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x - 84.3376)),
                     ((int) (my->y + 75.0)),
                     ((int) (my->x - 5.0002)),
                     ((int) (my->y - 62.4167)),
                     1, color) != 0)
    status = 1;

  /* body */
  if (respiration_green(my))
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

  if (tenm_draw_circle((int) (my->x), (int) (my->y), 45, 3, color) != 0)
    status = 1;

  /* hit point stat */
  if ((priority == 0) && (my->count[1] > 0))
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 10, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "respiration_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

/* return 1 (true) or 0 (false) */
static int
respiration_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if (my->count[2] < 1160)
    return 1;

  return 0;
}

static int
respiration_signal(tenm_object *my, int n)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (strcmp(my->name, "plan 6 more 1") != 0)
    return 0;

  my->count[1] = my->count[0];
  if (my->count[1] < 0)
    my->count[1] = 0;

  return 0;
}
