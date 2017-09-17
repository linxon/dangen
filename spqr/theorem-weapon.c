/* $Id: theorem-weapon.c,v 1.224 2011/08/23 20:51:03 oohara Exp $ */
/* [normal] Theorem Weapon */

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
/* tenm_same_side, tenm_line_nearer */
#include "tenm_collision.h"

#include "theorem-weapon.h"

#define NEAR_ZERO 0.0001

static int theorem_weapon_move(tenm_object *my, double turn_per_frame);
static int theorem_weapon_hit(tenm_object *my, tenm_object *your);
static void theorem_weapon_explode(tenm_object *my);
static int theorem_weapon_act(tenm_object *my, const tenm_object *player);
static void theorem_weapon_theorem(tenm_object *my, const tenm_object *player,
                                   int what, int t);
static int theorem_weapon_draw(tenm_object *my, int priority);
static int theorem_weapon_green(const tenm_object *my);

static tenm_object * theorem_weapon_shot_absolute_new(double x, double y,
                                                      double player_x,
                                                      double player_y,
                                                      double speed,
                                                      int theta, int what,
                                                      int color);
static tenm_object * theorem_weapon_shot_aim_new(double x, double y,
                                                 double player_x,
                                                 double player_y,
                                                 double speed,
                                                 int theta, int what,
                                                 int color);
static tenm_object * theorem_weapon_shot_new(double x, double y,
                                             double player_x, double player_y,
                                             double speed_x, double speed_y,
                                             int what, int color);
static int theorem_weapon_shot_move(tenm_object *my, double turn_per_frame);
static int theorem_weapon_shot_hit(tenm_object *my, tenm_object *your);
static int theorem_weapon_shot_act(tenm_object *my, const tenm_object *player);
static int theorem_weapon_shot_seek(double x, double y,
                                    double player_x, double player_y,
                                    double speed_x, double speed_y,
                                    int what);
static int theorem_weapon_shot_draw(tenm_object *my, int priority);

tenm_object *
theorem_weapon_new(void)
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
    fprintf(stderr, "theorem_weapon_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_circle_new(x, y, 40.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "theorem_weapon_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 7);
  if (count == NULL)
  {
    fprintf(stderr, "theorem_weapon_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "theorem_weapon_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] for deal_damage
   * [1] "damaged" timer
   * [2] move mode
   * [3] move timer
   * [4] decoration timer
   * [5] shoot direction
   * [6] "was green when killed" flag
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */

  count[0] = 0;
  count[1] = 0;
  count[2] = 0;
  count[3] = 0;
  count[4] = 90;
  count[5] = 0;
  count[6] = 0;

  count_d[0] = 0.0;
  count_d[1] = (((double) (WINDOW_HEIGHT / 6)) - y) / 120.0;

  new = tenm_object_new("Theorem Weapon", ATTR_BOSS, ATTR_PLAYER_SHOT,
                        1000, x, y,
                        7, count, 2, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&theorem_weapon_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&theorem_weapon_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&theorem_weapon_act),
                        (int (*)(tenm_object *, int))
                        (&theorem_weapon_draw));
  if (new == NULL)
  {
    fprintf(stderr, "theorem_weapon_new: tenm_object_new failed\n");
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
theorem_weapon_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "theorem_weapon_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "theorem_weapon_move: strange turn_per_frame (%f)\n",
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
theorem_weapon_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "theorem_weapon_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "theorem_weapon_hit: your is NULL\n");
    return 0;
  }

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;
  if (my->count[2] != 1)
    return 0;

  deal_damage(my, your, 0);
  if (theorem_weapon_green(my))
    add_chain(my, your);
  my->count[1] = 2;

  if (my->hit_point <= 0)
  {
    add_score(30000);
    set_background(1);
    theorem_weapon_explode(my);
    return 0;
  }

  return 0;
}

