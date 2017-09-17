/* $Id: afterdeath.c,v 1.69 2011/08/24 17:31:02 oohara Exp $ */
/* [very easy] Afterdeath */

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

#include "afterdeath.h"

#define NEAR_ZERO 0.0001

static int afterdeath_move(tenm_object *my, double turn_per_frame);
static int afterdeath_hit(tenm_object *my, tenm_object *your);
static void afterdeath_next(tenm_object *my);
static int afterdeath_act(tenm_object *my, const tenm_object *player);
static int afterdeath_draw(tenm_object *my, int priority);
static int afterdeath_green(const tenm_object *my);

static tenm_object *afterdeath_ship_new(double x, double y, int what);
static int afterdeath_ship_move(tenm_object *my, double turn_per_frame);
static int afterdeath_ship_act(tenm_object *my, const tenm_object *player);
static int afterdeath_ship_draw(tenm_object *my, int priority);

tenm_object *
afterdeath_new(void)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = (double) (WINDOW_WIDTH / 2);
  double y = -47.0;

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "afterdeath_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + 36.0, y - 48.0,
                                             x + 36.0, y + 48.0,
                                             x - 36.0, y + 48.0,
                                             x - 36.0, y - 48.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "afterdeath_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 6);
  if (count == NULL)
  {
    fprintf(stderr, "afterdeath_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 3);
  if (count_d == NULL)
  {
    fprintf(stderr, "afterdeath_new: malloc(count_d) failed\n");
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
   * [4] ship what
   * [5] "was green when killed" flag
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   * [2] ship y
   */

  count[0] = 0;
  count[1] = 0;
  count[2] = 0;
  count[3] = 0;
  count[4] = 0;
  count[5] = 0;

  count_d[0] = 0.0;
  count_d[1] = (((double) (WINDOW_HEIGHT / 4)) - y) / 90.0;
  count_d[2] = 0.0;

  new = tenm_object_new("Afterdeath", ATTR_BOSS, ATTR_PLAYER_SHOT,
                        509, x, y,
                        6, count, 3, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&afterdeath_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&afterdeath_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&afterdeath_act),
                        (int (*)(tenm_object *, int))
                        (&afterdeath_draw));

  if (new == NULL)
  {
    fprintf(stderr, "afterdeath_new: tenm_object_new failed\n");
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
afterdeath_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "afterdeath_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "afterdeath_move: strange turn_per_frame (%f)\n",
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
afterdeath_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "afterdeath_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "afterdeath_hit: your is NULL\n");
    return 0;
  }

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;
  if (my->count[2] != 1)
    return 0;

  deal_damage(my, your, 0);
  if (afterdeath_green(my))
    add_chain(my, your);
  my->count[1] = 2;

  if (my->hit_point <= 0)
  {
    add_score(5000);
    set_background(1);
    afterdeath_next(my);
    return 0;
  }

  return 0;
}

static void
afterdeath_next(tenm_object *my)
{
  int n;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "afterdeath_next: my is NULL\n");
    return;
  }

  if (my->count[2] != 1)
    return;

  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy_shot), 0);
  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy), 0);

  /* set "was green" flag before we change the life mode */
  if (afterdeath_green(my))
  {
    n = 8;
    my->count[5] = 1;
  }
  else
  {
    n = 7;
    my->count[5] = 0;
  }

  tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                               1, 5000, n, 10.0, 6));
  tenm_table_add(fragment_new(my->x, my->y - 48.0, 0.0, 0.0,
                              20.0, 10, n, 3.0, 15.0, 24));
  tenm_table_add(fragment_new(my->x + 36.0, my->y, 0.0, 0.0,
                              30.0, 5, n, 3.0, 15.0, 24));
  tenm_table_add(fragment_new(my->x - 36.0, my->y + 25.0, 0.0, 0.0,
                              30.0, 5, n, 3.0, 15.0, 24));

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
afterdeath_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int theta;
  double x;
  double y;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "afterdeath_act: my is NULL\n");
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
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
      my->count[2] = 1;
      my->count[3] = -1;
      return 0;
    }
    return 0;
  }

  /* dead */
  if (my->count[2] == 2)
  {
    if (afterdeath_green(my))
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
  if ((my->count[2] == 1) && (my->count[3] >= 1930))
  {
    set_background(2);
    clear_chain();
    afterdeath_next(my);
    return 0;
  }

  /* speed change */
  if ((my->count[3] > 0) && (my->count[3] < 1800))
  {
    if (my->count[3] % 200 == 0)
    {
      x = (double) (40 + (rand() % (WINDOW_WIDTH - 80 + 1)));
      y = (double) (50 + (rand() % (WINDOW_HEIGHT / 3 - 50 + 1)));
      my->count_d[0] = (x - my->x) / 60.0;
      my->count_d[1] = (y - my->y) / 60.0;
    }
    if (my->count[3] % 200 == 60)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
  }

  if (my->count[2] != 1)
    return 0;
  if (my->count[3] >= 1800)
    return 0;

  /* summon ship */
  if (my->count[3] % 200 == 50)
  {
    my->count[4] = rand() % 4;
    my->count_d[2] = (double) (50 + (rand() % (WINDOW_HEIGHT / 3 - 50 + 1)));
    if ((my->count_d[2] - 5.0 < my->y) && (my->count_d[2] + 5.0 > my->y))
    {
      if (my->count_d[2] > my->y)
        my->count_d[2] = my->y + 5.0;
      else
        my->count_d[2] = my->y - 5.0;
    }
  }
  if (my->count[3] % 200 == 80)
  {
    x = -35.0;
    if (my->count[4] % 2 != 0)
      x = ((double) (WINDOW_WIDTH)) - x;
    tenm_table_add(afterdeath_ship_new(x, my->count_d[2], my->count[4]));
  }

  /* shoot */
  if ((my->count[3] % 3 == 0) && (my->count[3] % 200 <= 60))
  {
    theta = -122 + (my->count[3] % 200) * 3;
    tenm_table_add(laser_angle_new(my->x - 30.0, my->y, 5.0,
                                   theta, 25.0, 2));
    tenm_table_add(laser_angle_new(my->x + 30.0, my->y, 5.0,
                                   180 - theta, 25.0, 2));
  }

  switch (my->count[3] % 200)
  {
  case 110:
    for (i = 0; i < 3; i++)
    {
      tenm_table_add(laser_angle_new(my->x, my->y, 3.5,
                                     10 + 16 * i, 25.0, 3));
      tenm_table_add(laser_angle_new(my->x, my->y, 3.5,
                                     170 - 16 * i, 25.0, 3));
    }
    break;
  case 140:
    for (i = 0; i < 3; i++)
    {
      tenm_table_add(laser_angle_new(my->x, my->y, 3.5,
                                     42 + 16 * i, 25.0, 3));
      tenm_table_add(laser_angle_new(my->x, my->y, 3.5,
                                     138 - 16 * i, 25.0, 3));
    }
    break;
  case 170:
    for (i = 0; i < 6; i++)
      tenm_table_add(laser_angle_new(my->x, my->y, 3.5,
                                     50 + 16 * i, 25.0, 3));
    break;
  default:
    break;
  }


  return 0;
}

