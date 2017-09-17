/* $Id: insane-hand.c,v 1.211 2011/08/23 20:09:40 oohara Exp $ */
/* [normal] Hell Salvage */

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

#include "insane-hand.h"

#define NEAR_ZERO 0.0001

static int insane_hand_move(tenm_object *my, double turn_per_frame);
static int insane_hand_hit(tenm_object *my, tenm_object *your);
static void insane_hand_next(tenm_object *my);
static int insane_hand_act(tenm_object *my, const tenm_object *player);
static int insane_hand_draw(tenm_object *my, int priority);
static int insane_hand_body_line(double a_x, double a_y,
                                 double b_x, double b_y,
                                 int width, tenm_color color, int theta);
static void insane_hand_body_rotate(double *result, const double *v,
                                    int theta);
static int insane_hand_green(const tenm_object *my);

static tenm_object *insane_hand_ship_new(double x, double y);
static int insane_hand_ship_move(tenm_object *my, double turn_per_frame);
static int insane_hand_ship_hit(tenm_object *my, tenm_object *your);
static int insane_hand_ship_signal_bit(tenm_object *my, int n);
static int insane_hand_ship_act(tenm_object *my, const tenm_object *player);
static int insane_hand_ship_draw(tenm_object *my, int priority);
static int insane_hand_ship_green(const tenm_object *my);

static tenm_object *insane_hand_ship_bit_new(double x, double y);
static int insane_hand_ship_bit_move(tenm_object *my, double turn_per_frame);
static int insane_hand_ship_bit_act(tenm_object *my,
                                    const tenm_object *player);
static int insane_hand_ship_bit_draw(tenm_object *my, int priority);

static tenm_object *insane_hand_ship_shot_new(double x, double y,
                                              double v_x, double v_y);
static int insane_hand_ship_shot_move(tenm_object *my, double turn_per_frame);
static int insane_hand_ship_shot_hit(tenm_object *my, tenm_object *your);
static int insane_hand_ship_shot_act(tenm_object *my,
                                    const tenm_object *player);
static int insane_hand_ship_shot_draw(tenm_object *my, int priority);

tenm_object *
insane_hand_new(void)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  double x = (double) (WINDOW_WIDTH / 2);
  double y = -75.0;

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "insane_hand_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + 50.0, y - 25.0,
                                             x + 50.0, y + 25.0,
                                             x - 50.0, y + 25.0,
                                             x - 50.0, y - 25.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "insane_hand_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 8);
  if (count == NULL)
  {
    fprintf(stderr, "insane_hand_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 4);
  if (count_d == NULL)
  {
    fprintf(stderr, "insane_hand_new: malloc(count_d) failed\n");
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
   * [4 -- 5] hand theta
   * [6] shoot randomness
   * [7] "was green when killed" flag
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   * [2 -- 3] shoot randomness
   */

  count[0] = 0;
  count[1] = 0;
  count[2] = 0;
  count[3] = 0;
  count[4] = 75;
  count[5] = 165;
  count[6] = 0;

  count_d[0] = 0.0;
  count_d[1] = (179.0 - y) / 90.0;
  count_d[2] = 0.0;
  count_d[3] = 0.0;

  new = tenm_object_new("Insane Hand", 0, 0,
                        1000, x, y,
                        8, count, 4, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&insane_hand_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&insane_hand_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&insane_hand_act),
                        (int (*)(tenm_object *, int))
                        (&insane_hand_draw));
  if (new == NULL)
  {
    fprintf(stderr, "insane_hand_new: tenm_object_new failed\n");
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
insane_hand_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "insane_hand_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "insane_hand_move: strange turn_per_frame (%f)\n",
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
insane_hand_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "insane_hand_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "insane_hand_hit: your is NULL\n");
    return 0;
  }

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;
  if ((my->count[2] != 1) && (my->count[2] != 3))
    return 0;

  deal_damage(my, your, 0);
  if (insane_hand_green(my))
    add_chain(my, your);
  my->count[1] = 2;

  if (my->hit_point <= 0)
  {
    if (my->count[2] == 1)
      add_score(6250);
    else if (my->count[2] == 3)
      add_score(18750);
    set_background(1);
    insane_hand_next(my);
    return 0;
  }

  return 0;
}

static void
insane_hand_next(tenm_object *my)
{
  int n;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "insane_hand_next: my is NULL\n");
    return;
  }

  if ((my->count[2] != 1) && (my->count[2] != 3))
    return;

  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy_shot), 0);
  tenm_table_apply_all((int (*)(tenm_object *, int)) (&delete_enemy), 0);

  /* set "was green" flag before we change the life mode */
  if (insane_hand_green(my))
  {
    n = 8;
    my->count[7] = 1;
  }
  else
  {
    n = 7;
    my->count[7] = 0;
  }

  if (my->count[2] == 1)
  {
    tenm_table_add(fragment_new(my->x, my->y, 0.0, 0.0,
                                50.0, 5, n, 2.0, 0.0, 16));
    tenm_table_add(fragment_new(my->x, my->y - 80.0, 0.0, 0.0,
                                50.0, 5, n, 2.0, 0.0, 16));
    tenm_table_add(fragment_new(my->x, my->y - 160.0, 0.0, 0.0,
                                50.0, 5, n, 2.0, 0.0, 16));

    my->hit_point = 500;
    my->count[2] = 2;
    my->count[3] = 0;
    my->count[1] = 0;
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
    return;
  }
  else if (my->count[2] == 3)
  {
    tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                                 1, 5000, n, 10.0, 6));
    tenm_table_add(fragment_new(my->x + 50.0, my->y + 25.0, 0.0, 0.0,
                                30.0, 5, n, 5.0, 15.0, 16));
    tenm_table_add(fragment_new(my->x - 50.0, my->y + 25.0, 0.0, 0.0,
                                30.0, 5, n, 5.0, 15.0, 16));

    my->count[2] = 4;
    my->count[3] = 0;
    my->count[1] = 0;
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.5;

    /* don't modify my->attr or my->hit_mask here, or the player shot
     * may fly through the enemy */
    tenm_mass_delete(my->mass);
    my->mass = NULL;

    return;
  }
}

