/* $Id: spellbook.c,v 1.30 2005/06/08 12:05:51 oohara Exp $ */

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

#include "spellbook.h"

static int spellbook_move(tenm_object *my, double turn_per_frame);
static int spellbook_hit(tenm_object *my, tenm_object *your);
static int spellbook_signal(tenm_object *my, int n);
static int spellbook_act(tenm_object *my, const tenm_object *player);
static int spellbook_draw(tenm_object *my, int priority);
static int spellbook_green(const tenm_object *my);

tenm_object *
spellbook_new(int table_index)
{
  int i;
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = 72.0;
  double y = -72.0;

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 2);
  if (p == NULL)
  {
    fprintf(stderr, "spellbook_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + 24.0, y - 72.0,
                                             x + 24.0, y + 72.0,
                                             x - 24.0, y + 72.0,
                                             x - 24.0, y - 72.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "spellbook_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }
  p[1] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + 72.0, y - 24.0,
                                             x + 72.0, y + 24.0,
                                             x - 72.0, y + 24.0,
                                             x - 72.0, y - 24.0);
  if (p[1] == NULL)
  {
    fprintf(stderr, "spellbook_new: cannot set p[1]\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 10);
  if (count == NULL)
  {
    fprintf(stderr, "spellbook_new: malloc(count) failed\n");
    for (i = 0; i < 2; i++)
      (p[i])->delete(p[i]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "spellbook_new: malloc(count_d) failed\n");
    free(count);
    for (i = 0; i < 2; i++)
      (p[i])->delete(p[i]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] for deal_damage
   * [1] "damaged" timer
   * [2] life mode
   * [3] life timer
   * [4] parent index
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */

  count[0] = 0;
  count[1] = 0;
  count[2] = 0;
  count[3] = 0;
  count[4] = table_index;

  count_d[0] = 0.0;
  count_d[1] = 4.0;

  new = tenm_object_new("Spellbook", ATTR_ENEMY, ATTR_PLAYER_SHOT,
                        600, x, y,
                        10, count, 2, count_d, 2, p,
                        (int (*)(tenm_object *, double))
                        (&spellbook_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&spellbook_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&spellbook_act),
                        (int (*)(tenm_object *, int))
                        (&spellbook_draw));

  if (new == NULL)
  {
    fprintf(stderr, "spellbook_new: tenm_object_new failed\n");
    if (count_d != NULL)
      free(count_d);
    if (count != NULL)
      free(count);
    for (i = 0; i < 2; i++)
      (p[i])->delete(p[i]);
    free(p);
    return NULL;
  }

  return new;
}

static int
spellbook_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "spellbook_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "spellbook_move: strange turn_per_frame (%f)\n",
            turn_per_frame);
    return 0;
  }

  dx_temp = my->count_d[0] / turn_per_frame;
  dy_temp = my->count_d[1] / turn_per_frame;
  my->x += dx_temp;
  my->y += dy_temp;
  if (my->mass != NULL)
    tenm_move_mass(my->mass, dx_temp, dy_temp);

  if ((my->count[3] >= 868) && (my->y - 72.0 > ((double) WINDOW_WIDTH)))
  {
    tenm_table_apply(my->count[4],
                     (int (*)(tenm_object *, int))
                     (&spellbook_signal),
                     spellbook_green(my));
    return 1;
  }

  return 0;
}

static int
spellbook_hit(tenm_object *my, tenm_object *your)
{
  int n;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "spellbook_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "spellbook_hit: your is NULL\n");
    return 0;
  }
  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;

  deal_damage(my, your, 0);
  if (spellbook_green(my))
    add_chain(my, your);
  my->count[1] = 41;

  if (my->hit_point <= 0)
  {
    add_score(75);

    if (spellbook_green(my))
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

    tenm_table_apply(my->count[4],
                     (int (*)(tenm_object *, int))
                     (&spellbook_signal),
                     spellbook_green(my));
    return 1;
  }

  return 0;
}

