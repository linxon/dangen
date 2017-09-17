/* $Id: w-ko.c,v 1.226 2011/08/23 20:51:50 oohara Exp $ */
/* [normal] W-KO */

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
#include "stage-clear.h"
#include "score.h"
#include "ship.h"

#include "w-ko.h"

#define NEAR_ZERO 0.0001

static int w_ko_move(tenm_object *my, double turn_per_frame);
static int w_ko_hit(tenm_object *my, tenm_object *your);
static void w_ko_explode(tenm_object *my);
static int w_ko_act(tenm_object *my, const tenm_object *player);
static int w_ko_draw(tenm_object *my, int priority);
static int w_ko_green(const tenm_object *my);

static tenm_object *w_ko_cut_new(double dx, double dy,
                                 double target_x, double target_y);
static int w_ko_cut_act(tenm_object *my, const tenm_object *player);
static int w_ko_cut_draw(tenm_object *my, int priority);

static tenm_object *w_ko_spear_new(double x, double y, double dx, double dy);
static int w_ko_spear_move(tenm_object *my, double turn_per_frame);
static int w_ko_spear_act(tenm_object *my, const tenm_object *player);
static int w_ko_spear_draw(tenm_object *my, int priority);

static tenm_object *w_ko_lock_on_new(double x, double y,
                                     double target_x, double target_y,
                                     int what);
static int w_ko_lock_on_act(tenm_object *my, const tenm_object *player);
static int w_ko_lock_on_draw(tenm_object *my, int priority);

static tenm_object *w_ko_more_1_new(void);
static int w_ko_more_1_act(tenm_object *my, const tenm_object *player);

tenm_object *
w_ko_new(void)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = (double) (WINDOW_WIDTH / 2);
  double y = -100.0;
  int i;

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "w_ko_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_circle_new(x, y, 30.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "w_ko_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 38);
  if (count == NULL)
  {
    fprintf(stderr, "w_ko_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 30);
  if (count_d == NULL)
  {
    fprintf(stderr, "w_ko_new: malloc(count_d) failed\n");
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
   * [4] dash mode
   * [5] dash timer
   * [6] blade mode
   * [7] blade timer
   * [8] normal shot mode
   * [9] normal shot timer
   * [10] spear mode
   * [11] spear timer
   * [12 -- 35] lock on
   *   [suffix + 0] mode
   *   [suffix + 1] timer
   * [36] last resort randomness
   * [37] "was green when killed" flag
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   * [2] aim x
   * [3] aim y
   * [4] dash x
   * [5] dash y
   * [6 -- 29] lock on (x, y)
   */

  count[0] = 0;
  count[1] = 0;
  count[2] = 0;
  count[3] = 0;
  count[4] = 1;
  count[5] = 0;
  count[6] = 0;
  count[7] = -30;
  count[8] = 0;
  count[9] = 0;
  count[10] = 0;
  count[11] = -460;
  for (i = 0; i < 12; i++)
  {
    count[12 + i * 2 + 0] = 0;
    count[12 + i * 2 + 1] = -20 - 20 * i;
  }
  count[36] = 0;
  count[37] = 0;

  count_d[0] = 0.0;
  count_d[1] = 0.0;
  count_d[2] = 0.0;
  count_d[3] = 60.0;
  count_d[4] = (double) (WINDOW_WIDTH / 2);
  count_d[5] = (double) (WINDOW_HEIGHT / 2);
  for (i = 0; i < 12; i++)
  {
    count_d[6 + i * 2 + 0] = 0.0;
    count_d[6 + i * 2 + 1] = 0.0;
  }

  new = tenm_object_new("W-KO", ATTR_BOSS, ATTR_PLAYER_SHOT,
                        400, x, y,
                        38, count, 30, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&w_ko_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&w_ko_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&w_ko_act),
                        (int (*)(tenm_object *, int))
                        (&w_ko_draw));
  if (new == NULL)
  {
    fprintf(stderr, "w_ko_new: tenm_object_new failed\n");
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
w_ko_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "w_ko_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "w_ko_move: strange turn_per_frame (%f)\n",
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
w_ko_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "w_ko_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "w_ko_hit: your is NULL\n");
    return 0;
  }

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;

  deal_damage(my, your, 0);
  if (w_ko_green(my))
    add_chain(my, your);
  my->count[1] = 2;

  if (my->hit_point <= 0)
  {
    add_score(30000);
    set_background(1);
    w_ko_explode(my);
    return 0;
  }

  return 0;
}

static void
w_ko_explode(tenm_object *my)
{
  int n;

  /* sanity check */
  if (my == NULL)
    return;

  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy_shot), 0);
  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy), 0);

  /* don't modify my->attr or my->hit_mask here, or the player shot
   * may fly through the enemy */
  tenm_mass_delete(my->mass);
  my->mass = NULL;

  /* set "was green" flag before we change the life mode */
  if (w_ko_green(my))
  {
    n = 8;
    my->count[37] = 1;
  }
  else
  {
    n = 7;
    my->count[37] = 0;
  }

  tenm_table_add(explosion_new(my->x, my->y,
                               0.0, 0.0,
                               1, 3000, n, 6.0, 8));
  tenm_table_add(explosion_new(my->x, my->y,
                               0.0, 0.0,
                               2, 800, n, 3.5, 8));

  my->count[1] = 0;
  my->count[2] = 1;
  my->count[3] = 1;
  my->count_d[0] = 0.0;
  my->count_d[1] = 0.0;
}

