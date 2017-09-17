/* $Id: mankanshoku.c,v 1.95 2005/01/06 06:16:42 oohara Exp $ */

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
#include "score.h"
#include "wall-8.h"
#include "silver-chimera.h"
#include "warning.h"

#include "mankanshoku.h"

static int mankanshoku_move(tenm_object *my, double turn_per_frame);
static int mankanshoku_hit(tenm_object *my, tenm_object *your);
static void mankanshoku_next(tenm_object *my);
static int mankanshoku_act(tenm_object *my, const tenm_object *player);
static int mankanshoku_draw(tenm_object *my, int priority);
static int mankanshoku_green(const tenm_object *my);

tenm_object *
mankanshoku_new(void)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = (double) (WINDOW_WIDTH / 2);
  double y = -255.0;

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "mankanshoku_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_polygon_new(8,
                                             x + 60.0, y - 15.0,
                                             x + 60.0, y + 15.0,
                                             x + 50.0, y + 25.0,
                                             x - 50.0, y + 25.0,
                                             x - 60.0, y + 15.0,
                                             x - 60.0, y - 15.0,
                                             x - 50.0, y - 25.0,
                                             x + 50.0, y - 25.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "mankanshoku_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 7);
  if (count == NULL)
  {
    fprintf(stderr, "mankanshoku_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 3);
  if (count_d == NULL)
  {
    fprintf(stderr, "mankanshoku_new: malloc(count_d) failed\n");
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
   * [4] the time when we escaped / were killed
   * [5] add enemy mode
   * [6] add enemy timer
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */

  count[0] = 0;
  count[1] = 0;
  count[2] = 0;
  count[3] = 0;
  count[4] = 0;
  count[5] = (rand() % 2) * 3;
  count[6] = 0;

  count_d[0] = 0.0;
  count_d[1] = 7.5;

  new = tenm_object_new("Mankanshoku", ATTR_ENEMY | ATTR_OBSTACLE,
                        ATTR_PLAYER_SHOT,
                        750, x, y,
                        7, count, 3, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&mankanshoku_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&mankanshoku_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&mankanshoku_act),
                        (int (*)(tenm_object *, int))
                        (&mankanshoku_draw));

  if (new == NULL)
  {
    fprintf(stderr, "mankanshoku_new: tenm_object_new failed\n");
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
mankanshoku_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "mankanshoku_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "mankanshoku_move: strange turn_per_frame (%f)\n",
            turn_per_frame);
    return 0;
  }

  if (my->count[2] != 0)
    return 0;

  dx_temp = my->count_d[0] / turn_per_frame;
  dy_temp = my->count_d[1] / turn_per_frame;
  my->x += dx_temp;
  my->y += dy_temp;
  if (my->mass != NULL)
    tenm_move_mass(my->mass, dx_temp, dy_temp);

  if ((my->count[3] >= 2344) && (my->y > ((double) WINDOW_HEIGHT) + 120.0))
    mankanshoku_next(my);

  return 0;
}

static int
mankanshoku_hit(tenm_object *my, tenm_object *your)
{
  int n;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "mankanshoku_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "mankanshoku_hit: your is NULL\n");
    return 0;
  }

  if (my->count[2] != 0)
    return 0;
  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;

  deal_damage(my, your, 0);
  if (mankanshoku_green(my))
    add_chain(my, your);
  my->count[1] = 41;

  if (my->hit_point <= 0)
  {
    add_score(100);

    if (mankanshoku_green(my))
      n = 8;
    else
      n = 7;
    tenm_table_add(explosion_new(my->x, my->y,
                                 my->count_d[0] * 0.5,
                                 my->count_d[1] * 0.5,
                                 1, 1000, n, 8.0, 6));
    tenm_table_add(fragment_new(my->x, my->y,
                                my->count_d[0] * 0.5, my->count_d[1] * 0.5,
                                30.0, 30, n, 5.0, 0.0, 20));

    mankanshoku_next(my);
    return 0;
  }

  return 0;
}

static void
mankanshoku_next(tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return;

  /* no explosion here --- this is called when we escape
   * as well as we are killed
   */
  my->count[2] = 1;
  my->count[4] = my->count[3];
  if (my->count[4] < 1082)
    my->count[4] = 1082;
  my->count[1] = 0;

  /* don't modify my->attr or my->hit_mask here, or the player shot
   * may fly through the enemy */
  tenm_mass_delete(my->mass);
  my->mass = NULL;
}

