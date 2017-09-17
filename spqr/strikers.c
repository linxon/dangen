/* $Id: strikers.c,v 1.119 2005/05/27 00:36:07 oohara Exp $ */
/* [easy] Strikers 1341 */

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

#include "strikers.h"

static int strikers_act(tenm_object *my, const tenm_object *player);
static int strikers_draw(tenm_object *my, int priority);
static int strikers_striker_signal(tenm_object *my, int n);

static tenm_object *striker_505_new(int table_index);
static int striker_505_move(tenm_object *my, double turn_per_frame);
static int striker_505_hit(tenm_object *my, tenm_object *your);
static void striker_505_next(tenm_object *my);
static int striker_505_act(tenm_object *my, const tenm_object *player);
static int striker_505_draw(tenm_object *my, int priority);
static int striker_505_draw_bit(double x, double y, tenm_color color);
static int striker_505_green(const tenm_object *my);

static tenm_object *striker_446_new(int table_index);
static int striker_446_move(tenm_object *my, double turn_per_frame);
static int striker_446_hit(tenm_object *my, tenm_object *your);
static void striker_446_next(tenm_object *my);
static int striker_446_act(tenm_object *my, const tenm_object *player);
static int striker_446_draw(tenm_object *my, int priority);
static int striker_446_draw_bit(double x, double y, tenm_color color);
static int striker_446_green(const tenm_object *my);

static tenm_object *striker_1341_new(int table_index);
static int striker_1341_move(tenm_object *my, double turn_per_frame);
static int striker_1341_hit(tenm_object *my, tenm_object *your);
static void striker_1341_next(tenm_object *my);
static int striker_1341_act(tenm_object *my, const tenm_object *player);
static int striker_1341_draw(tenm_object *my, int priority);
static int striker_1341_draw_bit(double x, double y, tenm_color color);
static int striker_1341_green(const tenm_object *my);

tenm_object *
strikers_new(void)
{
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;

  count = (int *) malloc(sizeof(int) * 8);
  if (count == NULL)
  {
    fprintf(stderr, "strikers_new: malloc(count) failed\n");
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 4);
  if (count_d == NULL)
  {
    fprintf(stderr, "strikers_new: malloc(count_d) failed\n");
    free(count);
    return NULL;
  }
  /* list of count
   * [0] number of strikers killed
   * [1] timer
   */
  /* list of count_d
   */

  count[0] = 0;
  count[1] = -30;

  new = tenm_object_new("Strikers 1341 manager", 0, 0,
                        1, 0.0, 0.0,
                        8, count, 4, count_d, 0, NULL,
                        (int (*)(tenm_object *, double))
                        NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&strikers_act),
                        (int (*)(tenm_object *, int))
                        (&strikers_draw));

  if (new == NULL)
  {
    fprintf(stderr, "strikers_new: tenm_object_new failed\n");
    if (count_d != NULL)
      free(count_d);
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

static int
strikers_act(tenm_object *my, const tenm_object *player)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "strikers_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[1])++;

  if ((my->count[0] == 0) && (my->count[1] == 180))
    tenm_table_add(striker_505_new(my->table_index));
  if ((my->count[0] == 1) && (my->count[1] == 180))
    tenm_table_add(striker_446_new(my->table_index));
  if ((my->count[0] == 2) && (my->count[1] == 30))
    tenm_table_add(striker_1341_new(my->table_index));

  if (my->count[0] >= 3)
  {
    if (my->count[1] >= 0)
    {
      tenm_table_add(stage_clear_new(100));
      return 1;
    }
  }

  return 0;
}

static int
strikers_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  int i;
  int j;
  int theta;
  double x;
  double y;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "strikers_draw: my is NULL\n");
    return 0;
  }

  if (priority != -1)
    return 0;
  if (my->count[0] >= 2)
    return 0;
  if ((my->count[1] < 0) || (my->count[1] >= 150))
    return 0;

  for (i = 0; i < 3; i++)
  {
    if (i > 2 - my->count[0])
      continue;

    theta = -10;
    color = tenm_map_color(182, 147, 123);

    if (my->count[1] < 50)
    {
      x = -15.0 + 6.0 * tenm_cos(-10) * ((double) (my->count[1]));
      y = 300.0 + 6.0 * tenm_sin(-10) * ((double) (my->count[1]));
    }
    else if (my->count[1] < 100)
    {
      x = -15.0 + 3.0 * tenm_cos(-10) * ((double) (my->count[1] + 50));
      y = 300.0 + 3.0 * tenm_sin(-10) * ((double) (my->count[1] + 50));
    }
    else if (i == 2 - my->count[0])
    {
      x = -15.0 + 6.0 * tenm_cos(-10) * 75.0
        + 250.0 * tenm_cos(-100);
      y = 300.0 + 6.0 * tenm_sin(-10) * 75.0
        + 250.0 * tenm_sin(-100);
      x += 250.0 * tenm_cos(80 - (my->count[1] - 100) * 3);
      y += 250.0 * tenm_sin(80 - (my->count[1] - 100) * 3);
      theta -= (my->count[1] - 100) * 3;
      color = tenm_map_color(95, 47, 13);
    }
    else
    {
      x = -15.0 + 6.0 * tenm_cos(-10) * ((double) (my->count[1] - 25));
      y = 300.0 + 6.0 * tenm_sin(-10) * ((double) (my->count[1] - 25));
    }

    if (i == 1)
    {
      x += 45.0 * tenm_cos(-160);
      y += 45.0 * tenm_sin(-160);
    }
    else if (i == 2)
    {
      x += 45.0 * tenm_cos(140);
      y += 45.0 * tenm_sin(140);
    }

    if (my->count[0] == 1)
    {  
      x = ((double) WINDOW_WIDTH) - x;
      theta = 180 - theta;
    }

    for (j = 0; j < 360; j += 120)
      if (tenm_draw_line((int) (x + 15.0 * tenm_cos(theta + j)),
                         (int) (y + 15.0 * tenm_sin(theta + j)),
                         (int) (x + 15.0 * tenm_cos(theta + 120 + j)),
                         (int) (y + 15.0 * tenm_sin(theta + 120 + j)),
                         1, color) != 0)
        status = 1;
  }

  return status;
}

