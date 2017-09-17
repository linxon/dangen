/* $Id: record_data.c,v 1.67 2005/07/22 00:34:02 oohara Exp $ */

/* FILE */
#include <stdio.h>
/* malloc */
#include <stdlib.h>
/* time_t, time */
#include <time.h>

/* in $(top_srcdir)/ */
#include "stage.h"
#include "score.h"
/* in $(top_srcdir)/spqr */
#include "stage-list.h"

#include "record_data.h"

plan_record *
plan_record_new(int valid)
{
  int timestamp;
  plan_record *new = NULL;

  new = (plan_record *) malloc(sizeof(plan_record));
  if (new == NULL)
  {
    fprintf(stderr, "plan_record_new: malloc failed\n");
    return NULL;
  }

  new->score_max = -1;
  new->score_max_when = -1;
  new->number_clear = -1;
  new->number_clear_when = -1;
  new->score_min_cleared = -1;
  new->score_min_cleared_when = -1;
  new->number_play = -1;
  new->number_play_when = -1;

  if (valid)
  {
    timestamp = (int) (time(NULL));
    if (timestamp < 0)
    {
      fprintf(stderr, "plan_record_new: timestamp is negative (%d)\n",
              timestamp);
      timestamp = -1;
    }

    new->score_max = 0;
    new->score_max_when = timestamp;
    new->number_clear = 0;
    new->number_clear_when = timestamp;
    new->score_min_cleared = 12345678;
    new->score_min_cleared_when = timestamp;
    new->number_play = 0;
    new->number_play_when = timestamp;
  }

  return new;
}

void
plan_record_delete(plan_record *p)
{
  if (p != NULL)
    free(p);
}

total_record *
total_record_new(int stage_n, int valid)
{
  int i;
  int timestamp;
  total_record *new = NULL;

  /* sanity check */
  if (stage_n <= 0)
  {
    fprintf(stderr, "total_record_new: stage_n is non-positive (%d)\n",
            stage_n);
    return NULL;
  }

  new = (total_record *) malloc(sizeof(total_record));
  if (new == NULL)
  {
    fprintf(stderr, "total_record_new: malloc failed\n");
    return NULL;
  }

  new->total_score = -1;
  new->total_score_when = -1;
  new->stage_n = -1;
  new->stage_id = NULL;
  new->stage_score = NULL;
  new->stage_cleared = NULL;
  new->number_play = -1;
  new->number_play_when = -1;

  new->stage_id = (int *) malloc(sizeof(int) * stage_n);
  if (new->stage_id == NULL)
  {
    fprintf(stderr, "total_record_new: malloc(stage_id) failed\n");
    total_record_delete(new);
    return NULL;
  }
  new->stage_score = (int *) malloc(sizeof(int) * stage_n);
  if (new->stage_score == NULL)
  {
    fprintf(stderr, "total_record_new: malloc(stage_score) failed\n");
    total_record_delete(new);
    return NULL;
  }
  new->stage_cleared = (int *) malloc(sizeof(int) * stage_n);
  if (new->stage_cleared == NULL)
  {
    fprintf(stderr, "total_record_new: malloc(stage_cleared) failed\n");
    total_record_delete(new);
    return NULL;
  }

  new->stage_n = stage_n;

  for (i = 0; i < stage_n; i++)
  {
    new->stage_id[i] = -1;
    new->stage_score[i] = -1;
    new->stage_cleared[i] = 0;
  }

  if (valid)
  {
    timestamp = (int) (time(NULL));
    if (timestamp < 0)
    {
      fprintf(stderr, "total_record_new: timestamp is negative (%d)\n",
              timestamp);
      timestamp = -1;
    }

    new->total_score = 0;
    new->total_score_when = timestamp;
    new->number_play = 0;
    new->number_play_when = timestamp;

    for (i = 0; i < stage_n; i++)
    {
      new->stage_id[i] = -1;
      new->stage_score[i] = 0;
      new->stage_cleared[i] = 0;
    }
  }

  return new;
}

void
total_record_delete(total_record *p)
{
  if (p != NULL)
  {
    if (p->stage_id != NULL)
      free(p->stage_id);
    if (p->stage_score != NULL)
      free(p->stage_score);
    if (p->stage_cleared != NULL)
      free(p->stage_cleared);

    free(p);
  }
}

