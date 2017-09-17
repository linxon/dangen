/* $Id: stage-select.c,v 1.159 2005/07/02 18:27:02 oohara Exp $ */

#include <stdio.h>
/* malloc */
#include <stdlib.h>
/* strdup, strcmp */
#include <string.h>

#include "stage.h"
#include "tenm_input.h"
#include "tenm_graphic.h"
#include "util.h"
#include "tenm_timer.h"
#include "background.h"
#include "const.h"
#include "score.h"
#include "ship.h"
#include "esc-ok.h"
#include "pause.h"
#include "option.h"
#include "info.h"
#include "stage-list.h"
#include "record_data.h"

#include "stage-select.h"

#define MENU_SIZE 5
#define TITLE_MOVE_BEGIN 30
#define TITLE_MOVE_END 45

static int stage_select_rule(stage_plan *p, void *data);

/* return
 * 0 if a stage is selected
 * 1 if the game should be over
 * 2 if the program should quit
 */
int
stage_select(game_record *record)
{
  int i;
  int x;
  int y;
  int head = 0;
  int cursor = 0;
  int delay = 6;
  int delay_next = 6;
  int t = 0;
  int chosen = 0;
  int last_move = 0;
  int t_demo = 0;
  stage_list *list = NULL;
  char temp[64];
  plan_record *p_rec = NULL;
  int menu_size;
  int n1;
  int n2;
  int max_id;

  /* to skip stage select */
  /*
    set_stage_id(1);
    return 0;
  */

  /* sanity check */
  if (TITLE_MOVE_BEGIN >= TITLE_MOVE_END)
  {
    fprintf(stderr, "stage_select: TITLE_MOVE_BEGIN >= TITLE_MOVE_END "
            "(%d, %d)\n", TITLE_MOVE_BEGIN, TITLE_MOVE_END);
    return 2;
  }
  if (record == NULL)
  {
    fprintf(stderr, "stage_select: record is NULL\n");
    return 2;
  }
  if (!game_record_valid(record, 0))
  {
    fprintf(stderr, "stage_select: record is invalid\n");
    return 2;
  }

  list = stage_list_new((int (*)(stage_plan *, void *))
                        (&stage_select_rule),
                        NULL);

  if (list == NULL)
  {
    fprintf(stderr, "stage_select: stage_list_new failed\n");
    return 2;
  }
  if (list->n <= 0)
  {
    if (get_stage_number() > 4)
      return 1;

    fprintf(stderr, "stage_select: list->n is non-positive (%d)\n",
            list->n);
    return 2;
  }
  qsort((void *) list->p, (size_t) (list->n), sizeof(stage_plan *),
        (int (*)(const void *, const void *)) stage_compare);

  max_id = -1;
  for (i = 0; i < list->n; i++)
  {
    if (max_id < (list->p[i])->stage_id)
      max_id = (list->p[i])->stage_id;
  }
  if (max_id > record->plan_n)
  {
    fprintf(stderr, "show_record: max_id > record->plan_n (%d > %d)\n",
            max_id, record->plan_n);
    stage_list_delete(list);
    return 1;
  }

  menu_size = MENU_SIZE;
  if (menu_size > list->n)
    menu_size = list->n;
  if (menu_size <= 0)
    menu_size = 1;

  /* we don't need to pause in the stage select */
  tenm_set_focus_handler((void (*)(int)) NULL);
  clear_pause();

  set_background(0);

  tenm_timer_reset();

  while (1 == 1)
  {
    /* quit the program if a SDL_QUIT event happened
     * (for example, if a SIGINT signal (sent by Ctrl+c) is received)
     */
    if (tenm_event_handle() != 0)
    {
      stage_list_delete(list);
      return 2;
    }
    
    /* quit the game if ESC is pressed */
    if (tenm_get_key_status() & 32)
    {
      if (get_esc_ok())
      {
        set_esc_ok(0);
        stage_list_delete(list);
        return 1;
      }
    }
    else
    {
      set_esc_ok(1);
    }

    /* space */
    if ((t_demo > TITLE_MOVE_END) && (!(chosen))
        && (tenm_get_key_status() & 16))
    {
      chosen = 1;
    }

    if (chosen)
    {
      t++;
      if (t > 50)
        break;
    }
    else if (t_demo > TITLE_MOVE_END)
    {
      switch (tenm_get_key_status() & 15)
      {
      case 1:
        /* up */
        if (delay <= 0)
        {
          if (head + cursor <= 0)
          {
            head = list->n - menu_size;
            cursor = menu_size - 1;
          }
          else if (cursor <= 0)
          {
            head--;
          }
          else
          {
            cursor--;
          }
          if (last_move == 1)
          {  
            delay_next -= 2;
            if (delay_next < 0)
              delay_next = 0;
          }
          delay = delay_next;
          last_move = 1;
        }
        else
        {
          delay--;
        }
          break;
      case 2:
        /* down */
        if (delay <= 0)
        {
          if (head + cursor >= list->n - 1)
          {
            head = 0;
            cursor = 0;
          }
          else if (cursor >= menu_size - 1)
          {
            head++;
          }
          else
          {
            cursor++;
          }
          if (last_move == 2)
          {  
            delay_next -= 2;
            if (delay_next < 0)
              delay_next = 0;
          }
          delay = delay_next;
          last_move = 2;
        }
        else
        {
          delay--;
        }
        break;
      case 4:
        /* right */
        if (delay <= 0)
        {
          for (i = 1; i < list->n; i++)
          {
            n1 = head + cursor + i;
            n2 = head + cursor;
            while (n1 >= list->n)
              n1 -= list->n;
            while (n1 < 0)
              n1 += list->n;
            while (n2 >= list->n)
              n2 -= list->n;
            while (n2 < 0)
              n2 += list->n;
            if ((list->p[n1])->difficulty
                != (list->p[n2])->difficulty)
            {
              head = n1;
              cursor = 0;
              if (head + menu_size - 1 > list->n - 1)
              {
                cursor = head + menu_size - list->n;
                head = list->n - menu_size;
              }
              break;
            }
          }
          delay = 6;
          delay_next = 6;
          last_move = 4;
        }
        else
        {
          delay--;
        }
        break;
      case 8:
        /* left */
        if (delay <= 0)
        {
          for (i = 1; i < list->n; i++)
          {
            n1 = head + cursor - i - 1;
            n2 = head + cursor - i;
            while (n1 >= list->n)
              n1 -= list->n;
            while (n1 < 0)
              n1 += list->n;
            while (n2 >= list->n)
              n2 -= list->n;
            while (n2 < 0)
              n2 += list->n;
            if ((list->p[n1])->difficulty
                != (list->p[n2])->difficulty)
            {
              head = n2;
              cursor = 0;
              if (head + menu_size - 1 > list->n - 1)
              {
                cursor = head + menu_size - list->n;
                head = list->n - menu_size;
              }
              break;
            }
          }
            delay = 6;
            delay_next = 6;
            last_move = 8;
        }
        else
        {
          delay--;
        }
        break;
      default:
        delay = 0;
        delay_next = 6;
        last_move = 0;
        break;
      }
    }

    if ((head < 0) || (head + menu_size - 1 > list->n - 1))
    {
      fprintf(stderr, "stage_select: strange head (%d)\n", head);
      head = 0;
    }
    if ((cursor < 0) || (cursor >= menu_size))
    {
      fprintf(stderr, "stage_select: strange cursor (%d)\n", cursor);
      cursor = 0;
    }
    
    clear_window_with_background();

    /* draw the list */
    if (t_demo > TITLE_MOVE_END)
    {
      for (i = 0; i < menu_size; i++)
      {
        if (head + i >= list->n)
          break;
        sprintf(temp, "[%.10s] %.50s",
                stage_difficulty_string((list->p[head + i])->difficulty),
                (list->p[head + i])->name);
        if (draw_string(80, 120 + 20 * i, temp, (int) strlen(temp)) != 0)
          fprintf(stderr, "stage_select: draw_string (list %d) failed\n", i);

        if (i == cursor)
        {
          /* a bit too noisy */
          /*
            sprintf(temp, "%2d", head + i + 1);
            if (draw_string(30, 120 + 20 * i, temp, (int) strlen(temp)) != 0)
            fprintf(stderr, "stage_select: draw_string (list #) failed\n");
          */
          
          if (t % 10 < 5)
          { 
            if (draw_string(60, 120 + 20 * i, ">", 1) != 0)
              fprintf(stderr, "stage_select: draw_string (cursor) failed\n");
          }
        }
      }
    }
    else if (t_demo < TITLE_MOVE_BEGIN)
    {
      if (draw_string(266, 190, "stage select", 12) != 0)
        fprintf(stderr, "stage_select: draw_string (title) failed\n");
    }

    show_score(NULL);
    show_ship(NULL);

    if (get_stage_number() <= 4)
      sprintf(temp, "stage %d", get_stage_number());
    else
      sprintf(temp, "final stage");
    if (t_demo > TITLE_MOVE_END)
    {
      x = WINDOW_WIDTH - 10 - ((int) strlen(temp)) * 9;
      y = 30;
    }
    else if (t_demo < TITLE_MOVE_BEGIN)
    {
      x = WINDOW_WIDTH / 2 - ((int) strlen(temp)) * 9 / 2;
      y = 210;
    }
    else
    {
      x = (WINDOW_WIDTH - 10 - ((int) strlen(temp)) * 9)
        * (t_demo - TITLE_MOVE_BEGIN);
      y = 30 * (t_demo - TITLE_MOVE_BEGIN);
      x += (WINDOW_WIDTH / 2 - ((int) strlen(temp)) * 9 / 2)
        * (TITLE_MOVE_END - t_demo);
      y += 210 * (TITLE_MOVE_END - t_demo);
      x /= TITLE_MOVE_END - TITLE_MOVE_BEGIN;
      y /= TITLE_MOVE_END - TITLE_MOVE_BEGIN;
    }
    if (draw_string(x, y,
                    temp, (int) strlen(temp)) != 0)
      fprintf(stderr, "stage_select: draw_string (stage number) "
              "failed\n");

    if (t_demo > TITLE_MOVE_END)
    {
      if (draw_string(WINDOW_WIDTH / 2 - 292, 400,
                      "up/down to move the cursor, left/right for "
                      "difficulty-based move", 65) != 0)
        fprintf(stderr, "stage_select: draw_string (instruction line 1) "
                "failed\n");
      if (draw_string(WINDOW_WIDTH / 2 - 166, 420,
                      "press space to decide, or ESC to quit", 37) != 0)
        fprintf(stderr, "stage_select: draw_string (instruction line 2) "
                "failed\n");
    }

    /* draw the play record */
    if (t_demo > TITLE_MOVE_END)
    {
      p_rec = record->plan_p[(list->p[head + cursor])->stage_id];
      if (draw_string(480, 140, "record", 6) != 0)
        fprintf(stderr, "stage_select: draw_string (record title) failed\n");

      if (draw_string(480, 160, "max", 3) != 0)
        fprintf(stderr, "stage_select: draw_string (record score max title) "
                "failed\n");

      if (p_rec->number_play > 0)
        sprintf(temp, "%8d", p_rec->score_max);
      else
        sprintf(temp, "     ---");
      if (draw_string(510, 180, temp, (int) strlen(temp)) != 0)
        fprintf(stderr, "stage_select: draw_string (record score max value) "
                "failed\n");

      if (draw_string(480, 200, "min cleared", 11) != 0)
        fprintf(stderr, "stage_select: draw_string (record score min cleared "
                "title) "
                "failed\n");

      if (p_rec->number_clear > 0)
        sprintf(temp, "%8d", p_rec->score_min_cleared);
      else
        sprintf(temp, "     ---");
      if (draw_string(510, 220, temp, (int) strlen(temp)) != 0)
        fprintf(stderr, "stage_select: draw_string (record score min cleared "
                "value) "
                "failed\n");

      if (draw_string(480, 240, "play", 11) != 0)
        fprintf(stderr, "stage_select: draw_string (record play title) "
                "failed\n");

      if (p_rec->number_play >= 0)
        sprintf(temp, "%8d", p_rec->number_play);
      else
        sprintf(temp, "     ---");
      if (draw_string(510, 260, temp, (int) strlen(temp)) != 0)
        fprintf(stderr, "stage_select: draw_string (record play value) "
                "failed\n");
    }
    
    tenm_redraw_window();

    if (t_demo <= TITLE_MOVE_END)
      t_demo++;
    tenm_wait_next_frame();
  }

  set_stage_id(get_stage_number(), (list->p[head + cursor])->stage_id);
  set_stage_name(get_stage_number(), (list->p[head + cursor])->name);
  set_stage_difficulty(get_stage_number(),
                       (list->p[head + cursor])->difficulty);

  stage_list_delete(list);

  return 0;
}

