/* flatdice --- the dice that remember their last value
 * by Oohara Yuuma <oohara@libra.interq.or.jp>
 * $Id: flatdice.c,v 1.20 2005/07/06 13:26:10 oohara Exp $
 */

#include <stdio.h>
/* malloc, rand  */
#include <stdlib.h>

#include "flatdice.h"

/* return a random integer in [0, side - 1]
 * the return value is adjusted so that
 * * it is likely to differ from the last value
 *   (this adjustment is strong if "repeat" is large)
 * * all possible values appear almost same times
 *   (this adjustment is weak if "randomness" is large)
 */
flatdice *
flatdice_new(int side, int randomness, int repeat)
{
  int i;
  flatdice *new = NULL;

  /* sanity check */
  if (side <= 0)
  {
    fprintf(stderr, "flatdice_new: side is non-positive (%d)\n", side);
    return NULL;
  }
  if (randomness <= 0)
  {
    fprintf(stderr, "flatdice_new: randomness is non-positive (%d)\n",
            randomness);
    return NULL;
  }
  if (repeat <= 0)
  {
    fprintf(stderr, "flatdice_new: repeat is non-positive (%d)\n", repeat);
    return NULL;
  }

  new = (flatdice *) malloc(sizeof(flatdice));
  if (new == NULL)
  {
    fprintf(stderr, "flatdice_new: malloc(new) failed\n");
    return NULL;
  }

  new->side = side;
  new->randomness = randomness;
  new->repeat = repeat;
  new->stock = NULL;
  new->last = -1;

  new->stock = (int *) malloc(sizeof(int) * side);
  if (new == NULL)
  {
    fprintf(stderr, "flatdice_new: malloc(new->stock) failed\n");
    flatdice_delete(new);
    return NULL;
  }
  for (i = 0; i < side; i++)
    new->stock[i] = randomness;

  return new;
}

void
flatdice_delete(flatdice *p)
{
  if (p != NULL)
  {
    if (p->stock != NULL)
    { 
      free(p->stock);
      p->stock = NULL;
    }
    free(p);
  }
}

/* return 1 (true) or 0 (false) */
int
flatdice_valid(flatdice *p, int quiet)
{
  int i;
  int total;

  /* sanity check */
  if (p == NULL)
  {
    if (!quiet)
      fprintf(stderr, "flatdice_valid: p is NULL\n");
    return 0;
  }
  if (p->side <= 0)
  {
    if (!quiet)
      fprintf(stderr, "flatdice_valid: p->side is non-positive (%d)\n",
              p->side);
    return 0;
  }
  if (p->randomness <= 0)
  {
    if (!quiet)
      fprintf(stderr, "flatdice_valid: p->randomness is non-positive (%d)\n",
              p->randomness);
    return 0;
  }
  if (p->repeat <= 0)
  {
    if (!quiet)
      fprintf(stderr, "flatdice_valid: p->repeat is non-positive (%d)\n",
              p->repeat);
    return 0;
  }
  if (p->stock == NULL)
  {
    if (!quiet)
      fprintf(stderr, "flatdice_valid: p->stock is NULL\n");
    return 0;
  }

  total = 0;
  for (i = 0; i < p->side; i++)
  {
    if (p->stock[i] < 0)
    {
      if (!quiet)
        fprintf(stderr, "flatdice_valid: p->stock[%d] is negative (%d)\n",
                i, p->stock[i]);
      return 0;
    }
    if (i == p->last)
      total += p->stock[i];
    else
      total += p->stock[i] * p->repeat;
  }
  if (total <= 0)
  {
    if (!quiet)
      fprintf(stderr, "flatdice_valid: total is non-positive (%d)\n", total);
    return 0;
  }

  return 1;
}

/* return [0, p->side - 1] on success, -1 on error */
int
flatdice_next(flatdice *p)
{
  int i;
  int total;
  int n1;
  int n2;
  int needed;
  int result = -1;

  /* sanity check */
  if (p == NULL)
  {
    fprintf(stderr, "flatdice_next: p is NULL\n");
    return -1;
  }
  if (!flatdice_valid(p, 0))
  {
    fprintf(stderr, "flatdice_next: p is invalid\n");
    return -1;
  }

  total = 0;
  for (i = 0; i < p->side; i++)
  {
    if (i == p->last)
      total += p->stock[i];
    else
      total += p->stock[i] * p->repeat;
  }

  n1 = rand() % total + 1;
  n2 = 0;
  for (i = 0; i < p->side; i++)
  {
    if (((i == p->last) && (n2 + p->stock[i] >= n1))
        || ((i != p->last) && (n2 + p->stock[i] * p->repeat >= n1)))
    {
      result = i;
      p->last = i;
      (p->stock[i])--;
      if (p->stock[i] < 0)
        p->stock[i] = 0;
      break;
    }
    if (i == p->last)
      n2 += p->stock[i];
    else
      n2 += p->stock[i] * p->repeat;
  }

  needed = p->randomness;
  for (i = 0; i < p->side; i++)
  {
    if (needed > p->randomness - p->stock[i])
      needed = p->randomness - p->stock[i];
  }
  if (needed < 0)
    needed = 0;
  for (i = 0; i < p->side; i++)
    p->stock[i] += needed;

  return result;
}

/* return 0 on success, 1 on error */
int
flatdice_reset(flatdice *p)
{
  int i;

  /* sanity check */
  if (p == NULL)
  {
    fprintf(stderr, "flatdice_reset: p is NULL\n");
    return 1;
  }
  if (!flatdice_valid(p, 0))
  {
    fprintf(stderr, "flatdice_reset: p is invalid\n");
    return 1;
  }

  for (i = 0; i < p->side; i++)
    p->stock[i] = p->randomness;
  p->last = -1;

  return 0;
}
