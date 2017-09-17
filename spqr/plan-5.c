/* $Id: plan-5.c,v 1.55 2004/09/06 21:19:45 oohara Exp $ */
/* [very hard] Perpeki */

#include <stdio.h>

#include "scheduler.h"
#include "tenm_table.h"
#include "perpeki.h"
#include "negation-engine.h"
#include "warning.h"
#include "stage-title.h"

#include "plan-5.h"

int
plan_5(int t)
{
  /* sanity check */
  if (t < 0)
    return SCHEDULER_ERROR;

  if (t == 30)
    tenm_table_add(stage_title_new());

  if (t == 160)
    tenm_table_add(negation_engine_new());

  if (t == 10960)
    tenm_table_add(warning_new());

  if (t == 11090)
    tenm_table_add(perpeki_new());

  return SCHEDULER_SUCCESS;
}
