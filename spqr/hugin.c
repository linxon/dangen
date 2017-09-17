/* $Id: hugin.c,v 1.72 2011/08/23 20:07:50 oohara Exp $ */
/* [very easy] Hugin */

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

#include "hugin.h"

#define NEAR_ZERO 0.0001

static int hugin_move(tenm_object *my, double turn_per_frame);
static int hugin_hit(tenm_object *my, tenm_object *your);
static void hugin_next(tenm_object *my);
static int hugin_act(tenm_object *my, const tenm_object *player);
static int hugin_draw(tenm_object *my, int priority);
static int hugin_green(const tenm_object *my);

static tenm_object *munin_new(int table_index);
static int munin_move(tenm_object *my, double turn_per_frame);
static int munin_hit(tenm_object *my, tenm_object *your);
static int munin_signal(tenm_object *my, int n);
static int munin_act(tenm_object *my, const tenm_object *player);
static int munin_draw(tenm_object *my, int priority);
static int munin_green(const tenm_object *my);

tenm_object *
hugin_new(void)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = (double) (WINDOW_WIDTH / 2);
  double y = -39.0;

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "hugin_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + 40.0, y - 40.0,
                                             x + 40.0, y + 40.0,
                                             x - 40.0, y + 40.0,
                                             x - 40.0, y - 40.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "hugin_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 8);
  if (count == NULL)
  {
    fprintf(stderr, "hugin_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 4);
  if (count_d == NULL)
  {
    fprintf(stderr, "hugin_new: malloc(count_d) failed\n");
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
   * [4] shoot randomness
   * [5] move direction
   * [6] "munin killed" flag
   * [7] "was green when killed" flag
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   * [2] shoot direction x
   * [3] shoot direction y
   */

  count[0] = 0;
  count[1] = 0;
  count[2] = 0;
  count[3] = 0;
  count[4] = 0;
  count[5] = 0;
  count[6] = 0;
  count[7] = 0;

  count_d[0] = 0.0;
  count_d[1] = (((double) (WINDOW_HEIGHT / 4)) - y) / 90.0;
  count_d[2] = 0.0;
  count_d[3] = 1.0;

  new = tenm_object_new("Hugin", ATTR_BOSS, ATTR_PLAYER_SHOT,
                        500, x, y,
                        8, count, 4, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&hugin_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&hugin_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&hugin_act),
                        (int (*)(tenm_object *, int))
                        (&hugin_draw));
  if (new == NULL)
  {
    fprintf(stderr, "hugin_new: tenm_object_new failed\n");
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
hugin_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "hugin_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "hugin_move: strange turn_per_frame (%f)\n",
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
hugin_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "hugin_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "hugin_hit: your is NULL\n");
    return 0;
  }

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;
  if (my->count[2] != 1)
    return 0;

  deal_damage(my, your, 0);
  if (hugin_green(my))
    add_chain(my, your);
  my->count[1] = 2;

  if (my->hit_point <= 0)
  {
    add_score(3000);
    set_background(1);
    hugin_next(my);
    return 0;
  }

  return 0;
}

static void
hugin_next(tenm_object *my)
{
  int n;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "hugin_next: my is NULL\n");
    return;
  }

  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy_shot), 0);
  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy), 0);

  /* set "was green" flag before we change the life mode */
  if (hugin_green(my))
  {
    n = 8;
    my->count[7] = 1;
  }
  else
  {
    n = 7;
    my->count[7] = 0;
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
hugin_act(tenm_object *my, const tenm_object *player)
{
  double result[2];
  double v[2];
  int theta;
  int t;
  double x;
  int i;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "hugin_act: my is NULL\n");
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
      my->count[3] = -1;
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
      return 0;
    }
    return 0;
  }

  /* dead */
  if (my->count[2] == 2)
  {
    if (hugin_green(my))
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
  if ((my->count[2] == 1) && (my->count[3] >= 2160))
  {
    set_background(2);
    clear_chain();
    hugin_next(my);
    return 0;
  }

  if (my->count[3] == 270)
    tenm_table_add(munin_new(my->table_index));

  t = my->count[3] % 160;

  /* move */
  if (my->count[3] == 270)
  {
    my->count_d[0] = (((double) (WINDOW_WIDTH * 3 / 4)) - my->x) / 45.0;
    my->count_d[1] = 0.0;
  }
  else if (my->count[3] == 315)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
  }
  else if (my->count[3] >= 320)
  {  
    if (t == 110)
    {
      if (my->x < (double) (WINDOW_WIDTH / 2))
        my->count[5] = 0;
      else
        my->count[5] = 1;
    }
    if ((t >= 110) && (t <= 155))
    {
      x = ((double) (WINDOW_WIDTH / 4)) * tenm_sin(-90 + (t - 110) * 4);
      x += (double) (WINDOW_WIDTH / 2);
      if (my->count[5] != 0)
        x = ((double) (WINDOW_WIDTH)) - x;
      my->count_d[0] = x - my->x;
      my->count_d[1] = 0.0;
    }
    else
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
  }

  /* shoot */
  if (my->count[3] >= 2080)
    return 0;

  if (t == 0)
  {   
    if (my->count[6] == 0)
    {
      my->count_d[2] = player->x;
      if ((double) (rand() % (WINDOW_WIDTH + 1)) > my->count_d[2])
        my->count[4] = 0;
      else
        my->count[4] = 1;
      if (rand() % 2 == 0)
        my->count[4] += 2;
    }
    else
    {
      my->count[4] = 4;
      my->count_d[2] = player->x - my->x;
      my->count_d[3] = player->y - my->y;
      if (my->count_d[2] * my->count_d[2]
          + my->count_d[3] * my->count_d[3] < NEAR_ZERO)
      {
        my->count_d[2] = 0.0;
        my->count_d[3] = 1.0;
      }
      v[0] = my->count_d[2];
      v[1] = my->count_d[3];
      result[0] = v[0];
      result[1] = v[1];
      vector_rotate(result, v, 5 - (rand() % 2) * 10);
      my->count_d[2] = result[0];
      my->count_d[3] = result[1];
    }
  }

  if (my->count[4] < 4)
  {
    if ((my->count[3] % 160 < 90) && ((my->count[3] % 160) % 6 == 0))
    {
      if (my->count[4] < 2)
      {
        tenm_table_add(normal_shot_point_new(my->x + 100.0, my->y - 85.0, 6.0,
                                             my->count_d[2] + 45.0,
                                             player->y, 4));
        tenm_table_add(normal_shot_point_new(my->x - 100.0, my->y - 85.0, 6.0,
                                             my->count_d[2] - 45.0,
                                             player->y, 4));
      }
      else
      {
        tenm_table_add(normal_shot_point_new(my->x + 100.0, my->y - 85.0, 6.0,
                                             my->count_d[2] - 120.0,
                                             player->y, 4));
        tenm_table_add(normal_shot_point_new(my->x - 100.0, my->y - 85.0, 6.0,
                                             my->count_d[2] + 120.0,
                                             player->y, 4));
      }

      if (my->count[4] == 0)
        my->count_d[2] += 15.0;
      else
        my->count_d[2] -= 15.0;
      if (my->count_d[2] < 0.0)
        my->count_d[2] = 0.0;
    if (my->count_d[2] > (double) WINDOW_WIDTH)
      my->count_d[2] = (double) WINDOW_WIDTH;
    }
  }
  else
  {
    if ((my->count[3] % 2 == 0) && (my->count[3] % 160 <= 110))
    {
      if (my->count[3] % 160 <= 30)
      {  
        theta = 0;
      }
      else
      {
        theta = rand() % (((my->count[3] % 160) - 30) * 2 + 1);
        theta -= (my->count[3] % 160) - 30;
      }
      v[0] = my->count_d[2];
      v[1] = my->count_d[3];
      result[0] = v[0];
      result[1] = v[1];
      vector_rotate(result, v, theta);
      tenm_table_add(normal_shot_point_new(my->x, my->y, 6.0,
                                           my->x + result[0],
                                           my->y + result[1], 2));
    }
  }

  return 0;
}