game_record *
game_record_new(int stage_n, int plan_n, int valid)
{
  int i;
  game_record *new = NULL;

  /* sanity check */
  if (stage_n <= 0)
  {
    fprintf(stderr, "game_record_new: stage_n is non-positive (%d)\n",
            stage_n);
    return NULL;
  }
  if (plan_n <= 0)
  {
    fprintf(stderr, "game_record_new: plan_n is non-positive (%d)\n",
            plan_n);
    return NULL;
  }

  new = (game_record *) malloc(sizeof(game_record));
  if (new == NULL)
  {
    fprintf(stderr, "game_record_new: malloc failed\n");
    return NULL;
  }

  new->total_p = NULL;
  new->plan_p = NULL;
  new->plan_n = -1;

  new->total_p = total_record_new(stage_n, valid);
  if (new->total_p == NULL)
  {
    fprintf(stderr, "game_record_new: total_record_new failed\n");
    game_record_delete(new);
    return NULL;
  }
  new->plan_p = (plan_record **) malloc(sizeof(plan_record *) * (plan_n + 1));
  if (new->plan_p == NULL)
  {
    fprintf(stderr, "game_record_new: malloc(plan_p) failed\n");
    game_record_delete(new);
    return NULL;
  }
  for (i = 0; i <= plan_n; i++)
    new->plan_p[i] = NULL;
  for (i = 0; i <= plan_n; i++)
  {
    new->plan_p[i] = plan_record_new(valid);
    if (new->plan_p[i] == NULL)
    {
      fprintf(stderr, "game_record_new: plan_record_new(%d) failed\n", i);
      game_record_delete(new);
      return NULL;
    }
  }

  new->plan_n = plan_n;
  return new;
}

void
game_record_delete(game_record *p)
{
  int i;

  if (p != NULL)
  {
    if (p->total_p != NULL)
      total_record_delete(p->total_p);
    if (p->plan_p != NULL)
    {
      for (i = p->plan_n; i >= 0; i--)
      {
        if (p->plan_p[i] != NULL)
          plan_record_delete(p->plan_p[i]);
      }
      free(p->plan_p);
    }

    free(p);
  }
}

/* return 1 (true) or 0 (false) */
int
plan_record_valid(plan_record *p, int quiet)
{
  if (p == NULL)
  {
    if (!quiet)
      fprintf(stderr, "plan_record_valid: p is NULL\n");
    return 0;
  }

  if (p->number_play < 0)
  {
    if (!quiet)
      fprintf(stderr, "plan_record_valid: p->number_play is negative (%d)\n",
              p->number_play);
    return 0;
  }
  if (p->number_play > 0)
  {
    if (p->score_max < 0)
    {
      if (!quiet)
        fprintf(stderr, "plan_record_valid: p->score_max is negative (%d)\n",
                p->score_max);
      return 0;
    }
    if (p->score_max_when < 0)
    {
      if (!quiet)
        fprintf(stderr, "plan_record_valid: p->score_max_when is negative "
                "(%d)\n",
                p->score_max_when);
      return 0;
    }
    if (p->number_play_when < 0)
    {
      if (!quiet)
        fprintf(stderr, "plan_record_valid: p->number_play_when is negative "
                "(%d)\n",
                p->number_play_when);
      return 0;
    }
  }

  if (p->number_clear < 0)
  {
    if (!quiet)
      fprintf(stderr, "plan_record_valid: p->number_clear is negative (%d)\n",
              p->number_clear);
    return 0;
  }
  if (p->number_clear > p->number_play)
  {
    if (!quiet)
      fprintf(stderr, "plan_record_valid: p->number_clear > p->number_play "
              "(%d > %d)\n",
              p->number_clear, p->number_play);
    return 0;
  }
  if (p->number_clear > 0)
  {
    if (p->number_clear_when < 0)
    {
      if (!quiet)
        fprintf(stderr, "plan_record_valid: p->number_clear_when is negative "
                "(%d)\n",
                p->number_clear_when);
      return 0;
    }
    if (p->score_min_cleared < 0)
    {
      if (!quiet)
        fprintf(stderr, "plan_record_valid: p->score_min_cleared is negative "
                "(%d)\n",
                p->score_min_cleared);
      return 0;
    }
    if (p->score_min_cleared_when < 0)
    {
      if (!quiet)
        fprintf(stderr, "plan_record_valid: p->score_min_cleared_when "
                "is negative (%d)\n",
                p->score_min_cleared_when);
      return 0;
    }
    if (p->score_min_cleared > p->score_max)
    {
      if (!quiet)
        fprintf(stderr, "plan_record_valid: p->score_min_cleared > "
                "p->score_max "
                "(%d > %d)\n",
                p->score_min_cleared, p->score_max);
      return 0;
    }
  }

  return 1;
}

