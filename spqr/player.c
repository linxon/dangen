/* $Id: player.c,v 1.187 2005/07/12 18:12:06 oohara Exp $ */

#include <stdio.h>
/* malloc */
#include <stdlib.h>
/* strcmp, strlen */
#include <string.h>

#include "const.h"
#include "tenm_graphic.h"
#include "tenm_input.h"
#include "tenm_object.h"
#include "tenm_primitive.h"
#include "player-shot.h"
#include "tenm_table.h"
#include "tenm_math.h"
#include "fragment.h"
#include "explosion.h"
#include "chain.h"
#include "ship.h"
/* delete_enemy_shot */
#include "util.h"
/* easter egg */
#include "plan-0.h"

#include "player.h"

#define TIME_RESTART 40
#define TIME_IMMUTABLE 50
#define TIME_OPPOSITE_DIRECTION 15
#define TIME_ROLLING 24

static int player_draw(tenm_object *my, int priority);
static int player_move(tenm_object *my, double turn_per_frame);
static int player_hit(tenm_object *my, tenm_object *your);
static int player_act(tenm_object *my, const tenm_object *player);

static int player_draw_wing(int x, int y, int n, int theta,
                            int red_orig, int green_orig, int blue_orig);
static int player_shoot(tenm_object *my, int k);
static int player_shoot_direction(int k);
static int player_tutorial_input(int t);

tenm_object *
player_new(int tutorial)
{
  double x;
  double y;
  tenm_primitive **p = NULL;
  int *count = NULL;
  double *count_d = NULL;
  tenm_object *player = NULL;

  /* sanity check */
  if ((tutorial < 0) || (tutorial > 1))
  {
    fprintf(stderr, "player_new: strange tutorial (%d)\n", tutorial);
    return NULL;
  }
  
  x = ((double) WINDOW_WIDTH) / 2.0;
  y = ((double) WINDOW_HEIGHT) * 0.9;
  
  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "player_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_circle_new(x, y, 5.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "player_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 10);
  if (count == NULL)
  {
    fprintf(stderr, "player_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "player_new: malloc(count_d) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    if (count != NULL)
      free(count);
    return NULL;
  }

  count[0] = 0;
  count[1] = 50;
  count[2] = tutorial;
  count[3] = 0;
  count[4] = 0;
  count[5] = 0;
  count[6] = -1;
  count[7] = 0;
  count[8] = 0;
  count[9] = 0;

  count_d[0] = 0.0;
  count_d[1] = 0.0;

  /* list of count
   * [0] shoot wait (no auto-repeat)
   *   -1: the shot key is being pressed and has not been released
   *       since the last shot
   *    0: the shot key is not pressed
   *    1: the shot key is being pressed and has been released
   *       since the last shot
   * [1] immutable time
   * [2] input mode
   *    0: normal game
   *    1: tutorial
   *    2: easter egg
   * [3] input timer (for tutorial)
   * [4] rolling/recharge timer
   * [5] rolling theta
   * [6] rolling direction
   * [7] normal roll theta
   * [8] slow move timer
   * [9] easter egg trigger timer
   */
  /*
   * list of cound_d
   * [0] speed x
   * [1] speed y
   */
  player = tenm_object_new("player", ATTR_PLAYER, 0,
                           1, x, y,
                           10, count, 2, count_d, 1, p, 
                           (int (*)(tenm_object *, double)) (&player_move),
                           (int (*)(tenm_object *, tenm_object *))
                           (&player_hit),
                           (int (*)(tenm_object *, const tenm_object *))
                           (&player_act),
                           (int (*)(tenm_object *, int)) (&player_draw));
  if (player == NULL)
  {
    fprintf(stderr, "player_new: tenm_object_new failed\n");
    (p[0])->delete(p[0]);
    free(p);
    if (count != NULL)
      free(count);
    if (count_d != NULL)
      free(count_d);
    return NULL;
  }

  return player;
}

