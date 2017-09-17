/* $Id: plan-1.c,v 1.4 2004/08/17 15:08:11 oohara Exp $ */
/* [hardest] L */

#include <stdio.h>

#include "scheduler.h"
#include "tenm_table.h"
#include "last-boss.h"
#include "warning.h"
#include "stage-title.h"

#include "plan-1.h"

int
plan_1(int t)
{
  /* sanity check */
  if (t < 0)
    return SCHEDULER_ERROR;

  if (t == 30)
    tenm_table_add(stage_title_new());

  if (t == 160)
    tenm_table_add(warning_new());

  if (t == 290)
    tenm_table_add(last_boss_new());

  return SCHEDULER_SUCCESS;
}