static void
theorem_weapon_explode(tenm_object *my)
{
  int n;
  double speed_theta;

  /* sanity check */
  if (my == NULL)
    return;

  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy_shot), 0);
  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy), 0);

  /* set "was green" flag before we change the life mode */
  if (theorem_weapon_green(my))
  {
    n = 8;
    my->count[6] = 1;
    speed_theta = -30.0;
  }
  else
  {
    n = 7;
    my->count[6] = 0;
    speed_theta = 15.0;
  }

  tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                               1, 5000, n, 10.0, 6));
  tenm_table_add(fragment_new(my->x - 80.0, my->y, -2.0, 0.0,
                              50.0, 12, n, 4.0, speed_theta, 20));
  tenm_table_add(fragment_new(my->x + 80.0, my->y, 2.0, 0.0,
                              50.0, 12, n, 4.0, speed_theta, 20));
  tenm_table_add(fragment_new(my->x - 160.0, my->y, -2.0, 0.0,
                              50.0, 12, n, 4.0, speed_theta, 20));
  tenm_table_add(fragment_new(my->x + 160.0, my->y, 2.0, 0.0,
                              50.0, 12, n, 4.0, speed_theta, 20));

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
theorem_weapon_act(tenm_object *my, const tenm_object *player)
{
  int theta;
  int n;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "theorem_weapon_act: my is NULL\n");
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
    if (my->count[3] >= 120)
    {
      my->count[2] = 1;
      my->count[3] = 0;
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    return 0;
  }

  /* self-destruction */
  if ((my->count[2] == 1) && (my->count[3] >= 4330))
  {
    set_background(2);
    clear_chain();
    theorem_weapon_explode(my);
    return 0;
  }

  /* dead */
  if (my->count[2] == 2)
  {
    if (theorem_weapon_green(my))
      n = 8;
    else
      n = 7;

    if ((my->count[3] >= 30) && (my->count[3] <= 75)
        && (my->count[3] % 15 == 0))
    {
      theta = rand() % 360;
      tenm_table_add(explosion_new(my->x + 30.0 * tenm_cos(theta),
                                   my->y + 30.0 * tenm_sin(theta),
                                   0.0, 0.0,
                                   2, 300, n, 5.0, 8));
    }
    if (my->count[3] > 120)
    {
      tenm_table_add(explosion_new(my->x, my->y,
                                   0.0, 0.0,
                                   1, 3000, n, 10.0, 8));
      tenm_table_add(explosion_new(my->x, my->y,
                                   0.0, 0.0,
                                   2, 800, n, 6.0, 8));

      tenm_table_add(stage_clear_new(100));
      return 1;
    }

    return 0;
  }

  /* decoration */
  if (theorem_weapon_green(my))
    my->count[4] -= 2;
  else
    my->count[4] += 1;

  if (my->count[2] != 1)
    return 0;

  /* shoot */
  if ((my->count[3] >= 60) && (my->count[3] <= 150))
    theorem_weapon_theorem(my, player, 0, my->count[3] - 60);
  if ((my->count[3] >= 300) && (my->count[3] <= 780))
    theorem_weapon_theorem(my, player, 2, my->count[3] - 300);
  if ((my->count[3] >= 930) && (my->count[3] <= 930))
    theorem_weapon_theorem(my, player, 3, my->count[3] - 930);
  if ((my->count[3] >= 1080) && (my->count[3] <= 1170))
    theorem_weapon_theorem(my, player, 6, my->count[3] - 1080);
  if ((my->count[3] >= 1320) && (my->count[3] <= 1370))
    theorem_weapon_theorem(my, player, 1, my->count[3] - 1320);
  if ((my->count[3] >= 1570) && (my->count[3] <= 1786))
    theorem_weapon_theorem(my, player, 4, my->count[3] - 1570);

  if ((my->count[3] >= 2000) && (my->count[3] <= 2160))
    theorem_weapon_theorem(my, player, 5, my->count[3] - 2000);
  if ((my->count[3] >= 2310) && (my->count[3] <= 2910))
    theorem_weapon_theorem(my, player, 9, my->count[3] - 2310);
  if ((my->count[3] >= 3060) && (my->count[3] <= 3636))
    theorem_weapon_theorem(my, player, 7, my->count[3] - 3060);
  if ((my->count[3] >= 3790) && (my->count[3] <= 4150))
    theorem_weapon_theorem(my, player, 8, my->count[3] - 3790);

  return 0;
}