static int
hugin_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  char temp[32];

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "hugin_draw: my is NULL\n");
    return 0;
  }

  /* dead enemy has low priority */
  if (((my->count[2] <= 1) && (priority != 0))
      || ((my->count[2] > 1) && (priority != -1)))
    return 0;

  /* body */
  if (hugin_green(my))
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

  /* wing */
  if (tenm_draw_line((int) (my->x + 40.0), (int) (my->y - 40.0),
                     (int) (my->x + 100.0), (int) (my->y - 85.0),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 40.0), (int) (my->y - 40.0),
                     (int) (my->x - 100.0), (int) (my->y - 85.0),
                     1, color) != 0)
    status = 1;

  /* core */
  if (tenm_draw_line((int) (my->x + 40.0), (int) (my->y - 40.0),
                     (int) (my->x + 40.0), (int) (my->y + 40.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x + 40.0), (int) (my->y + 40.0),
                     (int) (my->x - 40.0), (int) (my->y + 40.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 40.0), (int) (my->y + 40.0),
                     (int) (my->x - 40.0), (int) (my->y - 40.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 40.0), (int) (my->y - 40.0),
                     (int) (my->x + 40.0), (int) (my->y - 40.0),
                     3, color) != 0)
    status = 1;

  /* hit point stat */
  if (my->count[2] == 1)
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 10, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "hugin_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

