/* $Id: score.c,v 1.35 2005/07/10 04:40:58 oohara Exp $ */

#include <stdio.h>
/* strlen */
#include <string.h>

#include "util.h"
#include "tenm_object.h"
#include "const.h"
#include "stage.h"
#include "ship.h"

#include "score.h"

#define EXTEND_FIRST 200000
#define EXTEND_LATER_EVERY 200000

static int score = 0;
static int stage_score[6];
static int stage_cleared[6];
static int extend_next = EXTEND_FIRST;

void
clear_score(void)
{
  int i;

  score = 0;
  for (i = 0; i < 6; i++)
  {  
    stage_score[i] = 0;
    stage_cleared[i] = 0;
  }
  extend_next = EXTEND_FIRST;
}

int
get_score(void)
{
  return score;
}

int
get_stage_score(int stage)
{
  /* sanity check */
  if ((stage <= 0) || (stage > 6))
  {
    fprintf(stderr, "get_stage_score: strange stage (%d)\n", stage);
    return 0;
  }

  return stage_score[stage - 1];
}

int
get_stage_cleared(int stage)
{
  /* sanity check */
  if ((stage <= 0) || (stage > 6))
  {
    fprintf(stderr, "get_stage_cleared: strange stage (%d)\n", stage);
    return 0;
  }

  return stage_cleared[stage - 1];
}

void
set_stage_cleared(int stage, int n)
{
  /* sanity check */
  if ((stage <= 0) || (stage > 6))
  {
    fprintf(stderr, "set_stage_cleared: strange stage (%d)\n", stage);
    return;
  }

  stage_cleared[stage - 1] = n;
}

/* return the actual score added to the total */
int
add_score(int delta)
{
  if (score + delta < 0)
    delta = -score;

  score += delta;
  if ((get_stage_number() >= 1) && (get_stage_number() <= 6))
    stage_score[get_stage_number() - 1] += delta;

  while (score >= extend_next)
  {
    extend_next += EXTEND_LATER_EVERY;
    /* you don't get an extra ship if you have already
     * lost the game, if you have cleared the game or
     * if you are watching the tutorial demo
     */
    if ((get_ship() >= 0)
        && (get_stage_number() >= 1)
        && (get_stage_number() <= 5)
        && (get_stage_id(get_stage_number()) > 0))
      add_ship(1);
  }

  return delta;
}

/* return the damege that should be subtracted from
 * the hit point of the enemy */
int
add_damage_score(int hit_point, int damage)
{
  /* sanity check */
  if (hit_point <= 0)
    return 0;
  if (damage <= 0)
    return 0;

  if (hit_point > damage)
  {
    add_score(damage);
  }
  else
  {
    damage = hit_point;
    add_score(damage - 1);
  }

  return damage;
}
