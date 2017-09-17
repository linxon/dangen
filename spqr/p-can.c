/* $Id: p-can.c,v 1.37 2011/08/23 20:14:16 oohara Exp $ */
/* [easiest] P-can */

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
#include "stage-clear.h"
#include "score.h"

#include "p-can.h"

static int p_can_move(tenm_object *my, double turn_per_frame);
static int p_can_hit(tenm_object *my, tenm_object *your);
static void p_can_next(tenm_object *my);
static int p_can_act(tenm_object *my, const tenm_object *player);
static int p_can_draw(tenm_object *my, int priority);
static int p_can_green(const tenm_object *my);

tenm_object *
p_can_new(void)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = (double) (WINDOW_WIDTH / 2);
  double y = -49.0;

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "p_can_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + 50.0, y - 50.0,
                                             x + 50.0, y + 50.0,
                                             x - 50.0, y + 50.0,
                                             x - 50.0, y - 50.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "p_can_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 5);
  if (count == NULL)
  {
    fprintf(stderr, "p_can_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 4);
  if (count_d == NULL)
  {
    fprintf(stderr, "p_can_new: malloc(count_d) failed\n");
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
   * [4] "was green when killed" flag
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   * [2] turret aim x
   * [3] turret aim y
   */

  count[0] = 0;
  count[1] = 0;
  count[2] = 0;
  count[3] = 0;
  count[4] = 0;

  count_d[0] = 0.0;
  count_d[1] = (((double) (WINDOW_HEIGHT / 4)) - y) / 90.0;
  count_d[2] = 40.0;
  count_d[3] = 0.0;

  new = tenm_object_new("P-can", ATTR_BOSS, ATTR_PLAYER_SHOT,
                        400, x, y,
                        5, count, 4, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&p_can_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&p_can_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&p_can_act),
                        (int (*)(tenm_object *, int))
                        (&p_can_draw));

  if (new == NULL)
  {
    fprintf(stderr, "p_can_new: tenm_object_new failed\n");
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
p_can_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "p_can_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "p_can_move: strange turn_per_frame (%f)\n",
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
p_can_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "p_can_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "p_can_hit: your is NULL\n");
    return 0;
  }

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;
  if (my->count[2] != 1)
    return 0;

  deal_damage(my, your, 0);
  if (p_can_green(my))
    add_chain(my, your);
  my->count[1] = 2;

  if (my->hit_point <= 0)
  {
    add_score(1500);
    set_background(1);
    p_can_next(my);
    return 0;
  }

  return 0;
}

static void
p_can_next(tenm_object *my)
{
  int n;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "p_can_next: my is NULL\n");
    return;
  }

  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy_shot), 0);
  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy), 0);

  /* set "was green" flag before we change the life mode */
  if (p_can_green(my))
  {
    n = 8;
    my->count[4] = 1;
  }
  else
  {
    n = 7;
    my->count[4] = 0;
  }

  tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                               1, 5000, n, 10.0, 6));

  my->count[2] = 2;
  my->count[3] = 0;
  my->count[1] = 0;
  my->count_d[0] = 0.0;
  my->count_d[1] = 0.5;

  /* don't modify my->attr or my->hit_mask here, or the player shot
   * may fly through the enemy */
  tenm_mass_delete(my->mass);
  my->mass = NULL;
}

