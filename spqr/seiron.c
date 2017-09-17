/* $Id: seiron.c,v 1.312 2011/08/24 17:32:15 oohara Exp $ */
/* [hard] Seiron */

#include <stdio.h>
/* malloc */
#include <stdlib.h>
/* strlen */
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

#include "seiron.h"

static int seiron_move(tenm_object *my, double turn_per_frame);
static int seiron_hit(tenm_object *my, tenm_object *your);
static int seiron_act(tenm_object *my, const tenm_object *player);
static void seiron_act_firework(tenm_object *my, const tenm_object *player,
                               int what, int t);
static int seiron_draw(tenm_object *my, int priority);
static void seiron_explosion(tenm_object *my);

tenm_object *
seiron_new(void)
{
  int i;
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = (double) (WINDOW_WIDTH / 2);
  double y = -44.0;

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "seiron_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_circle_new(x, y, 45.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "seiron_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 13);
  if (count == NULL)
  {
    fprintf(stderr, "seiron_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "seiron_new: malloc(count_d) failed\n");
    if (count != NULL)
      free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] for deal_damage
   * [1] "damaged" timer
   * [2] shoot timer
   * [3 -- 6] firework management
   * [7] winder theta
   * [8 -- 10] decoration management
   * [11] life mode
   * [12] demo timer
   */
  count[0] = 0;
  count[1] = 0;
  count[2] = 0;
  for (i = 3; i <= 7; i++)
    count[i] = 0;
  count[8] = 0;
  count[9] = -23;
  count[10] = 1;
  count[11] = 0;
  count[12] = 0;

  /* list of count_d
   * [0] speed x
   * [1] speed y
   */
  count_d[0] = 0.0;
  count_d[1] = (((double) (WINDOW_HEIGHT / 4)) - (-44.0)) / 30.0;

  new = tenm_object_new("Seiron", ATTR_BOSS,
                        ATTR_PLAYER_SHOT,
                        1500, x, y,
                        13, count, 2, count_d, 1, p,
                        (int (*)(tenm_object *, double)) (&seiron_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&seiron_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&seiron_act),
                        (int (*)(tenm_object *, int)) (&seiron_draw));
  if (new == NULL)
  {
    fprintf(stderr, "seiron_new: tenm_object_new failed\n");
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
seiron_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
    return 0;
  if (turn_per_frame <= 0.5)
    return 0;

  dx_temp = my->count_d[0] / turn_per_frame;
  dy_temp = my->count_d[1] / turn_per_frame;
  my->x += dx_temp;
  my->y += dy_temp;
  if (my->mass != NULL)
    tenm_move_mass(my->mass, dx_temp, dy_temp);

  return 0;
}

static int
seiron_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (your == NULL)
    return 0;

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;
  if (my->count[11] != 1)
    return 0;

  deal_damage(my, your, 0);
  if ((my->count[2] >= 1752) && (my->count[2] < 3870))
    add_chain(my, your);
  my->count[1] = 2;

  if (my->hit_point <= 0)
  {
    add_score(60000);
    set_background(1);
    seiron_explosion(my);

    return 0;
  }

  return 0;
}

