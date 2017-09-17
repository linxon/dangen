/* $Id: record_parser.y,v 1.38 2011/08/24 16:21:52 oohara Exp $ */
/* process this file with bison -d */

/* C declarations */
%{
#include <stdio.h>

#include "record_data.h"
#include "record_lexical.h"

#include "record_parser_private.h"
#include "record_parser_public.h"

static game_record *p = NULL;

/* yyerror */
static void record_error(const char *s);
%}

/* bison declatations */
%name-prefix="record_"

%token SPACE
%token SEMICOLON
%token VALUE_INT

%token NUMBER_PLAY
%token END

%token PLAN
%token SCORE_MAX
%token NUMBER_CLEAR
%token SCORE_MIN_CLEARED

%token TOTAL
%token TOTAL_SCORE
%token STAGE_ID
%token STAGE_SCORE
%token STAGE_CLEARED

%token NO_MATCH

%%
/* grammar rules */
play_record: /* empty */
  {
    ($$).n0 = -1;
    ($$).n1 = -1;
  }
  | play_record SPACE
  {
    ($$).n0 = -1;
    ($$).n1 = -1;
  }
  | play_record SEMICOLON
  {
    ($$).n0 = -1;
    ($$).n1 = -1;
  }
  | play_record block
  {
    ($$).n0 = -1;
    ($$).n1 = -1;
  }
;

block: plan_record_block
  {
    ($$).n0 = -1;
    ($$).n1 = -1;
  }
  | total_record_block
  {
    ($$).n0 = -1;
    ($$).n1 = -1;
  }
;

plan_record_block: plan_record_stmt block_end
  {
    if (p == NULL)
    {
      record_error("p is NULL");
      YYABORT;
    }
    if (p->plan_p == NULL)
    {
      record_error("p->plan_p is NULL");
      YYABORT;
    }
    if ((($1).n0 < 0) || (($1).n0 > p->plan_n))
    {
      record_error("invalid plan");
      YYABORT;
    }
    if (!plan_record_valid(p->plan_p[($1).n0], 0))
    {
      record_error("incomplete plan record");
      YYABORT;
    }
    ($$).n0 = -1;
    ($$).n1 = -1;
  }
;

plan_record_stmt: plan_record_begin
  {
    if (p == NULL)
    {
      record_error("p is NULL");
      YYABORT;
    }
    if (p->plan_p == NULL)
    {
      record_error("p->plan_p is NULL");
      YYABORT;
    }
    if ((($1).n0 < 0) || (($1).n0 > p->plan_n))
    {
      record_error("invalid plan");
      YYABORT;
    }

    /* plan id */
    ($$).n0 = ($1).n0;
    ($$).n1 = -1;
  }
  | plan_record_stmt SPACE
  {
    if (p == NULL)
    {
      record_error("p is NULL");
      YYABORT;
    }
    if (p->plan_p == NULL)
    {
      record_error("p->plan_p is NULL");
      YYABORT;
    }
    if ((($1).n0 < 0) || (($1).n0 > p->plan_n))
    {
      record_error("invalid plan");
      YYABORT;
    }

    /* plan id */
    ($$).n0 = ($1).n0;
    ($$).n1 = -1;
  }
  | plan_record_stmt SEMICOLON
  {
    if (p == NULL)
    {
      record_error("p is NULL");
      YYABORT;
    }
    if (p->plan_p == NULL)
    {
      record_error("p->plan_p is NULL");
      YYABORT;
    }
    if ((($1).n0 < 0) || (($1).n0 > p->plan_n))
    {
      record_error("invalid plan");
      YYABORT;
    }

    /* plan id */
    ($$).n0 = ($1).n0;
    ($$).n1 = -1;
  }
  | plan_record_stmt score_max_stmt
  {
    if (p == NULL)
    {
      record_error("p is NULL");
      YYABORT;
    }
    if (p->plan_p == NULL)
    {
      record_error("p->plan_p is NULL");
      YYABORT;
    }
    if ((($1).n0 < 0) || (($1).n0 > p->plan_n))
    {
      record_error("invalid plan");
      YYABORT;
    }

    (p->plan_p[($1).n0])->score_max = ($2).n0;
    (p->plan_p[($1).n0])->score_max_when = ($2).n1;
    /* plan id */
    ($$).n0 = ($1).n0;
    ($$).n1 = -1;
  }
  | plan_record_stmt number_clear_stmt
  {
    if (p == NULL)
    {
      record_error("p is NULL");
      YYABORT;
    }
    if (p->plan_p == NULL)
    {
      record_error("p->plan_p is NULL");
      YYABORT;
    }
    if ((($1).n0 < 0) || (($1).n0 > p->plan_n))
    {
      record_error("invalid plan");
      YYABORT;
    }

    (p->plan_p[($1).n0])->number_clear = ($2).n0;
    (p->plan_p[($1).n0])->number_clear_when = ($2).n1;
    /* plan id */
    ($$).n0 = ($1).n0;
    ($$).n1 = -1;
  }
  | plan_record_stmt score_min_cleared_stmt
  {
    if (p == NULL)
    {
      record_error("p is NULL");
      YYABORT;
    }
    if (p->plan_p == NULL)
    {
      record_error("p->plan_p is NULL");
      YYABORT;
    }
    if ((($1).n0 < 0) || (($1).n0 > p->plan_n))
    {
      record_error("invalid plan");
      YYABORT;
    }

    (p->plan_p[($1).n0])->score_min_cleared = ($2).n0;
    (p->plan_p[($1).n0])->score_min_cleared_when = ($2).n1;
    /* plan id */
    ($$).n0 = ($1).n0;
    ($$).n1 = -1;
  }
  | plan_record_stmt number_play_stmt
  {
    if (p == NULL)
    {
      record_error("p is NULL");
      YYABORT;
    }
    if (p->plan_p == NULL)
    {
      record_error("p->plan_p is NULL");
      YYABORT;
    }
    if ((($1).n0 < 0) || (($1).n0 > p->plan_n))
    {
      record_error("invalid plan");
      YYABORT;
    }

    (p->plan_p[($1).n0])->number_play = ($2).n0;
    (p->plan_p[($1).n0])->number_play_when = ($2).n1;
    /* plan id */
    ($$).n0 = ($1).n0;
    ($$).n1 = -1;
  }
