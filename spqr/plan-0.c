/* $Id: plan-0.c,v 1.182 2005/07/12 18:20:43 oohara Exp $ */
/* [tutorial] dangen tutorial */

#include <stdio.h>
/* malloc, rand, atexit */
#include <stdlib.h>
/* strlen, strcmp */
#include <string.h>
/* errno */
#include <errno.h>

#include "scheduler.h"
#include "tenm_table.h"
#include "normal-enemy.h"
#include "const.h"
#include "flatdice.h"
#include "util.h"
#include "tutor.h"
#include "option.h"
#include "ship.h"
#include "score.h"
#include "ship.h"
#include "chain.h"
#include "stage.h"
#include "background.h"
#include "laser.h"
#include "wall-0.h"
/* set_this_stage_cleared */
#include "scheduler.h"

#include "plan-0.h"

static flatdice *dice_where = NULL;
static flatdice *dice_what = NULL;

static void plan_0_more_1_quit(void);

static tenm_object *plan_0_more_1_new(void);
static int plan_0_more_1_act(tenm_object *my, const tenm_object *player);
static int plan_0_more_1_draw(tenm_object *my, int priority);

int
plan_0(int t)
{
  /* sanity check */
  if (t < 0)
    return SCHEDULER_ERROR;

  if (t == 0)
    tenm_table_add(plan_0_more_1_new());

  /*
  if (t == 30)
    tenm_table_add(tutor_new(1, 100.0, 100.0,
                             0, 0, 11));
  */

  return SCHEDULER_SUCCESS;
}

static void
plan_0_more_1_quit(void)
{
  if (dice_where != NULL)
    flatdice_delete(dice_where);
  if (dice_what != NULL)
    flatdice_delete(dice_what);
  dice_where = NULL;
  dice_what = NULL;
}

static tenm_object *
plan_0_more_1_new(void)
{
  int i;
  tenm_object *new = NULL;
  int *count = NULL;
  double *count_d = NULL;

  count = (int *) malloc(sizeof(int) * 11);
  if (count == NULL)
  {
    fprintf(stderr, "plan_0_more_1_new: malloc(count) failed\n");
    return NULL;
  }
  count_d = (double *) malloc(sizeof(double) * 3);
  if (count_d == NULL)
  {
    fprintf(stderr, "plan_0_more_1_new: malloc(count_d) failed\n");
    free(count);
    return NULL;
  }

  /* list of count
   * [0] mode
   * [1] timer
   * [2] wait (main)
   * [3] wait (sub)
   * [4] wait (reject)
   * [5] number of tutors killed/escaped
   * [6] number of tutors created
   * [7 -- 9] large enemy timer
   * [10] level display position
   */
  count[0] = 0;
  count[1] = -1;
  count[2] = 0;
  count[3] = 0;
  count[4] = 0;
  count[5] = 0;
  count[6] = 0;
  for (i = 0; i < 3; i++)
    count[7 + i] = 0;
  count[10] = 20;

  new = tenm_object_new("plan 0 more 1", 0, 0,
                        1, 0.0, 0.0,
                        11, count, 3, count_d, 0, NULL,
                        (int (*)(tenm_object *, double))
                        NULL,
                        (int (*)(tenm_object *, tenm_object *))
                        NULL,
                        (int (*)(tenm_object *, const tenm_object *))
                        (&plan_0_more_1_act),
                        (int (*)(tenm_object *, int))
                        (&plan_0_more_1_draw));
  if (new == NULL)
  {
    fprintf(stderr, "plan_0_more_1_new: tenm_object_new failed\n");
    if (count_d != NULL)
      free(count_d);
    if (count != NULL)
      free(count);
    return NULL;
  }

  return new;
}