/* return 1 (true) or 0 (false) */
int
total_record_valid(total_record *p, int quiet)
{
  int i;
  int j;
  int total;

  if (p == NULL)
  {
    if (!quiet)
      fprintf(stderr, "total_record_valid: p is NULL\n");
    return 0;
  }
  if (p->stage_n <= 0)
  {
    if (!quiet)
      fprintf(stderr, "total_record_valid: p->stage_n is non-positive (%d)\n",
              p->stage_n);
    return 0;
  }
  if (p->stage_id == NULL)
  {
    if (!quiet)
      fprintf(stderr, "total_record_valid: p->stage_id is NULL\n");
    return 0;
  }
  if (p->stage_score == NULL)
  {
    if (!quiet)
      fprintf(stderr, "total_record_valid: p->stage_score is NULL\n");
    return 0;
  }
  if (p->stage_cleared == NULL)
  {
    if (!quiet)
      fprintf(stderr, "total_record_valid: p->stage_cleared is NULL\n");
    return 0;
  }

  if (p->number_play < 0)
  {
    if (!quiet)
      fprintf(stderr, "total_record_valid: p->number_play is negative (%d)\n",
              p->number_play);
    return 0;
  }
  if (p->number_play == 0)
    return 1;

  if (p->total_score < 0)
  {
    if (!quiet)
      fprintf(stderr, "total_record_valid: p->total_score is negative (%d)\n",
              p->total_score);
    return 0;
  }
  if (p->total_score_when < 0)
  {
    if (!quiet)
      fprintf(stderr, "total_record_valid: p->total_score_when is negative "
              "(%d)\n", p->total_score_when);
    return 0;
  }

  total = 0;
  for (i = 0; i < p->stage_n; i++)
  {
    if (p->stage_id[i] < 0)
    {
      break;
    }
    
    if (p->stage_score[i] < 0)
    {
      if (!quiet)
        fprintf(stderr, "total_record_valid: stage %d score is negative "
                "(%d)\n", i + 1, p->stage_score[i]);
      return 0;
    }
    
    total += p->stage_score[i];
  }
  for (j = p->stage_n - 1; j >= i; j--)
  {
    if (p->stage_id[j] >= 0)
    {
      if (!quiet)
        fprintf(stderr, "total_record_valid: stage %d record is missing "
                "while stage %d is recorded\n",
                i + 1, j + 1);
      return 0;
    }
  }
  if (p->total_score != total)
  {
    if (!quiet)
      fprintf(stderr, "total_record_valid: p->total_score (%d) does not match "
              "total (%d)\n",
              p->total_score, total);
    return 0;
  }

  return 1;
}

/* return 1 (true) or 0 (false) */
int
game_record_valid(game_record *p, int quiet)
{
  int i;

  if (p == NULL)
  {
    if (!quiet)
      fprintf(stderr, "game_record_valid: p is NULL\n");
    return 0;
  }
  if (p->total_p == NULL)
  {
    if (!quiet)
      fprintf(stderr, "game_record_valid: p->total_p is NULL\n");
    return 0;
  }
  if (p->plan_p == NULL)
  {
    if (!quiet)
      fprintf(stderr, "game_record_valid: p->play_p is NULL\n");
    return 0;
  }
  if (p->plan_n <= 0)
  {
    if (!quiet)
      fprintf(stderr, "game_record_valid: p->play_n is non-positive (%d)\n",
              p->plan_n);
    return 0;
  }

  if (!total_record_valid(p->total_p, quiet))
  {
    if (!quiet)
      fprintf(stderr, "game_record_valid: p->total_p is invalid\n");
    return 0;
  }
  for (i = 0; i <= p->plan_n; i++)
  {
    if (!plan_record_valid(p->plan_p[i], quiet))
    {
      if (!quiet)
        fprintf(stderr, "game_record_valid: plan %d record is invalid\n",
                i);
      return 0;
    }
  }

  return 1;
}

