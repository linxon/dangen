/* $Id: record_io.c,v 1.26 2011/08/24 16:24:55 oohara Exp $ */

/* NOT_HAVE_POSIX */
#include "config.h"

/* FILE, fopen, rename */
#include <stdio.h>
/* free, getenv */
#include <stdlib.h>
/* strdup, strerror */
#include <string.h>
/* errno */
#include <errno.h>

#ifndef NOT_HAVE_POSIX
/* DIR, opendir, closedir */
#include <dirent.h>
/* mkdir */
#include <sys/stat.h>
#endif /* not NOT_HAVE_POSIX */

#include "record_data.h"
#include "record_lexical.h"
#include "record_parser_public.h"
#include "record_util.h"
/* in $(top_srcdir)/spqr */
#include "stage-list.h"

#include "record_io.h"

#define SAVE_FILE "play-record.txt"
#define SAVE_FILE_BACKUP "play-record.txt.bak"

static FILE *open_game_record_file(int save);
static game_record *game_record_load_fallback(void);
static char *game_record_dir_name(void);

game_record *
game_record_load(void)
{
  game_record *p = NULL;
  FILE *file = NULL;

  file = open_game_record_file(0);
  if (file == NULL)
  {
    /* this is not an error */
    return game_record_load_fallback();
  }

  p = game_record_new(6, NUMBER_PLAN, 0);
  if (p == NULL)
  {
    fprintf(stderr, "game_record_load: game_record_new failed\n");
    fclose(file);
    return game_record_load_fallback();
  }

  record_lineno = 1;
  record_read_from_file(file);
  set_game_record(p);
  if (record_parse() != 0)
  {
    fprintf(stderr, "game_record_load: record_parse failed\n");
    fclose(file);
    game_record_delete(p);
    return game_record_load_fallback();
  }
  p = get_game_record();
  set_game_record(NULL);
  fclose(file);

  if (!game_record_valid(p, 0))
  {
    fprintf(stderr, "game_record_load: invalid game record loaded\n");
    game_record_delete(p);
    return game_record_load_fallback();
  }

  return p;
}

/* return NULL on error */
static FILE *
open_game_record_file(int save)
{
  FILE *file = NULL;
  char *dirname = NULL;
  char *filename = NULL;
  char *filename_backup = NULL;
#ifndef NOT_HAVE_POSIX
  DIR *p_dir = NULL;
#endif /* not NOT_HAVE_POSIX */
  const char *mode = NULL;

  if (save)
    mode = "w";
  else
    mode = "r";

  dirname = game_record_dir_name();
  if (dirname == NULL)
  {
    fprintf(stderr, "open_game_record_file: game_record_dir_name failed\n");
    return NULL;
  }
  filename = concatenate_string(NULL, 2, dirname, SAVE_FILE);
  if (filename == NULL)
  {
    fprintf(stderr, "open_game_record_file: can't set filename\n");
    free(dirname);
    return NULL;
  }
  filename_backup = concatenate_string(NULL, 2, dirname, SAVE_FILE_BACKUP);
  if (filename == NULL)
  {
    fprintf(stderr, "open_game_record_file: can't set filename_backup\n");
    free(filename);
    free(dirname);
    return NULL;
  }

#ifndef NOT_HAVE_POSIX
  /* create the play record directory unless there is already one */
  p_dir = opendir(dirname);
  if (p_dir != NULL)
  {
    errno = 0;
    if (closedir(p_dir) != 0)
    {
      fprintf(stderr, "open_game_record_file: closedir failed\n");
      if (errno != 0)
        fprintf(stderr, " (%s)", strerror(errno));
      fprintf(stderr, "\n");
      fprintf(stderr, "open_game_record_file: opening anyway\n");
      /* open anyway */
    }
  }
  else
  {
    errno = 0;
    if (mkdir(dirname, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0)
    {
      fprintf(stderr, "open_game_record_file: mkdir(%s) failed", dirname);
      if (errno != 0)
        fprintf(stderr, " (%s)", strerror(errno));
      fprintf(stderr, "\n");
      free(filename_backup);
      free(filename);
      free(dirname);
      return NULL;
    }
  }
#endif /* not NOT_HAVE_POSIX */

  /* rotate the play record file if possible */
  if (save)
  {  
    if (rename(filename, filename_backup) == -1)
    {
      /* this is not an error */
      ;
      /* open anyway */
    }
  }

  errno = 0;
  file = fopen(filename, mode);
  if (file == NULL)
  {
    if ((save) || (errno != ENOENT))
    {  
      fprintf(stderr, "open_game_record_file: fopen(%s) failed", filename);
      if (errno != 0)
        fprintf(stderr, " (%s)", strerror(errno));
      fprintf(stderr, "\n");
    }
    free(filename_backup);
    free(filename);
    free(dirname);
    return NULL;
  }

  free(filename_backup);
  free(filename);
  free(dirname);

  return file;
}

static game_record *
game_record_load_fallback(void)
{
  game_record *p = NULL;

  p = game_record_new(6, NUMBER_PLAN, 1);
  if (p == NULL)
  {
    fprintf(stderr, "game_record_load_fallback: game_record_new failed\n");
    return NULL;
  }
  if (!game_record_valid(p, 0))
  {
    fprintf(stderr, "game_record_load_fallback: sanity check failed\n");
    game_record_delete(p);
    return NULL;
  }

  return p;
}

/* return 0 on success, 1 on error */
int
game_record_save(game_record *p)
{
  FILE *file = NULL;
  stage_list *list = NULL;

  /* sanity check */
  if (p == NULL)
  {
    fprintf(stderr, "game_record_save: p is NULL\n");
    return 1;
  }
  if (!game_record_valid(p, 0))
  {
    fprintf(stderr, "game_record_save: p is invalid\n");
    return 1;
  }

  file = open_game_record_file(1);
  if (file == NULL)
  {
    fprintf(stderr, "game_record_save: open_game_record_file failed\n");
    return 1;
  }

  list = stage_list_new(NULL, NULL);
  if (list == NULL)
  {
    fprintf(stderr, "game_record_save: stage_list_new failed\n");
    fprintf(stderr, "game_record_save: saving anyway\n");
    /* save anyway */
  }

  print_game_record(file, p, list);
  if (list != NULL)
    stage_list_delete(list);

  errno = 0;
  if (fclose(file) != 0)
  {
    fprintf(stderr, "game_record_save: fclose failed");
    if (errno != 0)
      fprintf(stderr, " (%s)", strerror(errno));
    fprintf(stderr, "\n");
    return 1;
  }

  return 0;
}

/* return NULL on error */
static char *
game_record_dir_name(void)
{
#ifdef NOT_HAVE_POSIX
  return strdup("save/");
#else /* not NOT_HAVE_POSIX */
  char *s = getenv("HOME");
  if (s == NULL)
    return NULL;
  return concatenate_string(NULL, 2, s, "/.dangen/");
#endif /* not NOT_HAVE_POSIX */

  /* should not reach here */
  return NULL;
}