int
plan_0_more_1_easter_egg(tenm_object *my)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (my->name == NULL)
    return 0;

  /* this check is mandatory
   * --- this function is applied by tenm_table_apply_all()
   */
  if (strcmp(my->name, "plan 0 more 1") != 0)
    return 0;

  if (my->count[0] != 0)
    return 0;
  if (my->count[1] >= 2970)
    return 0;

  set_background(3);
  tenm_table_apply_all((int (*)(tenm_object *, int))
                       (&delete_enemy_shot), 0);
  tenm_table_apply_all((int (*)(tenm_object *, int))
                       (&delete_enemy), 0);

  clear_chain();
  clear_score();
  clear_ship();
  add_ship(-get_ship());
  set_stage_number(1);
  set_stage_id(get_stage_number(), 0);

  my->count[0] = 1;
  my->count[1] = -1;

  return 0;
}

static int
plan_0_more_1_act(tenm_object *my, const tenm_object *player)
{
  int i;
  int n;
  int temp;
  int where;
  int what;
  int rank;
  double x;
  double y;
  double speed;
  const option *op = NULL;

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "plan_0_more_1_act: my is NULL\n");
    return 0;
  }
  if (player == NULL)
    return 0;

  op = get_option();
  if (op == NULL)
  {
    fprintf(stderr, "plan_0_more_1_act: get_option failed\n");
    return 1;
  }

  (my->count[1])++;

  /* hide the stat if the game is still on and
   * if the player is near it
   */
  if ((player != NULL) && (get_ship() >= 0)
      && (player->x < (double) (WINDOW_WIDTH / 3))
      && (player->y < 60.0))
  {
    if (my->count[10] < 20)
      my->count[10] += 2;
  }
  else
  {
    if (my->count[10] > 0)
      my->count[10] -= 2;
  }
  if (my->count[0] == 0)
    my->count[10] = 20;

  switch (my->count[0])
  {
  case 0:
    if (my->count[1] == 310)
      tenm_table_add(normal_enemy_new(181.4, -24.0,
                                      BALL_CAPTAIN, 0,
                                      0, -1, 0, -1, 0, 2, 1,
                                      /* move 0 */
                                      60, 0.0, 4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      9999, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* shoot 0 */
                                      9999, 9999, 0, 0, 0, 0));
    if ((my->count[1] == 750)
        || (my->count[1] == 765)
        || (my->count[1] == 780))
      tenm_table_add(normal_enemy_new(-18.0, 284.5,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999, 3.6, 1.5, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, 9999, 0, 0, 0, 0));
    if ((my->count[1] == 697)
        || (my->count[1] == 712)
        || (my->count[1] == 727))
      tenm_table_add(normal_enemy_new(102.0, -19.0,
                                      BALL_SOLDIER, 0,
                                      0, -1, 0, -1, 0, 1, 1,
                                      /* move 0 */
                                      9999, 0.0, 4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 0,
                                      /* shoot 0 */
                                      9999, 9999, 0, 0, 0, 0));
    if (my->count[1] == 950)
    {
      tenm_table_add(normal_enemy_new(321.4, -24.0,
                                      BALL_CAPTAIN, 0,
                                      0, -1, 0, -1, 0, 2, 3,
                                      /* move 0 */
                                      60, 0.0, 4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      9999, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* shoot 0 */
                                      800, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      60, 9999, 0, 0, 1, 2,
                                      /* shoot 2 */
                                      60, 9999, 0, 0, 0, 1));
      tenm_table_add(normal_enemy_new(181.4, -24.0,
                                      BALL_CAPTAIN, 0,
                                      0, -1, 0, -1, 0, 2, 3,
                                      /* move 0 */
                                      60, 0.0, 4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      9999, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* shoot 0 */
                                      800, 9999, 9909, 0, 0, 1,
                                      /* shoot 1 */
                                      60, 9999, 0, 0, 0, 2,
                                      /* shoot 2 */
                                      60, 9999, 0, 0, 1, 1));
      tenm_table_add(normal_enemy_new(41.4, -24.0,
                                      BALL_CAPTAIN, 0,
                                      0, -1, 0, -1, 0, 2, 3,
                                      /* move 0 */
                                      60, 0.0, 4.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* move 1 */
                                      9999, 0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0, 1,
                                      /* shoot 0 */
                                      800, 9999, 0, 0, 0, 1,
                                      /* shoot 1 */
                                      60, 9999, 0, 0, 1, 2,
                                      /* shoot 2 */
                                      60, 9999, 0, 0, 0, 1));
    }
    
    if ((my->count[1] >= 1350) && (my->count[1] < 1410)
        && ((my->count[1] - 1350) % 3 == 0))
    {
      speed = 3.0 + ((double) (my->count[1] - 1350)) * 0.1;

      x = (double) (WINDOW_WIDTH / 2 + (my->count[1] - 1350) * 4);
      tenm_table_add(laser_point_new(x, 0.0, speed,
                                     player->x - 10.0, player->y,
                                     25.0, 1));

      x = (double) (WINDOW_WIDTH / 2 - (my->count[1] - 1350) * 4);
      tenm_table_add(laser_point_new(x, 0.0, speed,
                                     player->x + 10.0, player->y,
                                     25.0, 1));

      x = (double) (WINDOW_WIDTH / 2 - 240 + (my->count[1] - 1350) * 8);
      tenm_table_add(laser_point_new(x, 0.0, speed + 2.0,
                                     player->x - 30.0, player->y,
                                     25.0, 0));

      x = (double) (WINDOW_WIDTH / 2 + 240 - (my->count[1] - 1350) * 8);
      tenm_table_add(laser_point_new(x, 0.0, speed + 2.0,
                                     player->x + 30.0, player->y,
                                     25.0, 0));
    }

    if (my->count[1] == 2810)
      tenm_table_add(wall_0_new(65.0));

    if (my->count[1] >= 3030)
    {
      /* no set_stage_cleared() here */
      set_this_stage_cleared(1);
      return 1;
    }
    
    break;
  case 1:
    if (my->count[1] == 0)
    {
      if ((dice_where == NULL) && (dice_what == NULL))
      {
        dice_where = flatdice_new(3, 3, 5);
        dice_what = flatdice_new(45, 1, 1);

        errno = 0;
        if (atexit(&plan_0_more_1_quit) != 0)
        {  
          fprintf(stderr, "plan_0_more_1_act: cannot register "
                  "plan_0_more_1_quit to exit");
          if (errno != 0)
            fprintf(stderr, " (%s)", strerror(errno));
          fprintf(stderr, "\n");
        }
      }
      else
      {
        if ((dice_where == NULL) || (dice_what == NULL))
        {
          fprintf(stderr, "plan_0_more_1_act: some flatdice are missing\n");
          return 1;
        }

        flatdice_reset(dice_where);
        flatdice_reset(dice_what);
      }
    }
    if (my->count[1] >= 130)
    {
      my->count[0] = 2;
      my->count[1] = -1;
    }
    break;
  case 2:
    if (my->count[5] >= 500)
    {
      my->count[0] = 3;
      my->count[1] = -1;
      return 0;
    }
    
    (my->count[2])--;
    if (my->count[2] < 0)
    {      
      my->count[2] = 0; 

      (my->count[3])--;
      if (my->count[3] < 0)
        my->count[3] = 0;

      (my->count[4])--;
      if (my->count[4] < 0)
        my->count[4] = 0;
    }
    for (i = 0; i < 3; i++)
    {
      (my->count[7 + i])--;
      if (my->count[7 + i] < 0)
        my->count[7 + i] = 0;
    }

    if ((my->count[2] <= 0) && (my->count[4] <= 0) && (my->count[6] < 500))
    {
      if ((my->count[3] > 0) && (rand() % 100 < my->count[3]))
      {
        if (my->count[4] < my->count[3])
          my->count[4] = my->count[3];
      }
      else if ((my->count[6] >= 100)
               && (my->count[6] % 100 == 0)
               && (my->count[5] < my->count[6]))
      {
        /* do nothing */
        ;
      }
      else
      { 
        where = flatdice_next(dice_where);

        n = flatdice_next(dice_what);
        if (n < 0)
        {
          fprintf(stderr, "plan_0_more_1_act: flatdice_next(dice_what) "
                  "failed\n");
        }
        if (n < 5)
          what = 2;
        else if (n < 9)
          what = 3;
        else if (n < 33)
          what = 0;
        else
          what = 1;

        if (my->count[6] % 100 >= 97)
        {
          if (rand() % 3 != 0)
            what = 0;
          else
            what = 1;
        }

        if ((what >= 2) && (my->count[7 + where] > 0))
        {
          if (rand() % 2 == 0)
            temp = 1;
          else
            temp = -1;
          for (i = 1; i <= 2; i++)
          {
            if (my->count[7 + (where + i * temp + 3) % 3] <= 0)
            {
              where = (where + i * temp + 3) % 3;
              break;
            }
          }
          if (my->count[7 + where] > 0)
          {
            if (rand() % 3 != 0)
              what = 0;
            else
              what = 1;
          }
        }

        switch (where)
        {
        case 0:
          x = 160.0;
          break;
        case 1:
          x = 320.0;
          break;
        case 2:
          x = 480.0;
          break;
        default:
          fprintf(stderr, "plan_0_more_1_act: undefined where (%d)\n",
                  where);
          x = (double) (WINDOW_WIDTH / 2);
          break;
        }

        switch (what)
        {
        case 0:
          y = -19.0;
          x += (double) (-60 + rand() % 121);
          break;
        case 1:
          y = -24.0;
          x += (double) (-60 + rand() % 121);
          break;
        case 2:
          y = -35.0;
          x += (double) (-10 + rand() % 21);
          break;
        case 3:
          y = -47.0;
          x += (double) (-10 + rand() % 21);
          break;
        default:
          fprintf(stderr, "plan_0_more_1_act: strange what when deciding "
                  "position (%d)\n", what);
          y = 240.0;
          break;
        }

        rank = my->count[6];
        if (op->free_select != 0)
        {
          if (rank < 499)
            rank = 499;
        }
        
        tenm_table_add(tutor_new(what, x, y,
                                 my->table_index, rank));

        switch (what)
        {
        case 0:
          my->count[2] = 37;
          my->count[3] += 6;
          my->count[6] += 1;
          break;
        case 1:
          my->count[2] = 29;
          my->count[3] += 3;
          my->count[6] += 1;
          break;
        case 2:
          my->count[2] = 40;
          my->count[3] += 20;
          my->count[6] += 3;
          break;
        case 3:
          my->count[2] = 42;
          my->count[3] += 28;
          my->count[6] += 3;
          break;
        default:
          fprintf(stderr, "plan_0_more_1_act: strange what when adding "
                  "wait (%d)\n", what);
          break;
        }

        if (what >= 2)
          my->count[7 + where] += 100;
        
        if ((my->count[2] > 5) && (rand() % 3 != 0))
        {
          temp = rand() % ((my->count[2] + 1) / 2);
          if (temp < 5)
            temp = 5;
          my->count[2] -= temp;
          my->count[3] += temp;
        }
      }
    }
    break;
  case 3:
    if (get_ship() < 0)
      return 1;
    if (my->count[1] == 0)
    {
      set_background(1);
      tenm_table_apply_all((int (*)(tenm_object *, int))
                           (&delete_enemy_shot), 0);
      tenm_table_apply_all((int (*)(tenm_object *, int))
                           (&delete_enemy), 0);
      set_stage_cleared(get_stage_number(), 1);
    }
    if (my->count[1] >= 100)
    {
      if (get_ship() >= 0)
      {
        set_this_stage_cleared(1);
      }
      return 1;
    }
    break;
  default:
    break;
  }

  return 0;
}