static int
afterdeath_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  char temp[32];
  double x;
  int c;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "afterdeath_draw: my is NULL\n");
    return 0;
  }

  /* decoration */
  /* ship */
  if ((priority == 0) && (my->count[2] == 1) && (my->count[3] < 1800)
      && (my->count[3] % 200 >= 50) && (my->count[3] % 200 < 112))
  {
    if (my->count[3] % 200 <= 80)
    {  
      color = tenm_map_color(158, 158, 158);
    }
    else
    {
      c = my->count[3] % 200 - 80;
      if (c < 0)
        c = 0;
      if (c > 32)
        c = 32;
      color = tenm_map_color((158 * (32-c) + DEFAULT_BACKGROUND_RED*c) / 32,
                             (158 * (32-c) + DEFAULT_BACKGROUND_GREEN*c) / 32,
                             (158 * (32-c) + DEFAULT_BACKGROUND_BLUE*c) / 32);
    }

    x = -35.0;
    if (my->count[3] % 200 > 80)
      x += 6.0 * ((double) ((my->count[3] % 200) - 80));
    if (my->count[4] % 2 != 0)
      x = ((double) (WINDOW_WIDTH)) - x;

    if (tenm_draw_line((int) (my->x), (int) (my->y - 48.0 - 25.0),
                       (int) (x), (int) (my->count_d[2]),
                       1, color) != 0)
      status = 1;
  }

  /* body */
  /* dead enemy has low priority */
  if (((my->count[2] <= 1) && (priority == 0))
      || ((my->count[2] > 1) && (priority == -1)))
  {
    if (afterdeath_green(my))
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

    if (my->count[2] <= 1)
    {  
      /* antenna */
      if (tenm_draw_line((int) (my->x), (int) (my->y - 48.0),
                         (int) (my->x + 25.0 * tenm_cos(-60)),
                         (int) (my->y - 48.0 + 25.0 * tenm_sin(-60)),
                         1, color) != 0)
        status = 1;
      if (tenm_draw_line((int) (my->x), (int) (my->y - 48.0),
                         (int) (my->x + 25.0 * tenm_cos(-120)),
                         (int) (my->y - 48.0 + 25.0 * tenm_sin(-120)),
                         1, color) != 0)
        status = 1;
      if (tenm_draw_line((int) (my->x), (int) (my->y - 48.0),
                         (int) (my->x),
                         (int) (my->y - 48.0 - 25.0),
                         1, color) != 0)
        status = 1;

      /* hand */
      c = 110;
      if ((my->count[2] == 1) && (my->count[3] < 1800)
          && (my->count[4] % 2 == 0))
      {
        if ((my->count[3] % 200 >= 70) && (my->count[3] % 200 < 75))
          c = 110 + (my->count[3] % 200 - 70) * 18;
        else if ((my->count[3] % 200 >= 75) && (my->count[3] % 200 < 80))
          c = 200;
        else if ((my->count[3] % 200 >= 80) && (my->count[3] % 200 < 85))
          c = 200 + (my->count[3] % 200 - 80) * 28;
        else if ((my->count[3] % 200 >= 85) && (my->count[3] % 200 < 120))
          c = 340;
        else if ((my->count[3] % 200 >= 120) && (my->count[3] % 200 < 143))
          c = 340 - (my->count[3] % 200 - 120) * 10;
        else
          c = 110;
      }
      if (tenm_draw_line((int) (my->x - 36.0), (int) (my->y),
                         (int) (my->x - 36.0 + 50.0 * tenm_cos(c)),
                         (int) (my->y + 50.0 * tenm_sin(c)),
                         1, color) != 0)
        status = 1;

      c = 70;
      if ((my->count[2] == 1) && (my->count[3] < 1800)
          && (my->count[4] % 2 != 0))
      {
        if ((my->count[3] % 200 >= 70) && (my->count[3] % 200 < 75))
          c = 70 - (my->count[3] % 200 - 70) * 18;
        else if ((my->count[3] % 200 >= 75) && (my->count[3] % 200 < 80))
          c = -20;
        else if ((my->count[3] % 200 >= 80) && (my->count[3] % 200 < 85))
          c = -20 - (my->count[3] % 200 - 80) * 28;
        else if ((my->count[3] % 200 >= 85) && (my->count[3] % 200 < 120))
          c = -160;
        else if ((my->count[3] % 200 >= 120) && (my->count[3] % 200 < 143))
          c = -160 + (my->count[3] % 200 - 120) * 10;
        else
          c = 70;
      }
      if (tenm_draw_line((int) (my->x + 36.0), (int) (my->y),
                         (int) (my->x + 36.0 + 50.0 * tenm_cos(c)),
                         (int) (my->y + 50.0 * tenm_sin(c)),
                         1, color) != 0)
        status = 1;
    }

    /* core */
    if (tenm_draw_line((int) (my->x + 36.0), (int) (my->y - 48.0),
                       (int) (my->x + 36.0), (int) (my->y + 48.0),
                       3, color) != 0)
      status = 1;
    if (tenm_draw_line((int) (my->x + 36.0), (int) (my->y + 48.0),
                       (int) (my->x - 36.0), (int) (my->y + 48.0),
                       3, color) != 0)
      status = 1;
    if (tenm_draw_line((int) (my->x - 36.0), (int) (my->y + 48.0),
                       (int) (my->x - 36.0), (int) (my->y - 48.0),
                       3, color) != 0)
      status = 1;
    if (tenm_draw_line((int) (my->x - 36.0), (int) (my->y - 48.0),
                       (int) (my->x + 36.0), (int) (my->y - 48.0),
                       3, color) != 0)
      status = 1;
  }

  /* hit point stat */
  if ((priority == 0) && (my->count[2] == 1))
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 10, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "afterdeath_draw: draw_string failed\n");
      status = 1;
    }
  }
  
  return status;
}