static int
strikers_striker_signal(tenm_object *my, int n)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (strcmp(my->name, "Strikers 1341 manager") != 0)
    return 0;

  (my->count[0])++;
  my->count[1] = -30;

  return 0;
}

static tenm_object *
striker_505_new(int table_index)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = (double) (WINDOW_WIDTH / 2);
  double y = -59.0;

  /* sanity check */
  if (table_index < 0)
  {
    fprintf(stderr, "striker_505_new: table_index is negative (%d)\n",
            table_index);
    return NULL;
  }

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "striker_505_new: malloc(p) failed\n");
    return NULL;
  }
  p[0] = (tenm_primitive *) tenm_polygon_new(3,
                                             x + 51.9615, y - 30.0,
                                             x, y + 60.0,
                                             x - 51.9615, y - 30.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "striker_505_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 9);
  if (count == NULL)
  {
    fprintf(stderr, "striker_505_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "striker_505_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] for deal_damage
   * [1] "damaged" timer
   * [2] manager index
   * [3] life mode
   * [4] life timer
   * [5] move mode
   * [6] move timer
   * [7] bit theta
   * [8] "was green when killed" flag
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */

  count[0] = 0;
  count[1] = 0;
  count[2] = table_index;
  count[3] = 0;
  count[4] = 0;
  count[5] = 0;
  count[6] = 0;
  count[7] = 0;
  count[8] = 0;

  count_d[0] = 0.0;
  count_d[1] = (34.0 - y) / 30.0;

  new = tenm_object_new("Striker 505", ATTR_BOSS, ATTR_PLAYER_SHOT,
                        505, x, y,
                        9, count, 2, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&striker_505_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&striker_505_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&striker_505_act),
                        (int (*)(tenm_object *, int))
                        (&striker_505_draw));
  if (new == NULL)
  {
    fprintf(stderr, "striker_505_new: tenm_object_new failed\n");
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
striker_505_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "striker_505_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "striker_505_move: strange turn_per_frame (%f)\n",
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
striker_505_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "striker_505_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "striker_505_hit: your is NULL\n");
    return 0;
  }

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;
  if (my->count[3] != 1)
    return 0;

  deal_damage(my, your, 0);
  if (striker_505_green(my))
    add_chain(my, your);
  my->count[1] = 2;

  if (my->hit_point <= 0)
  {
    add_score(3000);
    set_background(1);
    striker_505_next(my);
    return 0;
  }

  return 0;
}

static void
striker_505_next(tenm_object *my)
{
  int n;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "striker_505_next: my is NULL\n");
    return;
  }

  /* set "was green" flag before we change the life mode */
  if (striker_505_green(my))
  {
    n = 8;
    my->count[8] = 1;
  }
  else
  {
    n = 7;
    my->count[8] = 0;
  }

  tenm_table_add(explosion_new(my->x, my->y,
                               0.0, 0.0,
                               1, 3000, n, 6.0, 8));
  tenm_table_add(explosion_new(my->x, my->y,
                               0.0, 0.0,
                               2, 800, n, 3.5, 8));

  my->count[1] = 0;
  my->count[3] = 2;
  my->count[4] = 0;
  my->count_d[0] = 0.0;
  my->count_d[1] = 0.0;

  tenm_table_apply(my->count[2],
                   (int (*)(tenm_object *, int)) (&strikers_striker_signal),
                   0);

  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy_shot), 0);
  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy), 0);
}