void
print_plan_record(FILE *stream, plan_record *p)
{
  /* sanity check */
  if (p == NULL)
    return;
  if (!plan_record_valid(p, 1))
    return;

  fprintf(stream, "score-max %d %d;\n",
          p->score_max, p->score_max_when);
  fprintf(stream, "number-clear %d %d;\n",
          p->number_clear, p->number_clear_when);
  fprintf(stream, "score-min-cleared %d %d;\n",
          p->score_min_cleared, p->score_min_cleared_when);
  fprintf(stream, "number-play %d %d;\n",
          p->number_play, p->number_play_when);
}

void
print_total_record(FILE *stream, total_record *p)
{
  int i;

  /* sanity check */
  if (p == NULL)
    return;
  if (!total_record_valid(p, 1))
    return;

  fprintf(stream, "total-score %d %d;\n",
          p->total_score, p->total_score_when);
  for (i = 0; i < p->stage_n; i++)
  {
    fprintf(stream, "stage-id %d %d;\n",
            i + 1, p->stage_id[i]);
    fprintf(stream, "stage-score %d %d;\n",
            i + 1, p->stage_score[i]);
    fprintf(stream, "stage-cleared %d %d;\n",
            i + 1, p->stage_cleared[i]);
  }
  fprintf(stream, "number-play %d %d;\n",
          p->number_play, p->number_play_when);
}

void
print_game_record(FILE *stream, game_record *p, stage_list *list)
{
  int i;
  int j;

  /* sanity check */
  if (p == NULL)
    return;
  if (!game_record_valid(p, 1))
    return;

  fprintf(stream, "/* dangen play record */\n");
  fprintf(stream, "\n");
  fprintf(stream, "total;\n");
  print_total_record(stream, p->total_p);
  fprintf(stream, "end;\n");

  for (i = 0; i <= p->plan_n; i++)
  {
    if (plan_record_valid(p->plan_p[i], 1))
    {
      fprintf(stream, "\n");
      if (list != NULL)
      {
        for (j = 0; j < list->n; j++)
        {
          if ((list->p[j])->stage_id == i)
          {
            fprintf(stream, "/* [%s] %s */\n",
                    stage_difficulty_string((list->p[j])->difficulty),
                    (list->p[j])->name);
            break;
          }
        }
      }
      fprintf(stream, "plan %d;\n", i);
      print_plan_record(stream, p->plan_p[i]);
      fprintf(stream, "end;\n");
    }
  }
}

/* return 0 on success, 1 on error */
int
game_record_update(game_record *p)
{
  int i;
  int n;
  int timestamp;
  plan_record *q = NULL;

  /* sanity check */
  if (p == NULL)
  {
    fprintf(stderr, "game_record_update: p is NULL\n");
    return 1;
  }
  if (!game_record_valid(p, 0))
  {
    fprintf(stderr, "game_record_update: p is invalid\n");
    return 1;
  }

  timestamp = (int) (time(NULL));
  if (timestamp < 0)
  {
    fprintf(stderr, "game_record_update: timestamp is negative (%d)\n",
            timestamp);
    return 1;
  }

  /* if this is a real game, not the tutorial */
  if (get_stage_id(1) > 0)
  {  
    if (p->total_p->total_score < get_score())
    {
      p->total_p->total_score = get_score();
      p->total_p->total_score_when = timestamp;
      p->total_p->number_play_when = timestamp;
      for (i = 0; i < p->total_p->stage_n; i++)
      {
        p->total_p->stage_id[i] = -1;
        p->total_p->stage_score[i] = -1;
        p->total_p->stage_cleared[i] = 0;
      }
      for (i = 1; i <= get_stage_number(); i++)
      {
        if (get_stage_id(i) < 0)
          break;
        if (i > p->total_p->stage_n)
        {
          fprintf(stderr, "game_record_update: p->total_p->stage_n is "
                  "too small (%d)\n", p->total_p->stage_n);
          return 1;
        }

        p->total_p->stage_id[i - 1] = get_stage_id(i);
        p->total_p->stage_score[i - 1] = get_stage_score(i);
        p->total_p->stage_cleared[i - 1] = get_stage_cleared(i);
      }
    }
  }

  for (i = 1; (i <= 5) && (i <= get_stage_number()); i++)
  {
    n = get_stage_id(i);
    if (n < 0)
      break;
    if (n > p->plan_n)
    {
      fprintf(stderr, "game_record_update: p->plan_n is "
              "too small (%d)\n", p->plan_n);
      return 1;
    }
    q = p->plan_p[n];

    if (q->score_max < get_stage_score(i))
    {
      q->score_max = get_stage_score(i);
      q->score_max_when = timestamp;
      q->number_play_when = timestamp;
      p->total_p->number_play_when = timestamp;
    }
    if ((get_stage_cleared(i))
        && (q->score_min_cleared > get_stage_score(i)))
    {
      q->score_min_cleared = get_stage_score(i);
      q->score_min_cleared_when = timestamp;
      q->number_play_when = timestamp;
      p->total_p->number_play_when = timestamp;
    }
  }

  return 0;
}

