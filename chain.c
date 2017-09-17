/* $Id: chain.c,v 1.37 2005/06/30 15:07:06 oohara Exp $ */

#include <stdio.h>
/* strlen */
#include <string.h>

#include "const.h"
#include "util.h"
#include "score.h"
#include "tenm_math.h"
#include "ship.h"

#include "chain.h"

static int chain = 0;

static int add_chain2(int delta);

void
clear_chain(void)
{
  chain = 0;
}

int
get_chain(void)
{
  return chain;
}

/* chain handling function for hit() of an enemy
 * my is the enemy, your is the player shot
 * this must be called _after_ damage is dealt
 */
int
add_chain(tenm_object *my, tenm_object *your)
{
  /* sanity check */
  if (my == NULL)
    return 0;
  if (your == NULL)
    return 0;

  if ((my->hit_point <= 0) || (your->count[0] == 1))
    return add_chain2(1);

  return 0;
}

/* return the actual chain added to the total */
static int
add_chain2(int delta)
{
  int i;

  /* sanity check */
  /* decrease is not allowed --- simply clear it instead */
  if (delta <= 0)
    return 0;

  for (i = 0; i < delta; i++)
  {
    if (chain >= 10)
      add_score(tenm_pow2(10));
    else
      add_score(tenm_pow2(chain));

    chain++;
  }

  return delta;
}
