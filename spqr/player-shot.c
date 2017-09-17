/* $Id: player-shot.c,v 1.93 2004/08/30 16:23:27 oohara Exp $ */

#include <stdio.h>
/* malloc, rand */
#include <stdlib.h>
/* strcmp */
#include <string.h>

#include "const.h"
#include "tenm_graphic.h"
#include "tenm_object.h"
#include "tenm_primitive.h"
#include "tenm_math.h"
#include "tenm_input.h"
#include "util.h"
#include "score.h"
#include "chain.h"

#include "player-shot.h"

#define NEAR_ZERO 0.0001

static int player_shot_move(tenm_object *my, double turn_per_frame);
static int player_shot_hit(tenm_object *my, tenm_object *your);
static int player_shot_draw(tenm_object *my, int priority);
static int player_shot_act(tenm_object *my, const tenm_object *player);

static int player_shot_explode(tenm_object *my);

static int deal_damage2(tenm_object *my, tenm_object *your);

tenm_object *
player_shot_new(double x, double y, int n)
{
  tenm_primitive **p;
  tenm_object *new;
  int *count = NULL;
  double *count_d = NULL;

  /* sanity check */
  if ((n < 0) || (n >= 4))
  {
    fprintf(stderr, "player_shot_new: strange n (%d)\n", n);
    return NULL;
  }

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "player_shot_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_circle_new(x, y, 10.0);

  if (p[0] == NULL)
  {
    fprintf(stderr, "player_shot_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 4);
  if (count == NULL)
  {
    fprintf(stderr, "player_shot_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  count_d = (double *) malloc(sizeof(double) * 4);
  if (count_d == NULL)
  {
    fprintf(stderr, "player_new: malloc(count_d) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    if (count != NULL)
      free(count);
    return NULL;
  }

  count[0] = 1;
  count[1] = 0;
  count[2] = 0;
  count[3] = 0;

  if (n == 0)
  {
    count_d[0] = 0.0;
    count_d[1] = -52.0;
  }
  else if (n == 1)
  {
    count_d[0] = 0.0;
    count_d[1] = 52.0;
  }
  else if (n == 2)
  {
    count_d[0] = 48.0;
    count_d[1] = -20.0;
  }
  else
  {
    count_d[0] = -48.0;
    count_d[1] = -20.0;
  }

  count_d[2] = x;
  count_d[3] = y;

  /* list of count
   * [0] phase (begins with 1)
   * [1] explosion timer
   * [2] trail timer
   * [3] "hit during phase 1" flag
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   * [2] origin x
   * [3] origin y
   */

  new =tenm_object_new("player shot", ATTR_PLAYER_SHOT,
                       ATTR_BOSS | ATTR_ENEMY | ATTR_OBSTACLE | ATTR_OPAQUE,
                       20, x, y,
                       4, count, 4, count_d, 1, p, 
                       (int (*)(tenm_object *, double)) (&player_shot_move),
                       (int (*)(tenm_object *, tenm_object *))
                       (&player_shot_hit),
                       (int (*)(tenm_object *, const tenm_object *))
                       (&player_shot_act),
                       (int (*)(tenm_object *, int)) (&player_shot_draw));
  if (new == NULL)
  {
    fprintf(stderr, "player_shot_new: tenm_object_new failed\n");
    free(count_d);
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  return new;
}

static int
player_shot_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
    return 0;
  if (turn_per_frame <= 0.5)
    return 0;

  if (my->count[0] != 1)
    return 0;
  if (my->hit_point <= 0)
    return 0;

  if (my->count[3] != 0)
  {
    my->attr = 0;
    my->hit_mask = 0;
    return 0;
  }

  dx_temp = my->count_d[0] / turn_per_frame;
  dy_temp = my->count_d[1] / turn_per_frame;
  my->x += dx_temp;
  my->y += dy_temp;
  tenm_move_mass(my->mass, dx_temp, dy_temp);

  if (!in_window_object(my))
  {
    my->attr = 0;
    my->hit_mask = 0;
    my->hit_point = 0;

    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;

    clear_chain();
    /* we have to wait for the trail to disappear */
    return 0;
  }

  return 0;
}

static int
player_shot_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (your == NULL)
    return 0;

  if (my->count[0] == 1)
  {
    /* don't call player_shot_explode() here --- hit() of the enemy
     * may or may not be called before this
     */
    my->count[3] = 1;
    return 0;
  }

  return 0;
}

static int
player_shot_act(tenm_object *my, const tenm_object *player)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (player == NULL)
    return 0;
  if ((my->count[0] < 1) || (my->count[0] > 2))
  {
    fprintf(stderr, "player_shot_act: strange mode (%d)\n", my->count[0]);
    return 1;
  }

  if (my->hit_point > 0)
  {
    if (my->count[0] == 1)
    {
      /* explode if we hit something */
      if (my->count[3] != 0)
        player_shot_explode(my);
    }
    else if (my->count[0] == 2)
    {
      (my->count[1])++;
      if (my->count[1] > 30)
      {
        my->attr = 0;
        my->hit_mask = 0;
        my->hit_point = 0;

        my->count_d[0] = 0.0;
        my->count_d[1] = 0.0;
        /* no "return 1" here --- we have to wait
         * for the trail to disappear
         */
      }
    }
  }

  if (my->count[2] >= 32)
  {
    if (my->hit_point <= 0)
      return 1;
  }
  else
  {
    (my->count[2])++;
  }
  
  return 0;
}

static int
player_shot_draw(tenm_object *my, int priority)
{
  int i;
  double length;
  int theta;
  int red_orig;
  int green_orig;
  int blue_orig;
  tenm_color color;
  int status = 0;

  /* sanity check */
  if (my == NULL)
    return 0;
  if ((priority != 0) && (priority != -1))
    return 0;

  color = tenm_map_color(134, 215, 170);

  if (priority == 0)
  { 
    if ((my->count[0] == 1) && (my->hit_point > 0))
    {
      if (tenm_draw_circle((int) my->x, (int) my->y, 10,
                           1, color) != 0)
        status = 1;
    }
    else if (my->count[0] == 2)
    {
      for (i = 0; i < 10 - my->count[1] / 3; i++)
      {
        length = 50.0 - 5.0 * ((double) (rand() % 32)) / 32.0;
        theta = rand() % 360;
        if (tenm_draw_line((int) my->x, (int) my->y,
                           (int) (my->x + length * tenm_cos(theta)),
                           (int) (my->y + length * tenm_sin(theta)),
                           1, color) != 0)
          status = 1;
      }
      for (i = 0; i < 30 - my->count[1]; i++)
      {
        length = 50.0 - 25.0 * ((double) (rand() % 32)) / 32.0;
        theta = rand() % 360;
        if (tenm_draw_line((int) my->x, (int) my->y,
                           (int) (my->x + length * tenm_cos(theta)),
                           (int) (my->y + length * tenm_sin(theta)),
                           1, color) != 0)
          status = 1;
      }

      if (tenm_draw_circle((int) my->x, (int) my->y, 50,
                           1, color) != 0)
        status = 1;
    }
  }
  else if (priority == -1)
  {
    /* don't draw the trail if the shot did not move at all */
    if ((my->count[2] > 0) && (my->count[2] < 32))
    {
      if ((my->count[0] == 1) && (my->hit_point <= 0))
      {
        red_orig = 63;
        green_orig = 63;
        blue_orig = 63;
      }
      else
      {
        red_orig = 134;
        green_orig = 215;
        blue_orig = 170;
      }

      color = tenm_map_color((red_orig * (32- my->count[2])
                              + DEFAULT_BACKGROUND_RED * my->count[2]) / 32,
                             (green_orig * (32- my->count[2])
                              + DEFAULT_BACKGROUND_GREEN * my->count[2]) / 32,
                             (blue_orig * (32- my->count[2])
                              + DEFAULT_BACKGROUND_BLUE * my->count[2]) / 32);
      if (tenm_draw_line((int) (my->x), (int) (my->y),
                         (int) (my->count_d[2]), (int) (my->count_d[3]),
                         1, color) != 0)
        status = 1;
    }
  }

  return status;
}

/* return 0 on success, 1 on error */
static int
player_shot_explode(tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "player_shot_explode: my is NULL\n");
    return 1;
  }
  if (my->name == NULL)
  {
    fprintf(stderr, "player_shot_explode: my->name is NULL\n");
    return 1;
  }
  if (strcmp(my->name, "player shot") != 0)
  {
    fprintf(stderr, "player_shot_explode: my->name is not \"player shot\"\n");
    return 1;
  }

  my->attr = ATTR_PLAYER_SHOT;
  my->hit_mask = 0;

  my->hit_point = 1;

  my->count[0] = 2;
  my->count[1] = 1;

  ((tenm_circle *) my->mass->p[0])->r = 50.0;

  return 0;
}

/* damage handling function for hit() of an enemy
 * my is the enemy, your is the player shot
 * n (arg 3) is the index of count[] for phase 2 boomerang handling
 * you must set my->count[n] to 0 in act() every frame
 * warning: turn_per_frame is hard-coded
 */
int
deal_damage(tenm_object *my, tenm_object *your, int n)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "deal_damage: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "deal_damage: your is NULL\n");
    return 0;
  }
  if ((n < 0) || (n >= my->n))
  {
    fprintf(stderr, "deal_damage: strange n (%d)\n", n);
    return 0;
  }

  if (your->count[0] == 1)
  {
    deal_damage2(my, your);
  }
  else if (your->count[0] == 2)
  {
    my->count[n] -= 1;
    if (my->count[n] <= 0)
    {
      deal_damage2(my, your);
      /* turn_per_frame */
      my->count[n] = 30;
    }
  }

  return 0;
}

/* change the hit point of the enemy and add score
 * my is the enemy, your is the player shot
 */
static int
deal_damage2(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "deal_damage: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "deal_damage: your is NULL\n");
    return 0;
  }

  if (my->hit_point <= your->hit_point)
  {
    add_score(my->hit_point - 1);
    my->hit_point = 0;
  }
  else
  {
    add_score(your->hit_point);
    my->hit_point -= your->hit_point;
  }

  return 0;
}