/* return 0 on success, 1 on error */
int
increment_play_total(game_record *p)
{
  int timestamp;

  /* sanity check */
  if (p == NULL)
  {
    fprintf(stderr, "increment_play_total: p is NULL\n");
    return 1;
  }
  if (!game_record_valid(p, 0))
  {
    fprintf(stderr, "increment_play_total: p is invalid\n");
    return 1;
  }

  timestamp = (int) (time(NULL));
  if (timestamp < 0)
  {
    fprintf(stderr, "increment_play_total: timestamp is negative (%d)\n",
            timestamp);
    return 1;
  }

  (p->total_p->number_play)++;
  if (p->total_p->number_play >= 10000000)
    p->total_p->number_play = 10000000;
  p->total_p->number_play_when = timestamp;

  return 0;
}

/* return 0 on success, 1 on error */
int
increment_play_plan(game_record *p, int n)
{
  int timestamp;
  plan_record *q = NULL;

  /* sanity check */
  if (p == NULL)
  {
    fprintf(stderr, "increment_play_plan: p is NULL\n");
    return 1;
  }
  if (!game_record_valid(p, 0))
  {
    fprintf(stderr, "increment_play_plan: p is invalid\n");
    return 1;
  }
  if (n < 0)
  {
    fprintf(stderr, "increment_play_plan: n is negative (%d)\n", n);
    return 1;
  }
  if (n > p->plan_n)
  {
    fprintf(stderr, "increment_play_plan: p->plan_n is too small (%d)\n",
            p->plan_n);
    return 1;
  }

  timestamp = (int) (time(NULL));
  if (timestamp < 0)
  {
    fprintf(stderr, "increment_play_plan: timestamp is negative (%d)\n",
            timestamp);
    return 1;
  }

  q = p->plan_p[n];
  (q->number_play)++;
  if (q->number_play >= 10000000)
    q->number_play = 10000000;
  q->number_play_when = timestamp;
  if (n > 0)
    p->total_p->number_play_when = timestamp;

  return 0;
}

/* return 0 on success, 1 on error */
int
increment_clear_plan(game_record *p, int n)
{
  int timestamp;
  plan_record *q = NULL;

  /* sanity check */
  if (p == NULL)
  {
    fprintf(stderr, "increment_clear_plan: p is NULL\n");
    return 1;
  }
  if (!game_record_valid(p, 0))
  {
    fprintf(stderr, "increment_clear_plan: p is invalid\n");
    return 1;
  }
  if (n < 0)
  {
    fprintf(stderr, "increment_clear_plan: n is negative (%d)\n", n);
    return 1;
  }
  if (n > p->plan_n)
  {
    fprintf(stderr, "increment_clear_plan: p->plan_n is too small (%d)\n",
            p->plan_n);
    return 1;
  }

  timestamp = (int) (time(NULL));
  if (timestamp < 0)
  {
    fprintf(stderr, "increment_clear_plan: timestamp is negative (%d)\n",
            timestamp);
    return 1;
  }

  q = p->plan_p[n];
  (q->number_clear)++;
  if (q->number_clear >= 10000000)
    q->number_clear = 10000000;
  q->number_clear_when = timestamp;
  q->number_play_when = timestamp;
  if (n > 0)
    p->total_p->number_play_when = timestamp;

  return 0;
}