static int
player_draw(tenm_object *my, int priority)
{
  int red;
  int green;
  int blue;
  int theta;
  int k;
  int status = 0;

  /* sanity check */
  if ((priority < 0) || (priority >= 2))
    return 0;
  /* we don't check my == NULL here because this can be called
   * even if the game is over */

  /* no need to draw the player if the game is over */
  if (get_ship() < 0)
  {
    if (priority == 1)
      draw_string(280, 280, "game over", 9);
    return 0;
  }

  /* sanity check again */
  if (my == NULL)
    return 0;
  if (priority != 0)
    return 0;

  /* input display */
  if (my->count[2] == 1)
  {
    k = player_tutorial_input(my->count[3]);

    if (k & 1)
    {
      if (draw_string(490, 320, " UP  ", 5) != 0)
        status = 1;
    }
    else
    {
      if (draw_string(490, 320, "-----", 5) != 0)
        status = 1;
    }

    if (k & 2)
    {
      if (draw_string(490, 360, "DOWN ", 5) != 0)
        status = 1;
    }
    else
    {
      if (draw_string(490, 360, "-----", 5) != 0)
        status = 1;
    }

    if (k & 4)
    {
      if (draw_string(540, 340, "RIGHT", 5) != 0)
        status = 1;
    }
    else
    {
      if (draw_string(540, 340, "-----", 5) != 0)
        status = 1;
    }

    if (k & 8)
    {
      if (draw_string(440, 340, "LEFT ", 5) != 0)
        status = 1;
    }
    else
    {
      if (draw_string(440, 340, "-----", 5) != 0)
        status = 1;
    }

    if (k & 16)
    {
      if (draw_string(590, 360, "SPACE", 5) != 0)
        status = 1;
    }
    else
    {
      if (draw_string(590, 360, "-----", 5) != 0)
        status = 1;
    }

    if (draw_string(460, 400, "press ESC to quit", 17) != 0)
      status = 1;
  }

  if (my->count[1] > TIME_IMMUTABLE)
    return 0;

  /* decoration */
  if (my->count[1] > 0)
  {
    if (my->count[4] > 0)
    {
      red = 216;
      green = 167;
      blue = 214;
    }
    else
    {
      red = 172;
      green = 204;
      blue = 214;
    }

    if (tenm_draw_circle((int) (my->x), (int) (my->y),
                         5 + my->count[1] * 3, 1,
                         tenm_map_color(red, green, blue)) != 0)
      status = 1;
  }
  
  /* body */
  if (my->count[1] > 0)
  {
    if (my->count[4] > 0)
    {
      red = 177;
      green = 89;
      blue = 237;
    }
    else
    {
      red = 89;
      green = 164;
      blue = 237;
    }
  }
  else
  {
    if (my->count[4] > 0)
    {
      red = 86;
      green = 10;
      blue = 139;
    }
    else
    {
      red = 10;
      green = 75;
      blue = 139;
    }
  }

  theta = my->count[5] + my->count[7];
  player_draw_wing((int) (my->x), (int) (my->y), 0, theta,
                   red, green, blue);
  player_draw_wing((int) (my->x), (int) (my->y), 0, theta + 120,
                   red, green, blue);
  player_draw_wing((int) (my->x), (int) (my->y), 0, theta - 120,
                   red, green, blue);

  player_draw_wing((int) (my->x), (int) (my->y), 1, theta + 90,
                   red, green, blue);
  player_draw_wing((int) (my->x), (int) (my->y), 1, theta - 90,
                   red, green, blue);

  if (tenm_draw_circle((int) (my->x), (int) (my->y), 5, 2,
                       tenm_map_color(red, green, blue)) != 0)
    status = 1;
  
  /* may be useful for debugging */
  /*
  if (tenm_draw_primitive(my->mass->p[0], tenm_map_color(0, 0, 0)) != 0)
    status = 1;
  */
  /*
  {
    char temp[32];
    sprintf(temp, "%3.1f", my->x);
    if (my->x > ((double) WINDOW_WIDTH * 2 / 3))
    {      
      if (draw_string((int) (my->x - 100.0), (int) (my->y - 10.0),
                      temp, (int) strlen(temp)) != 0)
        status = 1;
    }
    else
    {
      if (draw_string((int) (my->x + 50.0), (int) (my->y - 10.0),
                      temp, (int) strlen(temp)) != 0)
        status = 1;
    }
    sprintf(temp, "%3.1f", my->y);
    if (my->x > ((double) WINDOW_WIDTH * 2 / 3))
    {      
      if (draw_string((int) (my->x - 100.0), (int) (my->y + 10.0),
                      temp, (int) strlen(temp)) != 0)
        status = 1;
    }
    else
    {
      if (draw_string((int) (my->x + 50.0), (int) (my->y + 10.0),
                      temp, (int) strlen(temp)) != 0)
        status = 1;
    }
  }
  */

  return status;
}

