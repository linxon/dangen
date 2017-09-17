/* $Id: tadashi.c,v 1.132 2005/01/01 17:03:47 oohara Exp $ */
/* [normal] Tadashi */

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

#include "tadashi.h"

#define NEAR_ZERO 0.0001

static int tadashi_move(tenm_object *my, double turn_per_frame);
static int tadashi_hit(tenm_object *my, tenm_object *your);
static void tadashi_explode(tenm_object *my);
static int tadashi_act(tenm_object *my, const tenm_object *player);
static int tadashi_draw(tenm_object *my, int priority);
static int tadashi_green(const tenm_object *my);

static tenm_object *tadashi_square_new(double x, int what);
static int tadashi_square_act(tenm_object *my, const tenm_object *player);
static int tadashi_square_draw(tenm_object *my, int priority);

tenm_object *
tadashi_new(void)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = (double) (WINDOW_WIDTH / 2);
  double y = -160.0;

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "tadashi_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_circle_new(x, y, 30.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "tadashi_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 9);
  if (count == NULL)
  {
    fprintf(stderr, "tadashi_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 4);
  if (count_d == NULL)
  {
    fprintf(stderr, "tadashi_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] for deal_damage
   * [1] "damaged" timer
   * [2] life mode
   * [3] shoot timer
   * [4] move timer
   * [5] main attack base
   * [6] main attack direction
   * [7] decoration management
   * [8] "was green when killed" flag
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   * [2] main attack x
   * [3] main attack y
   */

  count[0] = 0;
  count[1] = 0;
  count[2] = 0;
  count[3] = 0;
  count[4] = 0;
  count[5] = 0;
  count[6] = 1;
  count[7] = 45;
  count[8] = 0;

  count_d[0] = 0.0;
  count_d[1] = (((double) (WINDOW_HEIGHT * 2 / 5)) - y) / 60.0;
  count_d[2] = 0.0;
  count_d[3] = 0.0;

  new = tenm_object_new("Tadashi", ATTR_BOSS, ATTR_PLAYER_SHOT,
                        750, x, y,
                        9, count, 4, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&tadashi_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&tadashi_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&tadashi_act),
                        (int (*)(tenm_object *, int))
                        (&tadashi_draw));
  if (new == NULL)
  {
    fprintf(stderr, "tadashi_new: tenm_object_new failed\n");
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
tadashi_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "tadashi_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "tadashi_move: strange turn_per_frame (%f)\n",
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
tadashi_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "tadashi_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "tadashi_hit: your is NULL\n");
    return 0;
  }

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;
  if (my->count[2] != 1)
    return 0;

  deal_damage(my, your, 0);
  if (tadashi_green(my))
    add_chain(my, your);
  my->count[1] = 2;

  if (my->hit_point <= 0)
  {
    add_score(30000);
    set_background(1);
    tadashi_explode(my);
    return 0;
  }

  return 0;
}

static void
tadashi_explode(tenm_object *my)
{
  int n;

  /* sanity check */
  if (my == NULL)
    return;

  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy_shot), 0);
  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy), 0);

  /* set "was green" flag before we change the life mode */
  if (tadashi_green(my))
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
  tenm_table_add(fragment_new(my->x, my->y + 80.0, 0.0, 0.0,
                              50.0, 12, n, 4.0, 0.0, 20));

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
tadashi_act(tenm_object *my, const tenm_object *player)
{
  double dx;
  double dy;
  double length;
  int theta;
  double speed;
  int i;
  int t;
  int phi;
  int what;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "tadashi_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  /* for deal_damage */
  my->count[0] = 0;

  /* "damaged" count down */
  if (my->count[1] > 0)
    (my->count[1])--;

  /* decoration management */
  if (my->count_d[1] > 2.0)
  {
    my->count[7] = 45;
  }
  else
  {
    (my->count[7])--;
  }
  if (my->count[7] > 45)
    my->count[7] = 45;
  if (my->count[7] < 10)
    my->count[7] = 10;

  (my->count[3])++;

  /* encounter */
  if (my->count[2] == 0)
  {
    if (my->count[3] == 60)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = (((double) (WINDOW_HEIGHT * 3 / 5)) - my->y) / 60.0;
    }
    if (my->count[3] >= 120)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
      my->count[2] = 1;
      my->count[3] = -30;
    }
    return 0;
  }

  /* dead */
  if (my->count[2] == 2)
  {
    if (tadashi_green(my))
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

  /* self-destruction */
  if ((my->count[2] == 1) && (my->count[3] >= 4050))
  {
    set_background(2);
    clear_chain();
    tadashi_explode(my);
    return 0;
  }
  
  /* spped change */
  if (my->count[4] < 0)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
    (my->count[4])++;
  }
  if ((my->count[4] == 0)
      && ((my->x - 100.0 > player->x) || (my->x + 100.0 < player->x)))
    my->count[4] = 60;
  if (my->count[4] > 0)
  {
    if (my->x - 6.0 > player->x)
      my->count_d[0] = -6.0;
    else if (my->x + 6.0 < player->x)
      my->count_d[0] = 6.0;
    else
      my->count_d[0] = player->x - my->x;
    (my->count[4])--;
    if (my->count[4] <= 0)
      my->count[4] = -60;
  }

  if (my->count[2] != 1)
    return 0;

  /* shoot */
  if (my->count[3] <= 980)
    t = my->count[3] - 630;
  else if (my->count[3] <= 1960)
    t = my->count[3] - 1610;
  else if (my->count[3] <= 2940)
    t = my->count[3] - 2590;
  else
    t = my->count[3] - 3570;

  if ((my->count[3] >= 0) && (my->count[3] < 3920)
      && ((t < 0) || (t >= 350))
      && (my->count[3] % 100 == 60))
  {
    if (my->count[3] >= 630)
      what = 1;
    else
      what = 0;
    tenm_table_add(tadashi_square_new(my->x, what));
  }

  if ((my->count[3] >= 0) && (my->count[3] < 3920)
      && ((t < 0) || (t >= 350))
      && (my->count[3] % 10 == 0) && (my->count[3] % 70 <= 20))
  {
    tenm_table_add(normal_shot_point_new(my->x, my->y, 5.0,
                                         player->x + 100.0, player->y - 50.0,
                                         3));
    tenm_table_add(normal_shot_point_new(my->x, my->y, 5.0,
                                         player->x - 100.0, player->y - 50.0,
                                         3));
  }
  if ((my->count[3] >= 0) && (my->count[3] < 3920)
      && ((t < 0) || (t >= 350))
      && (my->count[3] % 70 == 40))
  {
    tenm_table_add(normal_shot_point_new(my->x, my->y, 5.0,
                                         player->x, player->y,
                                         3));
  }

  /* main attack */
  if (t == 0)
  {
    my->count[5] = rand() % 360;
    if (rand() % 2 == 0)
      my->count[6] = 1;
    else
      my->count[6] = -1;
  }
  if ((t >= 50) && (t < 300))
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
    my->count[4] = -60;
  }
  if ((t >= 0) && (t <= 100))
  {
    my->count_d[2] = player->x;
    my->count_d[3] = player->y;
  }
  if ((t >= 120) && (t < 300) && (t % 3 == 0))
  {

    dx = my->count_d[2] - my->x;
    dy = my->count_d[3] - my->y;
    length = tenm_sqrt((int) (dx * dx + dy * dy));
    if (length < NEAR_ZERO)
      length = 1.0;

    if (length < 100.0)
      theta = 75;
    else if (length > 400.0)
      theta = 15;
    else
      theta = 75 - ((int) ((length - 100.0) / 5.0));
    if (theta < 0)
      theta = 1;
    if (theta >= 90)
      theta = 89;

    for (i = 0; i < 2; i++)
    { 
      speed = 6.0 + ((double) i) * 1.0;
      phi = (my->count[5] + t * 7 + i * 7) * my->count[6];
      tenm_table_add(normal_shot_new(my->x, my->y,
                                     dx * speed / length
                                     - (dy * speed / length)
                                     * (tenm_sin(theta) / tenm_cos(theta))
                                     * tenm_sin(phi),
                                     dy * speed / length
                                     + (dx  * speed / length)
                                     * (tenm_sin(theta) / tenm_cos(theta))
                                     * tenm_sin(phi),
                                     4, -2, 0));
    }
  }

  return 0;
}