static int
striker_505_act(tenm_object *my, const tenm_object *player)
{
  int n;
  double speed;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "striker_505_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  /* for deal_damage */
  my->count[0] = 0;

  /* "damaged" count down */
  if (my->count[1] > 0)
    (my->count[1])--;

  (my->count[4])++;
  /* encounter */
  if (my->count[3] == 0)
  {
    if (my->count[4] == 30)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[4] >= 90)
    {
      my->count[3] = 1;
      my->count[4] = 0;
    }
    return 0;
  }

  /* dead */
  if (my->count[3] == 2)
  {
    if ((my->count[4] <= 9) && (my->count[4] % 3 == 0))
    {
      if (striker_505_green(my))
        n = 8;
      else
        n = 7;
      tenm_table_add(explosion_new(my->x + ((double) (my->count[4]/3)) * 60.0,
                                   my->y,
                                   0.0, 0.0,
                                   1, 200, n, 2.0, 8));
      tenm_table_add(explosion_new(my->x - ((double) (my->count[4]/3)) * 60.0,
                                   my->y,
                                   0.0, 0.0,
                                   1, 200, n, 2.0, 8));
    }
    if (my->count[4] >= 9)
      return 1;

    return 0;
  }
  
  /* self-destruction */
  if ((my->count[3] == 1) && (my->count[4] >= 1075))
  {
    set_background(2);
    clear_chain();
    striker_505_next(my);
    return 0;
  }

  /* move */
  speed = 6.0;
  if (my->count[7] == 75)
    speed = 3.0;
  if ((my->x + 120.0 > player->x) && (my->x - 120.0 < player->x)
      && (my->y + 60.0 < player->y))
  {
    my->count[7] += 15;
    if (my->count[7] > 75)
      my->count[7] = 75;
    speed = 3.0;
  }
  else
  {
    my->count[7] -= 15;
    if (my->count[7] < 0)
      my->count[7] = 0;
  }

  (my->count[6])++;
  if (my->count[5] == 0)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
    if ((my->count[6] >= 0)
        && ((my->x + speed * 15.0 < player->x)
            || (my->x - speed * 15.0 > player->x)))
    {
      my->count[5] = 1;
      my->count[6] = 0;
    }
    else if ((my->count[6] >= 0)
        && ((my->x + speed < player->x)
            || (my->x - speed > player->x)))
    {
      my->count[5] = 1;
      my->count[6] = 50;
    }
  }
  else if (my->count[5] == 1)
  {
    my->count_d[1] = 0.0;
    if (my->x - speed > player->x)
      my->count_d[0] = -speed;
    else if (my->x + speed < player->x)
      my->count_d[0] = speed;
    else
      my->count_d[0] = player->x - my->x;
    if (my->count[6] >= 60)
    {
      my->count[5] = 0;
      my->count[6] = -60;
    }
  }

  if (my->count[4] <= 95)
  {
    my->count[5] = 0;
    my->count[6] = 0;
    my->count[7] = 0;
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
  }

  if ((my->count[3] != 1) || (my->count[4] > 950))
    return 0;

  /* shoot */
  if ((my->count[7] == 0) && (my->count[4] % 19 == 0))
  {
    tenm_table_add(normal_shot_point_new(my->x + 100.0*tenm_cos(my->count[7]),
                                         my->y + 100.0*tenm_sin(my->count[7]),
                                         5.5,
                                         player->x, player->y, 2));
    tenm_table_add(normal_shot_point_new(my->x - 100.0*tenm_cos(my->count[7]),
                                         my->y + 100.0*tenm_sin(my->count[7]),
                                         5.5,
                                         player->x, player->y, 2));

    tenm_table_add(normal_shot_angle_new(my->x + 30.0, my->y, 4.5,
                                         75, 0));
    tenm_table_add(normal_shot_angle_new(my->x + 30.0, my->y, 4.5,
                                         90, 0));
    tenm_table_add(normal_shot_angle_new(my->x - 30.0, my->y, 4.5,
                                         90, 0));
    tenm_table_add(normal_shot_angle_new(my->x - 30.0, my->y, 4.5,
                                         105, 0));
  }

  if ((my->count[7] == 75) && (my->count[4] % 19 == 0))
  {
    tenm_table_add(normal_shot_angle_new(my->x + 100.0*tenm_cos(my->count[7]),
                                         my->y + 100.0*tenm_sin(my->count[7]),
                                         4.5,
                                         70, 3));
    tenm_table_add(normal_shot_angle_new(my->x + 100.0*tenm_cos(my->count[7]),
                                         my->y + 100.0*tenm_sin(my->count[7]),
                                         4.5,
                                         100, 3));
    tenm_table_add(normal_shot_angle_new(my->x - 100.0*tenm_cos(my->count[7]),
                                         my->y + 100.0*tenm_sin(my->count[7]),
                                         4.5,
                                         110, 3));
    tenm_table_add(normal_shot_angle_new(my->x - 100.0*tenm_cos(my->count[7]),
                                         my->y + 100.0*tenm_sin(my->count[7]),
                                         4.5,
                                         80, 3));

    tenm_table_add(normal_shot_angle_new(my->x, my->y, 5.5,
                                         75, 1));
    tenm_table_add(normal_shot_angle_new(my->x, my->y, 5.5,
                                         90, 1));
    tenm_table_add(normal_shot_angle_new(my->x, my->y, 5.5,
                                         105, 1));
  }

  return 0;
}

