/* $Id: main.c,v 1.271 2009/11/10 18:50:55 oohara Exp $ */

#include <stdio.h>
/* rand, exit */
#include <stdlib.h>
/* time */
#include <time.h>

#include "const.h"
#include "tenm_graphic.h"
#include "tenm_input.h"
#include "tenm_math.h"
#include "tenm_table.h"
#include "tenm_timer.h"
#include "util.h"
#include "loop.h"
#include "title.h"
#include "option.h"
#include "background.h"
#include "show-record.h"

/* note that delay granularity is 10 ms */
#define NUM_WAIT 3

int
main(int argc, char *argv[])
{
  int temp;
  int choice;
  const option *op = NULL;
  int graphic_flag;

  if (set_option(argc, argv) != 0)
  {
    fprintf(stderr, "main: set_option failed\n");
    return 1;
  }
  op = get_option();
  if (op == NULL)
  {
    fprintf(stderr, "main: get_option failed\n");
    return 1;
  }

  if (op->help != 0)
  {
    do_help();
    return 0;
  }
  if (op->version != 0)
  {
    do_version();
    return 0;
  }

  srand((unsigned int) time(NULL));

  if (op->full_screen != 0)
    graphic_flag = TENM_FULLSCREEN;
  else
    graphic_flag = 0;
  tenm_graphic_init(WINDOW_WIDTH, WINDOW_HEIGHT, graphic_flag, "dangen");
  set_background(0);
  clear_window_with_background();
  tenm_timer_init(NUM_WAIT);
  if (tenm_math_init(810, 11) != 0)
    fprintf(stderr, "main: tenm_math_init failed, continuing (can be slow)\n");
  tenm_table_init(256, -1, 1);
  tenm_set_key(8, TENM_KEY_UP, TENM_KEY_DOWN, TENM_KEY_RIGHT, TENM_KEY_LEFT,
               TENM_KEY_SPACE, TENM_KEY_ESCAPE, TENM_KEY_p, TENM_KEY_s);

  temp = tenm_joystick_init(8192);
  if (temp == TENM_JOYSTICK_INIT_ERROR)
  {
    fprintf(stderr, "main: tenm_joystick_init failed\n");
    return 1;
  }
  else if (temp == TENM_JOYSTICK_INIT_NO_JOYSTICK)
  {
    fprintf(stderr, "main: don't worry, just use the keyboard\n");
  }
  else
  {
    tenm_joystick_map_axis(TENM_JOYSTICK_UP, TENM_KEY_UP);
    tenm_joystick_map_axis(TENM_JOYSTICK_DOWN, TENM_KEY_DOWN);
    tenm_joystick_map_axis(TENM_JOYSTICK_LEFT, TENM_KEY_LEFT);
    tenm_joystick_map_axis(TENM_JOYSTICK_RIGHT, TENM_KEY_RIGHT);
    tenm_joystick_map_button(0, TENM_KEY_SPACE);
  }
  util_init(WINDOW_WIDTH, WINDOW_HEIGHT);

  while (1 == 1)
  {
    choice = title();
    if (choice == 3)
      break;
    if (choice == 2)
    {
      if (show_record() != 0)
        break;

      continue;
    }

    if (game_loop(choice) != 0)
      break;
  }

  return 0;
}