static int
tadashi_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  char temp[32];
  double dx;
  double dy;
  double length;
  double v[2];
  double result[2];
  int theta;
  int i;
  int t;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "tadashi_draw: my is NULL\n");
    return 0;
  }

  /* decoration */
  if ((priority == 0) && (my->count[2] <= 1))
  {
    if (tadashi_green(my))
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

    if (my->count_d[0] < -0.5)
      theta = 85;
    else if (my->count_d[0] > 0.5)
      theta = 95;
    else
      theta = 90;
    if (tenm_draw_line((int) (my->x + 50.0),
                       (int) (my->y + 40.0),
                       (int) (my->x + 50.0
                              + 120.0 * tenm_cos(theta - my->count[7])),
                       (int) (my->y + 40.0
                              + 120.0 * tenm_sin(theta - my->count[7])),
                       1, color) != 0)
      status = 1;
    if (tenm_draw_line((int) (my->x - 50.0),
                       (int) (my->y + 40.0),
                       (int) (my->x - 50.0
                              + 120.0 * tenm_cos(theta + my->count[7])),
                       (int) (my->y + 40.0
                              + 120.0 * tenm_sin(theta + my->count[7])),
                       1, color) != 0)
      status = 1;
    if (tenm_draw_line((int) (my->x + 5.0),
                       (int) (my->y + 35.0),
                       (int) (my->x + 5.0 + 30.0 * tenm_cos(85)),
                       (int) (my->y + 35.0 + 30.0 * tenm_sin(85)),
                       1, color) != 0)
      status = 1;
    if (tenm_draw_line((int) (my->x - 5.0),
                       (int) (my->y + 35.0),
                       (int) (my->x - 5.0 + 30.0 * tenm_cos(95)),
                       (int) (my->y + 35.0 + 30.0 * tenm_sin(95)),
                       1, color) != 0)
      status = 1;
  }
  
  /* main attack */
  if (my->count[3] <= 980)
    t = my->count[3] - 630;
  else if (my->count[3] <= 1960)
    t = my->count[3] - 1610;
  else if (my->count[3] <= 2940)
    t = my->count[3] - 2590;
  else
    t = my->count[3] - 3570;

  if ((priority == 0) && (my->count[2] == 1) && (t >= 0) && (t < 300))
  {
    dx = my->count_d[2] - my->x;
    dy = my->count_d[3] - my->y;
    length = tenm_sqrt((int) (dx * dx + dy * dy));
    if (length < NEAR_ZERO)
      length = 1.0;
    if (t < 100)
      color = tenm_map_color(158, 158, 158);
    else
      color = tenm_map_color(118, 99, 158);
    if (tenm_draw_line((int) (my->x),
                       (int) (my->y),
                       (int) (my->x + dx * 800.0 / length),
                       (int) (my->y + dy * 800.0 / length),
                       1, color) != 0)
      status = 1;

    if (length < 100.0)
      theta = 75;
    else if (length > 400.0)
      theta = 15;
    else
      theta = 75 - ((int) ((length - 100.0) / 5.0));
    if (theta < 0)
      theta = 1;
    if (theta >= 90)
      theta = 89;
    for (i = -1; i <= 1; i += 2)
    {
      v[0] = dx;
      v[1] = dy;
      result[0] = dx;
      result[1] = dy;
      vector_rotate(result, v, theta * i);
      if (tenm_draw_line((int) (my->x),
                         (int) (my->y),
                         (int) (my->x + result[0] * 800.0 / length),
                         (int) (my->y + result[1] * 800.0 / length),
                         1, color) != 0)
        status = 1;
    }
  }

  /* body */
  /* dead enemy has low priority */
  if (((my->count[2] <= 1) && (priority == 0))
      || ((my->count[2] > 1) && (priority == -1)))
  {
    if (tadashi_green(my))
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
  if ((priority == 0) && (my->count[2] == 1))
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 10, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "tadashi_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

/* return 1 (true) or 0 (false) */
static int
tadashi_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if ((my->count[2] == 1) && (my->count[3] > 980) && (my->count[3] < 4020))
    return 1;
  if ((my->count[2] == 2) && (my->count[8] != 0))
    return 1;

  return 0;
}