static int
striker_505_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  char temp[32];
  double c;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "striker_505_draw: my is NULL\n");
    return 0;
  }

  /* decoration */
  if (priority == 0)
  {
    if (striker_505_green(my))
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
  }

  /* body */
  if (priority == 0)
  {
    if (striker_505_green(my))
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

    if (((my->count[3] == 0) && (my->count[4] >= 45))
        || (my->count[3] == 1))
    {
      if (my->count[3] == 0)
      {
        if (my->count[4] >= 75)
          c = 100.0;
        else
          c = 100.0 * ((double) (my->count[4] - 45)) / 30.0;
      }
      else
      {
        c = 100.0;
      }

      if (striker_505_draw_bit(my->x + c * tenm_cos(my->count[7]),
                               my->y + c * tenm_sin(my->count[7]),
                               color) != 0)
        status = 1;
      if (striker_505_draw_bit(my->x - c * tenm_cos(my->count[7]),
                               my->y + c * tenm_sin(my->count[7]),
                               color) != 0)
        status = 1;
    }
    
    if (my->count[3] <= 1)
    { 
      if (tenm_draw_line((int) (my->x + 51.9615), (int) (my->y - 30.0),
                         (int) (my->x), (int) (my->y + 60.0),
                         3, color) != 0)
        status = 1;
      if (tenm_draw_line((int) (my->x), (int) (my->y + 60.0),
                         (int) (my->x - 51.9615), (int) (my->y - 30.0),
                         3, color) != 0)
        status = 1;
      if (tenm_draw_line((int) (my->x - 51.9615), (int) (my->y - 30.0),
                         (int) (my->x + 51.9615), (int) (my->y - 30.0),
                         3, color) != 0)
        status = 1;
    }
  }

  /* hit point stat */
  if ((priority == 0) && (my->count[3] == 1))
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 10, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "striker_505_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

static int
striker_505_draw_bit(double x, double y, tenm_color color)
{
  int status = 0;

  if (tenm_draw_line((int) (x + 25.9808), (int) (y - 15.0),
                     (int) (x + 8.6603), (int) (y + 15.0),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (x + 8.6603), (int) (y + 15.0),
                     (int) (x - 8.6603), (int) (y + 15.0),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (x - 8.6603), (int) (y + 15.0),
                     (int) (x - 25.9808), (int) (y - 15.0),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (x - 25.9808), (int) (y - 15.0),
                     (int) (x + 25.9808), (int) (y - 15.0),
                     1, color) != 0)
    status = 1;

  return status;
}

/* return 1 (true) or 0 (false) */
static int
striker_505_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if ((my->count[3] == 1)
      && (my->count[4] > 95) && (my->count[4] < 1045))
    return 1;
  if ((my->count[3] == 2) && (my->count[8] != 0))
    return 1;

  return 0;
}

static tenm_object *
striker_446_new(int table_index)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = (double) (WINDOW_WIDTH / 2);
  double y = -59.0;

  /* sanity check */
  if (table_index < 0)
  {
    fprintf(stderr, "striker_446_new: table_index is negative (%d)\n",
            table_index);
    return NULL;
  }

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "striker_446_new: malloc(p) failed\n");
    return NULL;
  }
  p[0] = (tenm_primitive *) tenm_polygon_new(3,
                                             x + 51.9615, y - 30.0,
                                             x, y + 60.0,
                                             x - 51.9615, y - 30.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "striker_446_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 9);
  if (count == NULL)
  {
    fprintf(stderr, "striker_446_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "striker_446_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] for deal_damage
   * [1] "damaged" timer
   * [2] manager index
   * [3] life mode
   * [4] life timer
   * [5] move mode
   * [6] move timer
   * [7] bit theta
   * [8] "was green when killed" flag
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */

  count[0] = 0;
  count[1] = 0;
  count[2] = table_index;
  count[3] = 0;
  count[4] = 0;
  count[5] = 0;
  count[6] = 0;
  count[7] = 0;
  count[8] = 0;

  count_d[0] = 0.0;
  count_d[1] = (34.0 - y) / 30.0;

  new = tenm_object_new("Striker 446", ATTR_BOSS, ATTR_PLAYER_SHOT,
                        446, x, y,
                        9, count, 2, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&striker_446_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&striker_446_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&striker_446_act),
                        (int (*)(tenm_object *, int))
                        (&striker_446_draw));
  if (new == NULL)
  {
    fprintf(stderr, "striker_446_new: tenm_object_new failed\n");
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
striker_446_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "striker_446_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "striker_446_move: strange turn_per_frame (%f)\n",
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
striker_446_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "striker_446_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "striker_446_hit: your is NULL\n");
    return 0;
  }

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;
  if (my->count[3] != 1)
    return 0;

  deal_damage(my, your, 0);
  if (striker_446_green(my))
    add_chain(my, your);
  my->count[1] = 2;

  if (my->hit_point <= 0)
  {
    add_score(3000);
    set_background(1);
    striker_446_next(my);
    return 0;
  }

  return 0;
}

static void
striker_446_next(tenm_object *my)
{
  int n;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "striker_446_next: my is NULL\n");
    return;
  }

  /* set "was green" flag before we change the life mode */
  if (striker_446_green(my))
  {
    n = 8;
    my->count[8] = 1;
  }
  else
  {
    n = 7;
    my->count[8] = 0;
  }

  tenm_table_add(explosion_new(my->x, my->y,
                               0.0, 0.0,
                               1, 3000, n, 6.0, 8));
  tenm_table_add(explosion_new(my->x, my->y,
                               0.0, 0.0,
                               2, 800, n, 3.5, 8));

  my->count[1] = 0;
  my->count[3] = 2;
  my->count[4] = 0;
  my->count_d[0] = 0.0;
  my->count_d[1] = 0.0;

  tenm_table_apply(my->count[2],
                   (int (*)(tenm_object *, int)) (&strikers_striker_signal),
                   0);

  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy_shot), 0);
  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy), 0);
}