static int
w_ko_act(tenm_object *my, const tenm_object *player)
{
  double result[2];
  double v[2];
  double a[2];
  int i;
  int j;
  double x;
  double y;
  double c;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "w_ko_act: my is NULL\n");
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
  if (my->count[2] == 1)
  {
    if ((my->count[3] <= 9) && (my->count[3] % 3 == 0))
    {  
      if (w_ko_green(my))
        i = 8;
      else
        i = 7;
      tenm_table_add(explosion_new(my->x + ((double) (my->count[3]/3)) * 60.0,
                                   my->y,
                                   0.0, 0.0,
                                   1, 200, i, 2.0, 8));
      tenm_table_add(explosion_new(my->x - ((double) (my->count[3]/3)) * 60.0,
                                   my->y,
                                   0.0, 0.0,
                                   1, 200, i, 2.0, 8));
    }
    if (my->count[3] >= 9)
    {
      tenm_table_add(stage_clear_new(100));
      return 1;
    }
    
    return 0;
  }

  /* self-destruction */
  if (my->count[3] >= 4630)
  {
    set_background(2);
    clear_chain();
    w_ko_explode(my);
    return 0;
  }

  /* normal enemy */
  if (my->count[3] == 1500)
    tenm_table_add(w_ko_more_1_new());

  /* speed change */
  if (my->count[4] == 0)
  {
    for (i = 0; i <= 1; i++)
    {
      if ((player->count_d[i] < -0.11) || (player->count_d[i] > 0.11))
        my->count_d[i] = player->count_d[i] * (-0.75);
      else if ((my->count_d[i] > -0.11) && (my->count_d[i] < 0.11))
        my->count_d[i] = 0.0;
      else if (my->count_d[i] < -0.11)
        my->count_d[i] += 0.1;
      else
        my->count_d[i] -= 0.1;
    }

    if (((my->x < 15.0) && (my->count_d[0] < NEAR_ZERO))
        || ((my->x > ((double) WINDOW_WIDTH) - 15.0)
            && (my->count_d[0] > -NEAR_ZERO)))
    {  
      my->count_d[0] = 0.0;
      if (my->count[4] == 0)
        (my->count[5])++;
    }
    if (((my->y < 15.0) && (my->count_d[1] < NEAR_ZERO))
        || ((my->y > ((double) WINDOW_HEIGHT) - 15.0)
            && (my->count_d[1] > -NEAR_ZERO)))
    {  
      my->count_d[1] = 0.0;
      if (my->count[4] == 0)
        (my->count[5])++;
    }
  }

  /* aim the player */
  a[0] = player->x - my->x;
  a[1] = player->y - my->y;
  v[0] = my->count_d[2];
  v[1] = my->count_d[3];
  result[0] = v[0];
  result[1] = v[1];
  vector_rotate_bounded(result, v, a, 1);
  my->count_d[2] = result[0];
  my->count_d[3] = result[1];

  /* last resort */
  if (my->count[3] == 4200)
  {
    /* dash to the center */
      my->count[4] = 1;
      my->count[5] = 0;
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
      my->count_d[4] = (double) (WINDOW_WIDTH / 2);
      my->count_d[5] = (double) (WINDOW_HEIGHT / 2);
  }
  if (my->count[3] >= 4260)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
  }
  if (my->count[3] == 4260)
  {
    my->count[36] = 1 + rand() % 12;
    if (rand() % 2 == 0)
      my->count[36] *= -1;
  }
  if ((my->count[3] >= 4260) && (my->count[3] <= 4370)
      && (my->count[3] % 10 == 0))
  {
    /* super lock on */
    i = (my->count[3] - 4260) / 10;
    i = (i + my->count[36] + 12) % 12;
    if ((i >= 0) && (i < 12))
    {
      x = (double) (WINDOW_WIDTH / 5) * ((double) ((i % 4) + 1));
      y = (double) (WINDOW_HEIGHT / 4) * ((double) ((i % 3) + 1));
      if (my->count[36] < 0)
        y = (double) (WINDOW_HEIGHT) - y;
      x += (double) (-5 + rand() % 11);
      y += (double) (-5 + rand() % 11);
      my->count[12 + i * 2 + 0] = 1;
      my->count[12 + i * 2 + 1] = 1;
      my->count_d[6 + i * 2 + 0] = x;
      my->count_d[6 + i * 2 + 1] = y;
    }
  }
  if (my->count[3] == 4440)
    my->count[36] = rand() % 360;
  if ((my->count[3] >= 4445) && (my->count[3] <= 4535)
      && (my->count[3] % 10 == 0))
  {
    i = (my->count[3] - 4445) / 10;
    i = i * 36 * 7 + my->count[36];
    /* super blade */
      tenm_table_add(w_ko_cut_new(60.0 * tenm_cos(i), 60.0 * tenm_sin(i),
                                  player->x, player->y));
  }

  /* normal lock on */
  for (i = 0; i < 12; i++)
  {
    if (my->count[12 + i * 2 + 0] == 0)
    {
      if (i >= 4)
        continue;
      if ((my->count[3] < 1000) || (my->count[3] >= 4000))
        continue;
      (my->count[12 + i * 2 + 1])++;
      if (my->count[12 + i * 2 + 1] <= 0)
        continue;
      if (i % 2 == 0)
      {
        x = player->x;
        y = player->y;
      }
      else
      {
        x = ((double) WINDOW_WIDTH) - player->x;
        y = ((double) WINDOW_HEIGHT) - player->y;
      }
      if ((x - my->x) * (x - my->x)
          + (y - my->y) * (y - my->y)
          <= (double) (my->count[12 + i * 2 + 1]
                       * my->count[12 + i * 2 + 1]) * 64.0)
      {
        my->count[12 + i * 2 + 0] = 1;
        my->count[12 + i * 2 + 1] = 1;
        my->count_d[6 + i * 2 + 0] = x;
        my->count_d[6 + i * 2 + 1] = y;
        for (j = 0; j < 4; j++)
        {
          if ((j == i) || (my->count[12 + j * 2 + 0] != 0))
            continue;
          my->count[12 + j * 2 + 1] -= 5;
          if ((j - i) % 2 == 0)
            my->count[12 + j * 2 + 1] -= 25;
        }
      } 
    }
    else if (my->count[12 + i * 2 + 0] == 1)
    {  
      (my->count[12 + i * 2 + 1])++;
      if (my->count[12 + i * 2 + 1] > 20)
      {
        x = player->x - my->count_d[6 + i * 2 + 0];
        y = player->y - my->count_d[6 + i * 2 + 1];
        if ((x * x + y * y
            <= (double) (my->count[12 + i * 2 + 1]
                         * my->count[12 + i * 2 + 1]) * 25.0)
            && ((my->x - my->count_d[6 + i * 2 + 0] < -NEAR_ZERO)
                || (my->x - my->count_d[6 + i * 2 + 0] > NEAR_ZERO)))
        {
          tenm_table_add(w_ko_lock_on_new(my->x, my->y,
                                          my->count_d[6 + i * 2 + 0],
                                          my->count_d[6 + i * 2 + 1],
                                          i % 2));
          my->count[12 + i * 2 + 0] = 0;
          my->count[12 + i * 2 + 1] = -30;
        }
      }
    }
  }

  /* spear */
  if (my->count[10] == 0)
  {
    (my->count[11])++;
    if ((my->count[11] >= 200) && (my->count[4] == 0) && (my->count[6] == 0)
        && (my->count[3] < 4000))
    {
      a[0] = player->x - my->x;
      a[1] = player->y - my->y;
      v[0] = my->count_d[2];
      v[1] = my->count_d[3];
      result[0] = v[0];
      result[1] = v[1];
      vector_rotate_bounded(result, v, a, 360);
      my->count_d[2] = result[0];
      my->count_d[3] = result[1];

      my->count[10] = 1;
      my->count[11] = 0;
      return 0;
    }
  }
  else if (my->count[10] == 1)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
    (my->count[11])++;
    if (my->count[11] >= 30)
    {
      tenm_table_add(w_ko_spear_new(my->x, my->y,
                                    my->count_d[2], my->count_d[3]));
      my->count[10] = 0;
      my->count[11] = 0;
    }
    return 0;
  }

  /* dash */
  if (my->count[4] == 0)
  {
    (my->count[5])++;
    /* we want to dash if the player is behind us */
    if ((player->x - my->x) * my->count_d[2]
        + (player->y - my->y) * my->count_d[3] < 0.0)
      (my->count[5])++;
    my->count_d[4] = my->x;
    my->count_d[5] = my->y;
    if ((my->count[5] >= 0) && (my->count[6] == 0) && (my->count[10] == 0)
        && (my->count[3] < 4000))
    {
      my->count[4] = 1;
      my->count[5] = 0;
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
      my->count_d[4] = player->x;
      my->count_d[5] = player->y;
      my->count_d[4] += (double) (-30 + (rand() % 61));
      my->count_d[5] += (double) (-30 + (rand() % 61));
      if (my->count_d[4] < 15.0)
        my->count_d[4] = 15.0;
      if (my->count_d[4] > ((double) WINDOW_WIDTH) - 15.0)
        my->count_d[4] = ((double) WINDOW_WIDTH) - 15.0;
      if (my->count_d[5] < 15.0)
        my->count_d[5] = 15.0;
      if (my->count_d[5] > ((double) WINDOW_HEIGHT) - 15.0)
        my->count_d[5] = ((double) WINDOW_HEIGHT) - 15.0;
    }
  }
  else if (my->count[4] == 1)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
    (my->count[5])++;
    if (my->count[5] == 60)
    {
      if (my->mass != NULL)
        tenm_move_mass(my->mass,
                       my->count_d[4] - my->x,
                       my->count_d[5] - my->y);
      my->x = my->count_d[4];
      my->y = my->count_d[5];
    }
    if (my->count[5] == 30)
    { 
      if (my->count[6] == 0)
        my->count[7] = 0;
    }
    if (my->count[5] >= 30)
    {
      c = ((double) (60 - my->count[5])) / 30.0;
      c *= 3.0 / 4.0;
      x = my->count_d[4] * (1.0 - c) + my->x * c;
      y = my->count_d[5] * (1.0 - c) + my->y * c;

      a[0] = player->x - x;
      a[1] = player->y - y;
      v[0] = my->count_d[2];
      v[1] = my->count_d[3];
      result[0] = v[0];
      result[1] = v[1];
      vector_rotate_bounded(result, v, a, 360);
      my->count_d[2] = result[0];
      my->count_d[3] = result[1];
    }
    if (my->count[5] >= 60)
    {
      my->count[4] = 0;
      my->count[5] = -100;
      if (my->count[6] == 0)
        my->count[7] = 0;
    }
  }

  /* blade */
  if (my->count[6] == 0)
  {
    if (my->count[7] < 0)
      (my->count[7])++;
    if ((my->count[7] >= 0)
        && (!((my->count[4] == 1) && (my->count[5] < 30)))
        && (my->count[3] > 60) && (my->count[3] < 4000))
    {
      x = player->x - my->count_d[4];
      y = player->y - my->count_d[5];
      if ((x * x + y * y < 180.0 * 180.0)
          && (x * my->count_d[2] + y * my->count_d[3] > 0.0))
      {
        my->count[6] = 1;
        my->count[7] = 0;
      }
    }
  }
  else if (my->count[6] == 1)
  {
    (my->count[7])++;
    /* we can't cut the player if the player is immutable */
    if ((get_ship() < 0) || (player->count[1] > 0))
    {
      my->count[6] = 0;
      my->count[7] = -30;
    }
    if (my->count[7] == 5)
    {
      v[0] = my->count_d[2];
      v[1] = my->count_d[3];
       result[0] = v[0];
      result[1] = v[1];
      vector_rotate(result, v, -30);
      tenm_table_add(w_ko_cut_new(result[0], result[1],
                                  player->x, player->y));
    }
    if (my->count[7] >= 10)
    {
      if ((my->count[4] == 1) && (my->count[5] < 30))
      {
        my->count[6] = 0;
        my->count[7] = -30;
      }
      else
      {
        x = player->x - my->count_d[4];
        y = player->y - my->count_d[5];
        if ((x * x + y * y < 180.0 * 180.0)
            && (x * my->count_d[2] + y * my->count_d[3] > 0.0))
        {
          my->count[6] = 2;
          my->count[7] = 0;
        }
        else
        {
          my->count[6] = 0;
          my->count[7] = -30;
        }
      }
    }
  }
  else if (my->count[6] == 2)
  {
    (my->count[7])++;
    /* we can't cut the player if the player is immutable */
    if ((get_ship() < 0) || (player->count[1] > 0))
    {
      my->count[6] = 0;
      my->count[7] = -30;
    }
    if (my->count[7] == 5)
    {
      v[0] = my->count_d[2];
      v[1] = my->count_d[3];
      result[0] = v[0];
      result[1] = v[1];
      vector_rotate(result, v, 30);
      tenm_table_add(w_ko_cut_new(result[0], result[1],
                                  player->x, player->y));
    }
    if (my->count[7] >= 10)
    {
      my->count[6] = 0;
      my->count[7] = -30;
    }
  }

  /* normal shot */
  if ((my->count[4] == 1) && (my->count[5] >= 30))
    return 0;
  if ((my->count[3] <= 60) || (my->count[3] >= 4260))
    return 0;

  (my->count[9])++;
  if (my->count[8] == 0)
  {
    if (my->count[9] % 11 == 0)
    {
      for (i = -2; i <= 2; i++)
      {
        if (i == 0)
        {
          if (my->count[9] % 33 >= 22)
            continue;
        }
        else if ((i == -1) || (i == 1))
        {
          if (my->count[9] % 22 != 0)
            continue;
        }
        else if ((i == -2) || (i == 2))
        {
          if (my->count[9] % 22 != 11)
            continue;
        }
      
        v[0] = my->count_d[2];
        v[1] = my->count_d[3];
        result[0] = v[0];
        result[1] = v[1];
        vector_rotate(result, v, 15 * i);
        tenm_table_add(laser_point_new(my->x,
                                       my->y,
                                       4.5,
                                       my->x + result[0],
                                       my->y + result[1],
                                       25.0, 3));
      }
    }
    if (my->count[9] >= 132)
    {
      my->count[8] = 1;
      my->count[9] = 0;
    }
  }
  else if (my->count[8] == 1)
  {
    if (my->count[9] % 10 == 0)
    {
      for (i = -1; i <= 1; i += 2)
      {
        v[0] = my->count_d[2] * 20.0 / 60.0;
        v[1] = my->count_d[3] * 20.0 / 60.0;
        result[0] = v[0];
        result[1] = v[1];
        vector_rotate(result, v, 90 * i);
        tenm_table_add(laser_point_new(my->x + result[0],
                                       my->y + result[1],
                                       7.5,
                                       my->x + result[0] + my->count_d[2],
                                       my->y + result[1] + my->count_d[3],
                                       25.0, 2));
      }
    }
    if (my->count[9] >= 40)
    {
      my->count[8] = 0;
      my->count[9] = 0;
    }
  }

  return 0;
}

