/* $Id: tutor.c,v 1.141 2005/07/10 16:46:56 oohara Exp $ */

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

#include "tutor.h"

#define NEAR_ZERO 0.0001

static int tutor_move(tenm_object *my, double turn_per_frame);
static int tutor_hit(tenm_object *my, tenm_object *your);
static void tutor_increase_level(tenm_object *my);
static int tutor_signal(tenm_object *my, int n);
static int tutor_act(tenm_object *my, const tenm_object *player);
static int tutor_draw(tenm_object *my, int priority);
static int tutor_green(const tenm_object *my);

tenm_object *
tutor_new(int what, double x, double y, int parent_index, int rank)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  const char *name = NULL;
  int hit_point = 1;
  int t_shoot = 9999;

  /* sanity check */
  if ((what < 0) || (what > 3))
  {
    fprintf(stderr, "tutor_new: strange what (%d)\n", what);
    return NULL;
  }
  if (parent_index < 0)
  {
    fprintf(stderr, "tutor_new: strange parent_index (%d)\n", parent_index);
    return NULL;
  }
  if (rank < 0)
  {
    fprintf(stderr, "tutor_new: rank is non-positive (%d)\n", rank);
    return NULL;
  }

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "tutor_new: malloc(p) failed\n");
    return NULL;
  }

  switch(what)
  {
  case 0:
    p[0] = (tenm_primitive *) tenm_circle_new(x, y, 20.0);
    if (p[0] == NULL)
    {
      fprintf(stderr, "tutor_new: cannot set p[0] (0)\n");
      free(p);
      return NULL;
    }
    break;
  case 1:
    p[0] = (tenm_primitive *) tenm_circle_new(x, y, 25.0);
    if (p[0] == NULL)
    {
      fprintf(stderr, "tutor_new: cannot set p[0] (1)\n");
      free(p);
      return NULL;
    }
    break;
  case 2:
    p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                               x + 48.0, y - 36.0,
                                               x + 48.0, y + 36.0,
                                               x - 48.0, y + 36.0,
                                               x - 48.0, y - 36.0);
    if (p[0] == NULL)
    {
      fprintf(stderr, "tutor_new: cannot set p[0] (1)\n");
      free(p);
      return NULL;
    }
    break;
  case 3:
    p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                               x + 36.0, y - 48.0,
                                               x + 36.0, y + 48.0,
                                               x - 36.0, y + 48.0,
                                               x - 36.0, y - 48.0);
    if (p[0] == NULL)
    {
      fprintf(stderr, "tutor_new: cannot set p[0] (1)\n");
      free(p);
      return NULL;
    }
    break;
  default:
    fprintf(stderr, "tutor_new: undefined what when setting p[0] (%d)\n",
            what);
    free(p);
    return NULL;
    break;
  }

  count = (int *) malloc(sizeof(int) * 8);
  if (count == NULL)
  {
    fprintf(stderr, "tutor_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "tutor_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  switch(what)
  {
  case 0:
    name = "tutor soldier";
    hit_point = 1;
    if (rank < 300)
      t_shoot = 31;
    else if (rank < 400)
      t_shoot = 19;
    else
      t_shoot = 11;
    break;
  case 1:
    name = "tutor sniper";
    hit_point = 20;
    t_shoot = 30;
    break;
  case 2:
    name = "tutor divider";
    hit_point = 85;
    if (rank < 300)
      t_shoot = 37;
    else if (rank < 400)
      t_shoot = 27;
    else
      t_shoot = 17;
    break;
  case 3:
    name = "tutor conqueror";
    hit_point = 85;
    if (rank < 300)
      t_shoot = 31;
    else
      t_shoot = 17;
    break;
  default:
    fprintf(stderr, "tutor_new: undefined what when setting name (%d)\n",
            what);
    if (count_d != NULL)
      free(count_d);
    if (count != NULL)
      free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
    break;
  }

  /* list of count
   * [0] for deal_damage
   * [1] "damaged" timer
   * [2] what
   * [3] parent index
   * [4] rank
   * [5] t_shoot
   * [6] life timer
   * [7] shoot timer
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */
  count[0] = 0;
  count[1] = 0;
  count[2] = what;
  count[3] = parent_index;
  count[4] = rank;
  count[5] = t_shoot;
  count[6] = 0;
  if ((t_shoot > 5) && (t_shoot < 1000))
    count[7] = rand() % ((t_shoot + 1) / 2);
  else
    count[7] = 0;


  count_d[0] = 0.0;
  if (what == 0)
    count_d[1] = 4.0;
  else if (what == 1)
    count_d[1] = 5.0;
  else
    count_d[1] = 5.0;

  new = tenm_object_new(name, ATTR_ENEMY, ATTR_PLAYER_SHOT,
                        hit_point, x, y,
                        8, count, 2, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&tutor_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&tutor_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&tutor_act),
                        (int (*)(tenm_object *, int))
                        (&tutor_draw));
  if (new == NULL)
  {
    fprintf(stderr, "tutor_new: tenm_object_new failed\n");
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
tutor_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "tutor_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "tutor_move: strange turn_per_frame (%f)\n",
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
  {
    /* escaped */
    tutor_increase_level(my);
    return 1;
  }

  return 0;
}

static void
tutor_increase_level(tenm_object *my)
{
  int level;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "tutor_increase_level: my is NULL\n");
    return;
  }

  switch (my->count[2])
  {
  case 0:
    level = 1;
    break;
  case 1:
    level = 1;
    break;
  case 2:
    level = 3;
    break;
  case 3:
      level = 3;
      break;
  default:
    fprintf(stderr, "tutor_increase_level: undefined what (%d)\n",
            my->count[2]);
    level = 0;
    break;
  }
  tenm_table_apply(my->count[3],
                   (int (*)(tenm_object *, int))
                   (&tutor_signal),
                   level);
}

