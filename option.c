/* $Id: option.c,v 1.60 2005/07/01 03:23:10 oohara Exp $ */

/* VERSION */
#include <config.h>

#include <stdio.h>
/* strcmp */
#include <string.h>
/* malloc, atexit */
#include <stdlib.h>

/* COPYRIGHT_STRING */
#include "const.h"

#include "option.h"

static option *option_pointer = NULL;

static void option_quit(void);

/* return 0 on success, 1 on error */
int
set_option(int argc, char *argv[])
{
  int i;

  /* sanity check */
  if (argc <= 0)
  {
    fprintf(stderr, "set_option: argc is non-positive\n");
    return 1;
  }
  if (argv == NULL)
  {
    fprintf(stderr, "set_option: argv is NULL\n");
    return 1;
  }

  if (option_pointer != NULL)
    free(option_pointer);
  option_pointer = (option *) malloc(sizeof(option));
  if (option_pointer == NULL)
  {
    fprintf(stderr, "set_option: malloc failed\n");
    return 1;
  }

  if (atexit(option_quit) != 0)
  {
    fprintf(stderr, "set_option: cannot register option_quit to exit\n");
    option_quit();
    return 1;
  }

  option_pointer->free_select = 0;
  option_pointer->full_screen = 0;
  option_pointer->help = 0;
  option_pointer->slow = 0;
  option_pointer->version = 0;

  for (i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "--free-select") == 0)
    {
      option_pointer->free_select = 1;
    }
    else if (strcmp(argv[i], "--full-screen") == 0)
    {
      option_pointer->full_screen = 1;
    }
    else if (strcmp(argv[i], "--help") == 0)
    {
      option_pointer->help = 1;
      /* ignore the rest */
      break;
    }
    else if (strcmp(argv[i], "--slow") == 0)
    {
      option_pointer->slow = 1;
    }
    else if (strcmp(argv[i], "--version") == 0)
    {
      option_pointer->version = 1;
      /* ignore the rest */
      break;
    }
    else
    {
      fprintf(stderr, "set_option: unknown option (arg %d)\n", i);
      return 1;
    }
  }

  return 0;
}

/* return 1 (true) or 0 (false) */
int
cheating(void)
{
  /* sanity check */
  if (option_pointer == NULL)
  {
    fprintf(stderr, "cheating: option_pointer is NULL\n");
    return 0;
  }

  if (option_pointer->free_select != 0)
    return 1;
  if (option_pointer->slow != 0)
    return 1;

  return 0;
}

void
do_help(void)
{
  printf("Usage: dangen [options]\n"
         "shoot 'em up game where accurate shooting matters\n"
         "Options:\n"
         );
  printf("  --free-select     allow selecting any stage\n"
         "  --full-screen     run the game in the full screen mode\n"
         "  --help            print this message\n"
         );
  printf("  --slow            enable the slow mode (CAPS lock)\n"
         );
  printf("  --version         print version information\n"
         "\n"
         "Report bugs to <oohara@libra.interq.or.jp>.\n"
         );
}

void
do_version(void)
{
  printf("dangen %s\n", VERSION);
  printf("%s\n", COPYRIGHT_STRING);
  printf("This program is free software; you can redistribute it and/or\n"
         "modify it under the terms of either the GNU General Public License\n"
         "version 2 or the Artistic License Version 2.0beta5.\n");
  printf("dangen comes with NO WARRANTY, to the extent permitted by law.\n");
}

const option *
get_option(void)
{
  return (const option *) option_pointer;
}

static void
option_quit(void)
{
  if (option_pointer != NULL)
  {
    free(option_pointer);
    option_pointer = NULL;
  }
}