static
tenm_object *tadashi_square_new(double x, int what)
{
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double y = (double) (WINDOW_HEIGHT * 3 / 4);

  count = (int *) malloc(sizeof(int) * 7);
  if (count == NULL)
  {
    fprintf(stderr, "tadashi_square_new: malloc(count) failed\n");
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "tadashi_square_new: malloc(count_d) failed\n");
    free(count);
    return NULL;
  }

  /* list of count
   * [0] color (for delete_enemy_shot)
   * [1] move mode
   * [2] move timer
   * [3] rotate theta
   * [4] rotate direction
   * [5] shoot direction
   * [6] what
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */

  count[0] = 6;
  count[1] = 0;
  count[2] = 0;
  count[3] = rand() % 360;
  if (rand() % 2 == 0)
    count[4] = 1;
  else
    count[4] = -1;
  count[5] = 60 + rand() % 61;
  count[6] = what;

  count_d[0] = 0.0;
  count_d[1] = 0.0;

  /* ATTR_ENEMY_SHOT is only to clear it when the player is killed */
  new = tenm_object_new("Tadashi square", ATTR_ENEMY_SHOT, 0,
                        1, x, y,
                        6, count, 2, count_d, 0, NULL,
                        (int (*)(tenm_object *, double))
                        NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&tadashi_square_act),
                        (int (*)(tenm_object *, int))
                        (&tadashi_square_draw));
  if (new == NULL)
  {
    fprintf(stderr, "tadashi_square_new: tenm_object_new failed\n");
    if (count_d != NULL)
      free(count_d);
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
tadashi_square_act(tenm_object *my, const tenm_object *player)
{
  double x;
  double y;
  double dx_a;
  double dy_a;
  double dx_b;
  double dy_b;
  int i;
  int j;
  int theta;
  int phi;
  double length;
  double c;
  double s;
  double speed;
  int n;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "tadashi_square_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  /* no need to interpolate */
  my->x += my->count_d[0];
  my->y += my->count_d[1];

  (my->count[2])++;
  if (my->count[1] == 0)
  {
    if (my->count[2] >= 60)
    {
      my->count[1] = 1;
      my->count[2] = 0;
      my->count_d[0] = 0.0;
      my->count_d[1] = -3.0;
    }
  }
  else if (my->count[1] == 1)
  {
    if (my->count[2] == 80)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[2] >= 140)
    {
      length = 175.0;
      phi = 60;
      theta = 180 + 720 * my->count[4];
      theta += my->count[3];
      x = length * tenm_cos(theta);
      y = length * tenm_sin(theta) * tenm_cos(phi);
      dx_a = length * tenm_cos(theta + 90) - x;
      dy_a = length * tenm_sin(theta + 90) * tenm_cos(phi) - y;
      dx_b = length * tenm_cos(theta - 90) - x;
      dy_b = length * tenm_sin(theta - 90) * tenm_cos(phi) - y;

      for (i = 0; i <= 3; i++)
        for (j = 0; j <= 3; j++)
        {
          if (((i == 0) || (i == 3)) && ((j == 0) || (j == 3)))
            continue;
          c = ((double) i) / 3.0;
          s = ((double) j) / 3.0;
          if ((i + j) % 2 == 0)
          {  
            speed = 4.0;
            n = 0;
          }
          else
          {  
            speed = 6.0;
            n = 1;
          }
          if (my->y < player->y)
            tenm_table_add(laser_angle_new(my->x + x + dx_a * s + dx_b * c
                                           - 15.0 * tenm_cos(my->count[5]),
                                           my->y + y + dy_a * s + dy_b * c
                                           - 15.0 * tenm_sin(my->count[5]),
                                           speed, my->count[5], 30.0, n));
          else
            tenm_table_add(laser_angle_new(my->x + x + dx_a * s + dx_b * c
                                           + 15.0 * tenm_cos(my->count[5]),
                                           my->y + y + dy_a * s + dy_b * c
                                           + 15.0 * tenm_sin(my->count[5]),
                                           speed, 180 + my->count[5],
                                           30.0, n));
        }

      if (my->count[6] == 0)
        return 1;
      my->count[1] = 2;
      my->count[2] = 0;
      my->count_d[0] = 0.0;
      my->count_d[1] = ((double) (WINDOW_HEIGHT / 2) - my->y) / 90.0;
    }
  }
  else
  {
    if (my->count[2] == 90)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[2] >= 120)
    {
      length = 400.0;
      phi = 0;
      theta = 180 - 90 * my->count[4];
      theta += my->count[3];
      x = length * tenm_cos(theta);
      y = length * tenm_sin(theta) * tenm_cos(phi);
      dx_a = length * tenm_cos(theta + 90) - x;
      dy_a = length * tenm_sin(theta + 90) * tenm_cos(phi) - y;
      dx_b = length * tenm_cos(theta - 90) - x;
      dy_b = length * tenm_sin(theta - 90) * tenm_cos(phi) - y;

      for (i = 0; i < 5; i++)
        for (j = 0; j < 5; j++)
        {
          if ((i + j) % 2 == 0)
            continue;
          c = ((double) i) / 5.0;
          s = ((double) j) / 5.0;
          tenm_table_add(laser_point_new(my->x + x + dx_a * s + dx_b * c,
                                         my->y + y + dy_a * s + dy_b * c,
                                         2.0,
                                         my->x + x + dx_a * s + dx_b * c
                                         + dx_a * 50.0 / 400.0,
                                         my->y + y + dy_a * s + dy_b * c
                                         + dy_a * 50.0 / 400.0,
                                         50.0, 0));
          tenm_table_add(laser_point_new(my->x + x + dx_a * s + dx_b * c,
                                         my->y + y + dy_a * s + dy_b * c,
                                         3.0,
                                         my->x + x + dx_a * s + dx_b * c
                                         + dx_b * 50.0 / 400.0,
                                         my->y + y + dy_a * s + dy_b * c
                                         + dy_b * 50.0 / 400.0,
                                         50.0, 1));
        }
    }
    if (my->count[2] >= 120)
    {
      return 1;
    }
  }

  return 0;
}