static int
striker_446_act(tenm_object *my, const tenm_object *player)
{
  int n;
  double speed;
  int i;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "striker_446_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  /* for deal_damage */
  my->count[0] = 0;

  /* "damaged" count down */
  if (my->count[1] > 0)
    (my->count[1])--;

  (my->count[4])++;
  /* encounter */
  if (my->count[3] == 0)
  {
    if (my->count[4] == 30)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[4] >= 90)
    {
      my->count[3] = 1;
      my->count[4] = 0;
    }
    return 0;
  }

  /* dead */
  if (my->count[3] == 2)
  {
    if ((my->count[4] <= 9) && (my->count[4] % 3 == 0))
    {
      if (striker_505_green(my))
        n = 8;
      else
        n = 7;
      tenm_table_add(explosion_new(my->x + ((double) (my->count[4]/3)) * 60.0,
                                   my->y,
                                   0.0, 0.0,
                                   1, 200, n, 2.0, 8));
      tenm_table_add(explosion_new(my->x - ((double) (my->count[4]/3)) * 60.0,
                                   my->y,
                                   0.0, 0.0,
                                   1, 200, n, 2.0, 8));
    }
    if (my->count[4] >= 9)
      return 1;

    return 0;
  }
  
  /* self-destruction */
  if ((my->count[3] == 1) && (my->count[4] >= 1065))
  {
    set_background(2);
    clear_chain();
    striker_446_next(my);
    return 0;
  }

  /* move */
  speed = 6.0;
  if (my->count[7] == 75)
    speed = 3.0;
  if ((my->x + 120.0 > player->x) && (my->x - 120.0 < player->x)
      && (my->y + 60.0 < player->y))
  {
    my->count[7] += 15;
    if (my->count[7] > 75)
      my->count[7] = 75;
    speed = 3.0;
  }
  else
  {
    my->count[7] -= 15;
    if (my->count[7] < 0)
      my->count[7] = 0;
  }

  (my->count[6])++;
  if (my->count[5] == 0)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
    if ((my->count[6] >= 0)
        && ((my->x + speed * 15.0 < player->x)
            || (my->x - speed * 15.0 > player->x)))
    {
      my->count[5] = 1;
      my->count[6] = 0;
    }
    else if ((my->count[6] >= 0)
        && ((my->x + speed < player->x)
            || (my->x - speed > player->x)))
    {
      my->count[5] = 1;
      my->count[6] = 50;
    }
  }
  else if (my->count[5] == 1)
  {
    my->count_d[1] = 0.0;
    if (my->x - speed > player->x)
      my->count_d[0] = -speed;
    else if (my->x + speed < player->x)
      my->count_d[0] = speed;
    else
      my->count_d[0] = player->x - my->x;
    if (my->count[6] >= 60)
    {
      my->count[5] = 0;
      my->count[6] = -60;
    }
  }

  if (my->count[4] <= 92)
  {
    my->count[5] = 0;
    my->count[6] = 0;
    my->count[7] = 0;
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
  }

  if ((my->count[3] != 1) || (my->count[4] > 943))
    return 0;

  /* shoot */
  if (((my->count[7] == 0) || (my->count[7] == 75))
      && (my->count[4] % 23 == 19))
  {
    tenm_table_add(laser_angle_new(my->x + 13.0 * tenm_cos(0),
                                   my->y + 13.0 * tenm_sin(0),
                                   5.5,
                                   90, 25.0, 2));
    tenm_table_add(laser_angle_new(my->x + 13.0 * tenm_cos(180),
                                   my->y + 13.0 * tenm_sin(180),
                                   5.5,
                                   90, 25.0, 2));
  }
  if (((my->count[7] == 0) || (my->count[7] == 75))
      && (my->count[4] % 23 == 0))
  {
    for (i = -1; i <= 1; i += 2)
    {
      tenm_table_add(laser_angle_new(my->x + 13.0 * tenm_cos(i * 7),
                                     my->y + 13.0 * tenm_sin(i * 7),
                                     5.5,
                                     90 + i * 7, 25.0, 2));
      tenm_table_add(laser_angle_new(my->x + 13.0 * tenm_cos(180 + i * 7),
                                     my->y + 13.0 * tenm_sin(180 + i * 7),
                                     5.5,
                                     90 + i * 7, 25.0, 2));

    }
  }

  if ((my->count[7] == 0) && (my->count[4] % 23 == 0))
  {
    tenm_table_add(laser_angle_new(my->x + 100.0 * tenm_cos(my->count[7]),
                                   my->y + 100.0 * tenm_sin(my->count[7]),
                                   7.5,
                                   90, 50.0, 0));
    tenm_table_add(laser_angle_new(my->x - 100.0 * tenm_cos(my->count[7]),
                                   my->y + 100.0 * tenm_sin(my->count[7]),
                                   7.5,
                                   90, 50.0, 0));
  }

  if ((my->count[7] == 75) && (my->count[4] % 11 == 0))
  {
    tenm_table_add(normal_shot_angle_new(my->x,
                                         my->y + 100.0*tenm_sin(my->count[7]),
                                         7.5,
                                         90, 1));
  }

  return 0;
}