static int
player_move(tenm_object *my, double turn_per_frame)
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

  /* boundary check */
  if (my->x + dx_temp < 0.0)
    dx_temp = -(my->x);
  else if (my->x + dx_temp > (double) WINDOW_WIDTH)
    dx_temp = ((double) WINDOW_WIDTH) - my->x;
  if (my->y + dy_temp < 0.0)
    dy_temp = -(my->y);
  else if (my->y + dy_temp > (double) WINDOW_HEIGHT)
    dy_temp = ((double) WINDOW_HEIGHT) - my->y;

  my->x += dx_temp;
  my->y += dy_temp;
  tenm_move_mass(my->mass, dx_temp, dy_temp);
  
  return 0;
}

static int
player_hit(tenm_object *my, tenm_object *your)
{
  double speed_theta;

  /* sanity check */
  if (my == NULL)
    return 0;
  if (your == NULL)
    return 0;

  if (my->count[4] > 0)
    speed_theta = 30.0 * ((double) (my->count[6]));
  else
    speed_theta = 0.0;
  
  tenm_table_add(fragment_new(my->x, my->y,
                              my->count_d[0] * 0.5, my->count_d[1] * 0.5,
                              50.0, 5, 10, 5.0, speed_theta, 20));
  tenm_table_add(explosion_new(my->x, my->y,
                               my->count_d[0] * 0.5, my->count_d[1] * 0.5,
                               3, 50, 10, 2.0, 12));

  my->hit_mask = 0;
  my->hit_point = 0;
  my->count[0] = -1;
  my->count[1] = TIME_RESTART + TIME_IMMUTABLE;
  my->count_d[0] = 0.0;
  my->count_d[1] = 0.0;

  return 0;
}

/* list of keys
 * 1 TENM_KEY_UP
 * 2 TENM_KEY_DOWN
 * 4 TENM_KEY_RIGHT
 * 8 TENM_KEY_LEFT
 * 16 TENM_KEY_SPACE (shoot)
 * 32 TENM_KEY_ESCAPE (quit)
 * 64 TENM_KEY_p (pause)
 * 128 TENM_KEY_CAPSLOCK (cheat --- slow down)
 */
