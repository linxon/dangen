/* $Id: watcher-below.c,v 1.107 2005/05/03 16:21:43 oohara Exp $ */
/* [easy] Watcher Below */

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

#include "watcher-below.h"

#define NEAR_ZERO 0.0001

static int watcher_below_move(tenm_object *my, double turn_per_frame);
static int watcher_below_hit(tenm_object *my, tenm_object *your);
static void watcher_below_next(tenm_object *my);
static int watcher_below_signal(tenm_object *my, int n);
static int watcher_below_act(tenm_object *my, const tenm_object *player);
static int watcher_below_in_territory(double x, double y, double r);
static int watcher_below_draw(tenm_object *my, int priority);
static int watcher_below_green(const tenm_object *my);

static tenm_object *watcher_below_bit_new(double x, double y,
                                          double speed_x, double speed_y,
                                          int table_index, int what);
static int watcher_below_bit_move(tenm_object *my, double turn_per_frame);
static int watcher_below_bit_hit(tenm_object *my, tenm_object *your);
static void watcher_below_bit_explode(tenm_object *my);
static int watcher_below_bit_signal(tenm_object *my, int n);
static int watcher_below_bit_act(tenm_object *my, const tenm_object *player);
static int watcher_below_bit_draw(tenm_object *my, int priority);
static int watcher_below_bit_green(const tenm_object *my);

tenm_object *
watcher_below_new(void)
{
  int i;
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = (double) (WINDOW_WIDTH / 2);
  double y = ((double) (WINDOW_HEIGHT / 2)) + 20.0;

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "watcher_below_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_circle_new(x, y, 45.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "watcher_below_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 9);
  if (count == NULL)
  {
    fprintf(stderr, "watcher_below_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "watcher_below_new: malloc(count_d) failed\n");
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
   * [4] number of bits killed
   * [5 -- 7] bit index
   * [8] "was green when killed" flag
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */

  count[0] = 0;
  count[1] = 0;
  count[2] = 0;
  count[3] = 5;
  count[4] = 0;
  for (i = 5; i < 8; i++)
    count[i] = -1;
  count[8] = 0;

  count_d[0] = 0.0;
  count_d[1] = 0.0;

  new = tenm_object_new("Watcher Below", 0, 0,
                        600, x, y,
                        9, count, 2, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&watcher_below_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&watcher_below_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&watcher_below_act),
                        (int (*)(tenm_object *, int))
                        (&watcher_below_draw));

  if (new == NULL)
  {
    fprintf(stderr, "watcher_below_new: tenm_object_new failed\n");
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
watcher_below_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "watcher_below_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "watcher_below_move: strange turn_per_frame (%f)\n",
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
watcher_below_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "watcher_below_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "watcher_below_hit: your is NULL\n");
    return 0;
  }

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;
  if (my->count[2] != 1)
    return 0;

  deal_damage(my, your, 0);
  if (watcher_below_green(my))
    add_chain(my, your);
  my->count[1] = 2;

  if (my->hit_point <= 0)
  {
    add_score(10000);
    set_background(1);
    watcher_below_next(my);
    return 0;
  }

  return 0;
}

static void
watcher_below_next(tenm_object *my)
{
  int i;
  int n;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "watcher_below_next: my is NULL\n");
    return;
  }

  /* set "was green" flag before we change the life mode */
  if (watcher_below_green(my))
  {
    n = 8;
    my->count[8] = 1;
  }
  else
  {
    n = 7;
    my->count[8] = 0;
  }

  tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                               1, 5000, n, 10.0, 6));

  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy_shot), 0);
  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy), 0);

  for (i = 5; i < 8; i++)
    if (my->count[i] >= 0)
      tenm_table_apply(my->count[i],
                       (int (*)(tenm_object *, int)) (&watcher_below_signal),
                       0);

  my->count[2] = 2;
  my->count[3] = 0;
  my->count[1] = 0;
  my->count_d[0] = 0.0;
  my->count_d[1] = 0.5;

  my->attr = ATTR_BOSS;
  my->hit_mask = ATTR_PLAYER_SHOT;
  tenm_mass_delete(my->mass);
  my->mass = NULL;
}

static int
watcher_below_signal(tenm_object *my, int n)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (strcmp(my->name, "Watcher Below bit") != 0)
    return 0;

  /* the explosion should be brown since this is a suicide */
  my->count[2] = 1;
  watcher_below_bit_explode(my);

  return 1;
}