static void
theorem_weapon_theorem(tenm_object *my, const tenm_object *player,
                       int what, int t)
{
  int i;
  int j;
  int theta;
  double speed;
  double x;
  double y;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "theorem_weapon_theorem: my is NULL\n");
    return;
  }
  if (player == NULL)
    return;
  if (t < 0)
  {
    fprintf(stderr, "theorem_weapon_theorem: t is negative (%d)\n", t);
    return;
  }

  switch (what)
  {
  case 0:
    if (t == 0)
      my->count[5] = rand() % 2;
    if ((t >= 0) && (t <= 150) && (t % 15 == 0))
    {
      for (i = (60 - t / 3) * (-1); i <= (60 - t / 3); i += 60 - t / 3)
      {
        for (j = 0; j < 2; j++)
        {
          if (t % 30 == ((j + my->count[5]) % 2) * 15)
            speed = 5.5;
          else
            speed = 4.0;
          x = my->x - 200.0 + 400.0 * ((double) j);
          tenm_table_add(theorem_weapon_shot_aim_new(x, my->y,
                                                     player->x, player->y,
                                                     speed,
                                                     i, 0, 3));
        }
      }
    }
    break;
  case 1:
    if (t == 0)
    {
      for (j = 0; j < 2; j++)
      {
        for (i = 0; i < 3; i++)
        {
          x = my->x - 200.0 + 400.0 * ((double) j);
          speed = 4.0 + ((double) i) * 2.0;
          tenm_table_add(theorem_weapon_shot_absolute_new(x, my->y,
                                                          player->x, player->y,
                                                          speed,
                                                          45 + j * 90, 1, 1));
          tenm_table_add(theorem_weapon_shot_absolute_new(x, my->y,
                                                          player->x, player->y,
                                                          speed,
                                                          15 + j * 150, 1, 1));
        }
      }
    }
    if (t == 10)
    {
      for (j = 0; j < 2; j++)
      {
        for (i = 0; i < 3; i++)
        {
          x = my->x - 200.0 + 400.0 * ((double) j);
          speed = 3.0 + ((double) i) * 2.0;
          tenm_table_add(theorem_weapon_shot_aim_new(x, my->y,
                                                     player->x, player->y,
                                                     speed,
                                                     0, 1, 3));
        }
      }
    }
    if (t == 30)
    {
      for (j = 0; j < 2; j++)
      {
        for (i = 0; i < 3; i++)
        {
          x = my->x - 200.0 + 400.0 * ((double) j);
          speed = 4.0 + ((double) i) * 2.0;
          tenm_table_add(theorem_weapon_shot_absolute_new(x, my->y,
                                                          player->x, player->y,
                                                          speed,
                                                          60 + j * 60, 1, 1));
          tenm_table_add(theorem_weapon_shot_absolute_new(x, my->y,
                                                          player->x, player->y,
                                                          speed,
                                                          20 + j * 140, 1, 1));
        }
      }
    }
    if (t == 40)
    {
      for (j = 0; j < 2; j++)
      {
        for (i = 0; i < 3; i++)
        {
          x = my->x - 200.0 + 400.0 * ((double) j);
          speed = 4.0 + ((double) i) * 2.0;
          tenm_table_add(theorem_weapon_shot_absolute_new(x, my->y,
                                                          player->x, player->y,
                                                          speed,
                                                          30 + j * 120, 1, 1));
          tenm_table_add(theorem_weapon_shot_absolute_new(x, my->y,
                                                          player->x, player->y,
                                                          speed,
                                                          80 + j * 20, 1, 1));
        }
      }
    }
    if (t == 50)
    {
      for (j = 0; j < 2; j++)
      {
        for (i = 0; i < 3; i++)
        {
          x = my->x - 200.0 + 400.0 * ((double) j);
          speed = 3.0 + ((double) i) * 2.0;
          tenm_table_add(theorem_weapon_shot_aim_new(x, my->y,
                                                     player->x, player->y,
                                                     speed,
                                                     0, 1, 3));
        }
      }
    }
    break;
  case 2:
    if (t == 0)
      my->count[5] = rand() % 2;
    if ((t >= 0) && (t <= 480) && (t % 8 == 0))
    {
      for (i = -15; i <= 15; i += 30)
      {
        for (j = 0; j < 2; j++)
        {
          x = my->x - 200.0 + 400.0 * ((double) j);
          tenm_table_add(theorem_weapon_shot_aim_new(x, my->y,
                                                     player->x, player->y, 6.0,
                                                     i, 0, 2));
        }
      }
    }
    if ((t >= 0) && (t < 480) && (t % 8 == 0))
    {
      j = (t / 120 + my->count[5]) % 2;
      x = my->x - 40.0 + 80.0 * ((double) j);
      theta = 160 - 140 * j + (t % 120) * (j * 2 - 1);
      
      tenm_table_add(theorem_weapon_shot_absolute_new(x, my->y,
                                                      player->x, player->y,
                                                      4.5, theta, 1, 1));
      x = my->x + 80.0 - 160.0 * ((double) j);
      theta = 20 + 140 * j - (t % 120) * (j * 2 - 1);
      tenm_table_add(theorem_weapon_shot_absolute_new(x, my->y,
                                                      player->x, player->y,
                                                      5.0, theta, 1, 1));
    }
    break;
  case 3:
    if (t == 0)
    {
      my->count[5] = -1 + 2 * (rand() % 2);
      for (i = 0; i < 6; i++)
      {
        speed = 4.0 + 0.5 * ((double) i);
        x = my->x + (-200.0 + 80.0 * ((double) i)) * ((double) (my->count[5]));
        for (j = 0; j < 360; j += 24)
        {
          tenm_table_add(theorem_weapon_shot_aim_new(x, my->y,
                                                     player->x, player->y,
                                                     speed,
                                                     j, 1, 3));
        }
      }
    }
    break;
  case 4:
    if ((t >= 0) && (t < 216) && (t % 3 == 0))
    {
      if (t % 6 == 0)
      {
        tenm_table_add(theorem_weapon_shot_aim_new(my->x, my->y,
                                                   player->x, player->y,
                                                   6.0,
                                                   0, 0, 3));
      }
      else
      {
        theta = 6 + (t % 36);
        for (i = -1; i <= 1; i += 2)
        {
          for (j = 0; j < 2; j++)
          {
            x = my->x - 200.0 + 400.0 * ((double) j);
            tenm_table_add(theorem_weapon_shot_aim_new(x, my->y,
                                                       player->x, player->y,
                                                       6.0,
                                                       theta * i, 0, 2));
            tenm_table_add(theorem_weapon_shot_aim_new(x, my->y,
                                                       player->x, player->y,
                                                       6.0,
                                                       (75 - theta) * i,
                                                       0, 2));
          }
        }
      }
    }
    break;
  case 5:
    if (t == 0)
      my->count[5] = -1 + 2 * (rand() % 2);
    if (t == 0)
    {
      x = my->x + 200.0 * ((double) (my->count[5]));
      theta = 90 + 10 * my->count[5];
      for (i = 0; i < 320; i += 10)
        tenm_table_add(theorem_weapon_shot_absolute_new(x, my->y,
                                                        player->x, player->y,
                                                        4.0,
                                                        theta
                                                        + i * my->count[5],
                                                        0, 1));
    }
    if (t == 30)
    {
      x = my->x - 200.0 * ((double) (my->count[5]));
      theta = 90 - 10 * my->count[5];
      for (i = 0; i < 320; i += 10)
        tenm_table_add(theorem_weapon_shot_absolute_new(x, my->y,
                                                        player->x, player->y,
                                                        3.5,
                                                        theta
                                                        - i * my->count[5],
                                                        0, 1));
    }
    if (t == 70)
    {
      for (j = 0; j < 2; j++)
      { 
        for (i = 0; i < 320; i += 10)
        {
          theta = 65 + 50 * j;
          x = my->x - 200.0 + 400.0 * ((double) j);
          tenm_table_add(theorem_weapon_shot_absolute_new(x, my->y,
                                                          player->x, player->y,
                                                          3.0,
                                                          theta
                                                          + i * (1 - 2 * j),
                                                          0, 1));
        }
      }
    }
    if (t == 75)
    {
      for (j = 0; j < 2; j++)
      { 
        for (i = 0; i < 320; i += 10)
        {
          theta = 70 + 40 * j;
          x = my->x - 150.0 + 300.0 * ((double) j);
          tenm_table_add(theorem_weapon_shot_absolute_new(x, my->y,
                                                          player->x, player->y,
                                                          2.75,
                                                          theta
                                                          + i * (1 - 2 * j),
                                                          0, 1));
        }
      }
    }
    if ((t > 110) && (t < 160) && (t % 4 == 0))
    {
      for (j = 0; j < 2; j++)
      { 
        x = my->x + (8.0 * ((double) (135 - t))) * ((double) (-1 + 2 * j));
        speed = 3.0 + 0.12 * ((double) (t - 110));
        for (i = -1; i <= 1; i++)
          tenm_table_add(theorem_weapon_shot_aim_new(x, my->y,
                                                     player->x, player->y,
                                                     speed,
                                                     (t - 100) * i, 1, 3));
      }
    }
    break;
  case 6:
    if (t == 0)
      my->count[5] = rand() % 2;
    if (t == 0)
    {
      x = my->x - 200.0 + 400.0 * ((double) (my->count[5]));
      for (i = -7; i <= 7; i += 2)
        tenm_table_add(theorem_weapon_shot_aim_new(x, my->y,
                                                   player->x, player->y,
                                                   6.0,
                                                   10 * i, 0, 2));
    }
    if (t == 10)
    {
      x = my->x + 200.0 - 400.0 * ((double) (my->count[5]));
      for (i = -7; i <= 7; i += 2)
        tenm_table_add(theorem_weapon_shot_aim_new(x, my->y,
                                                   player->x, player->y,
                                                   6.0,
                                                   10 * i, 0, 2));
    }
    if (t == 20)
    {
      x = my->x - 200.0 + 400.0 * ((double) (my->count[5]));
      for (i = -3; i <= 3; i += 2)
        tenm_table_add(theorem_weapon_shot_aim_new(x, my->y,
                                                   player->x, player->y,
                                                   9.0,
                                                   4 * i, 0, 2));
    }
    if (t == 30)
    {
      x = my->x + 200.0 - 400.0 * ((double) (my->count[5]));
      for (i = -3; i <= 3; i += 2)
        tenm_table_add(theorem_weapon_shot_aim_new(x, my->y,
                                                   player->x, player->y,
                                                   9.0,
                                                   4 * i, 0, 2));
    }
    if ((t >= 40) && (t <= 60) && (t % 5 == 0))
    {
      for (i = 0; i < 2; i++)
      {
        theta = -175 + 40 * ((t - 40) / 5) - 60 * i;
        for (j = -1; j <= 1; j += 2)
        {
          tenm_table_add(theorem_weapon_shot_aim_new(my->x - 40.0, my->y,
                                                     player->x, player->y,
                                                     7.0,
                                                     theta * j, 1, 2));
          tenm_table_add(theorem_weapon_shot_aim_new(my->x + 40.0, my->y,
                                                     player->x, player->y,
                                                     7.0,
                                                     theta * j, 1, 2));
        }
      }
    }
    if (t == 65)
    {
      for (i = -2; i <= 2; i++)
      {
        tenm_table_add(theorem_weapon_shot_aim_new(my->x, my->y,
                                                   player->x, player->y,
                                                   7.0,
                                                   8 * i, 1, 3));
      }
    }
    if ((t >= 70) && (t <= 90) && (t % 5 == 0))
    {
      for (i = 0; i < 2; i++)
      {
        theta = 25 + 40 * ((t - 70) / 5) - 60 * i;
        for (j = -1; j <= 1; j += 2)
        {          
          tenm_table_add(theorem_weapon_shot_aim_new(my->x - 40.0, my->y,
                                                     player->x, player->y,
                                                     7.0,
                                                     theta * j, 1, 2));
          tenm_table_add(theorem_weapon_shot_aim_new(my->x + 40.0, my->y,
                                                     player->x, player->y,
                                                     7.0,
                                                     theta * j, 1, 2));
        }
      }
    }
    break;
  case 7:
    if ((t >= 0) && (t < 576) && (t % 8 == 0))
    {
      if (t % 48 < 24)
      {
        tenm_table_add(theorem_weapon_shot_aim_new(my->x, my->y,
                                                   player->x, player->y,
                                                   6.0,
                                                   0, 0, 3));
      }
      else
      {
        for (j = -1; j <= 1; j += 2)
          tenm_table_add(theorem_weapon_shot_aim_new(my->x, my->y,
                                                     player->x, player->y,
                                                     6.0,
                                                     10 * j, 0, 2));
      }
      for (i = 0; i < 2; i++)
      {
        x = my->x - 200.0 + 400.0 * ((double) i);
        if (t % 64 < 32)
        {
          tenm_table_add(theorem_weapon_shot_aim_new(x, my->y,
                                                     player->x, player->y,
                                                     6.0,
                                                     0, 0, 3));
        }
        else
        {
          for (j = -1; j <= 1; j += 2)
            tenm_table_add(theorem_weapon_shot_aim_new(x, my->y,
                                                       player->x, player->y,
                                                       6.0,
                                                       10 * j, 0, 2));
        }
      }
    }
    break;
  case 8:
    if (t > 360)
      break;
    if (t == 0)
      my->count[5] = -1 + 2 * (rand() % 2);
    if ((t >= 0) && (t % 7 == 0))
    {
      for (i = 0; i < 2; i++)
      {
        x = my->x + 200.0 * tenm_sin((2 * t + 90 * i) * my->count[5]);
        speed = 6.0 + 1.0 * ((double) ((t % 28) / 7));
        for (j = 0; j < 4; j++)
        {
          tenm_table_add(theorem_weapon_shot_absolute_new(x, my->y,
                                                          player->x, player->y,
                                                          speed,
                                                          35 + 90 * j,
                                                          1, 1));
          tenm_table_add(theorem_weapon_shot_absolute_new(x, my->y,
                                                          player->x, player->y,
                                                          speed,
                                                          55 + 90 * j,
                                                          1, 1));
        }
      }
    }
    if ((t >= 0) && (t % 5 == 0) && (t % 25 < 20))
    {
      theta = t * 4 * my->count[5];
      x = my->x + 40.0 * tenm_cos(theta);
      y = my->y + 40.0 * tenm_sin(theta);
      tenm_table_add(theorem_weapon_shot_aim_new(x, y,
                                                 player->x, player->y,
                                                 6.0,
                                                 0, 0, 3));
    }
    break;
  case 9:
    if (t > 600)
      break;
    if ((t >= 0) && (t % 2 == 0))
    {
      x = my->x - 200.0 + 40.0 * ((double) (t % 12));
      theta = (t % 12) * 5 + (t / 12) * 37 - 90;
      tenm_table_add(theorem_weapon_shot_absolute_new(x, my->y,
                                                      player->x, player->y,
                                                      6.0,
                                                      theta, 0, 1));
      x = my->x + 200.0 - 40.0 * ((double) (t % 12));
      tenm_table_add(theorem_weapon_shot_absolute_new(x, my->y,
                                                      player->x, player->y,
                                                      6.0,
                                                      -(theta + 180), 0, 1));
    }
    if ((t >= 0) && (t % 26 == 0))
    {
      for (i = -1; i <= 1; i++)
        tenm_table_add(theorem_weapon_shot_aim_new(my->x, my->y,
                                                   player->x, player->y,
                                                   5.0,
                                                   30 * i, 1, 3));
    }
    if ((t >= 0) && (t % 26 == 13))
    {
      for (i = -1; i <= 1; i++)
      {
        x = my->x + 120.0 * ((double) i);
        tenm_table_add(theorem_weapon_shot_aim_new(x, my->y,
                                                   player->x, player->y,
                                                   5.0,
                                                   0, 1, 3));
      }
    }
    break;
  default:
    fprintf(stderr, "theorem_weapon_theorem: undefined what (%d)\n", what);
    break;
  }
}

