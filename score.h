/* $Id: score.h,v 1.13 2005/06/30 15:11:02 oohara Exp $ */

#ifndef __DANGEN_SCORE_H__
#define __DANGEN_SCORE_H__

void clear_score(void);
int get_score(void);
int get_stage_score(int stage);
int get_stage_cleared(int stage);
void set_stage_cleared(int stage, int n);
int add_score(int delta);
int add_damage_score(int hit_point, int damage);

#endif /* not __DANGEN_SCORE_H__ */