static int
watcher_below_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int j;
  int theta;
  int dtheta;
  int direction;
  double x;
  double y;
  double dx;
  double dy;
  double result[2];
  double v[2];
  double a[2];
  double temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "watcher_below_act: my is NULL\n");
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

  /* dead */
  if (my->count[2] == 2)
  {
    if (watcher_below_green(my))
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
      tenm_table_add(explosion_new(my->x, my->y,
                                   0.0, 0.0,
                                   2, 800, i, 6.0, 8));

      tenm_table_add(stage_clear_new(100));
      return 1;
    }
    
    return 0;
  }
  
  /* phasing */
  if (my->count[2] == 1)
  {
    if (watcher_below_in_territory(my->x, my->y, 45.0))
    {
      my->attr = ATTR_BOSS;
      my->hit_mask = ATTR_PLAYER_SHOT;
    }
    else
    {
      my->attr = 0;
      my->hit_mask = 0;
    }
  }
  
  /* add bit */
  if ((my->count[2] == 1) && (my->count[3] >= 180) && (my->count[3] <= 300)
      && (my->count[3] % 60== 0))
  {
    if (my->count[3] == 180)
      theta = 40;
    else if (my->count[3] == 240)
      theta = 155;
    else
      theta = 55;
    x = my->x;
    y = my->y;
    if (x - 10.0 < 0.0)
      x = 10.0;
    if (x + 10.0 > (double) WINDOW_WIDTH)
      x = ((double) WINDOW_WIDTH) - 10.0;
    if (y - 10.0 < 0.0)
      y = 10.0;
    if (y + 10.0 > ((double) WINDOW_HEIGHT))
      y = ((double) WINDOW_HEIGHT) - 10.0;
    i = 2 + my->count[3] / 60;
    my->count[i] = tenm_table_add(watcher_below_bit_new(x, y,
                                                        6.0 * tenm_cos(theta),
                                                        6.0 * tenm_sin(theta),
                                                        my->table_index,
                                                        my->count[3]/60 - 3));
  }

  /* move direction change  */
  v[0] = my->count_d[0];
  v[1] = my->count_d[1];
  result[0] = v[0];
  result[1] = v[1];
  if ((my->count[2] == 1)
      && (my->x - 10.0 > 0.0) && (my->x + 10.0 < (double) WINDOW_WIDTH)
      && (my->y - 10.0 > 0.0)
      && (my->y + 10.0 < (double) WINDOW_HEIGHT))
  {
    if (my->attr != 0)
    {
      /* chase player */
      v[0] = my->count_d[0];
      v[1] = my->count_d[1];
      a[0] = player->x - my->x;
      a[1] = player->y - my->y;
      result[0] = v[0];
      result[1] = v[1];
      vector_rotate_bounded(result, v, a, 1);
    }
    else if ((my->count_d[0] > 0.0) && (my->count_d[0] < 1.0))
    {
      /* force a good direction */
      if (my->count_d[1] > 0.0)
        vector_rotate(result, v, -1);
      else
        vector_rotate(result, v, 1);
    }
    else if ((my->count_d[0] > -1.0) && (my->count_d[0] < NEAR_ZERO))
    {
      if (my->count_d[1] > 0.0)
        vector_rotate(result, v, 1);
      else
        vector_rotate(result, v, -1);
    }
    else if ((my->count_d[1] > 0.0) && (my->count_d[1] < 1.0))
    {
      if (my->count_d[0] > 0.0)
        vector_rotate(result, v, 1);
    else
      vector_rotate(result, v, -1);
    }
    else if ((my->count_d[1] > -1.0) && (my->count_d[1] < NEAR_ZERO))
    {
      if (my->count_d[0] > 0.0)
        vector_rotate(result, v, -1);
      else
        vector_rotate(result, v, 1);
    }
  }
  my->count_d[0] = result[0];
  my->count_d[1] = result[1];

  /* reflect */
  if ((my->x < 0.0) || (my->x > (double) WINDOW_WIDTH))
    my->count_d[0] *= -1.0;
  if ((my->y < 0.0) || (my->y > (double) WINDOW_HEIGHT))
    my->count_d[1] *= -1.0;

  /* encounter */
  if (my->count[2] == 0)
  {
    if (my->count[3] == 45)
    {
      my->count_d[0] = 4.0;
      my->count_d[1] = 3.0;
    }
    if (my->count[3] >= 180)
    {
      my->count[2] = 1;
      my->count[3] = 0;
    }
    return 0;
  }

  /* self-destruction */
  if ((my->count[2] == 1) && (my->count[3] >= 4130))
  {
    set_background(2);
    clear_chain();
    watcher_below_next(my);
    return 0;
  }

  /* shoot */
  if (my->attr == 0)
    return 0;
  if (my->count[2] != 1)
    return 0;
  if (my->count[3] > 4000)
    return 0;

  if (((my->count[4] >= 2) && (my->count[3] % 38 == 0))
      || ((my->count[4] >= 3) && (my->count[3] % 19 == 0)))
  {
    dx = player->x - my->x;
    dy = player->y - my->y;
    temp = tenm_sqrt((int) (dx * dx + dy * dy));
    if (temp < NEAR_ZERO)
      temp = 1.0;
    v[0] = dx / temp;
    v[1] = dy / temp;
    result[0] = v[0];
    result[1] = v[1];
    if (my->count[3] % 38 == 0)
      vector_rotate(result, v, (60 + rand() % 61) * (2 * (rand() % 2) - 1));
    dx = result[0];
    dy = result[1];

    theta = rand() % 360;
    if (rand() % 2)
      direction = 1;
    else
      direction = -1;
    if (rand() % 2)
    {
      for (i = 0; i < 3; i++)
        for (j = 0; j < 4; j++)
        {
          dtheta = (i * 120 + j * 30) * direction;
          x = my->x + 40.0 * tenm_cos(theta + dtheta);
          y = my->y + 40.0 * tenm_sin(theta + dtheta);
          dx = 4.0 * result[0];
          dy = 4.0 * result[1];
          dx -= 0.6 * ((double) (j + 1)) * tenm_cos(theta + dtheta);
          dy -= 0.6 * ((double) (j + 1)) * tenm_sin(theta + dtheta);
          tenm_table_add(normal_shot_new(x, y, dx, dy, 1, -2, 0));
        }
    }
    else
    {
      for (i = 0; i < 6; i++)
        for (j = 0; j < 2; j++)
        {

          dtheta = i * 60 + j * 30;
          if (j == 0)
          {            
            x = my->x + 40.0 * tenm_cos(theta + dtheta);
            y = my->y + 40.0 * tenm_sin(theta + dtheta);
            dx = 3.0 * result[0];
            dy = 3.0 * result[1];
            dx -= 1.0 * tenm_cos(theta + dtheta);
            dy -= 1.0 * tenm_sin(theta + dtheta);
          }
          else
          {            
            x = my->x;
            y = my->y;
            dx = 3.0 * result[0];
            dy = 3.0 * result[1];
            dx += 1.0 * tenm_cos(theta + dtheta);
            dy += 1.0 * tenm_sin(theta + dtheta);
          }
          tenm_table_add(normal_shot_new(x, y, dx, dy, 2, -2, 0));
        }
    }
  }

  if ((my->count[4] >= 1) && (my->count[3] % 11 == 0))
  {
    dx = my->count_d[0];
    dy = my->count_d[1];
    temp = tenm_sqrt((int) (dx * dx + dy * dy));
    if (temp < NEAR_ZERO)
      temp = 1.0;
    v[0] = dx / temp;
    v[1] = dy / temp;
    for (i = -1; i <= 1; i += 2)
    {
      result[0] = v[0];
      result[1] = v[1];
      vector_rotate(result, v, 120 * i);
      tenm_table_add(laser_new(my->x, my->y,
                               7.5 * result[0], 7.5 * result[1],
                               30.0 * result[0], 30.0 * result[1],
                               4, -2, 0));
    }
  }

  return 0;
}

