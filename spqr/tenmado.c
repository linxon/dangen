/* $Id: tenmado.c,v 1.61 2011/08/23 20:50:20 oohara Exp $ */

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

#include "tenmado.h"

static int tenmado_move(tenm_object *my, double turn_per_frame);
static int tenmado_hit(tenm_object *my, tenm_object *your);
static int tenmado_signal(tenm_object *my, int n);
static int tenmado_act(tenm_object *my, const tenm_object *player);
static int tenmado_draw(tenm_object *my, int priority);
static int tenmado_green(const tenm_object *my);

static tenm_object *tenmado_shot_new(double x, double y, int color);
static int tenmado_shot_move(tenm_object *my, double turn_per_frame);
static int tenmado_shot_hit(tenm_object *my, tenm_object *your);
static int tenmado_shot_act(tenm_object *my, const tenm_object *player);
static int tenmado_shot_draw(tenm_object *my, int priority);

tenm_object *
tenmado_new(double x, double y, int n, double dx, int t, int table_index,
            int t_shoot)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  int i;

  /* sanity check */
  if ((n < 0) || (n > 1))
  {
    fprintf(stderr, "tenmado_new: strange n (%d)\n", n);
    return NULL;
  }
  if (t <= 0)
  {
    fprintf(stderr, "tenmado_new: t is non-positive (%d)\n", t);
    return NULL;
  }
  if (t_shoot <= 0)
  {
    fprintf(stderr, "tenmado_new: t_shoot is non-positive (%d)\n", t_shoot);
    return NULL;
  }

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 2);
  if (p == NULL)
  {
    fprintf(stderr, "tenmado_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_polygon_new(3,
                                             x + 15.0, y - 30.0,
                                             x, y + 30.0,
                                             x - 15.0, y - 30.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "tenmado_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }
  p[1] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + 30.0, y - 30.0,
                                             x + 7.5, y,
                                             x - 7.5, y,
                                             x - 30.0, y - 30.0);
  if (p[1] == NULL)
  {
    fprintf(stderr, "tenmado_new: cannot set p[0]\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 9);
  if (count == NULL)
  {
    fprintf(stderr, "tenmado_new: malloc(count) failed\n");
    for (i = 0; i < 2; i++)
      (p[i])->delete(p[i]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 3);
  if (count_d == NULL)
  {
    fprintf(stderr, "tenmado_new: malloc(count_d) failed\n");
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
   * [4] shoot timer
   * [5] n
   * [6] t
   * [7] table index
   * [8] time shoot
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   * [2] y origin
   */

  count[0] = 0;
  count[1] = 0;
  count[2] = 0;
  count[3] = 0;
  count[4] = 0;
  count[5] = n;
  count[6] = t;
  count[7] = table_index;
  count[8] = t_shoot;

  count_d[0] = dx;
  count_d[1] = 6.0;
  count_d[2] = y;

  new = tenm_object_new("tenmado", ATTR_ENEMY, ATTR_PLAYER_SHOT,
                        55, x, y,
                        9, count, 3, count_d, 2, p,
                        (int (*)(tenm_object *, double))
                        (&tenmado_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&tenmado_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&tenmado_act),
                        (int (*)(tenm_object *, int))
                        (&tenmado_draw));

  if (new == NULL)
  {
    fprintf(stderr, "tenmado_new: tenm_object_new failed\n");
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
tenmado_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "tenmado_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "tenmado_move: strange turn_per_frame (%f)\n",
            turn_per_frame);
    return 0;
  }

  dx_temp = my->count_d[0] / turn_per_frame;
  dy_temp = my->count_d[1] / turn_per_frame;
  my->x += dx_temp;
  my->y += dy_temp;
  if (my->mass != NULL)
    tenm_move_mass(my->mass, dx_temp, dy_temp);

  if (!in_window_object(my))
    return 1;

  return 0;
}

static int
tenmado_hit(tenm_object *my, tenm_object *your)
{
  int n;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "tenmado_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "tenmado_hit: your is NULL\n");
    return 0;
  }

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;

  deal_damage(my, your, 0);
  if (tenmado_green(my))
    add_chain(my, your);
  my->count[1] = 41;

  if (my->hit_point <= 0)
  {
    if (tenmado_green(my))
      n = 8;
    else
      n = 7;
    tenm_table_add(explosion_new(my->x, my->y,
                                 my->count_d[0] / 2.0, my->count_d[1]/ 2.0,
                                 1, 20, n, 3.0, 8));
    tenm_table_add(fragment_new(my->x, my->y,
                                my->count_d[0] / 2.0, my->count_d[1] / 2.0,
                                20.0, 10, n, 3.0, 0.0, 8));
    add_score(11);
    if (my->count[2] <= 1)
      tenm_table_apply(my->count[7],
                       (int (*)(tenm_object *, int)) (&tenmado_signal),
                       0);
    return 1;
  }

  return 0;
}

static int
tenmado_signal(tenm_object *my, int n)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (strcmp(my->name, "plan 9 more 1") != 0)
    return 0;

  (my->count[2])++;

  return 0;
}

static int
tenmado_act(tenm_object *my, const tenm_object *player)
{
  double c;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "tenmado_act: my is NULL\n");
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
  if (my->count[2] == 0)
  {
    if (my->count[3] >= 18)
    {
      my->count[2] = 1;
      my->count[3] = 0;
      my->count_d[2] = my->y;
    }
  }
  else if (my->count[2] == 1)
  {
    if (my->count[5] == 0)
      c = 90.0 + ((double) ((my->count[3] % 11) * 3));
    else
      c = 60.0;
    if (player->x - my->x > c)
      my->count_d[0] += 0.5;
    if (player->x - my->x > my->x)
      my->count_d[0] = 6.0;
    if (my->count[5] == 1)
      c = 90.0 + ((double) ((my->count[3] % 11) * 3));
    else
      c = 60.0;
    if (player->x - my->x < -c)
      my->count_d[0] -= 0.5;
    if (my->x - player->x > ((double) WINDOW_WIDTH) - my->x)
      my->count_d[0] = -6.0;
    if (my->count_d[0] > 6.0)
      my->count_d[0] = 6.0;
    if (my->count_d[0] < -6.0)
      my->count_d[0] = -6.0;

    my->count_d[1] = my->count_d[2] + 50.0 * tenm_sin(my->count[3] * 7)
      - my->y;

    if (my->count[3] >= my->count[6])
    {
      my->count[2] = 2;
      my->count[3] = 0;
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
  }
  else
  {
    if (my->count[3] >= 10)
      my->count_d[1] = 6.0;
  }

  /* shoot */
  if (my->count[2] <= 1)
  {
    if (my->count[4] < my->count[8])
      (my->count[4])++;
    if (my->count[4] >= my->count[8])
    {
      if ((my->x < player->x - 42.0) || (my->x > player->x + 42.0))
      {
        tenm_table_add(tenmado_shot_new(my->x, my->y, 2));
        my->count[4] = 0;
      }
      else if ((my->x > player->x - 10.0) && (my->x < player->x + 10.0))
      {
      tenm_table_add(tenmado_shot_new(my->x, my->y, 4));
      my->count[4] = 0;
      }
    }
  }

  return 0;
}

static int
tenmado_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  char temp[32];

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "tenmado_draw: my is NULL\n");
    return 0;
  }

  if (priority != 0)
    return 0;

  /* decoration */
  if (tenmado_green(my))
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

  if (tenm_draw_line((int) (my->x + 15.0), (int) (my->y - 30.0),
                     (int) (my->x + 7.5), (int) (my->y),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 15.0), (int) (my->y - 30.0),
                     (int) (my->x - 7.5), (int) (my->y),
                     1, color) != 0)
    status = 1;

  if (tenm_draw_circle((int) (my->x), (int) (my->y), 5, 1, color) != 0)
    status = 1;

  /* body */
  if (tenmado_green(my))
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

  if (tenm_draw_line((int) (my->x + 30.0), (int) (my->y - 30.0),
                     (int) (my->x + 7.5), (int) (my->y),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x + 7.5), (int) (my->y),
                     (int) (my->x), (int) (my->y + 30.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x), (int) (my->y + 30.0),
                     (int) (my->x - 7.5), (int) (my->y),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 7.5), (int) (my->y),
                     (int) (my->x - 30.0), (int) (my->y - 30.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 30.0), (int) (my->y - 30.0),
                     (int) (my->x + 30.0), (int) (my->y - 30.0),
                     3, color) != 0)
    status = 1;

  /* hit point stat */
  if (my->count[1] > 0)
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string((int) (my->x - 10.0), (int) (my->y - 18.0),
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "tenmado_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

/* return 1 (true) or 0 (false) */
static int
tenmado_green(const tenm_object *my)
{
 /* sanity check */
  if (my == NULL)
    return 0;

  if (my->count[2] <= 1)
    return 1;

  return 0;
}

static tenm_object *
tenmado_shot_new(double x, double y, int color)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;

  /* sanity check */
  if ((color < 0) || (color > 5))
  {
    fprintf(stderr, "tenmado_shot_new: strange color (%d)\n", color);
    return NULL;
  }
  
  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "tenmado_shot_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_polygon_new(3,
                                             x, y - 46.0,
                                             x - 6.0, y + 7.0,
                                             x + 6.0, y + 7.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "tenmado_shot_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 6);
  if (count == NULL)
  {
    fprintf(stderr, "tenmado_shot_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 3);
  if (count_d == NULL)
  {
    fprintf(stderr, "tenmado_shot_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] color
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */

  count[0] = color;

  count_d[0] = 0.0;
  count_d[1] = 12.0;

  new = tenm_object_new("tenmado shot", ATTR_ENEMY_SHOT, ATTR_OPAQUE,
                        1, x, y,
                        6, count, 3, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&tenmado_shot_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&tenmado_shot_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&tenmado_shot_act),
                        (int (*)(tenm_object *, int))
                        (&tenmado_shot_draw));

  if (new == NULL)
  {
    fprintf(stderr, "tenmado_shot_new: tenm_object_new failed\n");
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
tenmado_shot_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "tenmado_shot_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "tenmado_shot_move: strange turn_per_frame (%f)\n",
            turn_per_frame);
    return 0;
  }

  dx_temp = my->count_d[0] / turn_per_frame;
  dy_temp = my->count_d[1] / turn_per_frame;
  my->x += dx_temp;
  my->y += dy_temp;
  if (my->mass != NULL)
    tenm_move_mass(my->mass, dx_temp, dy_temp);

  if (!in_window_object(my))
    return 1;

  return 0;
}

static int
tenmado_shot_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "tenmado_shot_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "tenmado_shot_hit: your is NULL\n");
    return 0;
  }

  if (your->attr & ATTR_OPAQUE)
    return 1;

  return 0;
}
  