static int
striker_446_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  char temp[32];
  double c;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "striker_446_draw: my is NULL\n");
    return 0;
  }

  /* decoration */
  if (priority == 0)
  {
    if (striker_446_green(my))
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
  }

  /* body */
  if (priority == 0)
  {
    if (striker_446_green(my))
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

    if (((my->count[3] == 0) && (my->count[4] >= 45))
        || (my->count[3] == 1))
    {
      if (my->count[3] == 0)
      {
        if (my->count[4] >= 75)
          c = 100.0;
        else
          c = 100.0 * ((double) (my->count[4] - 45)) / 30.0;
      }
      else
      {
        c = 100.0;
      }

      if (striker_446_draw_bit(my->x + c * tenm_cos(my->count[7]),
                               my->y + c * tenm_sin(my->count[7]),
                               color) != 0)
        status = 1;
      if (striker_446_draw_bit(my->x - c * tenm_cos(my->count[7]),
                               my->y + c * tenm_sin(my->count[7]),
                               color) != 0)
        status = 1;
    }
    
    if (my->count[3] <= 1)
    { 
      if (tenm_draw_line((int) (my->x + 51.9615), (int) (my->y - 30.0),
                         (int) (my->x), (int) (my->y + 60.0),
                         3, color) != 0)
        status = 1;
      if (tenm_draw_line((int) (my->x), (int) (my->y + 60.0),
                         (int) (my->x - 51.9615), (int) (my->y - 30.0),
                         3, color) != 0)
        status = 1;
      if (tenm_draw_line((int) (my->x - 51.9615), (int) (my->y - 30.0),
                         (int) (my->x + 51.9615), (int) (my->y - 30.0),
                         3, color) != 0)
        status = 1;
    }
  }

  /* hit point stat */
  if ((priority == 0) && (my->count[3] == 1))
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 10, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "striker_446_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

static int
striker_446_draw_bit(double x, double y, tenm_color color)
{
  int status = 0;

  if (tenm_draw_line((int) (x + 8.6603), (int) (y - 15.0),
                     (int) (x + 8.6603), (int) (y + 15.0),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (x + 8.6603), (int) (y + 15.0),
                     (int) (x - 8.6603), (int) (y + 15.0),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (x - 8.6603), (int) (y + 15.0),
                     (int) (x - 8.6603), (int) (y - 15.0),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (x - 25.9808), (int) (y - 15.0),
                     (int) (x + 25.9808), (int) (y - 15.0),
                     1, color) != 0)
    status = 1;

  return status;
}

/* return 1 (true) or 0 (false) */
static int
striker_446_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if ((my->count[3] == 1)
      && (my->count[4] > 92) && (my->count[4] < 1035))
    return 1;
  if ((my->count[3] == 2) && (my->count[8] != 0))
    return 1;

  return 0;
}

static tenm_object *
striker_1341_new(int table_index)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = (double) (WINDOW_WIDTH / 2);
  double y = -59.0;

  /* sanity check */
  if (table_index < 0)
  {
    fprintf(stderr, "striker_1341_new: table_index is negative (%d)\n",
            table_index);
    return NULL;
  }

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "striker_1341_new: malloc(p) failed\n");
    return NULL;
  }
  p[0] = (tenm_primitive *) tenm_polygon_new(3,
                                             x + 51.9615, y - 30.0,
                                             x, y + 60.0,
                                             x - 51.9615, y - 30.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "striker_1341_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 9);
  if (count == NULL)
  {
    fprintf(stderr, "striker_1341_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 4);
  if (count_d == NULL)
  {
    fprintf(stderr, "striker_1341_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] for deal_damage
   * [1] "damaged" timer
   * [2] manager index
   * [3] life mode
   * [4] life timer
   * [5] move mode
   * [6] move timer
   * [7] bit theta
   * [8] "was green when killed" flag
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   * [2] aim x
   * [3] aim y
   */

  count[0] = 0;
  count[1] = 0;
  count[2] = table_index;
  count[3] = 0;
  count[4] = 0;
  count[5] = 0;
  count[6] = 0;
  count[7] = 0;
  count[8] = 0;

  count_d[0] = 0.0;
  count_d[1] = (34.0 - y) / 30.0;
  count_d[2] = 0.0;
  count_d[3] = 25.0;

  /* 1341 HP is too many for an [easy] boss */
  new = tenm_object_new("Striker 1341", ATTR_BOSS, ATTR_PLAYER_SHOT,
                        712, x, y,
                        9, count, 4, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&striker_1341_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&striker_1341_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&striker_1341_act),
                        (int (*)(tenm_object *, int))
                        (&striker_1341_draw));
  if (new == NULL)
  {
    fprintf(stderr, "striker_1341_new: tenm_object_new failed\n");
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
striker_1341_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "striker_1341_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "striker_1341_move: strange turn_per_frame (%f)\n",
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
striker_1341_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "striker_1341_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "striker_1341_hit: your is NULL\n");
    return 0;
  }

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;
  if (my->count[3] != 1)
    return 0;

  deal_damage(my, your, 0);
  if (striker_1341_green(my))
    add_chain(my, your);
  my->count[1] = 2;

  if (my->hit_point <= 0)
  {
    add_score(9000);
    set_background(1);
    striker_1341_next(my);
    return 0;
  }

  return 0;
}

static void
striker_1341_next(tenm_object *my)
{
  int n;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "striker_1341_next: my is NULL\n");
    return;
  }

  /* set "was green" flag before we change the life mode */
  if (striker_1341_green(my))
  {
    n = 8;
    my->count[8] = 1;
  }
  else
  {
    n = 7;
    my->count[8] = 0;
  }

  tenm_table_add(explosion_new(my->x, my->y,
                               0.0, 0.0,
                               1, 3000, n, 6.0, 8));
  tenm_table_add(explosion_new(my->x, my->y,
                               0.0, 0.0,
                               2, 800, n, 3.5, 8));

  my->count[1] = 0;
  my->count[3] = 2;
  my->count[4] = 0;
  my->count_d[0] = 0.0;
  my->count_d[1] = 0.0;

  tenm_table_apply(my->count[2],
                   (int (*)(tenm_object *, int)) (&strikers_striker_signal),
                   0);

  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy_shot), 0);
  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy), 0);
}