static int
insane_hand_act(tenm_object *my, const tenm_object *player)
{
  int t;
  int theta;
  double x;
  double y;
  int i;
  double speed;
  double length;
  double dx;
  double dy;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "insane_hand_act: my is NULL\n");
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
    if (my->count[3] == 90)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }

    if (((my->count[3] >= 90) && (my->count[3] < 105))
        || ((my->count[3] >= 120) && (my->count[3] < 135)))
    {
      my->count[4] -= 5;
      my->count[5] -= 5;
    }
    if (((my->count[3] >= 105) && (my->count[3] < 120))
        || ((my->count[3] >= 135) && (my->count[3] < 150)))
    {
      my->count[4] += 5;
      my->count[5] += 5;
    }

    if (my->count[3] >= 240)
    {
      my->count[2] = 1;
      my->count[3] = 0;
      my->attr = ATTR_BOSS;
      my->hit_mask = ATTR_PLAYER_SHOT;
      return 0;
    }

    return 0;
  }
  if (my->count[2] == 2)
  {
    /* reset hand */
    if (my->count[3] >= 30)
    {  
      if ((my->count[4] + 5 >= 75) && (my->count[4] - 5 <= 75))
        my->count[4] = 75;
      else if (my->count[4] > 75)
        my->count[4] -= 5;
      else
        my->count[4] += 5;
      if ((my->count[5] + 5 >= 165) && (my->count[5] - 5 <= 165))
        my->count[5] = 165;
      else if (my->count[5] > 165)
        my->count[5] -= 5;
      else
        my->count[5] += 5;
    }

    if (my->count[3] == 30)
    {
      my->count_d[0] = (((double) (WINDOW_WIDTH / 2)) - my->x) / 90.0;
      my->count_d[1] = (179.0 - my->y) / 90.0;
      my->count[7] = 0;
    }
    if (my->count[3] == 120)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
      my->count[2] = 3;
      my->count[3] = 0;
      my->count[7] = 0;
    }
    return 0;
  }

  /* dead */
  if (my->count[2] == 4)
  {
    if (insane_hand_green(my))
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
  if ((my->count[2] == 1) && (my->count[3] >= 4070))
  {
    set_background(2);
    clear_chain();
    insane_hand_next(my);
    return 0;
  }
  if ((my->count[2] == 3) && (my->count[3] >= 3030))
  {
    set_background(2);
    clear_chain();
    insane_hand_next(my);
    return 0;
  }

  /* move hand */
  if (my->count[2] == 1)
  {
    if ((my->count[3] >= 800) && (my->count[3] < 815))
    {
      my->count[4] -= 5;
      my->count[5] -= 5;
    }
    if ((my->count[3] >= 890) && (my->count[3] < 905))
    {
      my->count[4] += 5;
      my->count[5] += 5;
    }
    if ((my->count[3] >= 1380) && (my->count[3] < 1395))
    {
      my->count[4] -= 5;
      my->count[5] -= 5;
    }
    if ((my->count[3] >= 1395) && (my->count[3] < 1410))
    {
      my->count[4] += 5;
      my->count[5] += 5;
    }
    if ((my->count[3] >= 1880) && (my->count[3] < 1895))
    {
      my->count[4] -= 5;
      my->count[5] -= 5;
    }
    if ((my->count[3] >= 1970) && (my->count[3] < 1985))
    {
      my->count[4] += 5;
      my->count[5] += 5;
    }
    if ((my->count[3] >= 2460) && (my->count[3] < 2475))
    {
      my->count[4] -= 5;
      my->count[5] -= 5;
    }
    if ((my->count[3] >= 2475) && (my->count[3] < 2490))
    {
      my->count[4] += 5;
      my->count[5] += 5;
    }
    if ((my->count[3] >= 2675) && (my->count[3] < 2690))
    {
      my->count[4] -= 5;
      my->count[5] -= 5;
    }
    if ((my->count[3] >= 2765) && (my->count[3] < 2780))
    {
      my->count[4] += 5;
      my->count[5] += 5;
    }
    if ((my->count[3] >= 3975) && (my->count[3] < 3990))
    {
      my->count[4] -= 5;
      my->count[5] -= 5;
    }
    if ((my->count[3] >= 3990) && (my->count[3] < 4005))
    {
      my->count[4] += 5;
      my->count[5] += 5;
    }
  }
  else if (my->count[2] == 3)
  {
    if (my->count[3] % 60 < 30)
    {
      my->count[4] -= 5;
      my->count[5] -= 10;
    }
    else
    {
      my->count[4] += 5;
      my->count[5] += 10;
    }
  }
  
  /* speed change */
  if (my->count[2] == 1)
  {
    if (my->count[3] == 450)
    {
      my->count_d[0] = -6.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[3] == 495)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[3] == 505)
    {
      my->count_d[0] = 6.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[3] == 595)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[3] == 600)
    {
      my->count_d[0] = -6.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[3] == 690)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[3] == 820)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 6.0;
    }
    if (my->count[3] == 890)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = -6.0;
    }
    if (my->count[3] == 960)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[3] == 1060)
    {
      my->count_d[0] = 6.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[3] == 1105)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[3] == 1530)
    {
      my->count_d[0] = 6.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[3] == 1575)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[3] == 1585)
    {
      my->count_d[0] = -6.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[3] == 1675)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[3] == 1680)
    {
      my->count_d[0] = 6.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[3] == 1770)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[3] == 1900)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 6.0;
    }
    if (my->count[3] == 1970)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = -6.0;
    }
    if (my->count[3] == 2040)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[3] == 2140)
    {
      my->count_d[0] = -6.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[3] == 2185)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[3] == 2630)
    {
      my->count_d[0] = -6.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[3] == 2675)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[3] == 2695)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 6.0;
    }
    if (my->count[3] == 2765)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = -6.0;
    }
    if (my->count[3] == 2835)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[3] == 3925)
    {
      my->count_d[0] = 6.0;
      my->count_d[1] = 0.0;
    }
    if (my->count[3] == 3970)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
  }
  else if (my->count[2] == 3)
  {
    if (my->count[3] < 480)
    {
      my->count_d[0] = 0.0;
      my->count_d[1] = 0.0;
    }
    else
    {
      t = my->count[3] - 480;
      my->count_d[0] = ((double) (WINDOW_WIDTH / 2))
        + 200.0 * tenm_sin(t * 2) - my->x;
      my->count_d[1] = 179.0 + 100.0 * tenm_sin(t * 3) - my->y;
    }
  }

  /* add normal enemy */
  if (my->count[2] == 1)
  {
    if (my->count[3] == 890)
    {
      tenm_table_add(normal_enemy_new(my->x,
                                      my->y + 25.0 + 50.0 * tenm_sin(60),
                                      BALL_CAPTAIN, 0,
                                      70, -1, 0, -1, 0, 5, 2,
                                      /* move 0 */
                                      70, 0.0, -6.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      100, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      45, 6.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 3,
                                      /* move 3 */
                                      275, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 4,
                                      /* move 4 */
                                      9999, 0.0, 0.0, 0.0, 0.1,
                                      0.0, 0.0, 0.0, 0.0, 4,
                                      /* shoot 0 */
                                      70, 10, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 10, 0, 0, 1, 1));
    }
    if (my->count[3] == 1970)
    {
      tenm_table_add(normal_enemy_new(my->x,
                                      my->y + 25.0 + 50.0 * tenm_sin(60),
                                      BALL_CAPTAIN, 0,
                                      70, -1, 0, -1, 0, 5, 2,
                                      /* move 0 */
                                      70, 0.0, -6.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      100, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 2,
                                      /* move 2 */
                                      45, -6.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 3,
                                      /* move 3 */
                                      275, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 4,
                                      /* move 4 */
                                      9999, 0.0, 0.0, 0.0, 0.1,
                                      0.0, 0.0, 0.0, 0.0, 4,
                                      /* shoot 0 */
                                      70, 10, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      9999, 10, 0, 0, 1, 1));
    }
    if (my->count[3] == 2765)
      tenm_table_add(insane_hand_ship_new(my->x + 25.0,
                                          my->y + 31.0 + 50.0 * tenm_sin(60)));
  }

  /* shoot */
  if (my->count[2] == 1)
  {
    if ((my->count[3] == 30) || (my->count[3] == 1110)
        || (my->count[3] == 2190))
    {
      my->count[6] = rand() % 4;
      my->count_d[2] = (double) (-5 + (rand() % 11));
      my->count_d[3] = (double) (-5 + (rand() % 11));
    }

    if ((((my->count[3] >= 30) && (my->count[3] <= 156))
         || ((my->count[3] >= 216) && (my->count[3] <= 342))
         || ((my->count[3] >= 1110) && (my->count[3] <= 1236))
         || ((my->count[3] >= 1296) && (my->count[3] <= 1422))
         || ((my->count[3] >= 2190) && (my->count[3] <= 2316))
         || ((my->count[3] >= 2376) && (my->count[3] <= 2502)))
        && (my->count[3] % 6 == 0))
    {
      if (my->count[3] <= 156)
        t = my->count[3] - 30;
      else if (my->count[3] <= 342)
        t = my->count[3] - 216;
      else if (my->count[3] <= 1236)
        t = my->count[3] - 1110;
      else if (my->count[3] <= 1422)
        t = my->count[3] - 1296;
      else if (my->count[3] <= 2316)
        t = my->count[3] - 2190;
      else
        t = my->count[3] - 2376;

      theta = (t / 6) * (-17);
      if (my->count[6] % 2 != 0)
        theta = 180 - theta;
      x = my->x;
      y = my->y;
      if (my->count[6] < 2)
        x += 35.0;
      else
        x -= 35.0;
      x += my->count_d[2];
      y += my->count_d[3];
      tenm_table_add(normal_shot_angle_new(x, y,
                                           1.5 + 0.2 * ((double) (t / 5)),
                                           theta, 4));
      if (my->count[6] % 2 != 0)
        theta += 178;
      else
        theta -= 178;
      tenm_table_add(normal_shot_angle_new(x, y, 2.5, theta, 2));
    }
    if ((((my->count[3] >= 60) && (my->count[3] <= 186))
         || ((my->count[3] >= 246) && (my->count[3] <= 372))
         || ((my->count[3] >= 1140) && (my->count[3] <= 1266))
         || ((my->count[3] >= 1326) && (my->count[3] <= 1452))
         || ((my->count[3] >= 2220) && (my->count[3] <= 2346))
         || ((my->count[3] >= 2406) && (my->count[3] <= 2532)))
        && (my->count[3] % 6 == 0))
    {
      if (my->count[3] <= 186)
        t = my->count[3] - 60;
      else if (my->count[3] <= 372)
        t = my->count[3] - 246;
      else if (my->count[3] <= 1266)
        t = my->count[3] - 1140;
      else if (my->count[3] <= 1452)
        t = my->count[3] - 1326;
      else if (my->count[3] <= 2346)
        t = my->count[3] - 2220;
      else
        t = my->count[3] - 2406;

      theta = (t / 6) * (-17);
      if (my->count[6] % 2 == 0)
        theta = 180 - theta;
      x = my->x;
      y = my->y;
      if (my->count[6] < 2)
        x -= 35.0;
      else
        x += 35.0;
      x += my->count_d[2];
      y += my->count_d[3];
      tenm_table_add(normal_shot_angle_new(x, y,
                                           1.5 + 0.2 * ((double) (t / 5)),
                                           theta, 4));
      if (my->count[6] % 2 == 0)
        theta += 178;
      else
        theta -= 178;
      tenm_table_add(normal_shot_angle_new(x, y, 2.5, theta, 2));
    }

    if ((my->count[3] == 186) || (my->count[3] == 1266))
    {
      my->count[6] = (my->count[6] + 2) % 4;
      my->count_d[2] = (double) (-5 + (rand() % 11));
      my->count_d[3] = (double) (-5 + (rand() % 11));
    }

    if (((my->count[3] >= 505) && (my->count[3] <= 595))
        || ((my->count[3] >= 1585) && (my->count[3] <= 1675)))
    {
      if (my->count[3] <= 595)
        t = my->count[3] - 505;
      else
        t = my->count[3] - 1585;
      if (t % 30 == 0)
        tenm_table_add(normal_shot_point_new(my->x, my->y, 4.0,
                                             player->x, player->y, 2));
      if ((t % 32 == 0) || (t % 32 == 12))
      {
        tenm_table_add(laser_angle_new(my->x - 48.0, my->y, 3.0,
                                       70, 25.0, 3));
        tenm_table_add(laser_angle_new(my->x + 48.0, my->y, 3.0,
                                       70, 25.0, 3));
        tenm_table_add(laser_angle_new(my->x - 48.0, my->y, 3.0,
                                       -70, 25.0, 3));
        tenm_table_add(laser_angle_new(my->x + 48.0, my->y, 3.0,
                                       -70, 25.0, 3));
        tenm_table_add(laser_angle_new(my->x - 48.0, my->y, 3.0,
                                       110, 25.0, 3));
        tenm_table_add(laser_angle_new(my->x + 48.0, my->y, 3.0,
                                       110, 25.0, 3));
        tenm_table_add(laser_angle_new(my->x - 48.0, my->y, 3.0,
                                       -110, 25.0, 3));
        tenm_table_add(laser_angle_new(my->x + 48.0, my->y, 3.0,
                                       -110, 25.0, 3));
      }
      if ((t % 32 == 4) || (t % 32 == 24))
      {
        tenm_table_add(laser_angle_new(my->x - 48.0, my->y, 1.5,
                                       70, 25.0, 3));
        tenm_table_add(laser_angle_new(my->x + 48.0, my->y, 1.5,
                                       70, 25.0, 3));
        tenm_table_add(laser_angle_new(my->x - 48.0, my->y, 1.5,
                                       -70, 25.0, 3));
        tenm_table_add(laser_angle_new(my->x + 48.0, my->y, 1.5,
                                       -70, 25.0, 3));
        tenm_table_add(laser_angle_new(my->x - 48.0, my->y, 1.5,
                                       110, 25.0, 3));
        tenm_table_add(laser_angle_new(my->x + 48.0, my->y, 1.5,
                                       110, 25.0, 3));
        tenm_table_add(laser_angle_new(my->x - 48.0, my->y, 1.5,
                                       -110, 25.0, 3));
        tenm_table_add(laser_angle_new(my->x + 48.0, my->y, 1.5,
                                       -110, 25.0, 3));
      }
    }

    if ((my->count[3] >= 600) && (my->count[3] <= 690)
        && (my->count[3] % 15 == 0))
    {
      for (i = -2; i <= 2; i++)
        tenm_table_add(normal_shot_new(my->x,
                                       my->y - 80.0 + 40.0 * ((double) i),
                                       1.5, 3.6 - 0.8 * ((double) i),
                                       5, -2, 0));
    }
    if ((my->count[3] >= 1680) && (my->count[3] <= 1770)
        && (my->count[3] % 15 == 0))
    {
      for (i = -2; i <= 2; i++)
        tenm_table_add(normal_shot_new(my->x,
                                       my->y - 80.0 + 40.0 * ((double) i),
                                       -1.5, 3.6 - 0.8 * ((double) i),
                                       5, -2, 0));
    }

    if ((my->count[3] == 820) || (my->count[3] == 1900)
        || (my->count[3] == 2695))
    {
      my->count_d[2] = (double) (-5 + (rand() % 11));
      my->count_d[3] = (double) (-5 + (rand() % 11));
    }
    if (((my->count[3] >= 820) && (my->count[3] < 880))
        || ((my->count[3] >= 2695) && (my->count[3] < 2755)))
    {
      if (my->count[3] < 880)
        t = my->count[3] - 820;
      else
        t = my->count[3] - 2695;
      tenm_table_add(laser_angle_new(my->x - 36.0 + my->count_d[2],
                                     my->y + my->count_d[3], 2.0,
                                     178 + 15 * (t % 10), 25.0, 2));
      tenm_table_add(laser_angle_new(my->x + 36.0 + my->count_d[2],
                                     my->y + my->count_d[3], 2.0,
                                     88 - 15 * (t % 10), 25.0, 2));
    }
    if (((my->count[3] >= 820) && (my->count[3] < 876))
        || ((my->count[3] >= 2695) && (my->count[3] < 2751)))
    {
      if (my->count[3] < 876)
        t = my->count[3] - 820;
      else
        t = my->count[3] - 2695;
      if (t % 2 == 0)
        speed = 3.5;
      else
        speed = 4.5;
      tenm_table_add(laser_angle_new(my->x - 36.0 + my->count_d[2],
                                     my->y + my->count_d[3],
                                     speed,
                                     178 + 5 * (t % 28), 25.0, 2));
      tenm_table_add(laser_angle_new(my->x + 36.0 + my->count_d[2],
                                     my->y + my->count_d[3],
                                     speed,
                                     88 - 5 * (t % 28), 25.0, 2));
    }
    if ((my->count[3] >= 1900) && (my->count[3] < 1960))
    {
      t = my->count[3] - 1900;
      tenm_table_add(laser_angle_new(my->x + 36.0 + my->count_d[2],
                                     my->y + my->count_d[3], 2.0,
                                     2 - 15 * (t % 10), 25.0, 2));
      tenm_table_add(laser_angle_new(my->x - 36.0 + my->count_d[2],
                                     my->y + my->count_d[3], 2.0,
                                     92 + 15 * (t % 10), 25.0, 2));
    }
    if ((my->count[3] >= 1900) && (my->count[3] < 1956))
    {
      t = my->count[3] - 1900;
      if (t % 2 == 0)
        speed = 3.5;
      else
        speed = 4.5;
      tenm_table_add(laser_angle_new(my->x + 36.0 + my->count_d[2],
                                     my->y + my->count_d[3],
                                     speed,
                                     2 - 5 * (t % 28), 25.0, 2));
      tenm_table_add(laser_angle_new(my->x - 36.0 + my->count_d[2],
                                     my->y + my->count_d[3],
                                     speed,
                                     92 + 5 * (t % 28), 25.0, 2));
    }

    if ((my->count[3] >= 2835) && (my->count[3] <= 3825)
        && (my->count[3] % 30 == 15))
    {
      x = player->x - my->x;
      y = player->y - (my->y - 80.0);
      length = tenm_sqrt((int) (x * x + y * y));
      if (length < NEAR_ZERO)
        length = 1.0;
      for (i = -2; i <= 2; i++)
        tenm_table_add(normal_shot_new(my->x,
                                       my->y - 80.0 + 40.0 * ((double) i),
                                       4.0 * x / length,
                                       4.0 * y / length - 0.6 * ((double) i),
                                       5, -2, 0));
    }
  }
  else if (my->count[2] == 3)
  {
    /* hand */
    if (my->count[3] == 60)
    {  
      my->count[6] = rand() % 2;
    }
    else if (my->count[3] % 420 == 60)
    {
      if (rand() % 3 != 0)
        my->count[6] = 1 - my->count[6];
    }
    if ((my->count[3] < 2940) && (my->count[3] % 420 >= 60))
    {
      t = (my->count[3] - 60) % 420;

      length = 50.0 / tenm_sqrt(2);
      x = 50.0;
      y = 25.0;
      for (i = 0; i < 2; i++)
      {
        dx = length * tenm_cos(my->count[4 + i]);
        dy = length * tenm_sin(my->count[4 + i]);

        switch (my->count[6])
        {
        case 0:
          if ((t % 5 == 0)
              || ((my->count[3] >= 900) && (t % 5 < 3)))
          {
            if (i == 1)
            {
              tenm_table_add(laser_angle_new(my->x + x + dx, my->y + y + dy,
                                             5.0, t * 11, 25.0, 2));
              tenm_table_add(laser_angle_new(my->x - x - dx, my->y + y + dy,
                                             5.0, 180 - t * 11, 25.0, 2));
            }
          }
          if (t % 13 == 0)
          {
            tenm_table_add(normal_shot_point_new(my->x, my->y, 4.0,
                                                 player->x, player->y, 4));
          }
          break;
        case 1:
          if (t % 19 == 0)
          {
            if (i == 0)
            {
              tenm_table_add(laser_point_new(my->x + x + dx, my->y + y + dy,
                                             5.5,
                                             player->x - 100.0, player->y,
                                             25.0, 0));
              tenm_table_add(laser_point_new(my->x - x - dx, my->y + y + dy,
                                             5.5,
                                             player->x + 100.0, player->y,
                                             25.0, 0));
              if (my->count[3] >= 900)
              {
                tenm_table_add(laser_point_new(my->x + x + dx, my->y + y + dy,
                                               7.5,
                                               player->x + 100.0, player->y,
                                               25.0, 0));
                tenm_table_add(laser_point_new(my->x - x - dx, my->y + y + dy,
                                               7.5,
                                               player->x - 100.0, player->y,
                                               25.0, 0));
              }
            }
            else if (i == 1)
            {
              tenm_table_add(laser_point_new(my->x + x + dx, my->y + y + dy,
                                             4.0,
                                             player->x, player->y, 25.0, 1));
              tenm_table_add(laser_point_new(my->x - x - dx, my->y + y + dy,
                                             4.0,
                                             player->x, player->y, 25.0, 1));
            }
          }
          break;
        default:
          fprintf(stderr, "insane_hand_draw: undefined shoot mode (%d)\n",
                  my->count[6]);
          break;
        }
        x += dx;
        y += dy;
      }
    }
    
  }
  
  return 0;
}

