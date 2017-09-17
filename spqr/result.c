/* $Id: result.c,v 1.28 2005/07/04 06:26:22 oohara Exp $ */

/* VERSION */
#include <config.h>

#include <stdio.h>
/* strlen */
#include <string.h>
/* malloc */
#include <stdlib.h>

#include "stage.h"
#include "score.h"
#include "tenm_graphic.h"
#include "util.h"
#include "tenm_input.h"
#include "tenm_timer.h"
#include "esc-ok.h"
#include "background.h"
#include "pause.h"
#include "const.h"
#include "option.h"

#include "result.h"

/* return 1 if the program should quit, 0 if not */
int
show_result(void)
{
  int i;
  int x;
  int y;
  char temp[128];
  const option *op = NULL;

  op = get_option();
  if (op == NULL)
  {
    fprintf(stderr, "game_loop: get_option failed\n");
    return 1;
  }

  /* no need to detect focus loss */
  tenm_set_focus_handler((void (*)(int)) NULL);
  clear_pause();

  tenm_timer_reset();

  set_background(0);
  clear_window_with_background();

  if (draw_string(100, 40, "dangen result", 14) != 0)
    fprintf(stderr, "show_result: draw_string (title) failed\n");

  sprintf(temp, "version %.20s", VERSION);
  if (draw_string(100, 60, temp, (int) strlen(temp)) != 0)
    fprintf(stderr, "show_result: draw_string (version) failed\n");

  if (cheating())
  {
    x = 253;
    y = 100;
    if (draw_string(100, y, "cheat option(s):", 16) != 0)
      fprintf(stderr, "show_result: draw_string (cheat option) failed\n");
    if (op->free_select != 0)
    {
      if (draw_string(x, y, "--free-select", 13) != 0)
        fprintf(stderr, "show_result: draw_string (--free-select) failed\n");
      x += 126;
    }
    if (op->slow != 0)
    {
      if (draw_string(x, y, "--slow", 6) != 0)
        fprintf(stderr, "show_result: draw_string (--slow) failed\n");
      x += 63;
    }
  }

  sprintf(temp, "total score:            %8d", get_score());
  if (draw_string(100, 150, temp, (int) strlen(temp)) != 0)
    fprintf(stderr, "show_result: draw_string (total score) failed\n");

  for (i = 1; i <= 5; i++)
  {
    if (i > get_stage_number())
      break;
    if (get_stage_id(i) < 0)
      break;
    if (get_stage_name(i) == NULL)
      break;

    y = 180 + (i - 1) * 20;
    sprintf(temp, "%-20.20s    %8d", get_stage_name(i),
            get_stage_score(i));
    if (draw_string(100, y, temp, (int) strlen(temp)) != 0)
      fprintf(stderr, "show_result: draw_string (stage %d) failed\n", i);

    if (get_stage_cleared(i) != 0)
    {
      if (draw_string(82, y, "*", 1) != 0)
        fprintf(stderr, "show_result: draw_string (stage %d clear mark) "
                "failed\n", i);
    }
  }
  if (get_stage_cleared(6) != 0)
  {
    y = 180 + (6 - 1) * 20;
    sprintf(temp, "* ship bonus              %8d", get_stage_score(6));
    if (draw_string(82, y, temp, (int) strlen(temp)) != 0)
      fprintf(stderr, "show_result: draw_string (ship bonus) "
              "failed\n");
  }
  

  if (draw_string(WINDOW_WIDTH / 2 - 76, 440, "press ESC to quit", 17) != 0)
    fprintf(stderr, "show_result: draw_string (ESC instruction) failed\n");

  tenm_redraw_window();

  while (1 == 1)
  {
    /* quit the program if a SDL_QUIT event happened
     * (for example, if a SIGINT signal (sent by Ctrl+c) is received)
     */
    if (tenm_event_handle() != 0)
    {
      return 1;
    }

    /* back to the title if ESC is pressed */
    if (tenm_get_key_status() & 32)
    {
      if (get_esc_ok())
      {
        set_esc_ok(0);
        break;
      }
    }
    else
    {
      set_esc_ok(1);
    }

    /* this wait is necessary to save CPU time */
    tenm_wait_next_frame();
  }
  
  return 0;
}