static int
tutor_hit(tenm_object *my, tenm_object *your)
{
  int n;
  int score;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "tutor_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "tutor_hit: your is NULL\n");
    return 0;
  }

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;

  deal_damage(my, your, 0);
  if (tutor_green(my))
    add_chain(my, your);
  my->count[1] = 41;

  if (my->hit_point <= 0)
  {
    tutor_increase_level(my);

    switch (my->count[2])
    {
    case 0:
      score = 2;
      break;
    case 1:
      score = 3;
      break;
    case 2:
      score = 5;
      break;
    case 3:
      score = 7;
      break;
    default:
      fprintf(stderr, "tutor_hit: undefined what when setting score (%d)\n",
              my->count[2]);
      score = 0;
      break;
    }
    add_score(score);

    if (tutor_green(my))
      n = 8;
    else
      n = 7;

    switch (my->count[2])
    {
    case 0:
      tenm_table_add(explosion_new(my->x, my->y,
                                   my->count_d[0] / 2.0, my->count_d[1]/ 2.0,
                                   2, 20, n, 3.0, 8));
      break;
    case 1:
      tenm_table_add(explosion_new(my->x, my->y,
                                   my->count_d[0] / 2.0, my->count_d[1]/ 2.0,
                                   2, 30, n, 4.0, 8));
      break;
    case 2:
    case 3:
      tenm_table_add(explosion_new(my->x, my->y,
                                   my->count_d[0] / 2.0, my->count_d[1]/ 2.0,
                                   1, 200, n, 5.0, 8));
      tenm_table_add(fragment_new(my->x, my->y,
                                  my->count_d[0] / 2.0, my->count_d[1] / 2.0,
                                  20.0, 20, n, 4.0, 0.0, 8));
      break;
    default:
      fprintf(stderr, "tutor_hit: undefined what when exploding (%d)\n",
              my->count[2]);
      break;
    }
    
    return 1;
  }

  return 0;
}

static int
tutor_signal(tenm_object *my, int n)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (strcmp(my->name, "plan 0 more 1") != 0)
    return 0;

  if (n <= 0)
    return 0;

  if (my->count[5] % 100 >= (my->count[5] + n ) % 100)
    my->count[4] += 80;

  my->count[5] += n;

  return 0;
}