static int
tenmado_shot_act(tenm_object *my, const tenm_object *player)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "tenmado_shot_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  return 0;
}

static int
tenmado_shot_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "tenmado_shot_draw: my is NULL\n");
    return 0;
  }

  /* return if it is not my turn */
  if (priority != 1)
    return 0;

  switch (my->count[0])
  {
  case 0:
    color = tenm_map_color(0, 191, 47);
    break;
  case 1:
    color = tenm_map_color(0, 191, 127);
    break;
  case 2:
    color = tenm_map_color(0, 167, 223);
    break;
  case 3:
    color = tenm_map_color(0, 111, 223);
    break;
  case 4:
    color = tenm_map_color(75, 0, 239);
    break;
  case 5:
    color = tenm_map_color(175, 0, 239);
    break;
  default:
    fprintf(stderr, "tenmado_shot_draw: strange my->count[0] (%d)\n",
            my->count[0]);
    color = tenm_map_color(0, 0, 0);
    break;
  }

  if (tenm_draw_line((int) (my->x + 6.0), (int) (my->y - 7.0),
                     (int) (my->x), (int) (my->y + 46.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x), (int) (my->y + 46.0),
                     (int) (my->x - 6.0), (int) (my->y - 7.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 6.0), (int) (my->y - 7.0),
                     (int) (my->x + 6.0), (int) (my->y - 7.0),
                     3, color) != 0)
    status = 1;

  return status;
}