/* return 1 (true) or 0 (false) */
static int
watcher_below_in_territory(double x, double y, double r)
{
  double x1;
  double y1;

  /* sanity check */
  if (r < NEAR_ZERO)
  {
    fprintf(stderr, "watcher_below_in_territory: r is non-positive (%f)\n", r);
    return 0;
  }

  x1 = (x - (double) (WINDOW_WIDTH / 2)) * 7.0 / tenm_sqrt(50)
    + (y - (double) (WINDOW_HEIGHT / 2)) * (-1.0) / tenm_sqrt(50);
  y1 = -(x - (double) (WINDOW_WIDTH / 2)) * (-1.0) / tenm_sqrt(50)
    + (y - (double) (WINDOW_HEIGHT / 2)) * 7.0 / tenm_sqrt(50);

  if ((x1 - r > -212.1320) && (x1 + r < 212.1320)
      && (y1 - r > -212.1320) && (y1 + r < 212.1320))
    return 1;

  return 0;
}

static int
watcher_below_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  char temp[32];
  int width;
  int r;
  int c;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "watcher_below_draw: my is NULL\n");
    return 0;
  }

  /* territory */
  if ((priority == -1)
      && (((my->count[2] == 0) && (my->count[3] >= 90))
          || (my->count[2] == 1)))
  {
    c = 0;
    color = tenm_map_color(182, 147, 123);
    if (my->count[2] == 0)
    {
      color = tenm_map_color(158, 158, 158);
      c = 180 - my->count[3];
      if (c < 0)
        c = 0;
      if (c > 90)
        c = 90;
    }
    
    if (tenm_draw_line((500 * (90 - c) + WINDOW_WIDTH * c) / 90,
                       (0 * (90 - c) + 0 * c) / 90,
                       (560 * (90 - c) + WINDOW_WIDTH * c) / 90,
                       (420 * (90 - c) + WINDOW_HEIGHT * c) / 90,
                       1, color) != 0)
      status = 1;
    if (tenm_draw_line((560 * (90 - c) + WINDOW_WIDTH * c) / 90,
                       (420 * (90 - c) + WINDOW_HEIGHT * c) / 90,
                       (140 * (90 - c) + 0 * c) / 90,
                       (480 * (90 - c) + WINDOW_HEIGHT * c) / 90,
                       1, color) != 0)
      status = 1;
    if (tenm_draw_line((140 * (90 - c) + 0 * c) / 90,
                       (480 * (90 - c) + WINDOW_HEIGHT * c) / 90,
                       (80 * (90 - c) + 0 * c) / 90,
                       (60 * (90 - c) + 0 * c) / 90,
                       1, color) != 0)
      status = 1;
    if (tenm_draw_line((80 * (90 - c) + 0 * c) / 90,
                       (60 * (90 - c) + 0 * c) / 90,
                       (500 * (90 - c) + WINDOW_WIDTH * c) / 90,
                       (0 * (90 - c) + 0 * c) / 90,
                       1, color) != 0)
      status = 1;
  }

  /* body */
  /* dead enemy has low priority */
  if (((my->count[2] <= 1) && (priority == 0))
      || ((my->count[2] > 1) && (priority == -1)))
  {
    if (watcher_below_green(my))
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

    if (my->attr != 0)
      width = 3;
    else
      width = 1;

    r = 45;
    if (my->count[2] == 0)
    {
      r = my->count[3];
      if (r < 5)
        r = 5;
      if (r > 45)
        r = 45;
    }

    if (tenm_draw_circle((int) (my->x), (int) (my->y), r, width, color) != 0)
      status = 1;
  }

  /* hit point stat */
  if ((priority == 0) && (my->count[2] == 1))
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 10, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "watcher_below_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