static int
tadashi_square_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  double x;
  double y;
  double dx_a;
  double dy_a;
  double dx_b;
  double dy_b;
  int i;
  int j;
  int theta;
  int phi;
  double length;
  double c;
  double s;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "tadashi_square_draw: my is NULL\n");
    return 0;
  }

  /* return if it is not my turn */
  if (priority != 0)
    return 0;

  if (my->count[1] == 0)
  {
    length = 50.0;
    phi = 60;
    theta = my->count[2] * 3 * my->count[4];
  }
  else if (my->count[1] == 1)
  {
    if (my->count[2] < 80)
    {
      length = 55.0 + ((double) (my->count[2])) * 1.5;
      phi = 60;
      theta = 180 + my->count[2] * 9 * my->count[4];
    }
    else
    {
      length = 175.0;
      phi = 60;
      theta = 180 + 720 * my->count[4];
    }
  }
  else
  {
    if (my->count[2] < 90)
    {
      length = 175.0 + ((double) (my->count[2])) * 2.5;
      if (my->count[2] < 60)
        phi = 60 - my->count[2];
      else
        phi = 0;
      theta = 180 + 720 * my->count[4] - my->count[2] * 9 * my->count[4];
    }
    else
    {
      length = 400.0;
      phi = 0;
      theta = 180 - 90 * my->count[4];
    }
  }
  theta += my->count[3];

  x = length * tenm_cos(theta);
  y = length * tenm_sin(theta) * tenm_cos(phi);
  dx_a = length * tenm_cos(theta + 90) - x;
  dy_a = length * tenm_sin(theta + 90) * tenm_cos(phi) - y;
  dx_b = length * tenm_cos(theta - 90) - x;
  dy_b = length * tenm_sin(theta - 90) * tenm_cos(phi) - y;

  if ((my->count[1] == 2) && (my->count[2] >= 100))
    color = tenm_map_color(206, 201, 175);
  else
    color = tenm_map_color(158, 158, 158);
  if (tenm_draw_line((int) (my->x + x), (int) (my->y + y),
                     (int) (my->x + x + dx_a), (int) (my->y + y + dy_a),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x + x + dx_b), (int) (my->y + y + dy_b),
                     (int) (my->x + x + dx_a + dx_b),
                     (int) (my->y + y + dy_a + dy_b),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x + x), (int) (my->y + y),
                     (int) (my->x + x + dx_b), (int) (my->y + y + dy_b),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x + x + dx_a), (int) (my->y + y + dy_a),
                     (int) (my->x + x + dx_a + dx_b),
                     (int) (my->y + y + dy_a + dy_b),
                     1, color) != 0)
    status = 1;

  if ((my->count[1] == 1) && (my->count[2] > 80))
  {
    color = tenm_map_color(158, 158, 158);
    if (my->count[2] > 110)
      s = 1.0;
    else
      s = ((double) (my->count[2] - 80)) / 30.0;
    c = 1.0 / 3.0;
    if (tenm_draw_line((int) (my->x + x + dx_b * c),
                       (int) (my->y + y + dy_b * c),
                       (int) (my->x + x + dx_a * s + dx_b * c),
                       (int) (my->y + y + dy_a * s + dy_b * c),
                       1, color) != 0)
      status = 1;
    if (tenm_draw_line((int) (my->x + x + dx_a + dx_b * (1.0 - c)),
                       (int) (my->y + y + dy_a + dy_b * (1.0 - c)),
                       (int) (my->x + x + dx_a * (1.0 - s) + dx_b * (1.0 - c)),
                       (int) (my->y + y + dy_a * (1.0 - s) + dy_b * (1.0 - c)),
                       1, color) != 0)
      status = 1;
    if (tenm_draw_line((int) (my->x + x + dx_a * c),
                       (int) (my->y + y + dy_a * c),
                       (int) (my->x + x + dx_b * s + dx_a * c),
                       (int) (my->y + y + dy_b * s + dy_a * c),
                       1, color) != 0)
      status = 1;
    if (tenm_draw_line((int) (my->x + x + dx_b + dx_a * (1.0 - c)),
                       (int) (my->y + y + dy_b + dy_a * (1.0 - c)),
                       (int) (my->x + x + dx_b * (1.0 - s) + dx_a * (1.0 - c)),
                       (int) (my->y + y + dy_b * (1.0 - s) + dy_a * (1.0 - c)),
                       1, color) != 0)
      status = 1;
  }
  if ((my->count[1] == 1) && (my->count[2] > 110))
  {
    for (i = 0; i <= 3; i++)
      for (j = 0; j <= 3; j++)
      {
        if (((i == 0) || (i == 3)) && ((j == 0) || (j == 3)))
          continue;
        c = ((double) i) / 3.0;
        s = ((double) j) / 3.0;
        if ((i + j) % 2 == 0)
          color = tenm_map_color(99, 158, 114);
        else
          color = tenm_map_color(99, 158, 138);
        if (tenm_draw_line((int) (my->x + x + dx_a * s + dx_b * c
                                  - 15.0 * tenm_cos(my->count[5])),
                           (int) (my->y + y + dy_a * s + dy_b * c
                                  - 15.0 * tenm_sin(my->count[5])),
                           (int) (my->x + x + dx_a * s + dx_b * c
                                  + 15.0 * tenm_cos(my->count[5])),
                           (int) (my->y + y + dy_a * s + dy_b * c
                                  + 15.0 * tenm_sin(my->count[5])),
                           1, color) != 0)
          status = 1;
      }
  }
  if ((my->count[1] == 2) && (my->count[2] < 100))
  {
    color = tenm_map_color(206, 201, 175);
    for (i = 0; i < 4; i++)
    {
      if (my->count[2] >= 90)
      {  
        c = ((double) (i + 1)) / 5.0;
      }
      else
      {
        c = ((double) (i + 1)) / 5.0;
        if (i < 2)
          s = 1.0 / 3.0;
        else
          s = 2.0 / 3.0;
        c = c * (double) (my->count[2]) + s * (double) (90 - my->count[2]);
        c /= 90.0;
      }

      if (tenm_draw_line((int) (my->x + x + dx_b * c),
                         (int) (my->y + y + dy_b * c),
                         (int) (my->x + x + dx_a + dx_b * c),
                         (int) (my->y + y + dy_a + dy_b * c),
                         1, color) != 0)
        status = 1;
      if (tenm_draw_line((int) (my->x + x + dx_a * c),
                         (int) (my->y + y + dy_a * c),
                         (int) (my->x + x + dx_b + dx_a * c),
                         (int) (my->y + y + dy_b + dy_a * c),
                         1, color) != 0)
        status = 1;
    }
  }
  if ((my->count[1] == 2) && (my->count[2] > 90))
  {
    for (i = 0; i < 5; i++)
      for (j = 0; j < 5; j++)
      {
        if ((i + j) % 2 == 0)
          continue;
        c = ((double) i) / 5.0;
        s = ((double) j) / 5.0;
        color = tenm_map_color(99, 158, 114);
        if (tenm_draw_line((int) (my->x + x + dx_a * s + dx_b * c),
                           (int) (my->y + y + dy_a * s + dy_b * c),
                           (int) (my->x + x + dx_a * s + dx_b * c
                                  + dx_a * 50.0 / (400.0 * tenm_sqrt(2))),
                           (int) (my->y + y + dy_a * s + dy_b * c
                                  + dy_a * 50.0 / (400.0 * tenm_sqrt(2))),
                           1, color) != 0)
          status = 1;
        color = tenm_map_color(99, 158, 138);
        if (tenm_draw_line((int) (my->x + x + dx_a * s + dx_b * c),
                           (int) (my->y + y + dy_a * s + dy_b * c),
                           (int) (my->x + x + dx_a * s + dx_b * c
                                  + dx_b * 50.0 / (400.0 * tenm_sqrt(2))),
                           (int) (my->y + y + dy_a * s + dy_b * c
                                  + dy_b * 50.0 / (400.0 * tenm_sqrt(2))),
                           1, color) != 0)
          status = 1;
      }
  }

  return status;
}