static int
w_ko_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  char temp[32];
  int i;
  int j;
  double result[2];
  double v[2];
  double x;
  double y;
  double c;
  int theta;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "w_ko_draw: my is NULL\n");
    return 0;
  }

  if (my->count[2] != 0)
    return 0;

  /* decoration */
  if (priority == 0)
  {
    if (w_ko_green(my))
    {
      if (my->count[1] >= 1)
        color = tenm_map_color(181, 190, 92);
      else
        color = tenm_map_color(157, 182, 123);
    }
    else
    {
      if (my->count[1] >= 1)
        color = tenm_map_color(200, 164, 92);
      else
        color = tenm_map_color(182, 147, 123);
    }

    if (tenm_draw_line((int) (my->x),
                       (int) (my->y),
                       (int) (my->x + my->count_d[2]),
                       (int) (my->y + my->count_d[3]),
                       1, color) != 0)
      status = 1;
    for (i = -1; i <= 1; i += 2)
    {
      v[0] = my->count_d[2] * 30.0 / 60.0;
      v[1] = my->count_d[3] * 30.0 / 60.0;
      result[0] = v[0];
      result[1] = v[1];
      vector_rotate(result, v, 75 * i);
      x = my->x + result[0];
      y = my->y + result[1];
      v[0] = result[0] * 60.0 / 30.0;
      v[1] = result[1] * 60.0 / 30.0;
      vector_rotate(result, v, 90 * i);
      if (tenm_draw_line((int) (x),
                         (int) (y),
                         (int) (x + result[0]),
                         (int) (y + result[1]),
                         1, color) != 0)
        status = 1;
      if (tenm_draw_circle((int) (x + result[0]),
                           (int) (y + result[1]),
                           5, 1, color) != 0)
        status = 1;
    }
  }

  /* lock on */
  if (priority == 0)
  {
    for (i = 0; i < 12; i++)
    {
      if (my->count[12 + i * 2 + 0] == 0)
        continue;
      if (my->count[12 + i * 2 + 1] <= 20)
      {
        c = 425.0 - ((double) (my->count[12 + i * 2 + 1])) * 20.0;
        theta = 45 + my->count[12 + i * 2 + 1] * 18;
        x = my->count_d[6 + i * 2 + 0];
        y = my->count_d[6 + i * 2 + 1];
        for (j = 0; j < 360; j += 90)
          if (tenm_draw_line((int) (x + c * tenm_cos(theta + j)),
                             (int) (y + c * tenm_sin(theta + j)),
                             (int) (x + c * tenm_cos(theta + 90 + j)),
                             (int) (y + c * tenm_sin(theta + 90 + j)),
                             1, tenm_map_color(206, 201, 175)) != 0)
            status = 1;
      }
      else
      {
        c = 25.0;
        theta = 30;
        x = my->count_d[6 + i * 2 + 0];
        y = my->count_d[6 + i * 2 + 1];
        for (j = 0; j < 360; j += 120)
          if (tenm_draw_line((int) (x + c * tenm_cos(theta + j)),
                             (int) (y + c * tenm_sin(theta + j)),
                             (int) (x + c * tenm_cos(theta + 120 + j)),
                             (int) (y + c * tenm_sin(theta + 120 + j)),
                             1, tenm_map_color(158, 158, 158)) != 0)
            status = 1;
      }
    }
  }

  /* spear */
  if ((priority == 0) && (my->count[10] == 1))
  {
    if (tenm_draw_line((int) (my->x + 90.0 * my->count_d[3] / 60.0),
                       (int) (my->y - 90.0 * my->count_d[2] / 60.0),
                       (int) (my->x + 180.0 * my->count_d[2] / 60.0),
                       (int) (my->y + 180.0 * my->count_d[3] / 60.0),
                       1, tenm_map_color(182, 123, 162)) != 0)
      status = 1;
    if (tenm_draw_line((int) (my->x + 180.0 * my->count_d[2] / 60.0),
                       (int) (my->y + 180.0 * my->count_d[3] / 60.0),
                       (int) (my->x - 90.0 * my->count_d[3] / 60.0),
                       (int) (my->y + 90.0 * my->count_d[2] / 60.0),
                       1, tenm_map_color(182, 123, 162)) != 0)
      status = 1;
    if (tenm_draw_line((int) (my->x - 90.0 * my->count_d[3] / 60.0),
                       (int) (my->y + 90.0 * my->count_d[2] / 60.0),
                       (int) (my->x - 90.0 * my->count_d[2] / 60.0),
                       (int) (my->y - 90.0 * my->count_d[3] / 60.0),
                       1, tenm_map_color(182, 123, 162)) != 0)
      status = 1;
    if (tenm_draw_line((int) (my->x - 90.0 * my->count_d[2] / 60.0),
                       (int) (my->y - 90.0 * my->count_d[3] / 60.0),
                       (int) (my->x + 90.0 * my->count_d[3] / 60.0),
                       (int) (my->y - 90.0 * my->count_d[2] / 60.0),
                       1, tenm_map_color(182, 123, 162)) != 0)
      status = 1;
  }

  /* dash */
  if ((priority == 0) && (my->count[4] == 1))
  {
    if (my->count[5] < 30)
      c = ((double) (my->count[5])) / 30.0;
    else
      c = 1.0;
    x = my->count_d[4] * c + my->x * (1.0 - c);
    y = my->count_d[5] * c + my->y * (1.0 - c);
    if (tenm_draw_circle((int) (x), (int) (y),
                         30, 1, color) != 0)
      status = 1;
    if (tenm_draw_line((int) (x),
                       (int) (y),
                       (int) (x + my->count_d[2]),
                       (int) (y + my->count_d[3]),
                       1, color) != 0)
      status = 1;
    if ((my->count[5] >= 30) && (my->count[5] < 60))
    {
      for (i = 1; i < 4; i++)
      {
        c = ((double) (60 - my->count[5])) / 30.0;
        c *= ((double) i) / 4.0;
        x = my->count_d[4] * (1.0 - c) + my->x * c;
        y = my->count_d[5] * (1.0 - c) + my->y * c;
        if (tenm_draw_circle((int) (x), (int) (y),
                             30, 1, color) != 0)
          status = 1;
      }
    }
  }

  /* super blade */
  if ((priority == 0) 
      && (my->count[3] >= 4440) && (my->count[3] < 4540))
  {
    i = (my->count[3] - 4440) / 10;
    theta = i * 36 * 7 + my->count[36];
    if (i % 2 == 0)
      theta += -60 + ((my->count[3] - 4440) % 10) * 12;
    else
      theta -= -60 + ((my->count[3] - 4440) % 10) * 12;
    if (tenm_draw_line((int) (my->x),
                       (int) (my->y),
                       (int) (my->x + 180.0 * tenm_cos(theta)),
                       (int) (my->y + 180.0 * tenm_sin(theta)),
                       1, tenm_map_color(75, 0, 239)) != 0)
      status = 1;
  }
  
  /* blade */
  if ((priority == 0) && (my->count[6] != 0))
  {
    if (my->count[6] == 1)
      theta = -90 + my->count[7] * 12;
    else
      theta = 90 - my->count[7] * 12;
    v[0] = my->count_d[2] * 180.0 / 60.0;
    v[1] = my->count_d[3] * 180.0 / 60.0;
    result[0] = v[0];
    result[1] = v[1];
    vector_rotate(result, v, theta);
    if (tenm_draw_line((int) (my->count_d[4]),
                       (int) (my->count_d[5]),
                       (int) (my->count_d[4] + result[0]),
                       (int) (my->count_d[5] + result[1]),
                       1, tenm_map_color(75, 0, 239)) != 0)
      status = 1;
  }

  /* body */
  if (priority == 0)
  {
    if (w_ko_green(my))
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

    if (tenm_draw_circle((int) (my->x), (int) (my->y), 30, 3, color) != 0)
      status = 1;
  }

  /* hit point stat */
  if (priority == 0)
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 10, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "w_ko_draw: draw_string failed\n");
      status = 1;
    }
  }
  
  return status;
}