static int
striker_1341_act(tenm_object *my, const tenm_object *player)
{
  int n;
  double speed;
  int theta;
  double result[2];
  double v[2];
  int i;
  double x;
  double y;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "striker_1341_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  /* for deal_damage */
  my->count[0] = 0;

  /* "damaged" count down */
  if (my->count[1] > 0)
    (my->count[1])--;

  (my->count[4])++;
  /* encounter */
  if (my->count[3] == 0)
  {
    if (my->count[4] == 30)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[4] >= 90)
    {
      my->count[3] = 1;
      my->count[4] = 0;
    }
    return 0;
  }

  /* dead */
  if (my->count[3] == 2)
  {
    if ((my->count[4] <= 9) && (my->count[4] % 3 == 0))
    {
      if (striker_1341_green(my))
        n = 8;
      else
        n = 7;
      tenm_table_add(explosion_new(my->x + ((double) (my->count[4]/3)) * 60.0,
                                   my->y,
                                   0.0, 0.0,
                                   1, 200, n, 2.0, 8));
      tenm_table_add(explosion_new(my->x - ((double) (my->count[4]/3)) * 60.0,
                                   my->y,
                                   0.0, 0.0,
                                   1, 200, n, 2.0, 8));
    }
    if (my->count[4] >= 9)
      return 1;

    return 0;
  }
  
  /* self-destruction */
  if ((my->count[3] == 1) && (my->count[4] >= 1377))
  {
    set_background(2);
    clear_chain();
    striker_1341_next(my);
    return 0;
  }

  /* move */
  speed = 6.0;
  if (my->count[7] == 75)
    speed = 3.0;
  if ((my->x + 120.0 > player->x) && (my->x - 120.0 < player->x)
      && (my->y + 60.0 < player->y))
  {
    my->count[7] += 15;
    if (my->count[7] > 75)
      my->count[7] = 75;
    speed = 3.0;
  }
  else
  {
    my->count[7] -= 15;
    if (my->count[7] < 0)
      my->count[7] = 0;
  }

  theta = 0;

  (my->count[6])++;
  if (my->count[5] == 0)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
    if ((my->count[6] >= 0)
        && ((my->x + speed * 15.0 < player->x)
            || (my->x - speed * 15.0 > player->x)))
    {
      my->count[5] = 1;
      my->count[6] = 0;
    }
    else if ((my->count[6] >= 0)
        && ((my->x + speed < player->x)
            || (my->x - speed > player->x)))
    {
      my->count[5] = 1;
      my->count[6] = 50;
    }
  }
  else if (my->count[5] == 1)
  {
    if (my->x > player->x)
      theta = 1;
    else
      theta = -1;

    my->count_d[1] = 0.0;
    if (my->x - speed > player->x)
      my->count_d[0] = -speed;
    else if (my->x + speed < player->x)
      my->count_d[0] = speed;
    else
      my->count_d[0] = player->x - my->x;
    if (my->count[6] >= 60)
    {
      my->count[5] = 0;
      my->count[6] = -60;
    }
  }

  if (my->count[4] <= 87)
  {
    my->count[5] = 0;
    my->count[6] = 0;
    my->count[7] = 0;
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
  }

  if ((my->count[3] != 1) || (my->count[4] > 1247))
    return 0;

  /* aim */
  if (speed > 5.9)
  {
    v[0] = my->count_d[2];
    v[1] = my->count_d[3];
    result[0] = v[0];
    result[1] = v[1];
    vector_rotate(result, v, theta);
    my->count_d[2] = result[0];
    my->count_d[3] = result[1];
    if (my->count_d[3] < 25.0 * tenm_cos(45))
    {
      if (my->count_d[2] > 0.0)
      {
        my->count_d[2] = 25.0 * tenm_cos(45);
        my->count_d[3] = 25.0 * tenm_sin(45);
      }
      else
      {
        my->count_d[2] = 25.0 * tenm_cos(135);
        my->count_d[3] = 25.0 * tenm_sin(135);
      }
    }
  }

  /* shoot */
  if ((my->count[7] == 0) && (my->count[4] % 58 == 0))
  {
    for (i = 0; i < 6; i ++)
    {
      if (i == 1)
        continue;
      x = my->x + 100.0 * tenm_cos(my->count[7]) * ((double) i);
      y = my->y + 100.0 * tenm_sin(my->count[7]);
      tenm_table_add(laser_angle_new(my->x + 100.0 * tenm_cos(my->count[7]),
                                     my->y + 100.0 * tenm_sin(my->count[7]),
                                     3.5,
                                     101 - 11 * i,
                                     25.0, 5));
      tenm_table_add(laser_angle_new(my->x - 100.0 * tenm_cos(my->count[7]),
                                     my->y + 100.0 * tenm_sin(my->count[7]),
                                     3.5,
                                     79 + 11 * i,
                                     25.0, 5));
    }
  }
  if ((my->count[7] == 0) && (my->count[4] % 58 == 29))
  {
    for (i = -2; i <= 2; i ++)
    {
      tenm_table_add(laser_angle_new(my->x,
                                     my->y,
                                     3.5,
                                     90 + 9 * i,
                                     25.0, 4));
    }
  }

  if ((my->count[7] == 75) && (my->count[4] % 23 == 0))
  {
    for (i = -1; i <= 1; i ++)
    {
      if (i == 0)
      {
        if (my->count[4] % 46 == 0)
          continue;
      }
      else
      {
        if (my->count[4] % 46 == 23)
          continue;
      }

      tenm_table_add(laser_angle_new(my->x + 40.0 * ((double) i),
                                     my->y - 30.0,
                                     6.5,
                                     90, 25.0, 2));
    }

    tenm_table_add(laser_point_new(my->x + 100.0 * tenm_cos(my->count[7]),
                                   my->y + 100.0 * tenm_sin(my->count[7]),
                                   3.5,
                                   player->x, player->y,
                                   25.0, 1));
    tenm_table_add(laser_point_new(my->x - 100.0 * tenm_cos(my->count[7]),
                                   my->y + 100.0 * tenm_sin(my->count[7]),
                                   3.5,
                                   player->x, player->y,
                                   25.0, 1));
  }

  if (((my->count[7] == 0) && (my->count[4] % 13 == 0))
      || ((my->count[7] == 75) && (my->count[4] % 19 == 0)))
  {
    if ((my->count[5] == 0) && (speed > 5.9))
    {
      my->count_d[2] = 0.0;
      my->count_d[3] = 25.0;
    }
    for (i = -1; i <= 1; i += 2)
    {
      x = my->x + 100.0 * tenm_cos(my->count[7]) * ((double) i);
      y = my->y + 100.0 * tenm_sin(my->count[7]);
      tenm_table_add(laser_point_new(x, y, 4.5,
                                     x + my->count_d[2],
                                     y + my->count_d[3],
                                     25.0, 3));
    }
  }

  return 0;
}