static int
theorem_weapon_draw(tenm_object *my, int priority)
{
  int i;
  int status = 0;
  tenm_color color;
  char temp[32];
  int theta;
  int red_orig;
  int green_orig;
  int blue_orig;
  int red;
  int green;
  int blue;
  int c;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "theorem_weapon_draw: my is NULL\n");
    return 0;
  }

  /* decoration */
  if ((priority == 0) && (my->count[2] <= 1))
  {
    if (theorem_weapon_green(my))
    {
      if (my->count[1] >= 1)
      {
        red_orig = 109;
        green_orig = 125;
        blue_orig = 9;
      }
      else
      {
        red_orig = 61;
        green_orig = 95;
        blue_orig = 13;
      }
    }
    else
    {
      if (my->count[1] >= 1)
      {
        red_orig = 135;
        green_orig = 89;
        blue_orig = 9;
      }
      else
      {
        red_orig = 95;
        green_orig = 47;
        blue_orig = 13;
      }
    }

    for (i = 0; i < 360; i += 180)
    {
      theta = my->count[4] * 3 + i;
      while (theta >= 360)
        theta -= 360;
      while (theta < 0)
        theta += 360;
      if (theta < 180)
        c = 180 - theta;
      else
        c = theta - 180;
      if (c < 0)
        c = 0;
      if (c > 256)
        c = 256;

      red = (red_orig * (256 - c) + DEFAULT_BACKGROUND_RED * c) / 256;
      green = (green_orig * (256 - c) + DEFAULT_BACKGROUND_GREEN * c) / 256;
      blue = (blue_orig * (256 - c) + DEFAULT_BACKGROUND_BLUE * c) / 256;
      color = tenm_map_color(red, green, blue);
      if (tenm_draw_line(((int) (my->x)) + 80,
                         (int) (my->y + 40.0 * tenm_sin(theta)),
                         ((int) (my->x)) + 200,
                         (int) (my->y + 40.0 * tenm_sin(theta)),
                         1, color) != 0)
        status = 1;
      if (tenm_draw_line(((int) (my->x)) - 80,
                         (int) (my->y + 40.0 * tenm_sin(theta)),
                         ((int) (my->x)) - 200,
                         (int) (my->y + 40.0 * tenm_sin(theta)),
                         1, color) != 0)
        status = 1;
    }

    c = 90;
    red = (red_orig * (256 - c) + DEFAULT_BACKGROUND_RED * c) / 256;
    green = (green_orig * (256 - c) + DEFAULT_BACKGROUND_GREEN * c) / 256;
    blue = (blue_orig * (256 - c) + DEFAULT_BACKGROUND_BLUE * c) / 256;
    color = tenm_map_color(red, green, blue);
    if (tenm_draw_line(((int) (my->x)) + 40,
                       (int) (my->y),
                       ((int) (my->x)) + 200,
                       (int) (my->y),
                       1, color) != 0)
      status = 1;
    if (tenm_draw_line(((int) (my->x)) - 40,
                       (int) (my->y),
                       ((int) (my->x)) - 200,
                       (int) (my->y),
                       1, color) != 0)
      status = 1;
  }

  /* body */
  /* dead enemy has low priority */
  if (((my->count[2] <= 1) && (priority == 0))
      || ((my->count[2] > 1) && (priority == -1)))
  {
    if (theorem_weapon_green(my))
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

    if (tenm_draw_circle((int) (my->x), (int) (my->y),
                         40, 3, color) != 0)
      status = 1;
  }

  /* hit point stat */
  if ((priority == 0) && (my->count[2] == 1))
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 10, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "theorem_weapon_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