/* return 1 (true) or 0 (false) */
static int
watcher_below_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if ((my->count[2] == 1) && (my->count[4] >= 3)
      && (my->count[3] <= 4100))
    return 1;
  if ((my->count[2] == 2) && (my->count[8] != 0))
    return 1;

  return 0;
}

static tenm_object *
watcher_below_bit_new(double x, double y,
                      double speed_x, double speed_y,
                      int table_index, int what)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  int attr;
  int hit_mask;

  /* sanity check */
  if (speed_x * speed_x + speed_y * speed_y < NEAR_ZERO)
  {
    fprintf(stderr, "watcher_below_bit_new: speed is too small (%f, %f)\n",
            speed_x, speed_y);
    return NULL;
  }
  if (table_index < 0)
  {
    fprintf(stderr, "watcher_below_bit_new: table_index is negative (%d)\n",
            table_index);
    return NULL;
  }

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "watcher_below_bit_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_circle_new(x, y, 25.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "watcher_below_bit_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 6);
  if (count == NULL)
  {
    fprintf(stderr, "watcher_below_bit_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "watcher_below_bit_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  if (watcher_below_in_territory(x, y, 25.0))
  {
    attr = ATTR_BOSS;
    hit_mask = ATTR_PLAYER_SHOT;
  }
  else
  {
    attr = 0;
    hit_mask = 0;
  }

  /* list of count
   * [0] for deal_damage
   * [1] "damaged" timer
   * [2] life mode
   * [3] life timer
   * [4] core index
   * [5] what
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
  count[5] = what;

  count_d[0] = speed_x;
  count_d[1] = speed_y;

  new = tenm_object_new("Watcher Below bit", attr, hit_mask,
                        200, x, y,
                        6, count, 2, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&watcher_below_bit_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&watcher_below_bit_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&watcher_below_bit_act),
                        (int (*)(tenm_object *, int))
                        (&watcher_below_bit_draw));

  if (new == NULL)
  {
    fprintf(stderr, "watcher_below_bit_new: tenm_object_new failed\n");
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
watcher_below_bit_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "watcher_below_bit_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "watcher_below_bit_move: strange turn_per_frame (%f)\n",
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
watcher_below_bit_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "watcher_below_bit_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "watcher_below_bit_hit: your is NULL\n");
    return 0;
  }

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;

  deal_damage(my, your, 0);
  if (watcher_below_bit_green(my))
    add_chain(my, your);
  my->count[1] = 41;

  if (my->hit_point <= 0)
  {
    add_score(7500);
    watcher_below_bit_explode(my);
    tenm_table_apply(my->count[4],
                     (int (*)(tenm_object *, int)) (&watcher_below_bit_signal),
                     my->table_index);
    return 1;
  }

  return 0;
}

static void
watcher_below_bit_explode(tenm_object *my)
{
  int n;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "watcher_below_bit_explode: my is NULL\n");
    return;
  }

  if (watcher_below_bit_green(my))
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
}

static int
watcher_below_bit_signal(tenm_object *my, int n)
{
  int i;

  /* sanity check */
  if (my == NULL)
    return 0;
  if (strcmp(my->name, "Watcher Below") != 0)
    return 0;

  (my->count[4])++;
  for (i = 5; i < 8; i++)
    if (my->count[i] == n)
      my->count[i] = -1;

  return 0;
}

static int
watcher_below_bit_act(tenm_object *my, const tenm_object *player)
{
  int i;
  double dx;
  double dy;
  double result[2];
  double v[2];
  double temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "watcher_below_bit_act: my is NULL\n");
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

  /* phasing */
  if (watcher_below_in_territory(my->x, my->y, 25.0))
  {
    my->attr = ATTR_BOSS;
    my->hit_mask = ATTR_PLAYER_SHOT;
  }
  else
  {
    my->attr = 0;
    my->hit_mask = 0;
  }

  /* reflect */
  if ((my->x < 0.0) || (my->x > (double) WINDOW_WIDTH))
    my->count_d[0] *= -1.0;
  if ((my->y < 0.0) || (my->y > (double) WINDOW_HEIGHT))
    my->count_d[1] *= -1.0;

  /* shoot */
  if (my->attr == 0)
    return 0;
  if (my->count[2] != 0)
    return 0;
  if ((my->count[3] < 100) || (my->count[3] > 4000 - (my->count[5]*60 + 180)))
    return 0;

  if (my->count[3] % 11 == 0)
  {
    dx = my->count_d[0];
    dy = my->count_d[1];
    temp = tenm_sqrt((int) (dx * dx + dy * dy));
    if (temp < NEAR_ZERO)
      temp = 1.0;
    v[0] = dx / temp;
    v[1] = dy / temp;
    for (i = -1; i <= 1; i += 2)
    {
      result[0] = v[0];
      result[1] = v[1];
      vector_rotate(result, v, 120 * i);
      tenm_table_add(laser_new(my->x, my->y,
                               4.5 * result[0], 4.5 * result[1],
                               25.0 * result[0], 25.0 * result[1],
                               3, -2, 0));
    }
  }

  if (my->count[3] % 17 == 0)
  {
    tenm_table_add(normal_shot_point_new(my->x, my->y, 3.5,
                                         player->x, player->y,
                                         5));
  }

  return 0;
}

static int
watcher_below_bit_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  char temp[32];
  int width;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "watcher_below_bit_draw: my is NULL\n");
    return 0;
  }

  /* decoration */
  if ((priority == 0) && (my->count[2] <= 1))
  {
    if (watcher_below_bit_green(my))
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
  }

  /* body */
  if (priority == 0)
  {
    if (watcher_below_bit_green(my))
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

    if (my->attr != 0)
      width = 3;
    else
      width = 1;

    if (tenm_draw_circle((int) (my->x), (int) (my->y), 25, width, color) != 0)
      status = 1;
  }

  /* hit point stat */
  if ((priority == 0) && (my->count[1] >= 1))
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 10, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "watcher_below_bit_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

/* return 1 (true) or 0 (false) */
static int
watcher_below_bit_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if ((my->count[2] == 0)
      && (my->count[3] >= 100)
      && (my->count[3] <= 4100 - (my->count[5] * 60 + 180)))
    return 1;

  return 0;
}
