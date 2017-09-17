/* $Id: background.c,v 1.9 2004/08/10 10:43:41 oohara Exp $ */

#include <stdio.h>

#include "tenm_graphic.h"
#include "const.h"

#include "background.h"

static int background_n = 0;
static int background_timer = 0;

void
set_background(int n)
{
  background_n = n;
  if (n == 0)
    background_timer = 0;
  else
    background_timer = -20;
}

int
clear_window_with_background(void)
{
  int red;
  int green;
  int blue;
  int red_base;
  int green_base;
  int blue_base;

  if ((background_n == 0) || (background_timer < -20)
      || (background_timer >= 32))
  {
    background_timer = 0;
    if (tenm_clear_window(tenm_map_color(DEFAULT_BACKGROUND_RED,
                                         DEFAULT_BACKGROUND_GREEN,
                                         DEFAULT_BACKGROUND_BLUE)))
    {
      fprintf(stderr, "clear_window_with_background: "
              "tenm_clear_window failed\n");
      return 1;
    }
    return 0;
  }

  switch (background_n)
  {
    /* boss destroyed */
  case 1:
    red_base = 255;
    green_base = 255;
    blue_base = 255;
    break;
    /* boss self-destruction */
  case 2:
    red_base = 160;
    green_base = 160;
    blue_base = 255;
    break;
    /* boss approaching */
  case 3:
    red_base = 255;
    green_base = 160;
    blue_base = 160;
    break;
  default:
    fprintf(stderr, "clear_window_with_background: undefined background_n "
            "(%d)\n", background_n);
    red_base = DEFAULT_BACKGROUND_RED;
    green_base = DEFAULT_BACKGROUND_GREEN;
    blue_base = DEFAULT_BACKGROUND_BLUE;
    break;
  }

  if (background_timer < 0)
  {
    red = red_base;
    green = green_base;
    blue = blue_base;
  }
  else
  {
    red = (DEFAULT_BACKGROUND_RED * background_timer
           + red_base * (32 - background_timer)) / 32;
    green = (DEFAULT_BACKGROUND_GREEN * background_timer
             + green_base * (32 - background_timer)) / 32;
    blue = (DEFAULT_BACKGROUND_BLUE * background_timer
            + blue_base * (32 - background_timer)) / 32;
  }

  background_timer++;
  if (background_timer >= 32)
  {
    background_n = 0;
    background_timer = 0;
  }

  if (tenm_clear_window(tenm_map_color(red, green, blue)))
  {
    fprintf(stderr, "clear_window_with_background: "
            "tenm_clear_window failed\n");
    return 1;
  }

  return 0;
}