;

plan_record_begin: PLAN spaces VALUE_INT stmt_end
  {
    if (p == NULL)
    {
      record_error("p is NULL");
      YYABORT;
    }
    if (p->plan_p == NULL)
    {
      record_error("p->plan_p is NULL");
      YYABORT;
    }
    if ((($3).n0 < 0) || (($3).n0 > p->plan_n))
    {
      record_error("invalid plan");
      YYABORT;
    }

    /* plan id */
    ($$).n0 = ($3).n0;
    ($$).n1 = -1;
  }
;

score_max_stmt: SCORE_MAX spaces VALUE_INT spaces VALUE_INT stmt_end
  {
    /* value */
    ($$).n0 = ($3).n0;
    /* when */
    ($$).n1 = ($5).n0;
  }
;

number_clear_stmt: NUMBER_CLEAR spaces VALUE_INT spaces VALUE_INT stmt_end
  {
    /* value */
    ($$).n0 = ($3).n0;
    /* last */
    ($$).n1 = ($5).n0;
  }
;

score_min_cleared_stmt: SCORE_MIN_CLEARED spaces VALUE_INT spaces VALUE_INT stmt_end
  {
    /* value */
    ($$).n0 = ($3).n0;
    /* when */
    ($$).n1 = ($5).n0;
  }
;

total_record_block: total_record_stmt block_end
  {
    if (p == NULL)
    {
      record_error("p is NULL");
      YYABORT;
    }
    if (p->total_p == NULL)
    {
      record_error("p->total_p is NULL");
      YYABORT;
    }
    if (!total_record_valid(p->total_p, 0))
    {
      record_error("incomplete total record");
      YYABORT;
    }
    
    ($$).n0 = -1;
    ($$).n1 = -1;
  }
;