static int
mankanshoku_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int t;
  int theta1;
  int theta2;
  int temp;
  double speed;
  double dx;
  double dy;
  double result[2];
  double v[2];
  double x;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "mankanshoku_act: my is NULL\n");
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

  /* dead / escaped */
  if (my->count[2] == 1)
  {
    if (my->count[3] == my->count[4] + 160)
      tenm_table_add(warning_new());

    if (my->count[3] == my->count[4] + 290)
    {
      tenm_table_add(silver_chimera_new());
      return 1;
    }
  }
  
  /* move */
  if (my->count[3] < 75)
    my->count_d[1] = 7.5 - ((double) (my->count[3])) * 0.1;
  else if (my->count[3] < 1032)
    my->count_d[1] = 0.0;
  else if (my->count[3] < 1082)
    my->count_d[1] = ((double) (my->count[3] - 1032)) * 0.1;
  else if (my->count[3] < 1187)
    my->count_d[1] = 5.0 - ((double) (my->count[3] - 1082)) * 0.1;
  else if (my->count[3] < 2344)
    my->count_d[1] = 0.0;
  else
    my->count_d[1] = 7.5;

  /* add wall */
  if ((my->count[3] >= 1133) && (my->count[3] < 9999)
      && ((my->count[2] == 0) || (my->count[3] < my->count[4] + 72))
      && ((my->count[3] - 1133) % 24 == 0))
  {
    tenm_table_add(wall_8_new(30.0, -29.0, 0, my->count[3] + 2458));
    tenm_table_add(wall_8_new(((double) WINDOW_WIDTH) - 30.0, -29.0, 0,
                              my->count[3] + 2458));
  }

  t = my->count[3] - 1187;
  if (t < 0)
    return 0;

  /* chase the player */
  if (player->x < 220.0)
    x = 120.0;
  else if (player->x < 420.0)
    x = 320.0;
  else
    x = 520.0;

  if ((my->x + 10.0 > x) && (my->x - 10.0 < x))
    my->count_d[0] = x - my->x;
  else if (my->x < x)
    my->count_d[0] = 10.0;
  else
    my->count_d[0] = -10.0;
  if (my->x + my->count_d[0] < 120.0)
    my->count_d[0] = 120.0 - my->x;
  if (my->x + my->count_d[0] > 520.0)
    my->count_d[0] = 520.0 - my->x;
  if (t >= 1058)
    my->count_d[0] = 0.0;

  /* add normal enemy */
  if (my->count[5] >= 0)
  {
    (my->count[6])++;
    if ((my->count[2] != 0) && (my->count[6] < 48))
    {  
      my->count[5] = -1;
    }
    else if ((my->count[6] >= 48) && (my->count[6] <= 60)
        && (my->count[6] % 6 == 0))
    {
      switch (my->count[5])
      {
      case 0:
        x = 120.0 + ((double) (my->count[6] - 56)) * 3.0;
        break;
      case 1:
        x = 320.0 - ((double) (my->count[6] - 48)) * 3.0;
        break;
      case 2:
        x = 320.0 + ((double) (my->count[6] - 48)) * 3.0;
        break;
      case 3:
        x = 520.0 - ((double) (my->count[6] - 56)) * 3.0;
      break;
      default:
        fprintf(stderr, "mankanshoku_act: undefined my->count[5] (add enemy) "
                "(%d)\n", my->count[5]);
        x = ((double) (WINDOW_WIDTH / 2));
        break;
      }
      if (my->count[6] == 48)
        temp = 37;
      else
        temp = 9999;
      tenm_table_add(normal_enemy_new(x, -19.0,
                                      BALL_SOLDIER, ENEMY_TYPE_WEAK,
                                      0, -1, 0, -1, 0, 1, 2,
                                      /* move 0 */
                                      9999,
                                      0.0, 7.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      21, temp, 34, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, temp, 55 % temp, 0, 1, 1));
    }
    if (my->count[6] >= 60)
    {
      switch (my->count[5])
      {
      case 0:
      case 3:
        my->count[5] = 1 + rand() % 2;
        break;
      case 1:
        my->count[5] = 0;
        break;
      case 2:
        my->count[5] = 3;
        break;
      default:
        fprintf(stderr, "mankanshoku_act: undefined my->count[5] "
                "(mode change) (%d)\n", my->count[5]);
        my->count[5] = 1;
        break;
      }
      if ((t >= 840) || (my->count[2] != 0))
        my->count[5] = -1;
 
      my->count[6] = 0;
    }
  }

  /* shoot */
  if (my->count[2] != 0)
    return 0;

  if (t < 907)
  {  
    if (t % 121 == 0)
    {
      tenm_table_add(laser_angle_new(my->x - 50.0, my->y, 6.5,
                                     125, 25.0, 2));
      tenm_table_add(laser_angle_new(my->x - 50.0, my->y, 6.5,
                                     146, 25.0, 2));
      tenm_table_add(laser_angle_new(my->x - 50.0, my->y, 6.5,
                                     167, 25.0, 2));

      tenm_table_add(laser_angle_new(my->x + 50.0, my->y, 6.5,
                                     55, 25.0, 2));
      tenm_table_add(laser_angle_new(my->x + 50.0, my->y, 6.5,
                                     34, 25.0, 2));
      tenm_table_add(laser_angle_new(my->x + 50.0, my->y, 6.5,
                                     13, 25.0, 2));
    }
    if (t % 121 == 11)
    {
      tenm_table_add(laser_angle_new(my->x - 50.0, my->y, 6.5,
                                     95, 25.0, 2));
      tenm_table_add(laser_angle_new(my->x - 50.0, my->y, 7.5,
                                     116, 25.0, 2));
      tenm_table_add(laser_angle_new(my->x - 50.0, my->y, 8.5,
                                     137, 25.0, 2));

      tenm_table_add(laser_angle_new(my->x + 50.0, my->y, 6.5,
                                     85, 25.0, 2));
      tenm_table_add(laser_angle_new(my->x + 50.0, my->y, 7.5,
                                     64, 25.0, 2));
      tenm_table_add(laser_angle_new(my->x + 50.0, my->y, 8.5,
                                     43, 25.0, 2));
    }
    if (t % 121 == 22)
    {
      tenm_table_add(laser_angle_new(my->x - 50.0, my->y, 6.5,
                                     65, 25.0, 2));
      tenm_table_add(laser_angle_new(my->x - 50.0, my->y, 8.5,
                                     86, 25.0, 2));
      tenm_table_add(laser_angle_new(my->x - 50.0, my->y, 10.5,
                                     107, 25.0, 2));

      tenm_table_add(laser_angle_new(my->x + 50.0, my->y, 6.5,
                                     115, 25.0, 2));
      tenm_table_add(laser_angle_new(my->x + 50.0, my->y, 8.5,
                                     94, 25.0, 2));
      tenm_table_add(laser_angle_new(my->x + 50.0, my->y, 10.5,
                                     73, 25.0, 2));
    }

    if ((t % 121 == 55) || (t % 121 == 88))
    {
      speed = 6.0;
      if (t % 121 == 55)
        theta1 = 45;
      else
        theta1 = 77;
      theta2 = 113;

      dx = speed * tenm_cos(theta1);
      dy = speed * tenm_sin(theta1);
      v[0] = speed * tenm_cos(180 - theta2) * tenm_cos(theta1 - theta2);
      v[1] = speed * tenm_cos(180 - theta2) * tenm_sin(theta1 - theta2);
      
      for (i = 0; i < 6; i++)
      {
        result[0] = v[0];
        result[1] = v[1];
        vector_rotate(result, v, i * 30);
        tenm_table_add(normal_shot_new(my->x - 50.0, my->y,
                                       dx + result[0], dy + result[1],
                                       1, -2, 0));
        tenm_table_add(normal_shot_new(my->x + 50.0, my->y,
                                       -(dx + result[0]), dy + result[1],
                                       1, -2, 0));
      }
    }
  }
  else if ((t >= 989) && (t < 1127))
  {
    speed = 7.0;
    if (t % 46 == 0)
    {
      tenm_table_add(laser_point_new(my->x + 120.0 * tenm_cos(-15),
                                     my->y + 120.0 * tenm_sin(-15),
                                     speed,
                                     player->x, player->y - 150.0,
                                     25.0, 0));
      tenm_table_add(laser_point_new(my->x + 120.0 * tenm_cos(-165),
                                     my->y + 120.0 * tenm_sin(-165),
                                     speed,
                                     player->x, player->y - 150.0,
                                     25.0, 0));
      tenm_table_add(laser_point_new(my->x + 120.0 * tenm_cos(-45),
                                     my->y + 120.0 * tenm_sin(-45),
                                     speed,
                                     player->x, player->y - 75.0,
                                     25.0, 0));
      tenm_table_add(laser_point_new(my->x + 120.0 * tenm_cos(-135),
                                     my->y + 120.0 * tenm_sin(-135),
                                     speed,
                                     player->x, player->y - 75.0,
                                     25.0, 0));
      tenm_table_add(laser_point_new(my->x + 120.0 * tenm_cos(-75),
                                     my->y + 120.0 * tenm_sin(-75),
                                     speed,
                                     player->x, player->y,
                                     25.0, 0));
      tenm_table_add(laser_point_new(my->x + 120.0 * tenm_cos(-105),
                                     my->y + 120.0 * tenm_sin(-105),
                                     speed,
                                     player->x, player->y,
                                     25.0, 0));
    }
    if (t % 46 == 23)
    {
      tenm_table_add(laser_point_new(my->x + 120.0 * tenm_cos(-15),
                                     my->y + 120.0 * tenm_sin(-15),
                                     speed,
                                     player->x, player->y,
                                     25.0, 0));
      tenm_table_add(laser_point_new(my->x + 120.0 * tenm_cos(-165),
                                     my->y + 120.0 * tenm_sin(-165),
                                     speed,
                                     player->x, player->y,
                                     25.0, 0));
      tenm_table_add(laser_point_new(my->x + 120.0 * tenm_cos(-45),
                                     my->y + 120.0 * tenm_sin(-45),
                                     speed,
                                     player->x, player->y - 75.0,
                                     25.0, 0));
      tenm_table_add(laser_point_new(my->x + 120.0 * tenm_cos(-135),
                                     my->y + 120.0 * tenm_sin(-135),
                                     speed,
                                     player->x, player->y - 75.0,
                                     25.0, 0));
      tenm_table_add(laser_point_new(my->x + 120.0 * tenm_cos(-75),
                                     my->y + 120.0 * tenm_sin(-75),
                                     speed,
                                     player->x, player->y - 150.0,
                                     25.0, 0));
      tenm_table_add(laser_point_new(my->x + 120.0 * tenm_cos(-105),
                                     my->y + 120.0 * tenm_sin(-105),
                                     speed,
                                     player->x, player->y - 150.0,
                                     25.0, 0));
    }
  }
  
  return 0;
}

