/* $Id: title.c,v 1.12 2005/07/05 17:52:52 oohara Exp $ */

/* VERSION */
#include <config.h>

#include <stdio.h>
/* strlen */
#include <string.h>

#include "tenm_input.h"
#include "tenm_graphic.h"
#include "util.h"
#include "esc-ok.h"
#include "tenm_timer.h"
#include "background.h"
/* COPYRIGHT_STRING */
#include "const.h"

#include "title.h"

/* return
 * 0 if a real game is requested
 * 1 if a tutorial demo is requested
 * 2 if a play record is requested
 * 3 if the program should quit
 */
int
title(void)
{
  int choice = 0;
  char temp[32];
  int delay = 6;

  /* we don't need to pause in the title */
  tenm_set_focus_handler((void (*)(int)) NULL);

  set_background(0);

  while (1 == 1)
  {
    if (tenm_event_handle() != 0)
    {
      choice = 3;
      break;
    }
    /* quit if ESC is pressed */
    if (tenm_get_key_status() & 32)
    {
      if (get_esc_ok())
      {
        /* this is completely useless, but let's be pedantic */
        set_esc_ok(0);
        choice = 3;
        break;
      }
    }
    else
    {
      set_esc_ok(1);
    }

    /* proceed if space if pressed */
    if (tenm_get_key_status() & 16)
      break;

    switch (tenm_get_key_status() & 15)
    {
    case 1:
      /* up */
      if (delay <= 0)
      {
        if (choice <= 0)
          choice = 3;
        else
          choice--;
        delay = 6;
      }
      else
      {
        delay--;
      }
      break;
    case 2:
      /* down */
      if (delay <= 0)
      {
        if (choice >= 3)
          choice = 0;
        else
          choice++;
        delay = 6;
      }
      else
      {
        delay--;
      }
      break;
    default:
      delay = 0;
      break;
    }
    

    clear_window_with_background();

    if (draw_string(80, 90, "dangen", 7) != 0)
      fprintf(stderr, "title: draw_string (title) failed\n");

    sprintf(temp, "version %.20s", VERSION);
    if (draw_string(80, 120, temp, (int) strlen(temp)) != 0)
      fprintf(stderr, "title: draw_string (version) failed\n");

    if (draw_string(100, 200, "start a game", 12) != 0)
      fprintf(stderr, "title: draw_string (start) failed\n");
    if (draw_string(100, 220, "tutorial", 8) != 0)
      fprintf(stderr, "title: draw_string (tutorial) failed\n");
    if (draw_string(100, 240, "play record", 11) != 0)
      fprintf(stderr, "title: draw_string (record) failed\n");
    if (draw_string(100, 260, "quit", 4) != 0)
      fprintf(stderr, "title: draw_string (quit) failed\n");

    if (draw_string(80, 200 + choice * 20, ">", 1) != 0)
      fprintf(stderr, "title: draw_string (cursor) failed\n");

    if (draw_string(120, 460, COPYRIGHT_STRING,
                    (int) strlen(COPYRIGHT_STRING)) != 0)
      fprintf(stderr, "title: draw_string (copyright) failed\n");

    tenm_redraw_window();

    tenm_wait_next_frame();
  }

  return choice;
}