static int
p_can_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int j;
  int theta;
  double v[2];
  double a[2];
  double result[2];
  double x;
  double y;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "p_can_act: my is NULL\n");
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

  /* encounter */
  if (my->count[2] == 0)
  {
    if (my->count[3] >= 90)
    {
      my->count[2] = 1;
      my->count[3] = 0;
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
      return 0;
    }
    return 0;
  }
  
  /* dead */
  if (my->count[2] == 2)
  {
    if (p_can_green(my))
      i = 8;
    else
      i = 7;

    if ((my->count[3] >= 30) && (my->count[3] <= 75)
        && (my->count[3] % 15 == 0))
    {
      theta = rand() % 360;
      tenm_table_add(explosion_new(my->x + 30.0 * tenm_cos(theta),
                                   my->y + 30.0 * tenm_sin(theta),
                                   0.0, 0.0,
                                   2, 300, i, 5.0, 8));
    }

    if (my->count[3] > 120)
    {
      tenm_table_add(explosion_new(my->x, my->y,
                                   0.0, 0.0,
                                   1, 3000, i, 10.0, 8));
      tenm_table_add(fragment_new(my->x, my->y, 0.0, 0.0,
                                  30.0, 100, i, 4.0, 0.0, 16));
      tenm_table_add(fragment_new(my->x, my->y, 0.0, 0.0,
                                  50.0, 30, i, 2.5, 0.0, 12));

      tenm_table_add(stage_clear_new(100));
      return 1;
    }

    return 0;
  }

  /* self-destruction */
  if ((my->count[2] == 1) && (my->count[3] >= 2980))
  {
    set_background(2);
    clear_chain();
    p_can_next(my);
    return 0;
  }

  /* aim at the player */
  v[0] = my->count_d[2];
  v[1] = my->count_d[3];
  if ((my->count[3] >= 1300) && (my->count[3] <= 1360))
  {
    a[0] = -40.0;
    a[1] = 0.0;
  }
  else if ((my->count[3] >= 2700) && (my->count[3] <= 2760))
  {
    a[0] = 40.0;
    a[1] = 0.0;
  }
  else
  {
    a[0] = player->x - my->x;
    a[1] = player->y - my->y;
  }
  result[0] = v[0];
  result[1] = v[1];
  if (((my->count[3] >= 100) && (my->count[3] <= 160))
      || ((my->count[3] >= 1300) && (my->count[3] <= 1360))
      || ((my->count[3] >= 1500) && (my->count[3] <= 1560))
      || ((my->count[3] >= 2700) && (my->count[3] <= 2760)))
    vector_rotate_bounded(result, v, a, 3);
  else if (((my->count[3] > 160) && (my->count[3] < 1300))
           || ((my->count[3] > 1560) && (my->count[3] <= 2700)))
    vector_rotate_bounded(result, v, a, 1);
  else if ((my->count[3] >= 1400) && (my->count[3] < 1490))
    vector_rotate(result, v, -2);
  else if ((my->count[3] >= 2800) && (my->count[3] < 2890))
    vector_rotate(result, v, 2);
  my->count_d[2] = result[0];
  my->count_d[3] = result[1];
  if (my->count_d[3] < 0.0)
  {
    if (my->count_d[2] > 0.0)
    {
      my->count_d[2] = 40.0;
      my->count_d[3] = 0.0;
    }
    else
    {
      my->count_d[2] = -40.0;
      my->count_d[3] = 0.0;
    }
  }

  /* shoot */
  if (((my->count[3] <= 51) && (my->count[3] % 17 == 0))
      || ((my->count[3] >= 160) && (my->count[3] < 1233)
          && ((my->count[3] - 160) % 37 == 0))
      || ((my->count[3] >= 1400) && (my->count[3] < 1490)
          && ((my->count[3] - 1400) % 13 == 0))
      || ((my->count[3] >= 1560) && (my->count[3] < 2633)
          && ((my->count[3] - 1560) % 37 == 0))
      || ((my->count[3] >= 2800) && (my->count[3] < 2890)
          && ((my->count[3] - 2800) % 13 == 0)))
  {
    for (i = -1; i <= 1; i += 2)
    {
      for (j = -45; j <= 45; j += 45)
      {
        x = my->x + my->count_d[2] + my->count_d[3] * 3.0/4.0 * ((double) i);
        y = my->y + my->count_d[3] - my->count_d[2] * 3.0/4.0 * ((double) i);
        v[0] = my->count_d[2];
        v[1] = my->count_d[3];
        result[0] = v[0];
        result[1] = v[1];
        vector_rotate(result, v, j);
        tenm_table_add(laser_point_new(x, y, 3.0,
                                       x + result[0], y + result[1],
                                       25.0, 2));
      }
    }
  }
  if (((my->count[3] >= 160) && (my->count[3] < 1233)
       && ((my->count[3] - 160) % 29 == 0))
      || ((my->count[3] >= 1560) && (my->count[3] < 2633)
          && ((my->count[3] - 1560) % 29 == 0)) 
      || (my->count[3] == 1370) || (my->count[3] == 2770))
  {
    tenm_table_add(normal_shot_point_new(my->x, my->y, 2.5,
                                         player->x, player->y,
                                         4));
  }

  return 0;
}