static int
seiron_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int n;
  int theta;

  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;

  /* encounter */
  if (my->count[11] == 0)
  {
    (my->count[12])++;
    if (my->count[12] >= 30)
    {
      my->count[11] = 1;
      my->count[2] = 0;
      my->count[12] = 0;

      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    return 0;
  }
  /* dead */
  if (my->count[11] == 2)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.5;
    if ((my->count[2] >= 1752) && (my->count[2] < 3870))
      n = 8;
    else
      n = 7;

    if ((my->count[12] <= 75) && (my->count[12] % 15 == 0))
    {
      tenm_table_add(explosion_new(my->x + ((double) (-30 + (rand() % 61))),
                                   my->y + ((double) (-30 + (rand() % 61))),
                                   0.0, 0.0,
                                   2, 300, n, 5.0, 8));
    }

    (my->count[12])++;
    if (my->count[12] >= 120)
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

  /* self-destruction */
  if (my->count[2] >= 3900)
  {
    set_background(2);
    seiron_explosion(my);
    clear_chain();

    return 0;
  }

  /* if we are here, we are fighting, my->count[11] == 1 */
  /* for deal_damage */
  my->count[0] = 0;

  /* "damaged" count down */
  if (my->count[1] > 0)
    (my->count[1])--;

  /* decoration */
  if ((my->count[2] >= 1752) && (my->count[2] < 3870))
    my->count[8] -= 9;
  else
    my->count[8] += 3;
  while (my->count[8] > 360)
    my->count[8] -= 360;
  while (my->count[8] < 0)
    my->count[8] += 360;
  my->count[9] += my->count[10];
  if ((my->count[9] >= 23) || (my->count[9] <= -23))
    my->count[10] *= -1;

  /* winder attack */
  if ((my->count[2] >= 1752) && (my->count[2] < 3870)
      && (my->count[2] % 8 == 0))
  {
    if (my->count[2] % 48 < 24)
      theta = 90;
    else
      theta = 70;
    for (i = 0; i < 360; i += 40)
    {
      tenm_table_add(laser_angle_new(my->x, my->y, 15.0, theta + i, 30.0, 4));
    }
  }

  /* firework attack */
  if ((my->count[2] >= 30) && (my->count[2] <= 30 + 60))
    seiron_act_firework(my, player, 0, my->count[2] - 30);
  if ((my->count[2] >= 160) && (my->count[2] <= 160 + 255))
    seiron_act_firework(my, player, 1, my->count[2] - 160);
  if ((my->count[2] >= 450) && (my->count[2] <= 450 + 55))
    seiron_act_firework(my, player, 5, my->count[2] - 450);
  if ((my->count[2] >= 560) && (my->count[2] <= 560 + 219))
    seiron_act_firework(my, player, 2, my->count[2] - 560);
  if ((my->count[2] >= 790) && (my->count[2] <= 790 + 35))
    seiron_act_firework(my, player, 3, my->count[2] - 790);

  if ((my->count[2] >= 870) && (my->count[2] <= 870 + 60))
    seiron_act_firework(my, player, 0, my->count[2] - 870);
  if ((my->count[2] >= 1000) && (my->count[2] <= 1000 + 219))
    seiron_act_firework(my, player, 2, my->count[2] - 1000);
  if ((my->count[2] >= 1230) && (my->count[2] <= 1230 + 179))
    seiron_act_firework(my, player, 4, my->count[2] - 1230);
  if ((my->count[2] >= 1380) && (my->count[2] <= 1380 + 255))
    seiron_act_firework(my, player, 1, my->count[2] - 1380);
  if ((my->count[2] >= 1650) && (my->count[2] <= 1650 + 35))
    seiron_act_firework(my, player, 3, my->count[2] - 1650);

  if ((my->count[2] >= 1840) && (my->count[2] <= 1840 + 18))
    seiron_act_firework(my, player, 6, my->count[2] - 1840);
  if ((my->count[2] >= 1920) && (my->count[2] <= 1920 + 274))
    seiron_act_firework(my, player, 8, my->count[2] - 1920);
  if ((my->count[2] >= 2250) && (my->count[2] <= 2250 + 60))
    seiron_act_firework(my, player, 7, my->count[2] - 2250);
  if ((my->count[2] >= 2400) && (my->count[2] <= 2400 + 299))
    seiron_act_firework(my, player, 9, my->count[2] - 2400);
  if ((my->count[2] >= 2750) && (my->count[2] <= 2750 + 40))
    seiron_act_firework(my, player, 10, my->count[2] - 2750);

  if ((my->count[2] >= 2850) && (my->count[2] <= 2850 + 18))
    seiron_act_firework(my, player, 6, my->count[2] - 2850);
  if ((my->count[2] >= 2930) && (my->count[2] <= 2930 + 299))
    seiron_act_firework(my, player, 9, my->count[2] - 2930);
  if ((my->count[2] >= 3310) && (my->count[2] <= 3310 + 0))
    seiron_act_firework(my, player, 11, my->count[2] - 3310);
  if ((my->count[2] >= 3440) && (my->count[2] <= 3440 + 274))
    seiron_act_firework(my, player, 8, my->count[2] - 3440);
  if ((my->count[2] >= 3770) && (my->count[2] <= 3770 + 40))
    seiron_act_firework(my, player, 10, my->count[2] - 3770);

  (my->count[2])++;
  if (my->count[2] > 3900)
  {
    /* should not reach here */
    my->count[2] = 0;
  }

  return 0;
}

