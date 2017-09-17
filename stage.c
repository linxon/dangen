/* $Id: stage.c,v 1.12 2004/08/16 15:48:09 oohara Exp $ */

#include <stdio.h>

#include "stage.h"

static int current_stage_number = 1;
static int current_stage_id[6];
static int current_stage_difficulty[6];
static const char *current_stage_name[6];

void
set_stage_number(int n)
{
  current_stage_number = n;
}

int
get_stage_number(void)
{
  return current_stage_number;
}

/* return the new value of stage_number */
int
add_stage_number(int delta)
{
  if (current_stage_number + delta <= 0)
  {
    fprintf(stderr, "add_stage_number: trying to let stage_number negative, "
            "assuming 1 instead\n");
    current_stage_number = 1;
  }
  else
  {
    current_stage_number += delta;
  }

  return current_stage_number;
}

void
set_stage_id(int stage, int n)
{
  /* sanity check */
  if ((stage < 1) || (stage > 6))
  {
    fprintf(stderr, "set_stage_id: strange stage (%d)\n", stage);
    return;
  }

  current_stage_id[stage - 1] = n;
}

void
set_stage_name(int stage, const char *p)
{
  /* sanity check */
  if ((stage < 1) || (stage > 6))
  {
    fprintf(stderr, "set_stage_name: strange stage (%d)\n", stage);
    return;
  }

  current_stage_name[stage - 1] = p;
}

void
set_stage_difficulty(int stage, int n)
{
  /* sanity check */
  if ((stage < 1) || (stage > 6))
  {
    fprintf(stderr, "set_stage_difficulty: strange stage (%d)\n", stage);
    return;
  }
  
  current_stage_difficulty[stage - 1] = n;
}

int
get_stage_id(int stage)
{
  /* sanity check */
  if ((stage < 1) || (stage > 6))
  {
    fprintf(stderr, "get_stage_id: strange stage (%d)\n", stage);
    return -1;
  }

  return current_stage_id[stage - 1];
}

const char *
get_stage_name(int stage)
{
  /* sanity check */
  if ((stage < 1) || (stage > 6))
  {
    fprintf(stderr, "get_stage_name: strange stage (%d)\n", stage);
    return NULL;
  }

  return current_stage_name[stage - 1];
}

int
get_stage_difficulty(int stage)
{
  /* sanity check */
  if ((stage < 1) || (stage > 6))
  {
    fprintf(stderr, "get_stage_difficulty: strange stage (%d)\n", stage);
    return -1;
  }

  return current_stage_difficulty[stage - 1];
}