static int
mankanshoku_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  char temp[32];
  int i;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "mankanshoku_draw: my is NULL\n");
    return 0;
  }

  if (priority != 0)
    return 0;
  if (my->count[2] != 0)
    return 0;

  /* decoration */
  if (mankanshoku_green(my))
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

  for (i = -15; i >= -165; i -= 30)
    if (tenm_draw_line(((int) (my->x + 80.0 * tenm_cos(i))),
                       ((int) (my->y + 80.0 * tenm_sin(i))),
                       ((int) (my->x + 120.0 * tenm_cos(i))),
                       ((int) (my->y + 120.0 * tenm_sin(i))),
                       1, color) != 0)
      status = 1;

  /* body */
  if (mankanshoku_green(my))
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

  if (tenm_draw_line(((int) (my->x + 60.0)),
                     ((int) (my->y - 15.0)),
                     ((int) (my->x + 60.0)),
                     ((int) (my->y + 15.0)),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x + 60.0)),
                     ((int) (my->y + 15.0)),
                     ((int) (my->x + 50.0)),
                     ((int) (my->y + 25.0)),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x + 50.0)),
                     ((int) (my->y + 25.0)),
                     ((int) (my->x - 50.0)),
                     ((int) (my->y + 25.0)),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x - 50.0)),
                     ((int) (my->y + 25.0)),
                     ((int) (my->x - 60.0)),
                     ((int) (my->y + 15.0)),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x - 60.0)),
                     ((int) (my->y + 15.0)),
                     ((int) (my->x - 60.0)),
                     ((int) (my->y - 15.0)),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x - 60.0)),
                     ((int) (my->y - 15.0)),
                     ((int) (my->x - 50.0)),
                     ((int) (my->y - 25.0)),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x - 50.0)),
                     ((int) (my->y - 25.0)),
                     ((int) (my->x + 50.0)),
                     ((int) (my->y - 25.0)),
                     3, color) != 0)
    status = 1;
  if (tenm_draw_line(((int) (my->x + 50.0)),
                     ((int) (my->y - 25.0)),
                     ((int) (my->x + 60.0)),
                     ((int) (my->y - 15.0)),
                     3, color) != 0)
    status = 1;

  /* hit point stat */
  if ((priority == 0) && (my->count[1] > 0))
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 10, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "mankanshoku_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

/* return 1 (true) or 0 (false) */
static int
mankanshoku_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if ((my->count[3] >= 1192) && (my->count[3] < 2314))
    return 1;

  return 0;
}