/* return 1 (true) or 0 (false) */
static int
stage_select_rule(stage_plan *p, void *data)
{
  int i;
  int ok_very_hard = 0;
  int ok_hardest = 0;
  const option *op = NULL;

  /* sanity check */
  if (p == NULL)
    return 0;

  op = get_option();
  if (op == NULL)
    return 0;

  if (p->difficulty == PLAN_TUTORIAL)
    return 0;

  if (op->free_select != 0)
    return 1;

  for (i = 1; i < get_stage_number(); i++)
  {
    if (get_stage_difficulty(i) >= PLAN_NORMAL)
    {
      ok_very_hard = 1;
      break;
    }
  }

  if (get_stage_number() == 5)
  {
    for (i = 1; i < get_stage_number(); i++)
    {
      if (get_stage_difficulty(i) >= PLAN_HARD)
      {
        ok_hardest = 1;
        break;
      }
    }
  }

  if (get_stage_number() == 5)
  {
    if ((p->difficulty == PLAN_HARDEST) && (ok_hardest))
      return 1;

    return 0;
  }

  if ((p->difficulty == PLAN_VERY_HARD) && (!ok_very_hard))
    return 0;
  if ((p->difficulty == PLAN_HARDEST) && (!ok_hardest))
    return 0;

  for (i = 1; i <= 5; i++)
  {
    if (get_stage_id(i) == p->stage_id)
      return 0;
  }

  return 1;
}
