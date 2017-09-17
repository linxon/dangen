/* $Id: plan-10.c,v 1.6 2005/01/01 17:02:15 oohara Exp $ */

#include <stdio.h>

#include "scheduler.h"
#include "tenm_table.h"
#include "normal-enemy.h"
#include "nexus.h"
#include "stage-title.h"

#include "plan-10.h"

int
plan_10(int t)
{
  /* sanity check */
  if (t < 0)
    return SCHEDULER_ERROR;

  if (t == 30)
    tenm_table_add(stage_title_new());

  if (t == 160)
    tenm_table_add(nexus_new());

  return SCHEDULER_SUCCESS;
}