static void
seiron_act_firework(tenm_object *my, const tenm_object *player,
                    int what, int t)
{
  int i;
  int j;
  int n;
  int theta;
  double x;
  double y;
  double speed;
  int suffix = 3;

  /* sanity check */
  if (my == NULL)
    return;
  if (player == NULL)
    return;
  if (t < 0)
    return;

  switch (what)
  {
  case 0:
    if (t > 60)
      break;
    /* list of count
     * [suffix + 0] center x (relative to my->x)
     * [suffix + 1] center y (relative to my->y)
     * [suffix + 2] theta
     * [suffix + 3] dtheta
     */
    if (t == 0)
    {
      my->count[suffix + 0] = -5 + rand() % 11;
      my->count[suffix + 1] = -5 + rand() % 11;
      my->count[suffix + 2] = rand() % 360;
      if (rand() % 2 == 0)
        my->count[suffix + 3] = 2;
      else
        my->count[suffix + 3] = -2;
    }
    if (t % 20 == 0)
    {
      x = my->x + (double) (my->count[suffix + 0]);
      y = my->y + (double) (my->count[suffix + 1]);
      if (t <= 20)
      {
        speed = 3.0;
        n = 0;
      }
      else
      {
        speed = 4.5;
        n = 1;
      }

      theta = my->count[suffix + 2] + my->count[suffix + 3] * (t / 20);
      for (i = 0; i < 360; i += 8)
        tenm_table_add(laser_angle_new(x, y, speed, theta + i, 25.0, n));
    }
    break;
  case 1:
    if (t > 255)
      break;
    if (t <= 49)
    {
      for (i = 0; i < 4; i++)
      {
        x = my->x + (double) (-5 + rand() % 11);
        y = my->y + (double) (-5 + rand() % 11);
        if (t < 25)
          speed = 2.0 + (double) (rand() % (t + 1)) / 6.0;
        else
          speed = 2.0 + (double) (rand() % (50 - t)) / 6.0;
        
        theta = rand() % 360;
        n = 2;
        tenm_table_add(normal_shot_angle_new(x, y, speed, theta, n));
      }
    }
    if ((t >= 80) && (t <= 255))
    {
      if (t % 10 == 0)
      {
        /* list of count
         * [suffix + 0] center x (relative to my->x)
         * [suffix + 1] center y (relative to my->y)
         * [suffix + 2] theta
         */
        my->count[suffix + 0] = -5 + rand() % 11;
        my->count[suffix + 1] = -5 + rand() % 11;
        my->count[suffix + 2] = rand() % 360;

        x = my->x + (double) (my->count[suffix + 0]);
        y = my->y + (double) (my->count[suffix + 1]);
        speed = 12.0;
        n = 4;

        theta = my->count[suffix + 2];
        for (i = 0; i < 360; i += 120)
          tenm_table_add(laser_angle_new(x, y, speed, theta + i, 50.0, n));
      }
      else if (t % 10 == 5)
      {
        x = my->x + (double) (my->count[suffix + 0]);
        y = my->y + (double) (my->count[suffix + 1]);
        speed = 15.0;
        n = 5;

        theta = my->count[suffix + 2] + 180;
        for (i = 0; i < 360; i += 72)
          tenm_table_add(laser_angle_new(x, y, speed, theta + i, 50.0, n));
      }
    }
    break;
  case 2:
    if (t > 219)
      break;
    if (t < 60)
      speed = 2.5;
    else if (t < 180)
      speed = 2.5 + ((double) (t - 60)) / 12.0;
    else
      speed = 12.5;

    if (t % 2 == 0)
    {
      x = my->x + (double) (-30 + rand() % 61);
      y = my->y + (double) (-30 + rand() % 61);
      theta = rand() % 360;
      if (speed < 8.0)
        n = 4;
      else
        n = 5;
      for (i = 0; i < 2; i++)
      {
        tenm_table_add(laser_angle_new(x, y, speed, theta, 30.0, n));
        theta += 60;
      }
    }
    else
    {
      x = my->x;
      y = my->y;
      speed += 2.0;
      if (speed < 10.0)
        n = 2;
      else
        n = 3;
      theta = rand() % 360;
      tenm_table_add(normal_shot_angle_new(x, y, speed, theta, n));
    }
    break;
  case 3:
    if (t > 35)
      break;
    if (t == 0)
    {
      /* list of count
       * [suffix + 0] center x (relative to my->x)
       * [suffix + 1] center y (relative to my->y)
       */
      my->count[suffix + 0] = -5 + rand() % 11;
      my->count[suffix + 1] = -5 + rand() % 11;
    }

    x = my->x + (double) (my->count[suffix + 0]);
    y = my->y + (double) (my->count[suffix + 1]);

    if (t == 35)
    {
      theta = 90;
      switch (rand() % 3)
      {
      case 0:
        theta += 6;
        break;
      case 1:
        theta += 12;
        break;
      default:
        break;
      }

      for (i = 0; i < 40; i++)
      {
        tenm_table_add(laser_angle_new(x, y, 10.0, theta, 25.0, 3));
        if (i % 2 == 0)
          theta += 6;
        else
          theta += 12;
      }
    }

    if ((t >= 0) && (t < 30))
    {
      theta = -81 + t * 12;
      for (i = 0; i < 2; i++)
      {
        tenm_table_add(laser_angle_new(x, y, 5.0, theta, 25.0, 0));
        theta = 180 - theta;
      }
    }
    if ((t >= 5) && (t < 35))
    {
      theta = -81 + (t - 5) * 12 - 5;
      for (i = 0; i < 2; i++)
      {
        tenm_table_add(laser_angle_new(x, y, 8.0, theta, 25.0, 2));
        theta = 180 - theta;
      }
      theta = -81 + (t - 5) * 12 + 5;
      for (i = 0; i < 2; i++)
      {
        tenm_table_add(laser_angle_new(x, y, 6.5, theta, 25.0, 1));
        theta = 180 - theta;
      }
    }
    break;
  case 4:
    if (t > 87)
      break;
    /* list of count
     * [suffix + 0] center x (relative to my->x)
     * [suffix + 1] center y (relative to my->y)
     * [suffix + 2] theta
     * [suffix + 3] dtheta
     */
    if (t == 0)
    {
      my->count[suffix + 0] = -5 + rand() % 11;
      my->count[suffix + 1] = -5 + rand() % 11;
      my->count[suffix + 2] = rand() % 360;
      if (rand() % 2 == 0)
        my->count[suffix + 3] = 1;
      else
        my->count[suffix + 3] = -1;
    }

    x = my->x + (double) (my->count[suffix + 0]);
    y = my->y + (double) (my->count[suffix + 1]);

    if (t % 3 == 0)
    {
      if (t < 45)
      {
        speed = 2.0 + ((double) (t)) / 15.0;

        theta = my->count[suffix + 2]
          + 8 * my->count[suffix + 3] * (t / 3);
        for (i = 0; i < 360; i += 120)
          tenm_table_add(normal_shot_angle_new(x, y, speed, theta + i, 0));

        theta = my->count[suffix + 2]
          + (-8) * my->count[suffix + 3] * (t / 3)
          + 60;
        for (i = 0; i < 360; i += 120)
          tenm_table_add(normal_shot_angle_new(x, y, speed, theta + i, 4));
      }
      else if (t < 90)
      {
        speed = 7.0 - ((double) (t - 45)) / 15.0;

        theta = my->count[suffix + 2]
          + 8 * my->count[suffix + 3] * ((t - 45)/ 3)
          + 2 * my->count[suffix + 3];
        for (i = 0; i < 360; i += 120)
          tenm_table_add(normal_shot_angle_new(x, y, speed, theta + i, 0));

        theta = my->count[suffix + 2]
          + (-8) * my->count[suffix + 3] * ((t - 45)/ 3)
          + 60 - 4  * my->count[suffix + 3];
        for (i = 0; i < 360; i += 120)
          tenm_table_add(normal_shot_angle_new(x, y, speed, theta + i, 4));
      }
    }
    break;
  case 5:
    if (t > 55)
      break;
    /* list of count
     * [suffix + 0] center x (relative to my->x)
     * [suffix + 1] center y (relative to my->y)
     * [suffix + 2] theta
     * [suffix + 3] dtheta
     */
    if (t == 0)
    {
      my->count[suffix + 0] = -5 + rand() % 11;
      my->count[suffix + 1] = -5 + rand() % 11;
      my->count[suffix + 2] = rand() % 360;
      if (rand() % 2 == 0)
        my->count[suffix + 3] = 2;
      else
        my->count[suffix + 3] = -2;
    }

    x = my->x + (double) (my->count[suffix + 0]);
    y = my->y + (double) (my->count[suffix + 1]);

    if ((t <= 30) && (t % 10 == 0))
    {
      theta = my->count[suffix + 2] + my->count[suffix + 3] * (t / 10);
      for (i = 0; i < 360; i += 10)
        tenm_table_add(normal_shot_angle_new(x, y, 4.0, theta + i, 5));
    }
    if (t == 55)
    {
      theta = my->count[suffix + 2] + my->count[suffix + 3] * 4;
      for (i = 0; i < 360; i += 10)
        tenm_table_add(normal_shot_angle_new(x, y, 8.0, theta + i, 1));
    }
    break;
  case 6:
    if (t > 18)
      break;
    /* list of count
     * [suffix + 0] center x (relative to my->x)
     * [suffix + 1] center y (relative to my->y)
     * [suffix + 2] theta
     */
    if (t == 0)
    {
      my->count[suffix + 0] = -5 + rand() % 11;
      my->count[suffix + 1] = -5 + rand() % 11;
      my->count[suffix + 2] = rand() % 360;
    }

    x = my->x + (double) (my->count[suffix + 0]);
    y = my->y + (double) (my->count[suffix + 1]);

    if ((t <= 18) && (t % 6 == 0))
    {
      theta = my->count[suffix + 2] + (t / 6) * 3;
      if ((t == 0) || (t == 18))
        i = 20;
      else
        i = 40;
      for (; i > 0; i--)
      {
        tenm_table_add(laser_angle_new(x, y, 8.0, theta, 25.0, 2));
        if ((t == 0) || (t == 18))
        {  
          theta += 18;
        }
        else if (t == 6)
        {
          if (i % 2 == 0)
            theta += 12;
          else
            theta += 6;
        }
        else
        {
          if (i % 2 == 0)
            theta += 6;
          else
            theta += 12;
        }
      }
    }
    break;
  case 7:
    if (t > 60)
      break;
    /* list of count
     * [suffix + 0] center x (relative to my->x)
     * [suffix + 1] center y (relative to my->y)
     * [suffix + 2] theta
     * [suffix + 3] dtheta
     */
    if (t == 0)
    {
      my->count[suffix + 0] = -5 + rand() % 11;
      my->count[suffix + 1] = -5 + rand() % 11;
      my->count[suffix + 2] = rand() % 360;
      if (rand() % 2 == 0)
        my->count[suffix + 3] = 9;
      else
        my->count[suffix + 3] = -9;
    }

    x = my->x + (double) (my->count[suffix + 0]);
    y = my->y + (double) (my->count[suffix + 1]);

    if (t == 0)
    {
      theta = my->count[suffix + 2];
      for (i = 0; i < 5; i++)
      {
        for (j = 0; j < 360; j += 15)
        {
          tenm_table_add(laser_angle_new(x, y, 3.0 + ((double) i) * 0.45,
                                         theta + j, 25.0, 3));
        }
        theta += my->count[suffix + 3];
      }
    }
    if ((t == 40) || (t == 60))
    {
      if (rand() % 2 == 0)
        my->count[suffix + 3] = 140;
      else
        my->count[suffix + 3] = -140;
      theta = rand() % 360;
      for (i = 0; i < 360; i += 8)
      {
        tenm_table_add(normal_shot_angle_new(x + 30.0 * tenm_cos(theta + i),
                                             y + 30.0 * tenm_sin(theta + i),
                                             10.0,
                                             theta + i + my->count[suffix + 3],
                                             0));
      }
    }
    break;
  case 8:
    if (t > 274)
      break;
    /* list of count
     * [suffix + 0] center x (relative to my->x)
     * [suffix + 1] center y (relative to my->y)
     * [suffix + 2] theta
     * [suffix + 3] dtheta
     */
    if (t == 0)
    {
      my->count[suffix + 0] = -5 + rand() % 11;
      my->count[suffix + 1] = -5 + rand() % 11;
      my->count[suffix + 2] = rand() % 360;
      if (rand() % 2 == 0)
        my->count[suffix + 3] = 8;
      else
        my->count[suffix + 3] = -8;
    }

    x = my->x + (double) (my->count[suffix + 0]);
    y = my->y + (double) (my->count[suffix + 1]);

    if (t == 0)
    {
      theta = my->count[suffix + 2];
      for (i = 0; i < 360; i += 8)
        tenm_table_add(normal_shot_angle_new(x, y, 1.5, theta + i, 3));
    }
    if (t == 30)
    {
      theta = my->count[suffix + 2] + 4;
      for (i = 0; i < 360; i += 8)
        tenm_table_add(normal_shot_angle_new(x, y, 1.6, theta + i, 3));
    }

    if ((t >= 140) && (t < 275))
    {
      speed = 9.0 + ((double) (rand() % 13)) / 3.0;
      theta = my->count[suffix + 2] + 2 + my->count[suffix + 3] * (t - 140);
      tenm_table_add(laser_angle_new(x, y, speed, theta, 25.0, 5));
      tenm_table_add(laser_angle_new(x, y, speed, theta + 180, 25.0, 5));
    }
    break;
  case 9:
    if (t > 300)
      break;
    /* list of count
     * [suffix + 0] center x (relative to my->x)
     * [suffix + 1] center y (relative to my->y)
     * [suffix + 2] theta
     * [suffix + 3] dtheta
     */
    if (t == 0)
    {
      my->count[suffix + 0] = -5 + rand() % 11;
      my->count[suffix + 1] = -5 + rand() % 11;
      my->count[suffix + 2] = rand() % 360;
      if (rand() % 2 == 0)
        my->count[suffix + 3] = 51;
      else
        my->count[suffix + 3] = -51;
    }

    x = my->x + (double) (my->count[suffix + 0]);
    y = my->y + (double) (my->count[suffix + 1]);

    if (t == 0)
    {
      theta = my->count[suffix + 2] + 180;
      for (i = 0; i < 360; i += 18)
        for (j = 0; j < 360; j += 120)
          tenm_table_add(laser_angle_new(x + 15.0 * tenm_cos(theta + i),
                                         y + 15.0 * tenm_sin(theta + i),
                                         1.5,
                                         theta + i + j + 180,
                                         50.0, 2));
    }
    if ((t >= 60) && (t < 300) && (t % 2 == 0))
    {
      theta = my->count[suffix + 2] + my->count[suffix + 3] * ((t - 60) / 2);
      if (t % 6 == 0)
        speed = 10.0;
      else
        speed = 12.0;
      for (i = 0; i < 5; i++)
      {
        tenm_table_add(laser_angle_new(x + 30.0 * tenm_cos(theta),
                                       y + 30.0 * tenm_sin(theta),
                                       speed,
                                       theta + i * 72 + 36,
                                       50.0, 0));
      }
    }
    break;
  case 10:
    if (t > 40)
      break;
    /* list of count
     * [suffix + 0] center x (relative to my->x)
     * [suffix + 1] center y (relative to my->y)
     * [suffix + 2] speed period
     */
    if (t == 0)
    {
      my->count[suffix + 0] = -5 + rand() % 11;
      my->count[suffix + 1] = -5 + rand() % 11;
      my->count[suffix + 2] = rand() % 3;
    }

    x = my->x + (double) (my->count[suffix + 0]);
    y = my->y + (double) (my->count[suffix + 1]);

    if ((t >= 0) && (t < 30))
    {
      if (my->count[suffix + 2] == 0)
        n = 3;
      else
        n = 5;
      theta = -81 + 12 * t - 8;
      speed = 8.0
        + 1.0 * tenm_cos(theta * n + 89);
      for (i = 0; i < 2; i++)
      {
        tenm_table_add(laser_angle_new(x, y, speed, theta, 25.0, 0));
        theta = 180 - theta;
      }
    }
    if ((t >= 5) && (t < 30 + 5))
    {
      if (my->count[suffix + 2] == 1)
        n = 3;
      else
        n = 5;
      theta = -81 + 12 * (t - 5) - 2;
      speed = 8.5
        + 1.5 * tenm_cos(theta * n + 83);
      for (i = 0; i < 2; i++)
      {
        tenm_table_add(laser_angle_new(x, y, speed, theta, 25.0, 1));
        theta = 180 - theta;
      }
    }
    if ((t >= 10) && (t < 30 + 10))
    {
      if (my->count[suffix + 2] == 2)
        n = 3;
      else
        n = 5;
      theta = -81 + 12 * (t - 10);
      speed = 9.0
        + 2.0 * tenm_cos(theta * n + 81);
      for (i = 0; i < 2; i++)
      {
        tenm_table_add(laser_angle_new(x, y, speed, theta, 25.0, 2));
        theta = 180 - theta;
      }
    }
    break;
  case 11:
    if (t > 0)
      break;
    /* list of count
     * [suffix + 0] center x (relative to my->x)
     * [suffix + 1] center y (relative to my->y)
     * [suffix + 2] theta
     */
    if (t == 0)
    {
      my->count[suffix + 0] = -5 + rand() % 11;
      my->count[suffix + 1] = -5 + rand() % 11;
      my->count[suffix + 2] = rand() % 360;
    }

    x = my->x + (double) (my->count[suffix + 0]);
    y = my->y + (double) (my->count[suffix + 1]);

    if (t == 0)
    {
      for (i = 0; i < 360; i += 12)
      {
        theta = my->count[suffix + 2];
        tenm_table_add(normal_shot_angle_new(x + 30.0 * tenm_cos(theta + i),
                                             y + 30.0 * tenm_sin(theta + i),
                                             7.0, theta + i + 180, 5));
        tenm_table_add(normal_shot_angle_new(x + 30.0 * tenm_cos(theta + i),
                                             y + 30.0 * tenm_sin(theta + i),
                                             8.0, theta + i + 180, 5));

        tenm_table_add(normal_shot_angle_new(x + 30.0 * tenm_cos(theta + i),
                                             y + 30.0 * tenm_sin(theta + i),
                                             6.5, theta + i + 180 - 20, 3));
        tenm_table_add(normal_shot_angle_new(x + 30.0 * tenm_cos(theta + i),
                                             y + 30.0 * tenm_sin(theta + i),
                                             7.5, theta + i + 180 - 40, 3));
        tenm_table_add(normal_shot_angle_new(x + 30.0 * tenm_cos(theta + i),
                                             y + 30.0 * tenm_sin(theta + i),
                                             6.5, theta + i + 180 + 20, 3));
        tenm_table_add(normal_shot_angle_new(x + 30.0 * tenm_cos(theta + i),
                                             y + 30.0 * tenm_sin(theta + i),
                                             7.5, theta + i + 180 + 40, 3));
      }
    }
    break;
  default:
    fprintf(stderr, "seiron_act_firework: strange what (%d)\n", what);
    break;
  }

  return;
}

