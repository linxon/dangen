/* $Id: pause.c,v 1.22 2004/08/26 16:48:17 oohara Exp $ */

#include <stdio.h>

/* WINDOW_WIDTH */
#include "const.h"
/* draw_string() */
#include "util.h"

#include "pause.h"

/* 0: the game is going
 * 1: the game is paused
 * 2: the game will continue soon
 */
static int paused = 0;
static int paused_modify_ok = 1;
static int paused_timer = 0;

static int show_pause_message(void);

void
clear_pause(void)
{
  paused = 0;
  paused_modify_ok = 1;
  paused_timer = 0;
}

/* return 1 if the game is paused, 0 if not */
int
do_pause(int pause_key_pressed)
{
  if (pause_key_pressed)
  {
    if (paused_modify_ok)
    {
      paused_modify_ok = 0;
      switch (paused)
      {
      case 0:
        paused = 1;
        paused_timer = 0;
        if (show_pause_message() != 0)
          fprintf(stderr, "do_pause: show_pause_message failed\n");
        break;
      case 1:
        paused = 2;
        paused_timer = 0;
        break;
      case 2:
        break;
      default:
        if (show_pause_message() != 0)
          fprintf(stderr, "do_pause: undefined paused (%d)\n", paused);
        break;
      }
      
    }
  }
  else
  {
    paused_modify_ok = 1;
  }

  if (paused == 2)
  {
    paused_timer++;
    if (paused_timer > 30)
    {
      paused = 0;
      paused_timer = 0;
    }
  }

  if (show_pause_message() != 0)
    fprintf(stderr, "do_pause: show_pause_message failed\n");

  if (paused)
    return 1;

  return 0;
}

/* pause the game if the mouse cursor is out of the window
 * note that moving the mouse cursor into the window does not
 * continue the game
 */
void
pause_by_mouse(int gain)
{
  if ((gain != 1) && (paused == 0))
  {
    paused = 1;
    paused_timer = 0;
    /* don't call show_pause_message() here */
  }
}

/* return 0 on success, 1 on error */
static int
show_pause_message(void)
{
  int status = 0;
  int i;

  switch (paused)
  {
  case 0:
    return 0;
    break;
  case 1:
    if (draw_string(WINDOW_WIDTH / 2 - 135, WINDOW_HEIGHT - 60,
                    "paused --- press p to continue", 30) != 0)
    {
      fprintf(stderr, "show_pause_message: draw_string (case 1) failed\n");
      status = 1;
    }
    break;
  case 2:
    if (draw_string(WINDOW_WIDTH / 2 - 157, WINDOW_HEIGHT - 40,
                    "ready", 5) != 0)
    {
      fprintf(stderr, "show_pause_message: draw_string (case 2) failed\n");
      status = 1;
    }
    for (i = 0; i < paused_timer; i++)
      if (draw_string(WINDOW_WIDTH / 2 - 112 + i * 9, WINDOW_HEIGHT - 40,
                      ".", 1) != 0)
      {
        fprintf(stderr, "show_pause_message: draw_string (dot %d) failed\n",
                i);
        status = 1;
      }
    break;
  default:
    fprintf(stderr, "show_pause_message: undefined paused (%d)\n", paused);
    status = 1;
    break;
  }

  return status;
}