static int
p_can_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  char temp[32];

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "p_can_draw: my is NULL\n");
    return 0;
  }

  /* dead enemy has low priority */
  if (((my->count[2] <= 1) && (priority != 0))
      || ((my->count[2] > 1) && (priority != -1)))
    return 0;

  if (p_can_green(my))
  {
    if (my->count[1] >= 1)
      color = tenm_map_color(109, 125, 9);
    else
      color = tenm_map_color(61, 95, 13);
  }
  else
  {
    if (my->count[1] >= 1)
      color = tenm_map_color(135, 89, 9);
    else
      color = tenm_map_color(95, 47, 13);
  }

  /* turret */
  if (tenm_draw_circle((int) (my->x), (int) (my->y), 50, 1, color) != 0)
    status = 1;

  if (tenm_draw_line((int) (my->x + my->count_d[2] + my->count_d[3] * 3.0/4.0),
                     (int) (my->y + my->count_d[3] - my->count_d[2] * 3.0/4.0),
                     (int) (my->x + my->count_d[2] - my->count_d[3] * 3.0/4.0),
                     (int) (my->y + my->count_d[3] + my->count_d[2] * 3.0/4.0),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x + my->count_d[2] - my->count_d[3] * 3.0/4.0),
                     (int) (my->y + my->count_d[3] + my->count_d[2] * 3.0/4.0),
                     (int) (my->x - my->count_d[2] - my->count_d[3] * 3.0/4.0),
                     (int) (my->y - my->count_d[3] + my->count_d[2] * 3.0/4.0),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - my->count_d[2] - my->count_d[3] * 3.0/4.0),
                     (int) (my->y - my->count_d[3] + my->count_d[2] * 3.0/4.0),
                     (int) (my->x - my->count_d[2] + my->count_d[3] * 3.0/4.0),
                     (int) (my->y - my->count_d[3] - my->count_d[2] * 3.0/4.0),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - my->count_d[2] + my->count_d[3] * 3.0/4.0),
                     (int) (my->y - my->count_d[3] - my->count_d[2] * 3.0/4.0),
                     (int) (my->x + my->count_d[2] + my->count_d[3] * 3.0/4.0),
                     (int) (my->y + my->count_d[3] - my->count_d[2] * 3.0/4.0),
                     1, color) != 0)
    status = 1;

  if (tenm_draw_line((int) (my->x + my->count_d[2] * 30.0 / 40.0
                            + my->count_d[3] * 3.0 / 4.0),
                     (int) (my->y + my->count_d[3] * 30.0 / 40.0
                            - my->count_d[2] * 3.0 / 4.0),
                     (int) (my->x + my->count_d[2] * 30.0 / 40.0
                            - my->count_d[3] * 3.0 / 4.0),
                     (int) (my->y + my->count_d[3] * 30.0 / 40.0
                            + my->count_d[2] * 3.0 / 4.0),
                     1, color) != 0)
    status = 1;

  /* body */
  if (tenm_draw_line((int) (my->x + 50.0), (int) (my->y - 50.0),
                     (int) (my->x + 50.0), (int) (my->y + 50.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x + 50.0), (int) (my->y + 50.0),
                     (int) (my->x - 50.0), (int) (my->y + 50.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 50.0), (int) (my->y + 50.0),
                     (int) (my->x - 50.0), (int) (my->y - 50.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 50.0), (int) (my->y - 50.0),
                     (int) (my->x + 50.0), (int) (my->y - 50.0),
                     3, color) != 0)
    status = 1;

  /* hit point stat */
  if (my->count[2] == 1)
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 10, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "p_can_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

/* return 1 (true) or 0 (false) */
static int
p_can_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if ((my->count[2] == 1)
      && (my->count[3] >= 1370) && (my->count[3] < 2950))
    return 1;
  if ((my->count[2] == 2) && (my->count[4] != 0))
    return 1;

  return 0;
}
