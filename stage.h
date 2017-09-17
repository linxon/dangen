/* $Id: stage.h,v 1.8 2004/08/15 12:03:59 oohara Exp $ */

#ifndef __DANGEN_STAGE_H__
#define __DANGEN_STAGE_H__

void set_stage_number(int n);
int get_stage_number(void);
int add_stage_number(int delta);
void set_stage_id(int stage, int n);
void set_stage_name(int stage, const char *p);
void set_stage_difficulty(int stage, int n);
int get_stage_id(int stage);
const char *get_stage_name(int stage);
int get_stage_difficulty(int stage);

#endif /* not __DANGEN_STAGE_H__ */