static int
spellbook_signal(tenm_object *my, int n)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (strcmp(my->name, "plan 16 more 1") != 0)
    return 0;

  (my->count[2])++;
  if (n)
    (my->count[3])++;

  return 0;
}

static int
spellbook_act(tenm_object *my, const tenm_object *player)
{
  int theta;
  double v[2];
  double from[2];
  double speed[2];

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "spellbook_act: my is NULL\n");
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

  /* speed change */
  if (my->count[3] == 120)
  {
    my->count_d[0] = 4.0;
    my->count_d[1] = 0.0;
  }
  if ((my->count[3] > 120) && (my->count[3] < 952))
  {
    if ((my->count[3] - 120) % 416 == 124)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = -4.0;
    }
    else if ((my->count[3] - 120) % 416 == 208)
    {
      my->count_d[0] = -4.0;
      my->count_d[1] = 0.0;
    }
    else if ((my->count[3] - 120) % 416 == 332)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 4.0;
    }
    else if ((my->count[3] - 120) % 416 == 0)
    {
      my->count_d[0] = 4.0;
      my->count_d[1] = 0.0;
    }
  }

  /* shoot */
  if (my->count[3] % 23 == 0)
  {
    tenm_table_add(laser_point_new(my->x, my->y, 4.5,
                                   player->x, player->y, 25.0, 0));
  }
  
  if (my->count[3] % 62 == 0)
  {
    theta = ((my->count[3] % 248) / 62) * 90;

    v[0] = 0.0;
    v[1] = -60.0;
    from[0] = v[0];
    from[1] = v[1];
    vector_rotate(from, v, theta);
    v[0] = 3.0;
    v[1] = 1.0;
    speed[0] = v[0];
    speed[1] = v[1];
    vector_rotate(speed, v, theta);
    tenm_table_add(normal_shot_new(my->x + from[0], my->y + from[1],
                                   speed[0], speed[1],
                                   4, -2, 0));

    v[0] = 0.0;
    v[1] = 60.0;
    from[0] = v[0];
    from[1] = v[1];
    vector_rotate(from, v, theta);
    v[0] = 3.0;
    v[1] = -1.0;
    speed[0] = v[0];
    speed[1] = v[1];
    vector_rotate(speed, v, theta);
    tenm_table_add(normal_shot_new(my->x + from[0], my->y + from[1],
                                   speed[0], speed[1],
                                   4, -2, 0));

    v[0] = 0.0;
    v[1] = -60.0;
    from[0] = v[0];
    from[1] = v[1];
    vector_rotate(from, v, theta);
    v[0] = 5.0;
    v[1] = 1.0;
    speed[0] = v[0];
    speed[1] = v[1];
    vector_rotate(speed, v, theta);
    tenm_table_add(normal_shot_new(my->x + from[0], my->y + from[1],
                                   speed[0], speed[1],
                                   4, -2, 0));

    v[0] = 0.0;
    v[1] = 60.0;
    from[0] = v[0];
    from[1] = v[1];
    vector_rotate(from, v, theta);
    v[0] = 5.0;
    v[1] = -1.0;
    speed[0] = v[0];
    speed[1] = v[1];
    vector_rotate(speed, v, theta);
    tenm_table_add(normal_shot_new(my->x + from[0], my->y + from[1],
                                   speed[0], speed[1],
                                   4, -2, 0));
  }

  if (my->count[3] % 62 == 31)
  {
    theta = (((my->count[3] - 31) % 248) / 62) * 90;

    v[0] = 60.0;
    v[1] = 0.0;
    from[0] = v[0];
    from[1] = v[1];
    vector_rotate(from, v, theta);
    v[0] = 3.0;
    v[1] = 1.0;
    speed[0] = v[0];
    speed[1] = v[1];
    vector_rotate(speed, v, theta + 45);
    tenm_table_add(normal_shot_new(my->x + from[0], my->y + from[1],
                                   speed[0], speed[1],
                                   4, -2, 0));

    v[0] = 0.0;
    v[1] = 60.0;
    from[0] = v[0];
    from[1] = v[1];
    vector_rotate(from, v, theta);
    v[0] = 3.0;
    v[1] = -1.0;
    speed[0] = v[0];
    speed[1] = v[1];
    vector_rotate(speed, v, theta + 45);
    tenm_table_add(normal_shot_new(my->x + from[0], my->y + from[1],
                                   speed[0], speed[1],
                                   4, -2, 0));

    v[0] = 60.0;
    v[1] = 0.0;
    from[0] = v[0];
    from[1] = v[1];
    vector_rotate(from, v, theta);
    v[0] = 5.0;
    v[1] = 1.0;
    speed[0] = v[0];
    speed[1] = v[1];
    vector_rotate(speed, v, theta + 45);
    tenm_table_add(normal_shot_new(my->x + from[0], my->y + from[1],
                                   speed[0], speed[1],
                                   4, -2, 0));

    v[0] = 0.0;
    v[1] = 60.0;
    from[0] = v[0];
    from[1] = v[1];
    vector_rotate(from, v, theta);
    v[0] = 5.0;
    v[1] = -1.0;
    speed[0] = v[0];
    speed[1] = v[1];
    vector_rotate(speed, v, theta + 45);
    tenm_table_add(normal_shot_new(my->x + from[0], my->y + from[1],
                                   speed[0], speed[1],
                                   4, -2, 0));
  }

  return 0;
}

