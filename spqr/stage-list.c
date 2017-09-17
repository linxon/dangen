/* $Id: stage-list.c,v 1.8 2005/07/01 19:24:16 oohara Exp $ */

#include <stdio.h>
/* malloc */
#include <stdlib.h>
/* strdup, strcmp */
#include <string.h>

#include "stage-list.h"

static int stage_list_add(stage_list *p, stage_plan *new_plan);

stage_plan *
stage_plan_new(int stage_id)
{
  stage_plan *new = NULL;
  const char *name = NULL;
  int difficulty;

  new = (stage_plan *) malloc(sizeof(stage_plan));
  if (new == NULL)
  {
    fprintf(stderr, "stage_plan_new: malloc failed\n");
    return NULL;
  }

  switch(stage_id)
  {
  case 0:
    name = "dangen tutorial";
    difficulty = PLAN_TUTORIAL;
    break;
  case 1:
    name = "L";
    difficulty = PLAN_HARDEST;
    break;
  case 2:
    name = "Senators";
    difficulty = PLAN_VERY_HARD;
    break;
  case 3:
    name = "Seiron";
    difficulty = PLAN_HARD;
    break;
  case 4:
    name = "Seiron Fake";
    difficulty = PLAN_EASY;
    break;
  case 5:
    name = "Perpeki";
    difficulty = PLAN_VERY_HARD;
    break;
  case 6:
    name = "Empty Wind";
    difficulty = PLAN_VERY_HARD;
    break;
  case 7:
    name = "cat tail (grep mix)";
    difficulty = PLAN_HARD;
    break;
  case 8:
    name = "Silver Chimera";
    difficulty = PLAN_HARD;
    break;
  case 9:
    name = "0x82da3104";
    difficulty = PLAN_HARD;
    break;
  case 10:
    name = "Tadashi";
    difficulty = PLAN_NORMAL;
    break;
  case 11:
    name = "Theorem Weapon";
    difficulty = PLAN_NORMAL;
    break;
  case 12:
    name = "W-KO";
    difficulty = PLAN_NORMAL;
    break;
  case 13:
    name = "Hell Salvage";
    difficulty = PLAN_NORMAL;
    break;
  case 14:
    name = "Strikers 1341";
    difficulty = PLAN_EASY;
    break;
  case 15:
    name = "Watcher Below";
    difficulty = PLAN_EASY;
    break;
  case 16:
    name = "cat tail";
    difficulty = PLAN_EASY;
    break;
  case 17:
    name = "Afterdeath";
    difficulty = PLAN_VERY_EASY;
    break;
  case 18:
    name = "Hugin";
    difficulty = PLAN_VERY_EASY;
    break;
  case 19:
    name = "Gosanpachi";
    difficulty = PLAN_VERY_EASY;
    break;
  case 20:
    name = "P-can";
    difficulty = PLAN_EASIEST;
    break;
  default:
    name = "default";
    difficulty = PLAN_NORMAL;
    break;
  }

  new->stage_id = stage_id;
  new->name = name;
  new->difficulty = difficulty;

  return new;
}

void
stage_plan_delete(stage_plan *p)
{
  if (p == NULL)
    return;

  /* don't free p->name */

  free(p);
}

/* if match_func (arg 1) is not NULL, an stage_plan *p is
 * added to the return value only if (*match_func)(p, data)
 * returns true (non-zero)
 * set match_func (arg 1) to NULL to get the full list (including the tutorial)
 */
stage_list *
stage_list_new(int (*match_func)(stage_plan *, void *),
               void *data)
{
  int i;
  stage_list *new = NULL;
  stage_plan *new_plan = NULL;

  new = (stage_list *) malloc(sizeof(stage_list));
  if (new == NULL)
  {
    fprintf(stderr, "stage_list_new: malloc failed\n");
    return NULL;
  }
  new->n = 0;
  new->p = NULL;

  for (i = 0; i <= NUMBER_PLAN; i++)
  {
    new_plan = stage_plan_new(i);
    if (new_plan == NULL)
    {
      fprintf(stderr, "stage_list_new: plan %d is NULL\n", i);
      continue;
    }

    if ((match_func != NULL) && (!((*match_func)(new_plan, data))))
      continue;

    stage_list_add(new, new_plan);
  }

  return new;
}

/* return 0 on success, 1 on error */
static int
stage_list_add(stage_list *p, stage_plan *new_plan)
{
  stage_plan **temp = NULL;

  /* sanity check */
  if (p == NULL)
  {
    fprintf(stderr, "stage_list_add: p is NULL\n");
    return 1;
  }
  if (new_plan == NULL)
  if (p == NULL)
  {
    fprintf(stderr, "stage_list_add: new_plan is NULL\n");
    return 1;
  }

  if (p->p == NULL)
  {
    p->n = 0;
    temp = (stage_plan **) malloc(sizeof(stage_plan *));
  }
  else
  {
    temp = (stage_plan **) realloc(p->p, sizeof(stage_plan *) * (p->n + 1));
  }
  if (temp == NULL)
  {
    fprintf(stderr, "stage_list_add: cannot allocate memory for p->p\n");
    return 1;
  }
  p->p = temp;

  p->p[p->n] = new_plan;
  (p->n)++;

  return 0;
}

void
stage_list_delete(stage_list *p)
{
  int i;

  if (p == NULL)
    return;
  if (p->p != NULL)
  {
    for (i = 0; i < p->n; i++)
      stage_plan_delete(p->p[i]);
    free(p->p);
  }
  free(p);
}

/* don't free the pointer returned by this function */
const char *
stage_difficulty_string(int n)
{
  const char *p = NULL;

  switch(n)
  {
  case PLAN_TUTORIAL:
    p = "tutorial";
    break;
  case PLAN_EASIEST:
    p = "easiest";
    break;
  case PLAN_VERY_EASY:
    p = "very easy";
    break;
  case PLAN_EASY:
    p = "easy";
    break;
  case PLAN_NORMAL:
    p = "normal";
    break;
  case PLAN_HARD:
    p = "hard";
    break;
  case PLAN_VERY_HARD:
    p = "very hard";
    break;
  case PLAN_HARDEST:
    p = "hardest";
    break;
  default:
    fprintf(stderr, "stage_difficulty_string: strange n (%d)\n", n);
    p = "unknown";
    break;
  }

  return p;
}

/* for qsort()
 * return a positive value if a (arg 1) should be after b (arg 2)
 *        a negative value if a (arg 1) should be before b (arg 2)
 *        0 if a (arg 1) and b (arg 2) are the same
 */
/* this void * should be stage_plan ** */
int
stage_compare(const void *a, const void *b)
{
  const stage_plan *a_temp = *((const stage_plan * const *) a);
  const stage_plan *b_temp = *((const stage_plan * const *) b);

  if (a_temp->difficulty > b_temp->difficulty)
    return 1;
  else if (a_temp->difficulty < b_temp->difficulty)
    return -1;

  if ((a_temp->name != NULL) && (b_temp->name != NULL))
  {
    if (strcmp(a_temp->name, b_temp->name) != 0)
      return strcmp(a_temp->name, b_temp->name);
  }

  if (a_temp->stage_id > b_temp->stage_id)
    return 1;
  else if (a_temp->stage_id < b_temp->stage_id)
    return -1;

  return 0;
}