/* return 1 (true) or 0 (false) */
static int
hugin_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if ((my->count[2] == 1) && (my->count[6] != 0)
      && (my->count[3] < 2130))
    return 1;
  if ((my->count[2] == 2) && (my->count[7] != 0))
    return 1;

  return 0;
}

static tenm_object *
munin_new(int table_index)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = (double) (WINDOW_WIDTH / 4);
  double y = -39.0;

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "munin_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + 40.0, y - 40.0,
                                             x + 40.0, y + 40.0,
                                             x - 40.0, y + 40.0,
                                             x - 40.0, y - 40.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "munin_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 8);
  if (count == NULL)
  {
    fprintf(stderr, "munin_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 4);
  if (count_d == NULL)
  {
    fprintf(stderr, "munin_new: malloc(count_d) failed\n");
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
   * [4] shoot randomness
   * [5] move direction
   * [6] Hugin index
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   * [2] shoot direction x
   * [3] shoot direction y
   */

  count[0] = 0;
  count[1] = 0;
  count[2] = 0;
  count[3] = 0;
  count[4] = -1;
  count[5] = 0;
  count[6] = table_index;

  count_d[0] = 0.0;
  count_d[1] = (((double) (WINDOW_HEIGHT / 4)) - y) / 45.0;
  count_d[2] = 0.0;
  count_d[3] = 1.0;

  new = tenm_object_new("Munin", ATTR_ENEMY, ATTR_PLAYER_SHOT,
                        500, x, y,
                        8, count, 4, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&munin_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&munin_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&munin_act),
                        (int (*)(tenm_object *, int))
                        (&munin_draw));
  if (new == NULL)
  {
    fprintf(stderr, "munin_new: tenm_object_new failed\n");
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
munin_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "munin_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "munin_move: strange turn_per_frame (%f)\n",
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
munin_hit(tenm_object *my, tenm_object *your)
{
  int n;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "munin_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "munin_hit: your is NULL\n");
    return 0;
  }

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;
  if (my->count[2] != 1)
    return 0;

  deal_damage(my, your, 0);
  if (munin_green(my))
    add_chain(my, your);
  my->count[1] = 41;

  if (my->hit_point <= 0)
  {
    add_score(3000);
    if (munin_green(my))
      n = 8;
    else
      n = 7;

    tenm_table_add(explosion_new(my->x, my->y,
                                 0.0, 0.0,
                                 1, 3000, n, 10.0, 8));
    tenm_table_add(fragment_new(my->x, my->y, 0.0, 0.0,
                                30.0, 100, n, 4.0, 0.0, 16));
    tenm_table_add(fragment_new(my->x, my->y, 0.0, 0.0,
                                50.0, 30, n, 2.5, 0.0, 12));

    tenm_table_apply(my->count[6],
                     (int (*)(tenm_object *, int))
                     (&munin_signal),
                     0);
    return 1;
  }

  return 0;
}
static int
munin_signal(tenm_object *my, int n)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (strcmp(my->name, "Hugin") != 0)
    return 0;

  my->count[6] = 1;

  return 0;
}

static int
munin_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int j;
  double x;
  double y;
  double v[2];
  double result[2];
  int theta;
  double speed;
  int t;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "munin_act: my is NULL\n");
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
  t = my->count[3] % 160;

  /* encounter */
  if (my->count[2] == 0)
  {
    if (my->count[3] == 45)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[3] >= 50)
    {
      my->count[2] = 1;
      my->count[3] = 320;
      return 0;
    }
    return 0;
  }

  /* move */
  if (t == 110)
  {
    if (my->x < (double) (WINDOW_WIDTH / 2))
      my->count[5] = 0;
    else
      my->count[5] = 1;
  }
  if ((t >= 110) && (t <= 155))
  {
    x = ((double) (WINDOW_WIDTH / 4)) * tenm_sin(-90 + (t - 110) * 4);
    x += (double) (WINDOW_WIDTH / 2);
    if (my->count[5] != 0)
      x = ((double) (WINDOW_WIDTH)) - x;
    my->count_d[0] = x - my->x;
    my->count_d[1] = 0.0;
  }
  else
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
  }

  /* shoot */
  if (my->count[3] < 480)
    return 0;
  if (my->count[3] >= 2080)
    return 0;

  if (t == 0)
  {
    if (my->count[4] < 0)
      my->count[4] = rand() % 3;
    else if (rand() % 5 != 0)
      my->count[4] += 1 + rand() % 2;
    while (my->count[4] >= 3)
      my->count[4] -= 3;
    while (my->count[4] < 0)
      my->count[4] += 3;
  }

  switch (my->count[4])
  {
  case 0:
    if ((t < 60) && (t % 15 == 0))
    {
      for (i = 0; i < 2; i++)
        for (j = -2; j <= 2; j++)
        {
          x = my->x - 100.0 + 200.0 * ((double) i);
          y = my->y - 85.0;
          theta = 80 + 20 * i + 24 * j;
          if (t < 30)
            theta += 12 - 24 * i;
          tenm_table_add(laser_angle_new(x, y, 4.0,
                                         theta, 25.0, 0));
        }
    }
    break;
  case 1:
    if (t == 10)
    {
      my->count_d[2] = player->x - my->x;
      my->count_d[3] = player->y - my->y;
      if (my->count_d[2] * my->count_d[2]
          + my->count_d[3] * my->count_d[3] < NEAR_ZERO)
      {
        my->count_d[2] = 0.0;
        my->count_d[3] = 1.0;
      }
    }

    if ((t == 20) || (t == 30) || (t == 40) || (t == 50))
    {
      if (t == 20)
        theta = 8;
      else if (t == 30)
        theta = 24;
      else if (t == 40)
        theta = 40;
      else
        theta = 0;
      for (i = 0; i < 2; i++)
        for (j = -1; j <= 1; j++)
        {
          if (j == 0)
            continue;
          x = my->x - 100.0 + 200.0 * ((double) i);
          y = my->y - 85.0;
          v[0] = my->count_d[2] - (-100.0 + 200.0 * ((double) i));
          v[1] = my->count_d[3] - (-85.0);
          result[0] = v[0];
          result[1] = v[1];
          vector_rotate(result, v, theta * j);
          tenm_table_add(laser_point_new(x, y, 5.5,
                                         x + result[0], y + result[1],
                                         25.0, 1));
          if (t == 50)
            break;
        }
    }
    break;
  case 2:
    if ((t % 8 == 0) && (t < 80))
    {
      for (i = 0; i < 2; i++)
      {
        x = my->x - 100.0 + 200.0 * ((double) i);
        y = my->y - 85.0;
        if (t < 40)
          theta = 80 + 13 * (t / 8);
        else
          theta = 152 - 13 * ((t - 40) / 8);
        if (i != 0)
          theta = 180 - theta;
        speed = 3.0 + 0.8 * ((double) ((t % 40) / 8));
        tenm_table_add(laser_angle_new(x, y, speed,
                                       theta, 25.0, 2));
      }
    }
    break;
  default:
    fprintf(stderr, "munin_act: undefined shoot mode (%d)\n", my->count[4]);
    break;
  }

  return 0;
}