/* return 1 (true) or 0 (false) */
static int
theorem_weapon_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if ((my->count[2] == 1)
      && (my->count[3] >= 1940) && (my->count[3] < 4300))
    return 1;
  if ((my->count[2] == 2) && (my->count[6] != 0))
    return 1;

  return 0;
}

static tenm_object *
theorem_weapon_shot_absolute_new(double x, double y,
                                 double player_x, double player_y,
                                 double speed, int theta, int what, int color)
{
  /* sanity check */
  if (speed < NEAR_ZERO)
  {
    fprintf(stderr, "theorem_weapon_shot_absolute_new: speed is non-positive "
            "(%f)\n", speed);
    return NULL;
  }
  if ((color != 0) && (color != 1))
  {
    fprintf(stderr, "theorem_weapon_shot_absolute_new: wrong color "
            "(%d)\n", color);
    return NULL;
  }

  return theorem_weapon_shot_new(x, y, player_x, player_y,
                                 speed * tenm_cos(theta),
                                 speed * tenm_sin(theta),
                                 what, color);
}

static tenm_object *
theorem_weapon_shot_aim_new(double x, double y,
                            double player_x, double player_y,
                            double speed, int theta, int what, int color)
{
  double dx;
  double dy;
  double length;
  double result[2];
  double v[2];

  /* sanity check */
  if (speed < NEAR_ZERO)
  {
    fprintf(stderr, "theorem_weapon_shot_aim_new: speed is non-positive "
            "(%f)\n", speed);
    return NULL;
  }
  if ((color != 2) && (color != 3))
  {
    fprintf(stderr, "theorem_weapon_shot_aim_new: wrong color "
            "(%d)\n", color);
    return NULL;
  }

  dx = player_x - x;
  dy = player_y - y;
  length = tenm_sqrt((int) (dx * dx + dy * dy));
  if (length < NEAR_ZERO)
    length = 1.0;

  v[0] = speed * dx / length;
  v[1] = speed * dy / length;
  result[0] = v[0];
  result[1] = v[1];
  vector_rotate(result, v, theta);
  return theorem_weapon_shot_new(x, y, player_x, player_y,
                                 result[0], result[1],
                                 what, color);
}