static int
tutor_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int theta;
  double dx;
  double dy;
  double length;
  double length2;
  double speed;
  double result[2];
  double v[2];

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "tutor_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  /* for deal_damage */
  my->count[0] = 0;

  /* "damaged" count down */
  if (my->count[1] > 0)
    (my->count[1])--;

  (my->count[6])++;
  (my->count[7])++;

  /* speed change */
  switch (my->count[2])
  {
  case 0:
    if (my->x < 50.0)
    {
      my->count_d[0] = 50.0 - my->x;
    }
    else if (my->x > ((double) WINDOW_WIDTH) - 50.0)
    {
      my->count_d[0] = (((double) WINDOW_WIDTH) - 50.0) - my->x;
    }
    else if (my->count[6] < 50)
    {
      if ((my->x > player->x - 2.0) && (my->x < player->x + 2.0))
      {
        my->count_d[0] = player->x - my->x;
      }
      else if (my->x < player->x)
      {
        my->count_d[0] += 0.1;
      }
      else
      {
        my->count_d[0] += -0.1;
      }
    }

    if (my->count_d[0] < -2.0)
      my->count_d[0] = -2.0;
    if (my->count_d[0] > 2.0)
      my->count_d[0] = 2.0;
    if (my->count[6] >= 50)
    {
      if ((my->count_d[0] > -0.1) && (my->count_d[0] < 0.1))
        my->count_d[0] = 0.0;
      else if (my->count_d[0] > 0.0)
        my->count_d[0] += -0.1;
      else
        my->count_d[0] += 0.1;
    }
    
    break;
  case 1:
    break;
  case 2:
  case 3:
    if (my->count[6] == 20)
    {
      my->count_d[1] = 2.0;
    }
    break;
  default:
    fprintf(stderr, "tutor_act: undefined what when changing speed (%d)\n",
            my->count[2]);
    break;
  }

  /* shoot */
  switch (my->count[2])
  {
  case 0:
    if (my->count[7] < my->count[5])
      break;
    if (my->y > 300.0)
      break;

    if (my->count[4] < 100)
      speed = 4.0 + ((double) (my->count[4])) * 0.04;
    else if (my->count[4] < 200)
      speed = 6.0 + ((double) (my->count[4] - 100)) * 0.06;
    else
      speed = 16.0;

    if (my->y > 150.0)
      speed *= 0.75;

    dx = player->x - my->x;
    dy = player->y - my->y;
    length2 = dx * dx + dy * dy;

    if ((length2 < 300.0 * 300.0)
        || (my->count[4] < 100))
    {
      tenm_table_add(normal_shot_point_new(my->x, my->y, speed,
                                           player->x, player->y, 3));
    }
    else
    {
      if (length2 < 400.0 * 400.0)
        theta = 6;
      else
        theta = 15;
      for (i = -1; i <= 1; i += 2)
      {
        v[0] = dx;
        v[1] = dy;
        result[0] = v[0];
        result[1] = v[1];
        vector_rotate(result, v, theta * i);
        tenm_table_add(normal_shot_point_new(my->x, my->y, speed,
                                             my->x + result[0],
                                             my->y + result[1],
                                             3));
      }
    }

    my->count[7] = 0;
    break;
  case 1:
    if (my->count[7] < my->count[5])
      break;

    if (my->count[4] < 100)
      speed = 6.0 + ((double) (my->count[4])) * 0.06;
    else if (my->count[4] < 200)
      speed = 8.0 + ((double) (my->count[4] - 100)) * 0.08;
    else
      speed = 24.0;

    if ((my->count[7] == my->count[5] + 0)
        || ((my->count[7] == my->count[5] + 10) && (my->count[4] >= 100)))
    {
      tenm_table_add(laser_point_new(my->x, my->y, speed,
                                     player->x, player->y,
                                     30.0, 4));
    }
    else if ((my->count[7] == my->count[5] + 5)
             && (my->count[4] >= 400))
    {
      tenm_table_add(laser_point_new(my->x, my->y, speed * 0.75,
                                     player->x + player->count_d[0] * 15.0,
                                     player->y + player->count_d[1] * 15.0,
                                     30.0, 5));
    }
    else if ((my->count[7] == my->count[5] + 15)
             && (my->count[4] >= 400))
    {
      tenm_table_add(laser_point_new(my->x, my->y, speed * 0.75,
                                     player->x + player->count_d[0] * 10.0,
                                     player->y + player->count_d[1] * 10.0,
                                     30.0, 5));
    }
    /* don't reset my->count[7] --- shoot only once */
    break;
  case 2:
    if (my->count[6] < 20)
    {
      (my->count[7])--;
      break;
    }
    if (my->count[7] < my->count[5])
      break;

    if (my->count[4] < 100)
      speed = 5.0 + ((double) (my->count[4])) * 0.05;
    else if (my->count[4] < 200)
      speed = 7.5 + ((double) (my->count[4] - 100)) * 0.075;
    else
      speed = 20.0;

    if (((my->count[7] == my->count[5] + 0)
         || (my->count[7] == my->count[5] + 10))
        && (my->y < 400.0))
    {
      if ((player->y > my->y) && (my->y < 300.0))
      {  
        if (player->x < my->x)
        {
          if (my->count[4] >= 100)
            tenm_table_add(laser_angle_new(my->x - 40.0, my->y - 28.0,
                                           speed * 0.75,
                                           80, 25.0, 1));
          tenm_table_add(laser_angle_new(my->x - 40.0, my->y - 28.0,
                                         speed,
                                         70, 25.0, 1));
          if (my->count[4] >= 100)
            tenm_table_add(laser_angle_new(my->x + 40.0, my->y - 28.0,
                                           speed * 0.75,
                                           140, 25.0, 1));
          tenm_table_add(laser_angle_new(my->x + 40.0, my->y - 28.0,
                                         speed,
                                         150, 25.0, 1));
        }
        else
        {
          if (my->count[4] >= 100)
            tenm_table_add(laser_angle_new(my->x - 40.0, my->y - 28.0,
                                           speed * 0.75,
                                           40, 25.0, 1));
          tenm_table_add(laser_angle_new(my->x - 40.0, my->y - 28.0,
                                         speed,
                                         30, 25.0, 1));
          if (my->count[4] >= 100)
            tenm_table_add(laser_angle_new(my->x + 40.0, my->y - 28.0,
                                           speed * 0.75,
                                           100, 25.0, 1));
          tenm_table_add(laser_angle_new(my->x + 40.0, my->y - 28.0,
                                         speed,
                                         110, 25.0, 1));
        }
      }
      else
      {  
        if (player->x < my->x)
        {
          if (my->count[4] >= 100)
            tenm_table_add(laser_angle_new(my->x - 40.0, my->y - 28.0,
                                           speed * 0.75,
                                           -80, 25.0, 1));
          tenm_table_add(laser_angle_new(my->x - 40.0, my->y - 28.0,
                                         speed,
                                         -70, 25.0, 1));
          if (my->count[4] >= 100)
            tenm_table_add(laser_angle_new(my->x + 40.0, my->y - 28.0,
                                           speed * 0.75,
                                           230, 25.0, 1));
          tenm_table_add(laser_angle_new(my->x + 40.0, my->y - 28.0,
                                         speed,
                                         220, 25.0, 1));
        }
        else
        {
          if (my->count[4] >= 100)
            tenm_table_add(laser_angle_new(my->x - 40.0, my->y - 28.0,
                                           speed * 0.75,
                                           -50, 25.0, 1));
          tenm_table_add(laser_angle_new(my->x - 40.0, my->y - 28.0,
                                         speed,
                                         -40, 25.0, 1));
          if (my->count[4] >= 100)
            tenm_table_add(laser_angle_new(my->x + 40.0, my->y - 28.0,
                                           speed * 0.75,
                                           -100, 25.0, 1));
          tenm_table_add(laser_angle_new(my->x + 40.0, my->y - 28.0,
                                         speed,
                                         -110, 25.0, 1));
        }
      }
    }
    else if (my->count[7] == my->count[5] + 25)
    {
      if (my->y > 200.0)
        speed *= 0.75;
      if (my->y < 300.0)
        tenm_table_add(normal_shot_point_new(my->x, my->y, speed,
                                             player->x, player->y,
                                             4));

    }

    if (my->count[7] >= my->count[5] + 25)
      my->count[7] = 0;
    break;
  case 3:
    if (my->count[6] < 20)
    {
      (my->count[7])--;
      break;
    }
    if (my->count[7] < my->count[5])
      break;

    if (my->count[4] < 100)
      speed = 4.0 + ((double) (my->count[4])) * 0.04;
    else if (my->count[4] < 200)
      speed = 6.0 + ((double) (my->count[4] - 100)) * 0.06;
    else
      speed = 16.0;

    if ((my->count[7] == my->count[5] + 0)
        || (my->count[7] == my->count[5] + 10))
    {
      if (my->y < 300.0)
      {
        dx = player->x - my->x;
        dy = player->y - my->y;
        length2 = dx * dx + dy * dy;
        length = tenm_sqrt((int) (length2));
        if (length < NEAR_ZERO)
          length = 1.0;
        v[0] = 30.0 * dx / length;
        v[1] = 30.0 * dy / length;
        if (length2 < 350.0 * 350.0)
        {
          tenm_table_add(laser_point_new(my->x - v[1], my->y + v[0], speed,
                                         player->x,
                                         player->y,
                                         25.0, 0));
          tenm_table_add(laser_point_new(my->x + v[1], my->y - v[0], speed,
                                         player->x,
                                         player->y,
                                         25.0, 0));
        }
        else
        {
          tenm_table_add(laser_point_new(my->x - v[1], my->y + v[0], speed,
                                         my->x - v[1] + v[0] - v[1] * 0.1,
                                         my->y + v[0] + v[1] + v[0] * 0.1,
                                         25.0, 0));
          tenm_table_add(laser_point_new(my->x + v[1], my->y - v[0], speed,
                                         my->x + v[1] + v[0] + v[1] * 0.1,
                                         my->y - v[0] + v[1] - v[0] * 0.1,
                                         25.0, 0));
        }
        if (my->count[4] >= 100)
        {
          tenm_table_add(laser_point_new(my->x - v[1], my->y + v[0],
                                         speed * 0.75,
                                         my->x - v[1] + v[0] - v[1] * 0.3,
                                         my->y + v[0] + v[1] + v[0] * 0.3,
                                         25.0, 0));
          tenm_table_add(laser_point_new(my->x + v[1], my->y - v[0],
                                         speed * 0.75,
                                         my->x + v[1] + v[0] + v[1] * 0.3,
                                         my->y - v[0] + v[1] - v[0] * 0.3,
                                         25.0, 0));
        }
      }
    }
    else if (((my->count[7] == my->count[5] + 15)
             || (my->count[7] == my->count[5] + 20)
             || (my->count[7] == my->count[5] + 25))
             && (my->count[4] >= 400))
    {
      if (my->y > 200.0)
        speed *= 0.75;
      if (my->count[7] == my->count[5] + 15)
        speed *= 0.6;
      else if (my->count[7] == my->count[5] + 20)
        speed *= 0.8;
      if (my->y < 300.0)
        tenm_table_add(normal_shot_point_new(my->x, my->y, speed,
                                             player->x, player->y,
                                             5));
    }
    if (my->count[7] >= my->count[5] + 25)
      my->count[7] = 0;
    break;
  default:
    fprintf(stderr, "tutor_act: undefined what when shooting (%d)\n",
            my->count[2]);
    break;
  }

  return 0;
}

