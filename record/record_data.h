/* $Id: record_data.h,v 1.26 2005/07/01 03:14:58 oohara Exp $ */ 

#ifndef __DANGEN_RECORD_DATA_H__
#define __DANGEN_RECORD_DATA_H__

/* FILE */
#include <stdio.h>

#include "stage-list.h"

struct _plan_record
{
  int score_max;
  int score_max_when;
  int number_clear;
  int number_clear_when;
  int score_min_cleared;
  int score_min_cleared_when;
  int number_play;
  int number_play_when;
};
typedef struct _plan_record plan_record;

struct _total_record
{
  int total_score;
  int total_score_when;
  int stage_n;
  int *stage_id;
  int *stage_score;
  int *stage_cleared;
  int number_play;
  int number_play_when;
};
typedef struct _total_record total_record;

struct _game_record
{
  total_record *total_p;
  plan_record **plan_p;
  int plan_n;
};
typedef struct _game_record game_record;

plan_record *plan_record_new(int valid);
void plan_record_delete(plan_record *p);
total_record *total_record_new(int stage_n, int valid);
void total_record_delete(total_record *p);
game_record *game_record_new(int stage_n, int plan_n, int valid);
void game_record_delete(game_record *p);

int plan_record_valid(plan_record *p, int quiet);
int total_record_valid(total_record *p, int quiet);
int game_record_valid(game_record *p, int quiet);

void print_plan_record(FILE *stream, plan_record *p);
void print_total_record(FILE *stream, total_record *p);
void print_game_record(FILE *stream, game_record *p, stage_list *list);

int game_record_update(game_record *p);
int increment_play_total(game_record *p);
int increment_play_plan(game_record *p, int n);
int increment_clear_plan(game_record *p, int n);

#endif /* __DANGEN_RECORD_DATA_H__ */