/* list of what
 * 0: normal shot
 * 1: laser
 */
static tenm_object *
theorem_weapon_shot_new(double x, double y, double player_x, double player_y,
                        double speed_x, double speed_y, int what, int color)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  int hit_mask;
  double length_x;
  double length_y;
  double speed;

  /* sanity check */
  if ((what < 0) || (what > 1))
  {
    fprintf(stderr, "theorem_weapon_shot_new: strange what (%d)\n", what);
    return NULL;
  }

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "theorem_weapon_shot_new: malloc(p) failed\n");
    return NULL;
  }

  if (what == 0)
  {  
    p[0] = (tenm_primitive *) tenm_circle_new(x, y, 5.0);
  }
  else
  {
    speed = tenm_sqrt((int) (speed_x * speed_x + speed_y * speed_y));
    if (speed < NEAR_ZERO)
      speed = 1.0;
    length_x = 25.0 * speed_x / speed;
    length_y = 25.0 * speed_y / speed;
    p[0] = (tenm_primitive *) tenm_segment_new(x, y,
                                               x + length_x, y + length_y);
  }
  if (p[0] == NULL)
  {
    fprintf(stderr, "theorem_weapon_shot_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 3);
  if (count == NULL)
  {
    fprintf(stderr, "theorem_weapon_shot_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "theorem_weapon_shot_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] base color
   * [1] what
   * [2] warning flag
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */
  count[0] = color;
  count[1] = what;
  count[2] = theorem_weapon_shot_seek(x, y, player_x, player_y,
                                      speed_x, speed_y, what);

  count_d[0] = speed_x;
  count_d[1] = speed_y;

  if (what == 0)
    hit_mask = ATTR_OPAQUE;
  else
    hit_mask = 0;
  new = tenm_object_new("Theorem Weapon shot", ATTR_ENEMY_SHOT, hit_mask,
                        1, x, y,
                        3, count, 2, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&theorem_weapon_shot_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&theorem_weapon_shot_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&theorem_weapon_shot_act),
                        (int (*)(tenm_object *, int))
                        (&theorem_weapon_shot_draw));
  if (new == NULL)
  {
    fprintf(stderr, "normal_shot_new: tenm_object_new failed\n");
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
theorem_weapon_shot_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "theorem_weapon_shot_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "theorem_weapon_shot_move: strange turn_per_frame (%f)\n",
            turn_per_frame);
    return 0;
  }

  dx_temp = my->count_d[0] / turn_per_frame;
  dy_temp = my->count_d[1] / turn_per_frame;
  /* slow down if the player is in the way */
  /*
  if (my->count[2] != 0)
  {
    dx_temp *= 0.5;
    dy_temp *= 0.5;
  }
  */
  my->x += dx_temp;
  my->y += dy_temp;
  tenm_move_mass(my->mass, dx_temp, dy_temp);

  if (!in_window_object(my))
    return 1;

  return 0;
}

