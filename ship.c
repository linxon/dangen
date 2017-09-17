/* $Id: ship.c,v 1.19 2005/06/30 15:09:26 oohara Exp $ */

#include <stdio.h>
/* strlen */
#include <string.h>

#include "ship.h"

static int ship = 5;

void
clear_ship(void)
{
  ship = 5;
}

/* return the new value of ships */
int
add_ship(int delta)
{
  ship += delta;
  if (ship < -1)
    ship = -1;
  return ship;
}

int
get_ship(void)
{
  return ship;
}