static int
spellbook_draw(tenm_object *my, int priority)
{
  double size;
  int status = 0;
  tenm_color color;
  char temp[32];

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "spellbook_draw: my is NULL\n");
    return 0;
  }

  if (priority != 0)
    return 0;

  /* body */
  if (spellbook_green(my))
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

  size = 24.0;

  if (tenm_draw_line(((int) (my->x + size)),
                     ((int) (my->y - size * 3.0)),
                     ((int) (my->x + size)),
                     ((int) (my->y - size)),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x + size)),
                     ((int) (my->y - size)),
                     ((int) (my->x + size * 3.0)),
                     ((int) (my->y - size)),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x + size * 3.0)),
                     ((int) (my->y - size)),
                     ((int) (my->x + size * 3.0)),
                     ((int) (my->y + size)),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x + size * 3.0)),
                     ((int) (my->y + size)),
                     ((int) (my->x + size)),
                     ((int) (my->y + size)),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x + size)),
                     ((int) (my->y + size)),
                     ((int) (my->x + size)),
                     ((int) (my->y + size * 3.0)),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x + size)),
                     ((int) (my->y + size * 3.0)),
                     ((int) (my->x - size)),
                     ((int) (my->y + size * 3.0)),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x - size)),
                     ((int) (my->y + size * 3.0)),
                     ((int) (my->x - size)),
                     ((int) (my->y + size)),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x - size)),
                     ((int) (my->y + size)),
                     ((int) (my->x - size * 3.0)),
                     ((int) (my->y + size)),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x - size * 3.0)),
                     ((int) (my->y + size)),
                     ((int) (my->x - size * 3.0)),
                     ((int) (my->y - size)),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x - size * 3.0)),
                     ((int) (my->y - size)),
                     ((int) (my->x - size)),
                     ((int) (my->y - size)),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x - size)),
                     ((int) (my->y - size)),
                     ((int) (my->x - size)),
                     ((int) (my->y - size * 3.0)),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x - size)),
                     ((int) (my->y - size * 3.0)),
                     ((int) (my->x + size)),
                     ((int) (my->y - size * 3.0)),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x + size)),
                     ((int) (my->y - size * 3.0)),
                     ((int) (my->x + size)),
                     ((int) (my->y - size)),
                     3, color) != 0)
    status = 1;

  /* hit point stat */
  if ((priority == 0) && (my->count[1] > 0))
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 10, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "spellbook_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

/* return 1 (true) or 0 (false) */
static int
spellbook_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if (my->count[3] < 868)
    return 1;

  return 0;
}