/* return 1 (true) or 0 (false) */
static int
afterdeath_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if ((my->count[2] == 1)
      && (my->count[3] >= 200) && (my->count[3] < 1900))
    return 1;
  if ((my->count[2] == 2) && (my->count[5] != 0))
    return 1;

  return 0;
}

static tenm_object *
afterdeath_ship_new(double x, double y, int what)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;

  /* sanity check */
  if ((what < 0) || (what > 3))
  {
    fprintf(stderr, "afterdeath_ship_new: strange what (%d)\n", what);
    return NULL;
  }

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "afterdeath_ship_new: malloc(p) failed\n");
    return NULL;
  }

  if (what % 2 == 0)
    p[0] = (tenm_primitive *) tenm_polygon_new(3,
                                               x + 36.0, y,
                                               x - 18.0, y + 31.1769,
                                               x - 18.0, y - 31.1769);
  else
    p[0] = (tenm_primitive *) tenm_polygon_new(3,
                                               x - 36.0, y,
                                               x + 18.0, y - 31.1769,
                                               x + 18.0, y + 31.1769);
  if (p[0] == NULL)
  {
    fprintf(stderr, "afterdeath_ship_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 2);
  if (count == NULL)
  {
    fprintf(stderr, "afterdeath_ship_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "afterdeath_ship_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] what
   * [1] shoot timer
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */

  count[0] = what;
  count[1] = 0;

  if (what % 2 == 0)
    count_d[0] = 6.0;
  else
    count_d[0] = -6.0;
  count_d[1] = 0.0;

  new = tenm_object_new("Afterdeath ship", ATTR_ENEMY, 0,
                        1, x, y,
                        2, count, 2, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&afterdeath_ship_move),
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&afterdeath_ship_act),
                        (int (*)(tenm_object *, int))
                        (&afterdeath_ship_draw));

  if (new == NULL)
  {
    fprintf(stderr, "afterdeath_ship_new: tenm_object_new failed\n");
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
afterdeath_ship_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "afterdeath_ship_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "afterdeath_ship_move: strange turn_per_frame (%f)\n",
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
afterdeath_ship_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int theta;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "afterdeath_ship_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[1])++;
  if (my->count[1] % 25 == 0)
  {
    if (my->count[0] < 2)
    {
      tenm_table_add(normal_shot_point_new(my->x, my->y + 15.5885, 4.0,
                                           player->x, player->y, 1));
      tenm_table_add(normal_shot_point_new(my->x, my->y - 15.5885, 4.0,
                                           player->x, player->y, 1));
    }
    else
    {
      for (i = 0 ; i < 6; i++)
      {
        if (my->count[0] % 2 == 0)
          theta = 105 + 30 * i;
        else
          theta = 75 - 30 * i;
        tenm_table_add(normal_shot_angle_new(my->x, my->y, 4.0,
                                             theta, 0));
      }
    }
  }
  

  return 0;
}

static int
afterdeath_ship_draw(tenm_object *my, int priority)
{
  tenm_color color;
  int status = 0;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "afterdeath_ship_draw: my is NULL\n");
    return 0;
  }

  if (priority != 0)
    return 0;

  color = tenm_map_color(95, 13, 68);

  if (my->count[0] % 2 == 0)
  {
    if (tenm_draw_line((int) (my->x + 36.0), (int) (my->y),
                       (int) (my->x - 18.0), (int) (my->y + 31.1769),
                       3, color) != 0)
      status = 1;
    if (tenm_draw_line((int) (my->x - 18.0), (int) (my->y + 31.1769),
                       (int) (my->x - 18.0), (int) (my->y - 31.1769),
                       3, color) != 0)
      status = 1;
    if (tenm_draw_line((int) (my->x - 18.0), (int) (my->y - 31.1769),
                       (int) (my->x + 36.0), (int) (my->y),
                       3, color) != 0)
      status = 1;
  }
  else
  {
    if (tenm_draw_line((int) (my->x - 36.0), (int) (my->y),
                       (int) (my->x + 18.0), (int) (my->y - 31.1769),
                       3, color) != 0)
      status = 1;
    if (tenm_draw_line((int) (my->x + 18.0), (int) (my->y - 31.1769),
                       (int) (my->x + 18.0), (int) (my->y + 31.1769),
                       3, color) != 0)
      status = 1;
    if (tenm_draw_line((int) (my->x + 18.0), (int) (my->y + 31.1769),
                       (int) (my->x - 36.0), (int) (my->y),
                       3, color) != 0)
      status = 1;
  }

  return status;
}