static int
theorem_weapon_shot_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "theorem_weapon_shot_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "theorem_weapon_shot_hit: your is NULL\n");
    return 0;
  }

  if (your->attr & ATTR_OPAQUE)
    return 1;

  return 0;
}

static int
theorem_weapon_shot_act(tenm_object *my, const tenm_object *player)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "normal_shot_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  my->count[2] = theorem_weapon_shot_seek(my->x, my->y,
                                          player->x, player->y,
                                          my->count_d[0], my->count_d[1],
                                          my->count[1]);
  return 0;
}

/* checks if the shot hit the player
 * return 1 (true) or 0 (false)
 */
static int
theorem_weapon_shot_seek(double x, double y,
                         double player_x, double player_y,
                         double speed_x, double speed_y,
                         int what)
{
  /* sanity check */
  if ((what < 0) || (what > 1))
    return 0;

  if (what == 0)
  {
    if (!tenm_line_nearer(player_x, player_y,
                          x, y, x + speed_x, y + speed_y, 10.6))
      return 0;
    if (tenm_same_side(player_x, player_y, x + speed_x, y + speed_y,
                       x, y, x - speed_y, y + speed_x))
      return 1;
    if (tenm_line_nearer(player_x, player_y,
                         x, y, x - speed_y, y + speed_x, 10.6))
      return 1;

    return 0;
  }
  else
  {
    if (!tenm_line_nearer(player_x, player_y,
                          x, y, x + speed_x, y + speed_y, 5.1))
      return 0;
    if (tenm_same_side(player_x, player_y, x + speed_x, y + speed_y,
                       x, y, x - speed_y, y + speed_x))
      return 1;
    if (tenm_line_nearer(player_x, player_y,
                         x, y, x - speed_y, y + speed_x, 5.1))
      return 1;

    return 0;
  }

  /* should not reach here */
  fprintf(stderr, "theorem_weapon_shot_seek: fall off\n");
  return 0;
}

static int
theorem_weapon_shot_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "theorem_weapon_shot_draw: my is NULL\n");
    return 0;
  }
  if (my->mass == NULL)
  {
    fprintf(stderr, "theorem_weapon_shot_draw: my->mass is NULL\n");
    return 0;
  }

  /* return if it is not my turn */
  if (priority != 1)
    return 0;

  if (my->count[2] != 0)
    color = tenm_map_color(175, 0, 239);
  else
  {
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
    default:
      fprintf(stderr, "normal_shot_draw: strange my->count[0] (%d)\n",
              my->count[0]);
      color = tenm_map_color(0, 0, 0);
      break;
    }
  }

  if (my->count[1] == 0)
  {
    
    if (tenm_draw_circle((int) (my->x), (int) (my->y), 5, 2, color) != 0)
      status = 1;
  }
  else
  {
    if (tenm_draw_line((int) (((tenm_segment *) my->mass->p[0])->a->x),
                       (int) (((tenm_segment *) my->mass->p[0])->a->y),
                       (int) (((tenm_segment *) my->mass->p[0])->b->x),
                       (int) (((tenm_segment *) my->mass->p[0])->b->y),
                       3, color) != 0)
      status = 1;
  }

  return status;
}
