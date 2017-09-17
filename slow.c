/* $Id: slow.c,v 1.3 2009/11/10 18:53:35 oohara Exp $ */

#include <stdio.h>

#include "slow.h"

static int slowed = 0;
static int slowed_modify_ok = 0;

void
clear_slow(void)
{
  slowed = 0;
  slowed_modify_ok = 0;
}

/* return 1 if the game is slowed, 0 if not */
int
do_slow(int slow_key_pressed)
{
  if (slow_key_pressed)
  {
    if (slowed_modify_ok)
    {
      slowed_modify_ok = 0;
      if (slowed)
        slowed = 0;
      else
        slowed = 1;
    }
  }
  else
  {
    slowed_modify_ok = 1;
  }

  return slowed;
}

