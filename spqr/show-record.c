/* $Id: show-record.c,v 1.69 2005/07/13 16:02:41 oohara Exp $ */

/* VERSION */
#include <config.h>

#include <stdio.h>
/* strlen */
#include <string.h>
/* malloc */
#include <stdlib.h>
/* localtime */
#include <time.h>

#include "tenm_graphic.h"
#include "util.h"
#include "tenm_input.h"
#include "tenm_timer.h"
#include "esc-ok.h"
#include "background.h"
#include "pause.h"
#include "const.h"
#include "record_data.h"
#include "record_io.h"
#include "option.h"

#include "show-record.h"

#define MENU_SIZE 5

static int show_result_rule(stage_plan *p, void *data);
static int draw_timestamp(int x, int y, int timestamp);

/* return 1 if the program should quit, 0 if not */
int
show_record(void)
{
  int i;
  char temp[128];
  stage_list *list = NULL;
  stage_list *list_full = NULL;
  game_record *record = NULL;
  int head = 0;
  int cursor = 0;
  int delay = 6;
  int delay_next = 6;
  int last_move = 0;
  plan_record *p_rec = NULL;
  int menu_size;
  int n1;
  int n2;
  int show_list = 0;
  int max_id;
  const option *op = NULL;
  int space_ok = 0;
  int n;

  op = get_option();
  if (op == NULL)
  {
    fprintf(stderr, "show_record: get_option failed\n");
    return 1;
  }
  
  record = game_record_load();
  if (record == NULL)
  {
    fprintf(stderr, "show_record: game_record_load failed\n");
    return 1;
  }
  if (!game_record_valid(record, 0))
  {
    fprintf(stderr, "show_record: record is invalid\n");
    game_record_delete(record);
    return 1;
  }

  if (op->free_select != 0)
    list = stage_list_new((int (*)(stage_plan *, void *))
                          NULL,
                          NULL);
  else
    list = stage_list_new((int (*)(stage_plan *, void *))
                          (&show_result_rule),
                          record);
  if (list == NULL)
  {
    fprintf(stderr, "show_record: stage_list_new (list) failed\n");
    game_record_delete(record);
    return 1;
  }
  if (list->n > 0)
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
    game_record_delete(record);
    return 1;
  }

  list_full = stage_list_new((int (*)(stage_plan *, void *))
                             NULL,
                             NULL);
  if (list_full == NULL)
  {
    fprintf(stderr, "show_record: stage_list_new (list_full) failed\n");
    stage_list_delete(list);
    game_record_delete(record);
    return 1;
  }

  max_id = -1;
  for (i = 0; i < record->total_p->stage_n; i++)
  {
    if (max_id < record->total_p->stage_id[i])
      max_id = record->total_p->stage_id[i];
  }
  if (max_id > list_full->n)
  {
    fprintf(stderr, "show_record: max_id > list_full->n (%d > %d)\n",
            max_id, list_full->n);
    stage_list_delete(list_full);
    stage_list_delete(list);
    game_record_delete(record);
    return 1;
  }

  menu_size = MENU_SIZE;
  if (menu_size > list->n + 1)
    menu_size = list->n + 1;
  if (menu_size <= 0)
    menu_size = 1;

  /* no need to detect focus loss */
  tenm_set_focus_handler((void (*)(int)) NULL);
  clear_pause();

  tenm_timer_reset();

  set_background(0);

  while (1 == 1)
  {
    /* quit the program if a SDL_QUIT event happened
     * (for example, if a SIGINT signal (sent by Ctrl+c) is received)
     */
    if (tenm_event_handle() != 0)
    {
      stage_list_delete(list_full);
      stage_list_delete(list);
      game_record_delete(record);
      return 1;
    }

    /* back to the title if ESC is pressed */
    if (tenm_get_key_status() & 32)
    {
      if (get_esc_ok())
      {
        set_esc_ok(0);
        break;
      }
    }
    else
    {
      set_esc_ok(1);
    }

    /* space */
    if (tenm_get_key_status() & 16)
    {
      if (space_ok)
      {
        if (show_list)
          show_list = 0;
        else
          show_list = 1;
        space_ok = 0;
      }
    }
    else
    {
      space_ok = 1;
    }

    switch (tenm_get_key_status() & 15)
    {
    case 1:
      /* up */
      if (delay <= 0)
      {
        if (head + cursor <= 0)
        {
          head = list->n - menu_size + 1;
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
        if (head + cursor >= list->n)
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
        for (i = 1; i <= list->n; i++)
        {
          n1 = head + cursor + i;
          n2 = head + cursor;
          if (n1 > list->n)
          {
            head = 0;
            cursor = 0;
            break;
          }
          if (n2 <= 0)
          {
            head = 1;
            cursor = 0;
            if (head + menu_size - 1 > list->n)
            {
              cursor = head + menu_size - list->n - 1;
              head = list->n + 1 - menu_size;
            }
            break;
          }
          while (n1 >= list->n + 1)
            n1 -= list->n;
          while (n1 < 1)
            n1 += list->n;
          while (n2 >= list->n + 1)
            n2 -= list->n;
          while (n2 < 1)
            n2 += list->n;
          if ((list->p[n1 - 1])->difficulty
              != (list->p[n2 - 1])->difficulty)
          {
            head = n1;
            cursor = 0;
            if (head + menu_size - 1> list->n)
            {
              cursor = head + menu_size - list->n - 1;
              head = list->n + 1 - menu_size;
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
        for (i = 1; i <= list->n; i++)
        {
          if (head + cursor <= 0)
          {
            head = list->n - menu_size + 1;
            cursor = menu_size - 1;
            break;
          }
          n1 = head + cursor - i - 1;
          n2 = head + cursor - i;
          if (n2 <= 0)
          {
            head = 0;
            cursor = 0;
            break;
          }
          if (n1 <= 0)
          {
            head = 1;
            cursor = 0;
            if (head + menu_size - 1 > list->n)
            {
              cursor = head + menu_size - list->n - 1;
              head = list->n + 1 - menu_size;
            }
            break;
          }
          while (n1 >= list->n + 1)
            n1 -= list->n;
          while (n1 < 1)
            n1 += list->n;
          while (n2 >= list->n + 1)
            n2 -= list->n;
          while (n2 < 1)
            n2 += list->n;
          if ((list->p[n1 - 1])->difficulty
              != (list->p[n2 - 1])->difficulty)
          {
            head = n2;
            cursor = 0;
            if (head + menu_size - 1 > list->n)
            {
              cursor = head + menu_size - list->n - 1;
              head = list->n + 1 - menu_size;
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

    if ((head < 0) || (head + menu_size - 1 > list->n))
    {
      fprintf(stderr, "show_result: strange head (%d)\n", head);
      head = 0;
    }
    if ((cursor < 0) || (cursor >= menu_size))
    {
      fprintf(stderr, "show_result: strange cursor (%d)\n", cursor);
      cursor = 0;
    }

    clear_window_with_background();

    if (draw_string(100, 40, "dangen play record", 18) != 0)
      fprintf(stderr, "show_record: draw_string (title) failed\n");

    sprintf(temp, "version %.20s", VERSION);
    if (draw_string(100, 60, temp, (int) strlen(temp)) != 0)
      fprintf(stderr, "show_record: draw_string (version) failed\n");

    if (draw_string(WINDOW_WIDTH / 2 - 256, 420,
                    "use cursor keys to choose, "
                    "press space to toggle the list", 57) != 0)
      fprintf(stderr, "show_record: draw_string "
              "(cursor/space instruction) failed\n");
    if (draw_string(WINDOW_WIDTH / 2 - 76, 440, "press ESC to quit", 17) != 0)
      fprintf(stderr, "show_record: draw_string (ESC instruction) failed\n");

    if ((head + cursor - 1 >= 0) && (head + cursor - 1 <= list->n - 1))
      p_rec = record->plan_p[(list->p[head + cursor - 1])->stage_id];
    else
      p_rec = NULL;

    if (show_list)
    {
      /* draw the list */
      for (i = 0; i < menu_size; i++)
      {
        if (head + i > list->n)
          break;

        if (head + i <= 0)
          sprintf(temp, "total");
        else
          sprintf(temp, "[%.10s] %.50s",
                  stage_difficulty_string((list->p[head + i - 1])->difficulty),
                  (list->p[head + i - 1])->name);
        if (draw_string(80, 120 + 20 * i, temp, (int) strlen(temp)) != 0)
          fprintf(stderr, "show_record: draw_string (list %d) failed\n", i);

        if (i == cursor)
        {
          if (draw_string(60, 120 + 20 * i, ">", 1) != 0)
            fprintf(stderr, "show_record: draw_string (cursor) failed\n");
        }
      }

      /* draw the play record */
      if (draw_string(480, 140, "record", 6) != 0)
        fprintf(stderr, "show_record: draw_string (record title) failed\n");

      if (draw_string(480, 160, "max", 3) != 0)
        fprintf(stderr, "show_record: draw_string (record score max title) "
                "failed\n");

      if (p_rec != NULL)
      {
        if (p_rec->number_play > 0)
          sprintf(temp, "%8d", p_rec->score_max);
        else
          sprintf(temp, "     ---");
      }
      else
      {
        if (record->total_p->number_play > 0)
          sprintf(temp, "%8d", record->total_p->total_score);
        else
          sprintf(temp, "     ---");
      }

      if (draw_string(510, 180, temp, (int) strlen(temp)) != 0)
        fprintf(stderr, "show_record: draw_string (record score max value) "
                "failed\n");

      if (p_rec != NULL)
      {  
        if (draw_string(480, 200, "min cleared", 11) != 0)
          fprintf(stderr, "show_record: draw_string (record score min cleared "
                  "title) "
                  "failed\n");

        if (p_rec->number_clear > 0)
          sprintf(temp, "%8d", p_rec->score_min_cleared);
        else
          sprintf(temp, "     ---");
        if (draw_string(510, 220, temp, (int) strlen(temp)) != 0)
          fprintf(stderr, "show_record: draw_string (record score min cleared "
                  "value) "
                  "failed\n");
      }

      if (draw_string(480, 240, "play", 11) != 0)
        fprintf(stderr, "show_record: draw_string (record play title) "
                "failed\n");

      if (p_rec != NULL)
      {  
        if (p_rec->number_play >= 0)
          sprintf(temp, "%8d", p_rec->number_play);
        else
          sprintf(temp, "     ---");
      }
      else
      {
        if (record->total_p->number_play >= 0)
          sprintf(temp, "%8d", record->total_p->number_play);
        else
          sprintf(temp, "     ---");
      }

      if (draw_string(510, 260, temp, (int) strlen(temp)) != 0)
        fprintf(stderr, "show_record: draw_string (record play value) "
                "failed\n");


    }
    else
    {
      n = head + cursor;
      if (n <= 0)
        sprintf(temp, "total");
      else
        sprintf(temp, "[%.10s] %.50s",
                stage_difficulty_string((list->p[n - 1])->difficulty),
                (list->p[n - 1])->name);
      if (draw_string(80, 120, temp, (int) strlen(temp)) != 0)
        fprintf(stderr, "show_record: draw_string (plan name) failed\n");

      if (draw_string(80, 150, "max", 3) != 0)
        fprintf(stderr, "show_record: draw_string (record score max title) "
                "failed\n");

      if (p_rec != NULL)
      {
        if (p_rec->number_play > 0)
          sprintf(temp, "%8d", p_rec->score_max);
        else
          sprintf(temp, "     ---");

        if (draw_string(200, 150, temp, (int) strlen(temp)) != 0)
          fprintf(stderr, "show_record: draw_string (record score max value) "
                  "failed\n");

        if ((p_rec->number_play > 0) && (p_rec->score_max_when > 0))
        {
          if (draw_timestamp(300, 150, p_rec->score_max_when) != 0)
            fprintf(stderr, "show_record: draw_timestamp (record score max) "
                    "failed\n");
        }

        if (draw_string(80, 170, "min cleared", 11) != 0)
          fprintf(stderr, "show_record: draw_string (record score min cleared "
                  "title) "
                  "failed\n");

        if (p_rec->number_clear > 0)
          sprintf(temp, "%8d", p_rec->score_min_cleared);
        else
          sprintf(temp, "     ---");
        if (draw_string(200, 170, temp, (int) strlen(temp)) != 0)
          fprintf(stderr, "show_record: draw_string (record score min cleared "
                  "value) "
                  "failed\n");

        if ((p_rec->number_clear > 0)
            && (p_rec->score_min_cleared_when > 0))
        {
          if (draw_timestamp(300, 170, p_rec->score_min_cleared_when) != 0)
            fprintf(stderr, "show_record: draw_timestamp "
                    "(record score mon cleared) "
                    "failed\n");
        }

        if (draw_string(80, 190, "play", 4) != 0)
          fprintf(stderr, "show_record: draw_string (record play "
                  "title) "
                  "failed\n");

        sprintf(temp, "%8d", p_rec->number_play);
        if (draw_string(200, 190, temp, (int) strlen(temp)) != 0)
          fprintf(stderr, "show_record: draw_string (record play "
                  "value) "
                  "failed\n");

        if ((p_rec->number_play > 0)
            && (p_rec->number_play_when > 0))
        {
          if (draw_timestamp(300, 190, p_rec->number_play_when) != 0)
            fprintf(stderr, "show_record: draw_timestamp "
                    "(record play) "
                    "failed\n");
        }

        if (draw_string(80, 210, "clear", 5) != 0)
          fprintf(stderr, "show_record: draw_string (record clear "
                  "title) "
                  "failed\n");

        sprintf(temp, "%8d", p_rec->number_clear);
        if (draw_string(200, 210, temp, (int) strlen(temp)) != 0)
          fprintf(stderr, "show_record: draw_string (record clear "
                  "value) "
                  "failed\n");

        if ((p_rec->number_clear > 0)
            && (p_rec->number_clear_when > 0))
        {
          if (draw_timestamp(300, 210, p_rec->number_clear_when) != 0)
            fprintf(stderr, "show_record: draw_timestamp "
                    "(record clear) "
                    "failed\n");
        }
      }
      else
      {
        if (record->total_p->number_play > 0)
          sprintf(temp, "%8d", record->total_p->total_score);
        else
          sprintf(temp, "     ---");
        
        if (draw_string(200, 150, temp, (int) strlen(temp)) != 0)
          fprintf(stderr, "show_record: draw_string (record score max value) "
                  "failed\n");

        if ((record->total_p->number_play > 0)
            && (record->total_p->total_score_when > 0))
        {
          if (draw_timestamp(300, 150, record->total_p->total_score_when) != 0)
            fprintf(stderr, "show_record: draw_timestamp (record score max) "
                    "failed\n");
        }

        if (record->total_p->number_play > 0)
        {  
          for (i = 0; i < record->total_p->stage_n; i++)
          {
            n = record->total_p->stage_id[i];
            if (n < 0)
              break;
            if ((i >= 5)
                || (list_full->p[n])->stage_id == record->total_p->stage_id[i])
            {
              if (i >= 5)
              {  
                sprintf(temp, "ship bonus");
              }
              else
              {  
                /*
                sprintf(temp, "[%s] %s",
                        stage_difficulty_string((list_full->p[n])->difficulty),
                        (list_full->p[n])->name);
                */
                sprintf(temp, "%s",
                        (list_full->p[n])->name);
              }
              if (draw_string(100, 200 + 20 * i,
                              temp, (int) strlen(temp)) != 0)
                fprintf(stderr, "show_record: draw_string (record stage %d) "
                        "failed\n", i + 1);

              if (record->total_p->stage_cleared[i] != 0)
              {
                if (draw_string(82, 200 + 20 * i, "*", 1) != 0)
                  fprintf(stderr, "show_record: draw_string (record stage %d "
                          "clear mark) "
                          "failed\n", i + 1);
              }

              sprintf(temp, "%8d", record->total_p->stage_score[i]);
              if (draw_string(316, 200 + 20 * i,
                              temp, (int) strlen(temp)) != 0)
                fprintf(stderr, "show_record: draw_string (record stage %d "
                        "score) "
                        "failed\n", i + 1);
            }
          } 
        }

        if (draw_string(80, 350, "play", 4) != 0)
          fprintf(stderr, "show_record: draw_string (record play "
                  "title) "
                  "failed\n");

        if (record->total_p->number_play >= 0)
          sprintf(temp, "%8d", record->total_p->number_play);
        else
          sprintf(temp, "     ---");
        if (draw_string(200, 350, temp, (int) strlen(temp)) != 0)
          fprintf(stderr, "show_record: draw_string (record play "
                  "value) "
                  "failed\n");

        if ((record->total_p->number_play > 0)
            && (record->total_p->number_play_when > 0))
        {
          if (draw_timestamp(300, 350, record->total_p->number_play_when) != 0)
            fprintf(stderr, "show_record: draw_timestamp "
                    "(record play) "
                    "failed\n");
        }
      }
    }

    tenm_redraw_window();

    /* this wait is necessary to save CPU time */
    tenm_wait_next_frame();
  }

  stage_list_delete(list_full);
  stage_list_delete(list);
  game_record_delete(record);

  return 0;
}

/* return 1 (true) or 0 (false) */
static int
show_result_rule(stage_plan *p, void *data)
{
  game_record *record = (game_record *) data;

  /* sanity check */
  if (p == NULL)
    return 0;
  if (record == NULL)
    return 0;

  if (p->stage_id < 0)
    return 0;
  if (p->stage_id > record->plan_n)
    return 0;

  if ((record->plan_p[p->stage_id])->number_play > 0)
    return 1;
  
  return 0;
}

/* return 0 on success, 1 on error */
static int
draw_timestamp(int x, int y, int timestamp)
{
  time_t t_temp;
  struct tm *lt;
  char temp[128];

  /* sanity check */
  if (timestamp < 0)
  {
    fprintf(stderr, "draw_timestamp: timestamp is negative (%d)\n",
            timestamp);
    return 1;
  }

  t_temp = (time_t) timestamp;
  lt = localtime(&t_temp);
  sprintf(temp, "%4d/%2d/%2d %2d:%2d",
          lt->tm_year + 1900,
          lt->tm_mon + 1,
          lt->tm_mday,
          lt->tm_hour,
          lt->tm_min);

  if (draw_string(x, y, temp, (int) strlen(temp)) != 0)
  {  
    fprintf(stderr, "draw_timestamp: draw_string failed\n");
    return 1;
  }

  return 0;
}