total_record_stmt: total_record_begin
  {
    ($$).n0 = -1;
    ($$).n1 = -1;
  }
  | total_record_stmt SPACE
  {
    ($$).n0 = -1;
    ($$).n1 = -1;
  }
  | total_record_stmt SEMICOLON
  {
    ($$).n0 = -1;
    ($$).n1 = -1;
  }
  | total_record_stmt total_score_stmt
  {
    if (p == NULL)
    {
      record_error("p is NULL");
      YYABORT;
    }
    if (p->total_p == NULL)
    {
      record_error("p->total_p is NULL");
      YYABORT;
    }

    p->total_p->total_score = ($2).n0;
    p->total_p->total_score_when = ($2).n1;
    ($$).n0 = -1;
    ($$).n1 = -1;
  }
  | total_record_stmt stage_id_stmt
  {
    if (p == NULL)
    {
      record_error("p is NULL");
      YYABORT;
    }
    if (p->total_p == NULL)
    {
      record_error("p->total_p is NULL");
      YYABORT;
    }
    if (p->total_p->stage_id == NULL)
    {
      record_error("p->total_p->stage_id is NULL");
      YYABORT;
    }
    if ((($2).n0 <= 0) || (($2).n0 > p->total_p->stage_n))
    {
      record_error("invalid stage");
      YYABORT;
    }

    p->total_p->stage_id[($2).n0 - 1] = ($2).n1;
    ($$).n0 = -1;
    ($$).n1 = -1;
  }
  | total_record_stmt stage_score_stmt
  {
    if (p == NULL)
    {
      record_error("p is NULL");
      YYABORT;
    }
    if (p->total_p == NULL)
    {
      record_error("p->total_p is NULL");
      YYABORT;
    }
    if (p->total_p->stage_score == NULL)
    {
      record_error("p->total_p->stage_score is NULL");
      YYABORT;
    }
    if ((($2).n0 <= 0) || (($2).n0 > p->total_p->stage_n))
    {
      record_error("invalid stage");
      YYABORT;
    }

    p->total_p->stage_score[($2).n0 - 1] = ($2).n1;
    ($$).n0 = -1;
    ($$).n1 = -1;
  }
  | total_record_stmt stage_cleared_stmt
  {
    if (p == NULL)
    {
      record_error("p is NULL");
      YYABORT;
    }
    if (p->total_p == NULL)
    {
      record_error("p->total_p is NULL");
      YYABORT;
    }
    if (p->total_p->stage_cleared == NULL)
    {
      record_error("p->total_p->stage_cleared is NULL");
      YYABORT;
    }
    if ((($2).n0 <= 0) || (($2).n0 > p->total_p->stage_n))
    {
      record_error("invalid stage");
      YYABORT;
    }

    p->total_p->stage_cleared[($2).n0 - 1] = ($2).n1;
    ($$).n0 = -1;
    ($$).n1 = -1;
  }
  | total_record_stmt number_play_stmt
  {
    if (p == NULL)
    {
      record_error("p is NULL");
      YYABORT;
    }
    if (p->total_p == NULL)
    {
      record_error("p->total_p is NULL");
      YYABORT;
    }

    p->total_p->number_play = ($2).n0;
    p->total_p->number_play_when = ($2).n1;
    ($$).n0 = -1;
    ($$).n1 = -1;
  }
;

total_record_begin: TOTAL stmt_end
  {
    ($$).n0 = -1;
    ($$).n1 = -1;
  }
;

total_score_stmt: TOTAL_SCORE spaces VALUE_INT spaces VALUE_INT stmt_end
  {
    /* value */
    ($$).n0 = ($3).n0;
    /* when */
    ($$).n1 = ($5).n0;
  }
;

stage_id_stmt: STAGE_ID spaces VALUE_INT spaces VALUE_INT stmt_end
  {
    /* stage number */
    ($$).n0 = ($3).n0;
    /* value */
    ($$).n1 = ($5).n0;
  }
;

stage_score_stmt: STAGE_SCORE spaces VALUE_INT spaces VALUE_INT stmt_end
  {
    /* stage number */
    ($$).n0 = ($3).n0;
    /* value */
    ($$).n1 = ($5).n0;
  }
;

stage_cleared_stmt: STAGE_CLEARED spaces VALUE_INT spaces VALUE_INT stmt_end
  {
    /* stage number */
    ($$).n0 = ($3).n0;
    /* value */
    ($$).n1 = ($5).n0;
  }
;

number_play_stmt: NUMBER_PLAY spaces VALUE_INT spaces VALUE_INT stmt_end
  {
    /* value */
    ($$).n0 = ($3).n0;
    /* last */
    ($$).n1 = ($5).n0;
  }
;

block_end: END stmt_end
  {
    ($$).n0 = -1;
    ($$).n1 = -1;
  }
;

stmt_end: SEMICOLON
  | spaces SEMICOLON
  {
    ($$).n0 = -1;
    ($$).n1 = -1;
  }
;

spaces: SPACE
  | spaces SPACE
  {
    ($$).n0 = -1;
    ($$).n1 = -1;
  }
;

%%
/* additional C code */

/* this is the yyerror for bison */
static void
record_error(const char *s)
{
  fprintf (stderr, "record_parse: %s (line %d)\n",
           s, record_lineno);
}

void
set_game_record(game_record *q)
{
  p = q;
}

game_record *
get_game_record(void)
{
  return p;
}
