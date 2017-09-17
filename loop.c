/* $Id: loop.c,v 1.146 2009/11/10 18:55:36 oohara Exp $ */

#include <stdio.h>

#include "tenm_object.h"
#include "tenm_table.h"
#include "player.h"
#include "tenm_timer.h"
#include "tenm_input.h"
#include "const.h"
#include "tenm_graphic.h"
#include "background.h"
#include "scheduler.h"
#include "score.h"
#include "chain.h"
#include "stage.h"
#include "ship.h"
#include "stage-select.h"
#include "pause.h"
#include "result.h"
#include "esc-ok.h"
#include "option.h"
#include "info.h"
#include "record_data.h"
#include "record_io.h"
#include "slow.h"

#include "loop.h"

static int main_loop(int tutorial);
static void ship_bonus(void);
/*
static int draw_all_mass(tenm_object *my, int n);
*/

/* returns 1 if the program should quit, 0 if not */
int
game_loop(int tutorial)
{
  int i;
  int status = 0;
  game_record *record = NULL;

  clear_chain();
  clear_chain_scroll();
  clear_score();
  clear_score_scroll();
  clear_ship();
  clear_ship_scroll();
  set_stage_number(1);
  for (i = 1; i <= 6; i++)
  {
    set_stage_id(i, -1);
    set_stage_name(i, NULL);
    set_stage_difficulty(i, -1);
  }

  record = game_record_load();
  if (record == NULL)
  {  
    fprintf(stderr, "game_loop: game_record_load failed\n");
    return 1;
  }
  if ((!tutorial) && (!cheating()))
    increment_play_total(record);

  while (1 == 1)
  {
    if (tutorial)
    {
      set_stage_id(get_stage_number(), -1);
    }
    else
    {
      clear_chain_scroll();
      clear_score_scroll();
      clear_ship_scroll();
      status = stage_select(record);
      if (status != 0)
        break;
    }

    status = main_loop(tutorial);

    if ((!cheating()) && (get_stage_id(get_stage_number()) >= 0))
    {
      /* the order does matter here (when you clear this plan first time) */
      increment_play_plan(record, get_stage_id(get_stage_number()));
      game_record_update(record);
      if (status == 0)
        increment_clear_plan(record, get_stage_id(get_stage_number()));
    }

    if (tutorial)
      break;
    if (status != 0)
      break;

    add_stage_number(1);
    if (get_stage_number() >= 6)
    {
      ship_bonus();
      game_record_update(record);
      break;
    }
  }

  if (!cheating())
  {  
    if (game_record_save(record) != 0)
      fprintf(stderr, "game_loop: game_record_save failed\n");
  }
  if (record != NULL)
    game_record_delete(record);
  record = NULL;

  if ((!tutorial) && (status != 2))
  {
    if (show_result() != 0)
      return 1;
  }

  if (status == 2)
    return 1;

  return 0;
}

/* return
 * 0 if the player cleared the stage
 * 1 if the game or the tutorial is over
 * 2 if the program should quit
 * player (arg 1) is freed if and only if the return value is non-zero
 */
static int
main_loop(int tutorial)
{
  int i;
  int t = 0;
  int frame_passed = 0;
  int status = 0;
  int temp;
  tenm_object *player = NULL;
  const option *op = NULL;

  op = get_option();
  if (op == NULL)
  {
    fprintf(stderr, "game_loop: get_option failed\n");
    return 2;
  }

  set_background(0);

  tenm_table_clear_all();

  clear_chain();

  player = player_new(tutorial);
  if (player == NULL)
  {
    fprintf(stderr, "game_loop: player_new failed\n");
    return 2;
  }

  tenm_set_focus_handler((void (*)(int)) pause_by_mouse);
  clear_pause();
  clear_slow();

  tenm_timer_reset();

  while (1 == 1)
  {
    /* quit the program if a SDL_QUIT event happened 
     * (for example, if a SIGINT signal (sent by Ctrl+c) is received)
     */
    if (tenm_event_handle() != 0)
    {
      status = 2;
      break;
    }

    /* back to the title if ESC is pressed  */
    if (tenm_get_key_status() & 32)
    {
      if (get_esc_ok())
      {
        set_esc_ok(0);
        status = 1;
        break;
      }
    }
    else
    {
      set_esc_ok(1);
    }

    /* pause */
    if (do_pause(tenm_get_key_status() & 64))
    {
      frame_passed++;
      /* update the "paused" message */
      tenm_redraw_window();
      /* do_pause() needs this wait */
      tenm_wait_next_frame();
      continue;
    }

    for (i = 1; i <= 30; i++)
    {
      if (tenm_table_detect_collision(player) != 0)
        player = NULL;
      if (tenm_table_move(player, 30) != 0)
        player = NULL;
    }

    /* scheduler() must be called before tenm_table_do_action()
     * to clear action_needed flag correctly */
    temp = scheduler(tutorial, t);

    if (tenm_table_do_action(player) != 0)
      player = NULL;

    clear_window_with_background();

    tenm_table_draw(player);

    show_score(player);
    show_ship(player);
    show_chain(player);

    /* for those who want to see the world as it is */
    /* note that
     * (1) the player is not in the table
     * (2) this is slow
     */
    /*
    tenm_table_apply_all((int (*)(tenm_object *, int)) draw_all_mass, 0);
    */

    tenm_redraw_window();

    tenm_wait_next_frame();
    /* slow down if CAPS lock is set */
    if ((op->slow != 0) && (do_slow(tenm_get_key_status() & 128)))
      tenm_wait_next_frame();

    if (player == NULL)
    {
      /* no more life, game over */
      status = 1;
      break;
    }

    t++;
    frame_passed++;
    if (temp == SCHEDULER_NEXT_STAGE)
      break;
  }

  if (frame_passed > 0)
  {
    printf("average fps: %f\n", tenm_calculate_fps(frame_passed));
    fflush(stdout);
  }

  tenm_table_clear_all();
  if (player != NULL)
    tenm_object_delete(player);

  return status;
}

static void
ship_bonus(void)
{
  if (get_stage_number() != 6)
    return;
  if (get_stage_cleared(6) != 0)
    return;

  set_stage_id(6, 0);
  add_score(get_ship() * 30000);
  set_stage_cleared(6, 1);
}

#if 0
static int
draw_all_mass(tenm_object *my, int n)
{
  /* sanity check */
  if (my == NULL)
    return 0;

  if (my->mass == NULL)
    return 0;
  if (my->attr == 0)
    return 0;

  if (tenm_draw_mass(my->mass, tenm_map_color(0, 0, 0)) != 0)
  {
    fprintf(stderr, "draw_all_mass: tenm_draw_mass failed (%d)\n",
            my->table_index);
  }

  return 0;
}
#endif /* 0 */