/* note that "player" (arg 2) is NULL */
static int
player_act(tenm_object *my, const tenm_object *player)
{
  int key_status;
  double speed;
  double speed_diagonal;

  /* sanity check */
  if (my == NULL)
    return 0;

  if ((get_ship() < 0) || (my->count[1] > 0))
    (my->count[1])--;

  if (get_ship() < 0)
  {
    if (my->count[1] <= TIME_IMMUTABLE * (-2))
      return 1;
    return 0;
  }

  key_status = tenm_get_key_status();
  if (my->count[2] == 1)
  {
    if (key_status & 31)
    {
      (my->count[9])++;
      if (my->count[9] >= 40)
      {
        tenm_table_apply_all((int (*)(tenm_object *, int))
                             (&plan_0_more_1_easter_egg), 0);
        my->count[2] = 2;
        my->count[9] = 0;
      }
    }
    else
    {
      my->count[9] = 0;
    }

    /* tutorial input */
    key_status = player_tutorial_input(my->count[3]);
    (my->count[3])++;
  }

  if (my->count[1] == TIME_IMMUTABLE)
  {
    /* player restarts */
    add_ship(-1);
    if (get_ship() < 0)
      return 0;

    player_neutral_position(my);

    /* clear enemy shots */
    tenm_table_apply_all((int (*)(tenm_object *, int))
                         (&delete_enemy_shot), 0);
  }
  if (my->count[1] <= 0)
    my->hit_mask = ATTR_BOSS | ATTR_ENEMY | ATTR_ENEMY_SHOT
      | ATTR_OBSTACLE | ATTR_OPAQUE;

  if (my->hit_point == 0)
    return 0;

  /* note that 8.0 * sqrt(2.0) / 2.0 = 5.656... */
  /*  
  speed = 8.0;
  speed_diagonal = 5.656;
  */
  if (my->count[8] < 0)
    my->count[8] = 0;
  if (my->count[8] > 10)
    my->count[8] = 10;
  /*
  speed = 5.5 + ((double) (my->count[8])) * 0.25;
  */
  speed = 2.5 + ((double) (my->count[8])) * 0.55;
  speed_diagonal = speed * tenm_sqrt(2) / 2.0;
  if (!(key_status & 16))
  {  
    if (key_status & 15)
    {
      my->count[8] += 2;
      if (my->count[8] > 10)
        my->count[8] = 10;
    }
    else
    {
      (my->count[8])--;
      if (my->count[8] < 0)
        my->count[8] = 0;
      /*
      my->count[8] = 0;
      */
    }
  }
  
  /* first, refuse to move for a stupid player */
  if ((key_status & 1) && (key_status & 2))
  {
    my->count_d[1] = 0.0;
    my->count_d[0] = 0.0;
    /* don't reset my->count[8] here to prevent abuse of "wrong" input */
  }
  else if ((key_status & 4) && (key_status & 8))
  {
    my->count_d[1] = 0.0;
    my->count_d[0] = 0.0;
    /* don't reset my->count[8] here to prevent abuse of "wrong" input */
  }
  else if (key_status & 1)
  {
    if (key_status & 4)
    {
      my->count_d[1] = -speed_diagonal;
      my->count_d[0] = speed_diagonal;
    }
    else if (key_status & 8)
    {
      my->count_d[1] = -speed_diagonal;
      my->count_d[0] = -speed_diagonal;
    }
    else
    {
      my->count_d[1] = -speed;
      my->count_d[0] = 0.0;
    }
  }
  else if (key_status & 2)
  {
    if (key_status & 4)
    {
      my->count_d[1] = speed_diagonal;
      my->count_d[0] = speed_diagonal;
    }
    else if (key_status & 8)
    {
      my->count_d[1] = speed_diagonal;
      my->count_d[0] = -speed_diagonal;
    }
    else
    {
      my->count_d[1] = speed;
      my->count_d[0] = 0.0;
    }
  }
  else
  {
    if (key_status & 4)
    {
      my->count_d[1] = 0.0;
      my->count_d[0] = speed;
    }
    else if (key_status & 8)
    {
      my->count_d[1] = 0.0;
      my->count_d[0] = -speed;
    }
    else
    {
      my->count_d[1] = 0.0;
      my->count_d[0] = 0.0;
    }
  }
  /* normal rolling */
  if ((key_status & 8) && (!(key_status & 4)))
  {
    my->count[7] += 3;
    if (my->count[7] > 30)
      my->count[7] = 30;
  }
  else if ((key_status & 4) && (!(key_status & 8)))
  {
    my->count[7] -= 3;
    if (my->count[7] < -30)
      my->count[7] = -30;
  }
  else
  {
    if (my->count[7] > 3)
      my->count[7] -= 3;
    else if (my->count[7] < -3)
      my->count[7] += 3;
    else
      my->count[7] = 0;
  }

  /* rolling/recharge count down */
  if (my->count[4] > 0)
  {
    my->count[5] += 15 * my->count[6];
    while (my->count[5] > 360)
      my->count[5] -= 360;
    while (my->count[5] < 0)
      my->count[5] += 360;
    (my->count[4])--;
  }

  /* shoot */
  if (key_status & 16)
  {
    my->count_d[1] = 0.0;
    my->count_d[0] = 0.0;
    /* don't reset my->count[8] --- no speed down */

    if ((my->count[4] <= 0) && (my->count[0] >= 0))
    {
      switch (key_status & 15)
      {
      case 1:
      case 2:
      case 4:
      case 8:
        player_shoot(my, key_status & 15);
        break;
      default:
        my->count[0] = 1;
        break;
      }
    }
  }
  else
  {
    if (my->count[0] > 0)
    {
      player_shoot(my, 0);
    }
    my->count[0] = 0;
  }

  return 0;
}

void
player_neutral_position(tenm_object *my)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
    return;
  if (strcmp(my->name, "player") != 0)
    return;

  my->hit_point = 1;

  dx_temp = ((double) WINDOW_WIDTH) / 2.0 - my->x;
  dy_temp = ((double) WINDOW_HEIGHT) * 0.9 - my->y;

  my->x += dx_temp;
  my->y += dy_temp;
  tenm_move_mass(my->mass, dx_temp, dy_temp);

  /* reset rolling */
  my->count[4] = 0;
  my->count[5] = 0;
  /*
  my->count[6] = 1;
  */

  /* reset shoot */
  my->count[0] = 0;

  /* neutral rolling */
  my->count[7] = 0;

  /* reset move speed */
  my->count[8] = 0;

  /* reset chain */
  clear_chain();
}