/* return 1 (true) or 0 (false) */
static int
w_ko_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if ((my->count[2] == 0)
      && (my->count[3] >= 1000) && (my->count[3] < 4600))
    return 1;
  if ((my->count[2] == 1) && (my->count[37] != 0))
    return 1;

  return 0;
}

static tenm_object *
w_ko_cut_new(double dx, double dy, double target_x, double target_y)
{
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double length;

  /* sanity check */
  length = tenm_sqrt((int) (dx * dx + dy * dy));
  if (length < NEAR_ZERO)
  {
    fprintf(stderr, "w_ko_cut_new: (dx, dy) too short\n");
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 2);
  if (count == NULL)
  {
    fprintf(stderr, "w_ko_cut_new: malloc(count) failed\n");
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "w_ko_cut_new: malloc(count_d) failed\n");
    free(count);
    return NULL;
  }

  /* list of count
   * [0] color (for delete_enemy_shot)
   * [1] timer
   */
  /* list of count_d
   * [0] vector x
   * [1] vector y
   */
  count[0] = 4;
  count[1] = 1;

  count_d[0] = (-dy) / length;
  count_d[1] = dx / length;

  /* ATTR_ENEMY_SHOT is only to clear it when the player is killed */
  new = tenm_object_new("W-KO cut", ATTR_ENEMY_SHOT, 0,
                        1, target_x, target_y,
                        2, count, 2, count_d, 0, NULL,
                        (int (*)(tenm_object *, double))
                        NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&w_ko_cut_act),
                        (int (*)(tenm_object *, int))
                        (&w_ko_cut_draw));
  if (new == NULL)
  {
    fprintf(stderr, "w_ko_cut_new: tenm_object_new failed\n");
    if (count_d != NULL)
      free(count_d);
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
w_ko_cut_act(tenm_object *my, const tenm_object *player)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "w_ko_cut_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[1])++;
  if (my->count[1] > 27)
  {
    tenm_table_add(laser_new(my->x - 810.0 * my->count_d[0],
                             my->y - 810.0 * my->count_d[1],
                             0.0, 0.0,
                             1620.0 * my->count_d[0],
                             1620.0 * my->count_d[1],
                             4, 1, 0));
    return 1;
  }

  return 0;
}