static int
seiron_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  char temp[32];

  /* sanity check */
  if (my == NULL)
    return 0;

  /* dead enemy has low priority */
  if (((my->count[11] != 2) && (priority == 0))
      || ((my->count[11] == 2) && (priority == -1)))
  {
    if (my->count[11] != 2)
    {
      /* decoration */
      if ((my->count[2] >= 1752) && (my->count[2] < 3870))
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

      if (tenm_draw_line((int) (my->x + 100.0 * tenm_cos(my->count[8])),
                         (int) (my->y - 30
                                + 100.0 * tenm_sin(my->count[8])
                                * tenm_sin(my->count[9])),
                         (int) (my->x + 100.0 * tenm_cos(my->count[8] + 120)),
                         (int) (my->y - 30
                                + 100.0 * tenm_sin(my->count[8] + 120)
                                * tenm_sin(my->count[9])),
                         1, color) != 0)
        status = 1;
      if (tenm_draw_line((int) (my->x + 100.0 * tenm_cos(my->count[8] + 120)),
                         (int) (my->y - 30
                                + 100.0 * tenm_sin(my->count[8] + 120)
                                * tenm_sin(my->count[9])),
                         (int) (my->x + 100.0 * tenm_cos(my->count[8] + 240)),
                         (int) (my->y - 30
                                + 100.0 * tenm_sin(my->count[8] + 240)
                                * tenm_sin(my->count[9])),
                         1, color) != 0)
        status = 1;
      if (tenm_draw_line((int) (my->x + 100.0 * tenm_cos(my->count[8] + 240)),
                         (int) (my->y - 30
                                + 100.0 * tenm_sin(my->count[8] + 240)
                                * tenm_sin(my->count[9])),
                         (int) (my->x + 100.0 * tenm_cos(my->count[8])),
                         (int) (my->y - 30
                                + 100.0 * tenm_sin(my->count[8])
                                * tenm_sin(my->count[9])),
                         1, color) != 0)
        status = 1;

      if (tenm_draw_line((int) (my->x + 100.0 * tenm_cos(60 - my->count[8])),
                         (int) (my->y + 30
                                + 100.0 * tenm_sin(60 - my->count[8])
                                * tenm_sin(my->count[9])),
                         (int) (my->x + 100.0 * tenm_cos(300 - my->count[8])),
                         (int) (my->y + 30
                                + 100.0 * tenm_sin(300 - my->count[8])
                                * tenm_sin(my->count[9])),
                         1, color) != 0)
        status = 1;
      if (tenm_draw_line((int) (my->x + 100.0 * tenm_cos(300 - my->count[8])),
                         (int) (my->y + 30
                                + 100.0 * tenm_sin(300 - my->count[8])
                                * tenm_sin(my->count[9])),
                         (int) (my->x + 100.0 * tenm_cos(180 - my->count[8])),
                         (int) (my->y + 30
                                + 100.0 * tenm_sin(180 - my->count[8])
                                * tenm_sin(my->count[9])),
                         1, color) != 0)
        status = 1;
      if (tenm_draw_line((int) (my->x + 100.0 * tenm_cos(180 - my->count[8])),
                         (int) (my->y + 30
                                + 100.0 * tenm_sin(180 - my->count[8])
                                * tenm_sin(my->count[9])),
                         (int) (my->x + 100.0 * tenm_cos(60 - my->count[8])),
                         (int) (my->y + 30
                                + 100.0 * tenm_sin(60 - my->count[8])
                                * tenm_sin(my->count[9])),
                         1, color) != 0)
        status = 1;
    }
    
    /* body */
    if ((my->count[2] >= 1752) && (my->count[2] < 3870))
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
                         45, 3, color) != 0)
      status = 1;

    /* hit point stat */
    if (my->count[11] == 1)
    {
      sprintf(temp, "%d", my->hit_point);
      if (draw_string((int) my->x, (int) my->y, temp, (int) strlen(temp)) != 0)
      {
        fprintf(stderr, "seiron_draw: draw_string failed\n");
        status = 1;
      }
    }
  }

  return status;
}

static void
seiron_explosion(tenm_object *my)
{
  int n;
  double speed_theta;

  /* sanity check */
  if (my == NULL)
    return;
  if ((my->count[2] >= 1752) && (my->count[2] < 3870))
  {    
    n = 8;
    speed_theta = -30.0;
  }
  else
  {
    n = 7;
    speed_theta = 15.0;
  }
  tenm_table_add(fragment_new(my->x, my->y - 30.0, 0.0, -2.0,
                              50.0, 36, n, 4.0, speed_theta, 20));
  tenm_table_add(fragment_new(my->x, my->y + 30.0, 0.0, 2.0,
                              50.0, 36, n, 4.0, -speed_theta, 20));
  tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                               1, 3000, n, 5.0, 8));

  /* don't modify my->attr or my->hit_mask here, or the player shot
   * may fly through the enemy */
  tenm_mass_delete(my->mass);
  my->mass = NULL;
  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy_shot), 0);
  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy), 0);
  my->count[0] = 0;
  my->count[1] = 0;
  my->count[11] = 2;
  my->count[12] = 0;
  /* don't reset count[2] here */

  return;
}