static int
insane_hand_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  char temp[32];
  int theta_rotate;
  double length_hand;
  double x;
  double y;
  double dx;
  double dy;
  int i;
  int width;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "insane_hand_draw: my is NULL\n");
    return 0;
  }

  theta_rotate = 0;
  if (my->count[2] == 0)
  {
    if (my->count[3] >= 240)
      theta_rotate = 0;
    else if (my->count[3] < 150)
      theta_rotate = -180;
    else
      theta_rotate = -180 + (my->count[3] - 150) * 2;
  }

  /* decoration */
  if ((priority == 0) && (my->count[2] <= 1))
  {
    if (insane_hand_green(my))
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

    /* shaft */
    if (insane_hand_body_line(my->x + 20.0, my->y - 25.0,
                              my->x + 20.0, my->y - 625.0,
                              1, color, theta_rotate) != 0)
      status = 1;
    if (insane_hand_body_line(my->x - 20.0, my->y - 25.0,
                              my->x - 20.0, my->y - 625.0,
                              1, color, theta_rotate) != 0)
      status = 1;
  }

  /* body */
  /* dead enemy has low priority */
  if (((my->count[2] <= 3) && (priority == 0))
      || ((my->count[2] > 3) && (priority == -1)))
  {
    if (insane_hand_green(my))
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

    /* hand */
    if (my->count[2] <= 3)
    {  
      length_hand = 50.0 / tenm_sqrt(2);
      x = 50.0;
      y = 25.0;
      for (i = 0; i < 2; i++)
      {
        dx = length_hand * tenm_cos(my->count[4 + i]);
        dy = length_hand * tenm_sin(my->count[4 + i]);
        if (insane_hand_body_line(my->x + x, my->y + y,
                                  my->x + x + dx, my->y + y + dy,
                                  1, color, theta_rotate) != 0)
          status = 1;
        if (insane_hand_body_line(my->x - x, my->y + y,
                                  my->x - x - dx, my->y + y + dy,
                                  1, color, theta_rotate) != 0)
          status = 1;
        x += dx;
        y += dy;
      }
    }

    /* core */
    if (my->count[2] == 0)
      width = 1;
    else
      width = 3;
    if (insane_hand_body_line(my->x + 50.0, my->y - 25.0,
                              my->x + 50.0, my->y + 25.0,
                              width, color, theta_rotate) != 0)
      status = 1;
    if (insane_hand_body_line(my->x + 50.0, my->y + 25.0,
                              my->x - 50.0, my->y + 25.0,
                              width, color, theta_rotate) != 0)
      status = 1;
    if (insane_hand_body_line(my->x - 50.0, my->y + 25.0,
                              my->x - 50.0, my->y - 25.0,
                              width, color, theta_rotate) != 0)
      status = 1;
    if (insane_hand_body_line(my->x - 50.0, my->y - 25.0,
                              my->x + 50.0, my->y - 25.0,
                              width, color, theta_rotate) != 0)
      status = 1;
  }

  /* hit point stat */
  if ((priority == 0)
      && ((my->count[2] == 1) || (my->count[2] == 3)))
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 10, (int) my->y,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "insane_hand_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

