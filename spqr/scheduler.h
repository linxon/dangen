/* $Id: scheduler.h,v 1.5 2005/07/10 04:49:15 oohara Exp $ */

#ifndef __DANGEN_SCHEDULER_H__
#define __DANGEN_SCHEDULER_H__

/* scheduler return value */
#define SCHEDULER_SUCCESS 0
#define SCHEDULER_ERROR 1
#define SCHEDULER_NEXT_STAGE 2

int scheduler(int tutorial, int t);
void set_this_stage_cleared(int n);

#endif /* not __DANGEN_SCHEDULER_H__ */