static int
striker_1341_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  char temp[32];
  double c;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "striker_1341_draw: my is NULL\n");
    return 0;
  }

  /* decoration */
  if (priority == 0)
  {
    if (striker_1341_green(my))
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
  }

  /* body */
  if (priority == 0)
  {
    if (striker_1341_green(my))
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

    if (((my->count[3] == 0) && (my->count[4] >= 45))
        || (my->count[3] == 1))
    {
      if (my->count[3] == 0)
      {
        if (my->count[4] >= 75)
          c = 100.0;
        else
          c = 100.0 * ((double) (my->count[4] - 45)) / 30.0;
      }
      else
      {
        c = 100.0;
      }

      if (striker_1341_draw_bit(my->x + c * tenm_cos(my->count[7]),
                               my->y + c * tenm_sin(my->count[7]),
                               color) != 0)
        status = 1;
      if (striker_1341_draw_bit(my->x - c * tenm_cos(my->count[7]),
                               my->y + c * tenm_sin(my->count[7]),
                               color) != 0)
        status = 1;
    }
    
    if (my->count[3] <= 1)
    { 
      if (tenm_draw_line((int) (my->x + 51.9615), (int) (my->y - 30.0),
                         (int) (my->x), (int) (my->y + 60.0),
                         3, color) != 0)
        status = 1;
      if (tenm_draw_line((int) (my->x), (int) (my->y + 60.0),
                         (int) (my->x - 51.9615), (int) (my->y - 30.0),
                         3, color) != 0)
        status = 1;
      if (tenm_draw_line((int) (my->x - 51.9615), (int) (my->y - 30.0),
                         (int) (my->x + 51.9615), (int) (my->y - 30.0),
                         3, color) != 0)
        status = 1;
    }
  }

  /* hit point stat */
  if ((priority == 0) && (my->count[3] == 1))
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 10, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "striker_1341_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

static int
striker_1341_draw_bit(double x, double y, tenm_color color)
{
  int status = 0;

  if (tenm_draw_line((int) (x + 8.6603), (int) (y - 15.0),
                     (int) (x + 25.9808), (int) (y + 15.0),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (x + 25.9808), (int) (y + 15.0),
                     (int) (x - 25.9808), (int) (y + 15.0),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (x - 25.9808), (int) (y + 15.0),
                     (int) (x - 8.6603), (int) (y - 15.0),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (x - 8.6603), (int) (y - 15.0),
                     (int) (x + 8.6603), (int) (y - 15.0),
                     1, color) != 0)
    status = 1;

  return status;
}

/* return 1 (true) or 0 (false) */
static int
striker_1341_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if ((my->count[3] == 1)
      && (my->count[4] > 87) && (my->count[4] < 1347))
    return 1;
  if ((my->count[3] == 2) && (my->count[8] != 0))
    return 1;

  return 0;
}