static int insane_hand_body_line(double a_x, double a_y,
                                 double b_x, double b_y,
                                 int width, tenm_color color, int theta)
{
  double a_result[2];
  double a_v[2];
  double b_result[2];
  double b_v[2];

  a_v[0] = a_x;
  a_v[1] = a_y;
  a_result[0] = a_v[0];
  a_result[1] = a_v[1];
  insane_hand_body_rotate(a_result, a_v, theta);

  b_v[0] = b_x;
  b_v[1] = b_y;
  b_result[0] = b_v[0];
  b_result[1] = b_v[1];
  insane_hand_body_rotate(b_result, b_v, theta);

  return tenm_draw_line((int) (a_result[0]), (int) (a_result[1]),
                        (int) (b_result[0]), (int) (b_result[1]),
                        width, color);
}

/* rotate the point v (arg 2) which is a part of the boss
 * by theta (arg 3) degree
 * result (arg 1) and v (arg 2) must be double[2] (you must allocate enough
 * memory before calling this function)
 * the result is undefined if result (arg 1) and v (arg 2) overlap
 */
static void insane_hand_body_rotate(double *result, const double *v,
                                    int theta)
{
  double temp_result[2];
  double temp_v[2];
  double center_x = (double) (WINDOW_WIDTH / 2);
  double center_y = (double) (WINDOW_HEIGHT / 2);


  /* sanity check */
  if (result == NULL)
  {
    fprintf(stderr, "insane_hand_body_rotate: result is NULL\n");
    return;
  }
  if (v == NULL)
  {
    fprintf(stderr, "insane_hand_body_rotate: v is NULL\n");
    return;
  }
  
  temp_v[0] = v[0] - center_x;
  temp_v[1] = v[1] - center_y;
  temp_result[0] = temp_v[0];
  temp_result[1] = temp_v[1];
  vector_rotate(temp_result, temp_v, theta);
  result[0] = temp_result[0] + center_x;
  result[1] = temp_result[1] + center_y;
}