static int
munin_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  char temp[32];

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "munin_draw: my is NULL\n");
    return 0;
  }

  if (priority != 0)
    return 0;

  /* body */
  if (munin_green(my))
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

  /* wing */
  if (tenm_draw_line((int) (my->x + 40.0), (int) (my->y - 40.0),
                     (int) (my->x + 100.0), (int) (my->y - 85.0),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 40.0), (int) (my->y - 40.0),
                     (int) (my->x - 100.0), (int) (my->y - 85.0),
                     1, color) != 0)
    status = 1;

  /* core */
  if (tenm_draw_line((int) (my->x + 40.0), (int) (my->y - 40.0),
                     (int) (my->x + 40.0), (int) (my->y + 40.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x + 40.0), (int) (my->y + 40.0),
                     (int) (my->x - 40.0), (int) (my->y + 40.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 40.0), (int) (my->y + 40.0),
                     (int) (my->x - 40.0), (int) (my->y - 40.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 40.0), (int) (my->y - 40.0),
                     (int) (my->x + 40.0), (int) (my->y - 40.0),
                     3, color) != 0)
    status = 1;

  /* hit point stat */
  if (my->count[1] >= 1)
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 10, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "munin_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

/* return 1 (true) or 0 (false) */
static int
munin_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if ((my->count[2] == 1) && (my->count[3] >= 480) && (my->count[3] < 2130))
    return 1;

  return 0;
}
