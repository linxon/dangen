/* $Id: info.c,v 1.8 2005/07/02 05:32:28 oohara Exp $ */

#include <stdio.h>
/* strlen */
#include <string.h>

#include "tenm_object.h"
#include "const.h"
#include "util.h"
#include "score.h"
#include "ship.h"
#include "chain.h"

#include "info.h"

static int chain_scroll = 0;
static int ship_scroll = 0;
static int score_scroll = 0;

void
clear_chain_scroll(void)
{
  chain_scroll = 0;
}

/* return 0 on success, 1 on error */
int
show_chain(const tenm_object *player)
{
  char temp[16];
  int chain;

  /* sanity check */
  chain = get_chain();
  if (chain < 0)
    return 0;

  /* hide the stat if the game is still on and
   * if the player is near it
   */
  if ((player != NULL) && (get_ship() >= 0)
      && (player->x > (double) (WINDOW_WIDTH * 2 / 3))
      && (player->y < 60.0))
  {
    if (chain_scroll < 20)
      chain_scroll += 2;
  }
  else
  {
    if (chain_scroll > 0)
      chain_scroll -= 2;
  }

  sprintf(temp, "chain %4d", chain);
  if (draw_string(WINDOW_WIDTH - 100, 10 - chain_scroll,
                  temp, (int) strlen(temp)) != 0)
  {
    fprintf(stderr, "show_chain: draw_string failed\n");
    return 1;
  }
  return 0;
}

void
clear_ship_scroll(void)
{
  ship_scroll = 0;
}

/* return 0 on success, 1 on error */
int
show_ship(const tenm_object *player)
{
  int ship;
  char temp[16];

  /* sanity check */
  ship = get_ship();
  if (ship < 0)
    return 0;

  /* hide the stat if the game is still on and
   * if the player is near it
   */
  if ((player != NULL) && (get_ship() >= 0)
      && (player->x > (double) (WINDOW_WIDTH * 2 / 3))
      && (player->y > (double) (WINDOW_HEIGHT - 60)))
  {
    if (ship_scroll < 20)
      ship_scroll += 2;
  }
  else
  {
    if (ship_scroll > 0)
      ship_scroll -= 2;
  }

  sprintf(temp, "ship %3d", ship);
  if (draw_string(WINDOW_WIDTH - 80, WINDOW_HEIGHT - 10 + ship_scroll,
                  temp, (int) strlen(temp)) != 0)
  {
    fprintf(stderr, "show_ship: draw_string failed\n");
    return 1;
  }

  return 0;
}

void
clear_score_scroll(void)
{
  score_scroll = 0;
}

/* return 0 on success, 1 on error */
int
show_score(const tenm_object *player)
{
  int score;
  char temp[32];

  score = get_score();

  /* hide the stat if the game is still on and
   * if the player is near it
   */
  if ((player != NULL) && (get_ship() >= 0)
      && (player->x < (double) (WINDOW_WIDTH / 3))
      && (player->y > (double) (WINDOW_HEIGHT - 60)))
  {
    if (score_scroll < 20)
      score_scroll += 2;
  }
  else
  {
    if (score_scroll > 0)
      score_scroll -= 2;
  }

  sprintf(temp, "score %8d", score);
  if (draw_string(10, WINDOW_HEIGHT - 10 + score_scroll,
                  temp, (int) strlen(temp)) != 0)
  {
    fprintf(stderr, "show_score: draw_string failed\n");
    return 1;
  }
  return 0;
}