/* return 1 (true) or 0 (false) */
static int
insane_hand_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if ((my->count[2] == 1)
      && (my->count[3] >= 800) && (my->count[3] < 4040))
    return 1;
  if ((my->count[2] == 3)
      && (my->count[3] >= 900) && (my->count[3] < 3000))
    return 1;

  if (((my->count[2] == 2) || (my->count[2] == 4))
      && (my->count[7] != 0))
    return 1;

  return 0;
}

static tenm_object *
insane_hand_ship_new(double x, double y)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;
  int i;

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 2);
  if (p == NULL)
  {
    fprintf(stderr, "insane_hand_ship_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + 32.0, y + 18.0,
                                             x + 23.0, y + 30.0,
                                             x - 41.0, y - 18.0,
                                             x - 32.0, y - 30.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "insane_hand_ship_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }
  p[1] = (tenm_primitive *) tenm_polygon_new(4,
                                             x + 9.0, y - 18.0,
                                             x - 18.0, y + 18.0,
                                             x - 50.0, y - 6.0,
                                             x - 23.0, y - 42.0);
  if (p[1] == NULL)
  {
    fprintf(stderr, "insane_hand_ship_new: cannot set p[0]\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 5);
  if (count == NULL)
  {
    fprintf(stderr, "insane_hand_ship_new: malloc(count) failed\n");
    for (i = 0; i < 2; i++)
      (p[i])->delete(p[i]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 2);
  if (count_d == NULL)
  {
    fprintf(stderr, "insane_hand_ship_new: malloc(count_d) failed\n");
    free(count);
    for (i = 0; i < 2; i++)
      (p[i])->delete(p[i]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] for deal_damage
   * [1] "damaged" timer
   * [2] life timer
   * [3] shoot timer
   * [4] bit index
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   */

  count[0] = 0;
  count[1] = 0;
  count[2] = -70;
  count[3] = 0;
  count[4] = -1;

  count_d[0] = 0.0;
  count_d[1] = -6.0;

  new = tenm_object_new("Insane Hand ship", ATTR_ENEMY, ATTR_PLAYER_SHOT,
                        400, x, y,
                        5, count, 2, count_d, 2, p,
                        (int (*)(tenm_object *, double))
                        (&insane_hand_ship_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&insane_hand_ship_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&insane_hand_ship_act),
                        (int (*)(tenm_object *, int))
                        (&insane_hand_ship_draw));

  if (new == NULL)
  {
    fprintf(stderr, "insane_hand_ship_new: tenm_object_new failed\n");
    if (count_d != NULL)
      free(count_d);
    if (count != NULL)
      free(count);
    for (i = 0; i < 2; i++)
      (p[i])->delete(p[i]);
    free(p);
    return NULL;
  }

  return new;
}

static int
insane_hand_ship_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "insane_hand_ship_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "insane_hand_ship_move: strange turn_per_frame (%f)\n",
            turn_per_frame);
    return 0;
  }

  dx_temp = my->count_d[0] / turn_per_frame;
  dy_temp = my->count_d[1] / turn_per_frame;
  my->x += dx_temp;
  my->y += dy_temp;
  if (my->mass != NULL)
    tenm_move_mass(my->mass, dx_temp, dy_temp);

  if ((my->count[2] > 0) && (!in_window_object(my)))
    return 1;

  return 0;
}

static int
insane_hand_ship_hit(tenm_object *my, tenm_object *your)
{
  int n;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "insane_hand_ship_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "insane_hand_ship_hit: your is NULL\n");
    return 0;
  }

  if (!(your->attr & ATTR_PLAYER_SHOT))
    return 0;

  deal_damage(my, your, 0);
  if (insane_hand_ship_green(my))
    add_chain(my, your);
  my->count[1] = 41;

  if (my->hit_point <= 0)
  {
    add_score(15000);
    if (my->count[4] >= 0)
      tenm_table_apply(my->count[4],
                       (int (*)(tenm_object *, int))
                       (&insane_hand_ship_signal_bit),
                       0);
    if (insane_hand_ship_green(my))
      n = 8;
    else
      n = 7;
    tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                                 1, 1000, n, 8.0, 6));
    tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                                 2, 300, n, 5.0, 8));
    return 1;
  }

  return 0;
}