static int
w_ko_cut_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  double length;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "w_ko_cut_draw: my is NULL\n");
    return 0;
  }

  if (priority == 1)
  {
    color = tenm_map_color(118, 99, 158);
    length = ((double) (my->count[1])) * 30.0;
    if (length < NEAR_ZERO)
      length = 1.0;
      if (tenm_draw_line((int) (my->x - my->count_d[0] * length),
                         (int) (my->y - my->count_d[1] * length),
                         (int) (my->x + my->count_d[0] * length),
                         (int) (my->y + my->count_d[1] * length),
                         1, color) != 0)
    status = 1;
  }
  
  return status;
}

static tenm_object *
w_ko_spear_new(double x, double y, double dx, double dy)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double length;
  
  /* sanity check */
  length = tenm_sqrt((int) (dx * dx + dy * dy));
  if (length < NEAR_ZERO)
  {
    fprintf(stderr, "w_ko_spear_new: (dx, dy) too short\n");
    return NULL;
  }

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "w_ko_spear_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + 90.0 * dy / length,
                                             y - 90.0 * dx / length,
                                             x + 180.0 * dx / length,
                                             y + 180.0 * dy / length,
                                             x - 90.0 * dy / length,
                                             y + 90.0 * dx / length,
                                             x - 90.0 * dx / length,
                                             y - 90.0 * dy / length);
  if (p[0] == NULL)
  {
    fprintf(stderr, "w_ko_spear_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 1);
  if (count == NULL)
  {
    fprintf(stderr, "w_ko_spear_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "w_ko_spear_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] timer
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */
  count[0] = 0;

  count_d[0] = 10.0 * dx / length;
  count_d[1] = 10.0 * dy / length;

  new = tenm_object_new("W-KO spear",
                        ATTR_ENEMY | ATTR_OBSTACLE | ATTR_OPAQUE, 0,
                        1, x, y,
                        1, count, 2, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&w_ko_spear_move),
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&w_ko_spear_act),
                        (int (*)(tenm_object *, int))
                        (&w_ko_spear_draw));

  if (new == NULL)
  {
    fprintf(stderr, "w_ko_spear_new: tenm_object_new failed\n");
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
w_ko_spear_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "w_ko_spear_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "w_ko_spear_move: strange turn_per_frame (%f)\n",
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
w_ko_spear_act(tenm_object *my, const tenm_object *player)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "w_ko_spear_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[0])++;
  if (my->count[0] > 130)
    return 1;

  return 0;
}

static int
w_ko_spear_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "w_ko_spear_draw: my is NULL\n");
    return 0;
  }

  if (priority != 0)
    return 0;

  /* decoration */
  color = tenm_map_color(218, 184, 177);
  if (tenm_draw_line((int) (my->x - 90.0 * my->count_d[0] / 10.0),
                     (int) (my->y - 90.0 * my->count_d[1] / 10.0),
                     (int) (my->x - 450.0 * my->count_d[0] / 10.0),
                     (int) (my->y - 450.0 * my->count_d[1] / 10.0),
                     1, color) != 0)
    status = 1;

  /* body */
  color = tenm_map_color(95, 13, 68);
  if (tenm_draw_line((int) (my->x + 90.0 * my->count_d[1] / 10.0),
                     (int) (my->y - 90.0 * my->count_d[0] / 10.0),
                     (int) (my->x + 180.0 * my->count_d[0] / 10.0),
                     (int) (my->y + 180.0 * my->count_d[1] / 10.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x + 180.0 * my->count_d[0] / 10.0),
                     (int) (my->y + 180.0 * my->count_d[1] / 10.0),
                     (int) (my->x - 90.0 * my->count_d[1] / 10.0),
                     (int) (my->y + 90.0 * my->count_d[0] / 10.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 90.0 * my->count_d[1] / 10.0),
                     (int) (my->y + 90.0 * my->count_d[0] / 10.0),
                     (int) (my->x - 90.0 * my->count_d[0] / 10.0),
                     (int) (my->y - 90.0 * my->count_d[1] / 10.0),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 90.0 * my->count_d[0] / 10.0),
                     (int) (my->y - 90.0 * my->count_d[1] / 10.0),
                     (int) (my->x + 90.0 * my->count_d[1] / 10.0),
                     (int) (my->y - 90.0 * my->count_d[0] / 10.0),
                     3, color) != 0)
    status = 1;

  return status;
}

