/* $Id: stage-list.h,v 1.4 2005/07/01 19:19:10 oohara Exp $ */

#ifndef __DANGEN_STAGE_LIST_H__
#define __DANGEN_STAGE_LIST_H__

struct _stage_plan
{
  int stage_id;
  const char *name;
  int difficulty;
};
typedef struct _stage_plan stage_plan;

struct _stage_list
{
  /* number of stage_plan */
  int n;
  stage_plan **p;
};
typedef struct _stage_list stage_list;

/* total number of stage plans (excluding the tutorial) */
#define NUMBER_PLAN 20

/* stage difficulty */
#define PLAN_TUTORIAL 0
#define PLAN_EASIEST 1
#define PLAN_VERY_EASY 2
#define PLAN_EASY 3
#define PLAN_NORMAL 4
#define PLAN_HARD 5
#define PLAN_VERY_HARD 6
#define PLAN_HARDEST 7

stage_plan *stage_plan_new(int stage_id);
void stage_plan_delete(stage_plan *p);
stage_list *stage_list_new(int (*match_func)(stage_plan *, void *),
                           void *data);
void stage_list_delete(stage_list *p);
const char *stage_difficulty_string(int n);
int stage_compare(const void *a, const void *b);

#endif /* not __DANGEN_STAGE_LIST_H__ */