static int
insane_hand_ship_signal_bit(tenm_object *my, int n)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (strcmp(my->name, "Insane Hand ship bit") != 0)
    return 0;

  tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                               1, 1000, 9, 8.0, 6));
  tenm_table_add(explosion_new(my->x, my->y, 0.0, 0.0,
                               2, 300, 9, 5.0, 8));

  return 1;
}

static int
insane_hand_ship_act(tenm_object *my, const tenm_object *player)
{
  int i;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "insane_hand_ship_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  /* for deal_damage */
  my->count[0] = 0;

  /* "damaged" count down */
  if (my->count[1] > 0)
    (my->count[1])--;

  (my->count[2])++;
  if (my->count[2] == -69)
    my->count[4] = tenm_table_add(insane_hand_ship_bit_new(my->x + 52.0,
                                                           my->y + 39.0));
  if (my->count[2] == 0)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
  }
  if (my->count[2] == 1090)
  {
    my->count_d[0] = 6.0;
    my->count_d[1] = 0.0;
  }
  if (my->count[2] == 1135)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
  }
  if (my->count[2] >= 1140)
  {
    my->count_d[1] += 0.1;
  }
  if ((my->count[2] <= 90) || (my->count[2] >= 990))
    return 0;

  (my->count[3])++;
  if (my->count[3] % 100 == 0)
  {
    for (i = -4; i <= 4; i++)
    {
      tenm_table_add(insane_hand_ship_shot_new(my->x, my->y,
                                               7.0 * tenm_cos(15 * i),
                                               7.0 * tenm_sin(15 * i)));
    }
  }
  
  return 0;
}