static tenm_object *
w_ko_lock_on_new(double x, double y,
                 double target_x, double target_y, int what)
{
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  /* just to avoid gcc -Wall warning
   * --- actually, -1.0 is the only _wrong_ value
   */
  double t = -1.0;
  double c;
  double temp;

  /* sanity check */
  if ((what < 0) || (what > 1))
  {
    fprintf(stderr, "w_ko_lock_on_new: strange what (%d)\n", what);
    return NULL;
  }
  if ((x - target_x >= -NEAR_ZERO) && (x - target_x <= NEAR_ZERO))
  {
    fprintf(stderr, "w_ko_lock_on_new: too close\n");
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 3);
  if (count == NULL)
  {
    fprintf(stderr, "w_ko_lock_on_new: malloc(count) failed\n");
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 4);
  if (count_d == NULL)
  {
    fprintf(stderr, "w_ko_lock_on_new: malloc(count_d) failed\n");
    free(count);
    return NULL;
  }

  if (y > (double) (WINDOW_HEIGHT / 2))
  {
    if ((y > 5.0) && (target_y > 5.0))
    {  
      c = 0.0;
    }
    else
    {
      if (y > target_y)
        c = target_y - 5.0;
      else
        c = y - 5.0;
    }
    t = tenm_sqrt((int) (y - c)) + tenm_sqrt((int) (target_y - c));
    t *= t;
    t /= (x - target_x) * (x - target_x);
    t -= 1.0;
    temp = 0.5 * (x + target_x - (target_y - y) / ((target_x - x) * (t + 1)));
    if (((x < temp) && (target_x < temp))
        || ((x > temp) && (target_x > temp)))
    {
      t = tenm_sqrt((int) (y - c)) - tenm_sqrt((int) (target_y - c));
      t *= t;
      t /= (x - target_x) * (x - target_x);
      t -= 1.0;
    }
  }
  else
  {
    if ((y < ((double) WINDOW_HEIGHT) - 5.0)
        && (target_y < ((double) WINDOW_HEIGHT) - 5.0))
    {  
      c = ((double) WINDOW_HEIGHT) - 5.0;
    }
    else
    {
      if (y > target_y)
        c = y + 5.0;
      else
        c = target_y + 5.0;
    }
    t = tenm_sqrt((int) (c - t)) + tenm_sqrt((int) (c - target_y));
    t *= t * (-1.0);
    t /= (x - target_x) * (x - target_x);
    t -= 1;
    temp = 0.5 * (x + target_x - (target_y - y) / ((target_x - x) * (t + 1)));
    if (((x < temp) && (target_x < temp))
        || ((x > temp) && (target_x > temp)))
    {
      t = tenm_sqrt((int) (c - y)) - tenm_sqrt((int) (c - target_y));
      t *= t * (-1.0);
      t /= (x - target_x) * (x - target_x);
      t -= 1.0;
    }
  }

  if ((t - (-1.0) < -NEAR_ZERO) && (t - (-1.0) > NEAR_ZERO))
  {
    fprintf(stderr, "w_ko_lock_on_new: strange t\n");
    /* little effect on the gameplay, continue */
  }

  /* list of count
   * [0] color (for delete_enemy_shot)
   * [1] timer
   * [2] what
   */
  /* list of count_d
   * [0] coefficient of x^2
   * [1] coefficient of x^1
   * [2] coefficient of x^0
   * [3] start point x
   */
  count[0] = 0;
  count[1] = 0;
  count[2] = what;

  count_d[0] = t + 1;
  count_d[1] = -(t + 1) * (x + target_x) + (target_y - y) / (target_x - x);
  count_d[2] = (t + 1) * x * target_x
    + (target_x * y - x * target_y) / (target_x - x);
  count_d[3] = x;

  /* ATTR_ENEMY_SHOT is only to clear it when the player is killed */
  new = tenm_object_new("W-KO lock on", ATTR_ENEMY_SHOT, 0,
                        1, target_x, target_y,
                        3, count, 4, count_d, 0, NULL,
                        (int (*)(tenm_object *, double))
                        NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&w_ko_lock_on_act),
                        (int (*)(tenm_object *, int))
                        (&w_ko_lock_on_draw));
  if (new == NULL)
  {
    fprintf(stderr, "w_ko_lock_on_new: tenm_object_new failed\n");
    if (count_d != NULL)
      free(count_d);
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
w_ko_lock_on_act(tenm_object *my, const tenm_object *player)
{
  int i;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "w_ko_lock_on_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[1])++;
  if (my->count[1] > 30)
  {
    for (i = 0; i < 360; i += 60)
    {  
      tenm_table_add(normal_shot_angle_new(my->x, my->y, 5.5,
                                           my->count[2] * 30 + i, 0));
      tenm_table_add(normal_shot_angle_new(my->x, my->y, 3.5,
                                           my->count[2] * 30 + i, 0));
    }

    return 1;
  }

  return 0;
}

static int
w_ko_lock_on_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  int i;
  double c;
  double x;
  double y;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "w_ko_lock_on_draw: my is NULL\n");
    return 0;
  }

  if (priority == 1)
  {
    color = tenm_map_color(99, 158, 114);
    for (i = 0; i < 360; i += 120)
      if (tenm_draw_line((int) (my->x + 25.0 * tenm_cos(-90 + i)),
                         (int) (my->y + 25.0 * tenm_sin(-90 + i)),
                         (int) (my->x + 25.0 * tenm_cos(30 + i)),
                         (int) (my->y + 25.0 * tenm_sin(30 + i)),
                         1, color) != 0)
        status = 1;
    if ((my->count[1] >= 0) && (my->count[1] <= 30))
    {
      c = ((double) (my->count[1])) / 30.0;
      x = my->count_d[3] * (1.0 - c) + my->x * c;
      y = my->count_d[0] * x * x + my->count_d[1] * x + my->count_d[2];
      if (tenm_draw_circle((int) (x), (int) (y), 5, 1,
                           tenm_map_color(99, 158, 114)) != 0)
        status = 1;
    }
  }

  return status;
}