static int
tutor_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  char temp[32];

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "tutor_draw: my is NULL\n");
    return 0;
  }

  if (priority != 0)
    return 0;

  /* decoration */
  if (tutor_green(my))
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

  switch (my->count[2])
  {
  case 0:
    if (tenm_draw_line(((int) (my->x + 20.0)),
                       ((int) (my->y)),
                       ((int) (my->x - 20.0)),
                       ((int) (my->y)),
                       1, color) != 0)
      status = 1;
    break;
  case 1:
    if (tenm_draw_line(((int) (my->x + 25.0)),
                       ((int) (my->y)),
                       ((int) (my->x - 25.0)),
                       ((int) (my->y)),
                       1, color) != 0)
      status = 1;
    break;
  case 2:
    break;
  case 3:
    if (tenm_draw_line(((int) (my->x - 24.0)),
                       ((int) (my->y - 48.0)),
                       ((int) (my->x - 12.0)),
                       ((int) (my->y + 48.0)),
                       1, color) != 0)
      status = 1;
    if (tenm_draw_line(((int) (my->x + 24.0)),
                       ((int) (my->y - 48.0)),
                       ((int) (my->x + 12.0)),
                       ((int) (my->y + 48.0)),
                       1, color) != 0)
      status = 1;
    break;
  default:
    fprintf(stderr, "tutor_draw: undefined what when drawing decoration "
            "(%d)\n", my->count[2]);
    break;
  }
  
  /* body */
  if (tutor_green(my))
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

  switch (my->count[2])
  {
  case 0:  
    if (tenm_draw_circle((int) (my->x), (int) (my->y), 20, 3, color) != 0)
      status = 1;
    break;
  case 1:  
    if (tenm_draw_circle((int) (my->x), (int) (my->y), 25, 3, color) != 0)
      status = 1;
    break;
  case 2:
    if (tenm_draw_line(((int) (my->x - 40.0)),
                       ((int) (my->y - 28.0)),
                       ((int) (my->x + 40.0)),
                       ((int) (my->y - 28.0)),
                       1, color) != 0)
      status = 1;
    if (tenm_draw_line(((int) (my->x - 40.0)),
                       ((int) (my->y - 28.0)),
                       ((int) (my->x - 40.0)),
                       ((int) (my->y + 36.0)),
                       1, color) != 0)
      status = 1;
    if (tenm_draw_line(((int) (my->x + 40.0)),
                       ((int) (my->y - 28.0)),
                       ((int) (my->x + 40.0)),
                       ((int) (my->y + 36.0)),
                       1, color) != 0)
      status = 1;

    if (tenm_draw_line(((int) (my->x + 48.0)),
                       ((int) (my->y - 36.0)),
                       ((int) (my->x + 48.0)),
                       ((int) (my->y + 36.0)),
                       3, color) != 0)
      status = 1;
    if (tenm_draw_line(((int) (my->x + 48.0)),
                       ((int) (my->y + 36.0)),
                       ((int) (my->x - 48.0)),
                       ((int) (my->y + 36.0)),
                       3, color) != 0)
      status = 1;
    if (tenm_draw_line(((int) (my->x - 48.0)),
                       ((int) (my->y + 36.0)),
                       ((int) (my->x - 48.0)),
                       ((int) (my->y - 36.0)),
                       3, color) != 0)
      status = 1;
    if (tenm_draw_line(((int) (my->x - 48.0)),
                       ((int) (my->y - 36.0)),
                       ((int) (my->x + 48.0)),
                       ((int) (my->y - 36.0)),
                       3, color) != 0)
      status = 1;
    break;
  case 3:
    if (tenm_draw_line(((int) (my->x + 36.0)),
                       ((int) (my->y - 48.0)),
                       ((int) (my->x + 36.0)),
                       ((int) (my->y + 48.0)),
                       3, color) != 0)
      status = 1;
    if (tenm_draw_line(((int) (my->x + 36.0)),
                       ((int) (my->y + 48.0)),
                       ((int) (my->x - 36.0)),
                       ((int) (my->y + 48.0)),
                       3, color) != 0)
      status = 1;
    if (tenm_draw_line(((int) (my->x - 36.0)),
                       ((int) (my->y + 48.0)),
                       ((int) (my->x - 36.0)),
                       ((int) (my->y - 48.0)),
                       3, color) != 0)
      status = 1;
    if (tenm_draw_line(((int) (my->x - 36.0)),
                       ((int) (my->y - 48.0)),
                       ((int) (my->x + 36.0)),
                       ((int) (my->y - 48.0)),
                       3, color) != 0)
      status = 1;
    break;
  default:
    fprintf(stderr, "tutor_draw: undefined what when drawing body (%d)\n",
            my->count[2]);
    break;
  }
  
  /* hit point stat */
  if (my->count[1] > 0)
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string((int) (my->x - 5.0), (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "tutor_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

/* return 1 (true) or 0 (false) */
static int
tutor_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  switch (my->count[2])
  {
  case 0:
    if (my->count[6] <= 75)
      return 1;
    break;
  case 1:
    if (my->count[6] <= 60)
      return 1;
    break;
  case 2:
    if (my->count[6] <= 145)
      return 1;
    break;
  case 3:
    if (my->count[6] <= 145)
      return 1;
    break;
  default:
    fprintf(stderr, "tutor_green: undefined what (%d)\n", my->count[2]);
    break;
  }

  return 0;
}