static int
insane_hand_ship_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;
  char temp[32];

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "insane_hand_ship_draw: my is NULL\n");
    return 0;
  }

  /* body */
  if (priority == 0)
  {
    if (insane_hand_ship_green(my))
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

    if (tenm_draw_line((int) (my->x + 32.0), (int) (my->y + 18.0),
                       (int) (my->x + 23.0), (int) (my->y + 30.0),
                       3, color) != 0)
      status = 1;
    if (tenm_draw_line((int) (my->x + 23.0), (int) (my->y + 30.0),
                       (int) (my->x - 9.0), (int) (my->y + 6.0),
                       3, color) != 0)
      status = 1;
    if (tenm_draw_line((int) (my->x - 9.0), (int) (my->y + 6.0),
                       (int) (my->x - 18.0), (int) (my->y + 18.0),
                       3, color) != 0)
      status = 1;
    if (tenm_draw_line((int) (my->x - 18.0), (int) (my->y + 18.0),
                       (int) (my->x - 50.0), (int) (my->y - 6.0),
                       3, color) != 0)
      status = 1;

    if (tenm_draw_line((int) (my->x - 50.0), (int) (my->y - 6.0),
                       (int) (my->x - 23.0), (int) (my->y - 42.0),
                       3, color) != 0)
      status = 1;
    if (tenm_draw_line((int) (my->x - 23.0), (int) (my->y - 42.0),
                       (int) (my->x + 9.0), (int) (my->y - 18.0),
                       3, color) != 0)
      status = 1;
    if (tenm_draw_line((int) (my->x + 9.0), (int) (my->y - 18.0),
                       (int) (my->x), (int) (my->y - 6.0),
                       3, color) != 0)
      status = 1;
    if (tenm_draw_line((int) (my->x), (int) (my->y - 6.0),
                       (int) (my->x + 32.0), (int) (my->y + 18.0),
                       3, color) != 0)
      status = 1;
  }

  /* hit point stat */
  if ((priority == 0) && (my->count[1] > 0))
  {
    sprintf(temp, "%d", my->hit_point);
    if (draw_string(((int) my->x) - 33, ((int) (my->y)) - 10,
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "insane_hand_ship_draw: draw_string failed\n");
      status = 1;
    }
  }

  return status;
}

/* return 1 (true) or 0 (false) */
static int
insane_hand_ship_green(const tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if ((my->count[2] > 90) && (my->count[2] < 1090))
    return 1;

  return 0;
}

static tenm_object *
insane_hand_ship_bit_new(double x, double y)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "insane_hand_ship_bit_new: malloc(p) failed\n");
    return NULL;
  }
  p[0] = (tenm_primitive *) tenm_circle_new(x, y, 25.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "insane_hand_ship_bit_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 2);
  if (count == NULL)
  {
    fprintf(stderr, "insane_hand_ship_bit_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 4);
  if (count_d == NULL)
  {
    fprintf(stderr, "insane_hand_ship_bit_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  /* list of count
   * [0] life timer
   * [1] shoot timer
   */
  /* list of count_d
   * [0] speed x
   * [1] speed y
   * [2] origin x
   * [3] origin y
   */

  count[0] = -69;
  count[1] = 0;

  count_d[0] = 0.0;
  count_d[1] = -6.0;
  count_d[2] = 0.0;
  count_d[3] = 0.0;

  new = tenm_object_new("Insane Hand ship bit", ATTR_ENEMY, 0,
                        1, x, y,
                        2, count, 4, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&insane_hand_ship_bit_move),
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&insane_hand_ship_bit_act),
                        (int (*)(tenm_object *, int))
                        (&insane_hand_ship_bit_draw));

  if (new == NULL)
  {
    fprintf(stderr, "insane_hand_ship_bit_new: tenm_object_new failed\n");
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
insane_hand_ship_bit_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "insane_hand_ship_bit_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "insane_hand_ship_bit_move: strange turn_per_frame (%f)\n",
            turn_per_frame);
    return 0;
  }

  dx_temp = my->count_d[0] / turn_per_frame;
  dy_temp = my->count_d[1] / turn_per_frame;
  my->x += dx_temp;
  my->y += dy_temp;
  if (my->mass != NULL)
    tenm_move_mass(my->mass, dx_temp, dy_temp);

  if ((my->count[0] > 0) && (!in_window_object(my)))
    return 1;

  return 0;
}

static int
insane_hand_ship_bit_act(tenm_object *my, const tenm_object *player)
{
  double x;
  double y;
  double px;
  double py;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "insane_hand_ship_bit_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  (my->count[0])++;
  if (my->count[0] == 0)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
    my->count_d[2] = my->x;
    my->count_d[3] = my->y;
  }
  if (my->count[0] == 990)
  {
    my->count_d[0] = (my->count_d[2] - my->x) / 100.0;
    my->count_d[1] = (my->count_d[3] - my->y) / 100.0;
  }
  if (my->count[0] == 1090)
  {
    my->count_d[0] = 6.0;
    my->count_d[1] = 0.0;
  }
  if (my->count[0] == 1135)
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
  }
  if (my->count[0] >= 1140)
  {
    my->count_d[1] += 0.1;
  }
  if ((my->count[0] <= 30) || (my->count[0] >= 990))
    return 0;

  (my->count[1])++;

  /* speed change */
  x = my->x * 0.8 + my->y * 0.6;
  y = -(my->x) * 0.6 + my->y * 0.8;
  px = player->x * 0.8 + player->y * 0.6;
  py = -(player->x) * 0.6 + player->y * 0.8;
  
  my->count_d[0] = 0.0;
  my->count_d[1] = 0.0;
  /* don't chase if the player is immutable */
  if ((get_ship() < 0) || (player->count[1] > 0))
  {
    my->count_d[0] = 0.0;
    my->count_d[1] = 0.0;
  }
  else if (x - 25.0 > px)
  {
    my->count_d[0] = -3.2;
    my->count_d[1] = -2.4;
  }
  else if (x - 10.0 < px)
  {
    my->count_d[0] = 3.2;
    my->count_d[1] = 2.4;
  }
  else if (y - 5.0 > py)
  {
    my->count_d[0] = 1.2;
    my->count_d[1] = -1.6;
  }
  else if (y + 5.0 < py)
  {
    my->count_d[0] = -1.2;
    my->count_d[1] = 1.6;
  }
  if (my->x + my->count_d[0] < 0.0)
    my->count_d[0] = 0.0 - my->x;
  if (my->x + my->count_d[0] > ((double) WINDOW_WIDTH))
    my->count_d[0] = ((double) WINDOW_WIDTH) - my->x;
  if (my->y + my->count_d[1] < 0.0)
    my->count_d[1] = 0.0 - my->y;
  if (my->y + my->count_d[1] > ((double) WINDOW_HEIGHT))
    my->count_d[1] = ((double) WINDOW_HEIGHT) - my->y;

  /* shoot */
  if (my->count[1] % 30 == 0)
  {
    tenm_table_add(laser_new(my->x, my->y, 3.0, -4.0,
                             15.0, -20.0, 0, -2, 0));
    tenm_table_add(laser_new(my->x, my->y, -3.0, 4.0,
                             -15.0, 20.0, 0, -2, 0));
    tenm_table_add(laser_new(my->x, my->y,
                             -4.0 * tenm_cos(15) + (-3.0) * (-tenm_sin(15)),
                             -4.0 * tenm_sin(15) + (-3.0) * tenm_cos(15),
                             -20.0 * tenm_cos(15) + (-15.0) * (-tenm_sin(15)),
                             -20.0 * tenm_sin(15) + (-15.0) * tenm_cos(15),
                             0, -2, 0));
    tenm_table_add(laser_new(my->x, my->y,
                             -4.0 * tenm_cos(-15) + (-3.0) * (-tenm_sin(-15)),
                             -4.0 * tenm_sin(-15) + (-3.0) * tenm_cos(-15),
                             -20.0 * tenm_cos(-15) +(-15.0) * (-tenm_sin(-15)),
                             -20.0 * tenm_sin(-15) +(-15.0) * tenm_cos(-15),
                             0, -2, 0));
  }
  
  return 0;
}