static int
player_draw_wing(int x, int y, int n, int theta,
                 int red_orig, int green_orig, int blue_orig)
{
  double dx;
  double dy;
  int temp;
  tenm_color color;
  int status = 0;
  double length;
  int red;
  int green;
  int blue;

  while (theta < 0)
    theta += 360;
  while (theta >= 360)
    theta -= 360;

  /* these magic numbers are chosen so that the rear wings indicate
   * the path of player shots
   */
  if (n == 0)
  {
    length = 35.0;
    dx = length * 0.8917 * tenm_sin(theta + 180);
    dy = length * (0.4526 * 0.9659 + 0.8917 * 0.2588 * tenm_cos(theta + 180));
    dx *= -1.0;
    dy *= -1.0;
  }
  else
  {
    length = 50.0;
    dx = length * 0.9182 * tenm_sin(theta);
    dy = length * (0.3961 * 0.9659 + 0.9182 * 0.2588 * tenm_cos(theta));
  }
  
  if (theta <= 180)
    temp = 180 - theta;
  else
    temp = theta - 180;

  red = (red_orig * (256 - temp) + DEFAULT_BACKGROUND_RED * temp) / 256;
  green = (green_orig * (256 - temp) + DEFAULT_BACKGROUND_GREEN * temp) / 256;
  blue = (blue_orig * (256 - temp) + DEFAULT_BACKGROUND_BLUE * temp) / 256;
  color = tenm_map_color(red, green, blue);

  if (tenm_draw_line(x, y, x + (int) dx, y + (int) dy, 1, color) != 0)
  {
    status = 1;
  }

  return status;
}

/* return 0 on success, 1 on error */
static int
player_shoot(tenm_object *my, int k)
{
  /* sanity check */
  if (my == NULL)
  {
    /* not an error */
    return 0;
  }
  if (k < 0)
  {
    fprintf(stderr, "player_shoot_: k is negative (%d)\n", k);
    return 1;
  }

  tenm_table_add(player_shot_new(my->x, my->y,
                                 player_shoot_direction(k)));
  my->count[4] = TIME_ROLLING;
  my->count[0] = -1;

  /* set rolling direction */
  switch (player_shoot_direction(k))
  {
  case 0:
    my->count[6] *= -1;
    break;
  case 1:
    my->count[6] *= -1;
    break;
  case 2:
    my->count[6] = -1;
    break;
  case 3:
    my->count[6] = 1;
    break;
  default:
    break;
  }

  return 0;
}

static int
player_shoot_direction(int k)
{
  /* sanity check */
  if (k < 0)
  {
    fprintf(stderr, "player_shoot_direction: k is negative (%d)\n", k);
    return 0;
  }

  if ((k & 1) && (k & 2))
  {
    return 0;
  }
  else if ((k & 4) && (k & 8))
  {
    return 0;
  }
  else
  {
    if (k & 4)
      return 2;
    else if (k & 8)
      return 3;
    else if (k & 2)
      return 1;
    else
      return 0;
  }

  /* should not reach here */
  fprintf(stderr, "player_shoot_direction: fall off (%d)\n", k);
  return 0;
}

static int
player_tutorial_input(int t)
{
  /* sanity check */
  if (t < 0)
  {
    fprintf(stderr, "player_tutorial_input: t is negative (%d)\n", t);
    return 0;
  }

  if ((t >= 160) && (t < 190))
    return 8;
  if ((t >= 200) && (t < 230))
    return 1;
  if ((t >= 240) && (t < 260))
    return 6;
  if ((t >= 260) && (t < 270))
    return 4;
  if ((t >= 270) && (t < 280))
    return 10;
  if ((t >= 280) && (t < 285))
    return 8;

  if ((t >= 340) && (t < 700))
    return 16;
  if ((t >= 700) && (t < 730))
    return 17;
  if ((t >= 730) && (t < 735))
    return 16;

  if ((t >= 770) && (t < 775))
    return 16;
  if ((t >= 775) && (t < 780))
    return 24;
  if ((t >= 780) && (t < 785))
    return 16;

  if ((t >= 2235) && (t < 2240))
    return 16;
  if ((t >= 2240) && (t < 2245))
    return 17;
  if ((t >= 2245) && (t < 2250))
    return 16;

  if ((t >= 2260) && (t < 2278))
    return 8;

  if ((t >= 2295) && (t < 2300))
    return 16;
  if ((t >= 2300) && (t < 2305))
    return 17;
  if ((t >= 2305) && (t < 2310))
    return 16;

  if ((t >= 2320) && (t < 2338))
    return 8;

  if ((t >= 2355) && (t < 2360))
    return 16;
  if ((t >= 2360) && (t < 2365))
    return 17;
  if ((t >= 2365) && (t < 2370))
    return 16;

  if ((t >= 2660) && (t < 2665))
    return 16;
  if ((t >= 2665) && (t < 2670))
    return 20;
  if ((t >= 2670) && (t < 2675))
    return 16;

  if ((t >= 2860) && (t < 2865))
    return 16;
  if ((t >= 2865) && (t < 2870))
    return 17;
  if ((t >= 2870) && (t < 2875))
    return 16;

  if ((t >= 2890) && (t < 2920))
    return 4;

  return 0;
}
