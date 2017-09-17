/* $Id: scheduler.c,v 1.92 2005/07/10 04:49:59 oohara Exp $ */

#include <stdio.h>

#include "const.h"
#include "tenm_table.h"
#include "last-boss.h"
#include "normal-enemy.h"
#include "seiron.h"
#include "seiron-fake.h"
#include "net-can-howl.h"
#include "stage.h"
#include "ship.h"
#include "plan-0.h"
#include "plan-1.h"
#include "plan-2.h"
#include "plan-3.h"
#include "plan-4.h"
#include "plan-5.h"
#include "plan-6.h"
#include "plan-7.h"
#include "plan-8.h"
#include "plan-9.h"
#include "plan-10.h"
#include "plan-11.h"
#include "plan-12.h"
#include "plan-13.h"
#include "plan-14.h"
#include "plan-15.h"
#include "plan-16.h"
#include "plan-17.h"
#include "plan-18.h"
#include "plan-19.h"
#include "plan-20.h"

#include "scheduler.h"

static int this_stage_cleared = 0;

int
scheduler(int tutorial, int t)
{
  /* sanity check */
  if (t < 0)
    return SCHEDULER_ERROR;

  if (this_stage_cleared != 0)
  {
    this_stage_cleared = 0;
    if (get_ship() < 0)
      return SCHEDULER_SUCCESS;
    return SCHEDULER_NEXT_STAGE;
  }

  if (tutorial)
    return plan_0(t);

  switch(get_stage_id(get_stage_number()))
  {
  case 0:
    return plan_0(t);
    break;
  case 1:
    return plan_1(t);
    break;
  case 2:
    return plan_2(t);
    break;
  case 3:
    return plan_3(t);
    break;
  case 4:
    return plan_4(t);
    break;
  case 5:
    return plan_5(t);
    break;
  case 6:
    return plan_6(t);
    break;
  case 7:
    return plan_7(t);
    break;
  case 8:
    return plan_8(t);
    break;
  case 9:
    return plan_9(t);
    break;
  case 10:
    return plan_10(t);
    break;
  case 11:
    return plan_11(t);
    break;
  case 12:
    return plan_12(t);
    break;
  case 13:
    return plan_13(t);
    break;
  case 14:
    return plan_14(t);
    break;
  case 15:
    return plan_15(t);
    break;
  case 16:
    return plan_16(t);
    break;
  case 17:
    return plan_17(t);
    break;
  case 18:
    return plan_18(t);
    break;
  case 19:
    return plan_19(t);
    break;
  case 20:
    return plan_20(t);
    break;
  default:
    fprintf(stderr, "scheduler: strange stage_id (%d) (t = %d)\n",
            get_stage_id(get_stage_number()), t);
    break;
  }

  return SCHEDULER_ERROR;
}

void
set_this_stage_cleared(int n)
{
  this_stage_cleared = n;
}