static int
insane_hand_ship_bit_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;

  /* sanity check */
  if (my == NULL)
    return 0;

  if (priority != 0)
    return 0;

  /* decoration */
  color = tenm_map_color(182, 123, 162);
  if (tenm_draw_line((int) (my->x + 15.0), (int) (my->y - 20.0),
                     (int) (my->x + 15.0
                            - 40.0 * tenm_cos(15)
                            - 30.0 * (-tenm_sin(15))),
                     (int) (my->y - 20.0
                            - 40.0 * tenm_sin(15)
                            - 30.0 * tenm_cos(15)),
                     1, color) != 0)
    status = 1;
  if (tenm_draw_line((int) (my->x - 15.0), (int) (my->y + 20.0),
                     (int) (my->x - 15.0
                            - 40.0 * tenm_cos(-15)
                            - 30.0 * (-tenm_sin(-15))),
                     (int) (my->y + 20.0
                            - 40.0 * tenm_sin(-15)
                            - 30.0 * tenm_cos(-15)),
                     1, color) != 0)
    status = 1;

  /* body */
  color = tenm_map_color(95, 13, 68);
  if (tenm_draw_circle((int) (my->x), (int) (my->y), 25, 3, color) != 0)
    status = 1;

  return status;
}

static tenm_object *
insane_hand_ship_shot_new(double x, double y, double v_x, double v_y)
{
  tenm_primitive **p = NULL;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;

  /* sanity check */
  if (v_x * v_x + v_y * v_y < NEAR_ZERO)
  {
    fprintf(stderr, "insane_hand_ship_shot_new: speed is too small "
            "(%f, %f)\n", v_x, v_y);
    return NULL;
  }

  p = (tenm_primitive **) malloc(sizeof(tenm_primitive *) * 1);
  if (p == NULL)
  {
    fprintf(stderr, "insane_hand_ship_shot_new: malloc(p) failed\n");
    return NULL;
  }

  p[0] = (tenm_primitive *) tenm_circle_new(x, y, 5.0);
  if (p[0] == NULL)
  {
    fprintf(stderr, "insane_hand_ship_shot_new: cannot set p[0]\n");
    free(p);
    return NULL;
  }

  count = (int *) malloc(sizeof(int) * 1);
  if (count == NULL)
  {
    fprintf(stderr, "insane_hand_ship_shot_new: malloc(count) failed\n");
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 4);
  if (count_d == NULL)
  {
    fprintf(stderr, "insane_hand_ship_shot_new: malloc(count_d) failed\n");
    free(count);
    (p[0])->delete(p[0]);
    free(p);
    return NULL;
  }

  if (v_y > 0.1)
  {
    count_d[2] = -(-0.25) * 0.6;
    count_d[3] = (-0.25) * 0.8;
  }
  else if (v_y < -0.1)
  {
    count_d[2] = -(0.25) * 0.6;
    count_d[3] = (0.25) * 0.8;
  }
  else
  {
    count_d[2] = 0.0;
    count_d[3] = 0.0;
  }

  /* list of count
   * [0] color (for delete_enemy_shot)
   */
  count[0] = 4;

  /* list of count_d
   * [0] dx
   * [1] dy
   * [2] ddx
   * [3] ddy
   */
  count_d[0] = v_x * 0.8 - v_y * 0.6;
  count_d[1] = v_x * 0.6 + v_y * 0.8;

  new = tenm_object_new("Insane Hand ship shot", ATTR_ENEMY_SHOT, ATTR_OPAQUE,
                        1, x, y,
                        1, count, 4, count_d, 1, p,
                        (int (*)(tenm_object *, double))
                        (&insane_hand_ship_shot_move),
                        (int (*)(tenm_object *, tenm_object *))
                        (&insane_hand_ship_shot_hit),
                        (int (*)(tenm_object *, const tenm_object *))
                        (&insane_hand_ship_shot_act),
                        (int (*)(tenm_object *, int))
                        (&insane_hand_ship_shot_draw));

  if (new == NULL)
  {
    fprintf(stderr, "insane_hand_ship_shot_new: tenm_object_new failed\n");
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
insane_hand_ship_shot_move(tenm_object *my, double turn_per_frame)
{
  double dx_temp;
  double dy_temp;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "insane_hand_ship_shot_move: my is NULL\n");
    return 0;
  }
  if (turn_per_frame <= 0.5)
  {
    fprintf(stderr, "insane_hand_ship_shot_move: "
            "strange turn_per_frame (%f)\n",
            turn_per_frame);
    return 0;
  }

  dx_temp = my->count_d[0] / turn_per_frame;
  dy_temp = my->count_d[1] / turn_per_frame;
  my->x += dx_temp;
  my->y += dy_temp;
  tenm_move_mass(my->mass, dx_temp, dy_temp);

  if (!in_window_object(my))
    return 1;

  return 0;
}

static int
insane_hand_ship_shot_hit(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "insane_hand_ship_shot_hit: my is NULL\n");
    return 0;
  }
  if (your == NULL)
  {
    fprintf(stderr, "insane_hand_ship_shot_hit: your is NULL\n");
    return 0;
  }

  if (your->attr & ATTR_OPAQUE)
    return 1;

  return 0;
}

static int
insane_hand_ship_shot_act(tenm_object *my, const tenm_object *player)
{
  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "insane_hand_ship_shot_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  my->count_d[0] += my->count_d[2];
  my->count_d[1] += my->count_d[3];

  return 0;
}

static int
insane_hand_ship_shot_draw(tenm_object *my, int priority)
{
  int status = 0;
  tenm_color color;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "insane_hand_ship_shot_draw: my is NULL\n");
    return 0;
  }

  /* return if it is not my turn */
  if (priority != 1)
    return 0;

  color = tenm_map_color(75, 0, 239);
  if (tenm_draw_circle((int) (my->x), (int) (my->y), 5, 2, color) != 0)
    status = 1;

  return status;
}