static int
plan_0_more_1_draw(tenm_object *my, int priority)
{
  int status = 0;
  char temp[32];

  /* sanity check */
  if (my == NULL)
  {
    fprintf(stderr, "plan_0_more_1_draw: my is NULL\n");
    return 0;
  }

  if (priority != 0)
    return 0;

  if ((my->count[0] == 1) && (my->count[1] < 100))
  {
    if (draw_string(257, 220, "OK, good luck!", 14) != 0)
    {
      fprintf(stderr, "plan_0_more_1_draw: draw_string (stage name) failed\n");
      status = 1;
    }
  }
  if (my->count[0] >= 1)
  {
    sprintf(temp, "level %3d", my->count[5]);
    if (draw_string(10, 10 - my->count[10],
                    temp, (int) strlen(temp)) != 0)
    {
      fprintf(stderr, "plan_0_more_1_draw: draw_string (level) failed\n");
      return 1;
    }
  }

  if (my->count[0] >= 1)
    return status;

  /* the tutorial */
  if ((my->count[1] >= 30) && (my->count[1] < 130))
  {  
    if (draw_string(253, 220,
                    "dangen tutorial", 15) != 0)
      status = 1;
  }
  if ((my->count[1] >= 160) && (my->count[1] < 290))
  {  
    if (draw_string(330, 50,
                    "Press cursor keys to move.", 26) != 0)
      status = 1;
  }

  if ((my->count[1] >= 340) && (my->count[1] < 600))
  {  
    if (draw_string(330, 50,
                    "Use the space key to shoot.", 27) != 0)
      status = 1;
  }
  if ((my->count[1] >= 400) && (my->count[1] < 600))
  {  
    if (draw_string(330, 90,
                    "No, just pressing the space key is", 34) != 0)
      status = 1;
    if (draw_string(330, 110,
                    "_NOT_ enough to fire a shot.", 28) != 0)
      status = 1;
  }

  if ((my->count[1] >= 650) && (my->count[1] < 900))
  {  
    if (draw_string(330, 50,
                    "To shoot, press a cursor key", 28) != 0)
      status = 1;
    if (draw_string(330, 70,
                    "while pressing the space key.", 29) != 0)
      status = 1;
    if (draw_string(330, 90,
                    "A shot is fired in the direction", 32) != 0)
      status = 1;
    if (draw_string(330, 110,
                    "of that cursor key.", 19) != 0)
      status = 1;
  }

  if ((my->count[1] >= 960) && (my->count[1] < 1200))
  {  
    if (draw_string(330, 50,
                    "Your ship is destroyed if it", 28) != 0)
      status = 1;
    if (draw_string(330, 70,
                    "gets hit by something.", 22) != 0)
      status = 1;
  }

  if ((my->count[1] >= 1260) && (my->count[1] < 1670))
  {  
    if (draw_string(330, 50,
                    "The circle at the center is", 27) != 0)
      status = 1;
    if (draw_string(330, 70,
                    "the only weak point of your ship.", 33) != 0)
      status = 1;
    if (draw_string(330, 90,
                    "The rest is safe.", 27) != 0)
      status = 1;
  }

  if ((my->count[1] >= 1730) && (my->count[1] < 2000))
  {  
    if (draw_string(330, 50,
                    "Enemies are usually brown and", 29) != 0)
      status = 1;
    if (draw_string(330, 70,
                    "sometimes change their color", 28) != 0)
      status = 1;
    if (draw_string(330, 90,
                    "to green.", 9) != 0)
      status = 1;
  }

  if ((my->count[1] >= 2060) && (my->count[1] < 2500))
  {  
    if (draw_string(330, 50,
                    "You can get an additional score", 31) != 0)
      status = 1;
    if (draw_string(330, 70,
                    "by shooting enemies while they are", 34) != 0)
      status = 1;
    if (draw_string(330, 90,
                    "green.", 6) != 0)
      status = 1;
    if (draw_string(330, 110,
                    "This is called the chain bonus.", 31) != 0)
      status = 1;
    if (draw_string(330, 130,
                    "(See the chain status at the", 28) != 0)
      status = 1;
    if (draw_string(330, 150,
                    "upper right of the window.)", 27) != 0)
      status = 1;
  }

  if ((my->count[1] >= 2560) && (my->count[1] < 2760))
  {  
    if (draw_string(330, 50,
                    "The number of chains is reset to", 32) != 0)
      status = 1;
    if (draw_string(330, 70,
                    "zero if your shot misses.", 25) != 0)
      status = 1;
  }

  if ((my->count[1] >= 2820) && (my->count[1] < 2970))
  {  
    if (draw_string(330, 50,
                    "You can't destroy a red enemy.", 30) != 0)
      status = 1;
  }

  return status;
}