static tenm_object *
w_ko_more_1_new(void)
{
  tenm_object *new = NULL;
  int *count = NULL;

  count = (int *) malloc(sizeof(int) * 4);
  if (count == NULL)
  {
    fprintf(stderr, "w_ko_more_1_new: malloc(count) failed\n");
    return NULL;
  }

  /* list of count
   * [0] life timer
   * [1] mode
   * [2] add enemy timer
   * [3] number of enemies killed/escaped
   */
  count[0] = 0;
  count[1] = rand() % 4;
  count[2] = 0;
  count[3] = 0;

  /* ATTR_ENEMY is only to clear it when the boss is dead */
  new = tenm_object_new("W-KO more 1", ATTR_ENEMY, 0,
                        1, 0.0, 0.0,
                        4, count, 0, NULL, 0, NULL,
                        (int (*)(tenm_object *, double))
                        NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&w_ko_more_1_act),
                        (int (*)(tenm_object *, int))
                        NULL);

  if (new == NULL)
  {
    fprintf(stderr, "w_ko_more_1_new: tenm_object_new failed\n");
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
w_ko_more_1_act(tenm_object *my, const tenm_object *player)
{
  (my->count[0])++;

  if ((my->count[0] > 2200) && (my->count[2] > 30))
    return 1;

  if ((my->count[3] >= 4) && (my->count[0] < 2000))
  {
    my->count[1] = rand() % 4;
    my->count[2] = 0;
    my->count[3] = 0;
  }
  
  if ((my->count[2] <= 30) && (my->count[2] % 10 == 0))
  {
    switch (my->count[1])
    {
    case 0:
      tenm_table_add(normal_enemy_new(24.0, -19.0,
                                      BALL_SOLDIER, ENEMY_TYPE_WEAK,
                                      0, my->table_index, 3,
                                      my->table_index, 3, 1, 1,
                                      /* move 0 */
                                      9999, 0.0, 0.0, 0.0, 0.0,
                                      505.0, -19.0, 0.0, -0.05, 0,
                                      /* shoot 0 */
                                      9999, 99999, 0, 0, 1, 0));
      break;
    case 1:
      tenm_table_add(normal_enemy_new(616.0, -19.0,
                                      BALL_SOLDIER, ENEMY_TYPE_WEAK,
                                      0, my->table_index, 3,
                                      my->table_index, 3, 1, 1,
                                      /* move 0 */
                                      9999, 0.0, 0.0, 0.0, 0.0,
                                      135.0, -19.0, 0.0, 0.05, 0,
                                      /* shoot 0 */
                                      9999, 99999, 0, 0, 1, 0));
      break;
    case 2:
      tenm_table_add(normal_enemy_new(24.0, -19.0,
                                      BALL_SOLDIER, ENEMY_TYPE_WEAK,
                                      0, my->table_index, 3,
                                      my->table_index, 3, 1, 1,
                                      /* move 0 */
                                      9999, 0.0, 0.0, 0.0, 0.0,
                                      148.0, 74.0, 0.0, -0.15, 0,
                                      /* shoot 0 */
                                      9999, 99999, 0, 0, 1, 0));
      break;
    case 3:
      tenm_table_add(normal_enemy_new(616.0, -19.0,
                                      BALL_SOLDIER, ENEMY_TYPE_WEAK,
                                      0, my->table_index, 3,
                                      my->table_index, 3, 1, 1,
                                      /* move 0 */
                                      9999, 0.0, 0.0, 0.0, 0.0,
                                      492.0, 74.0, 0.0, 0.15, 0,
                                      /* shoot 0 */
                                      9999, 99999, 0, 0, 1, 0));
      break;
    default:
      fprintf(stderr, "w_ko_more_1: undefined mode (%d)\n", my->count[1]);
      break;
    }
  }
  (my->count[2])++;

  return 0;
}
