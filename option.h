/* $Id: option.h,v 1.20 2005/07/01 03:21:16 oohara Exp $ */

#ifndef __DANGEN_OPTION_H__
#define __DANGEN_OPTION_H__

struct _option
{
  /* 1 if set, 0 if unset */
  int free_select;
  /* 1 if set, 0 if unset */
  int full_screen;
  /* 1 if set, 0 if unset */
  int help;
  /* 1 if set, 0 is unset */
  int slow;
  /* 1 if set, 0 if unset */
  int version;
};
typedef struct _option option;

int set_option(int argc, char *argv[]);
int cheating(void);
void do_help(void);
void do_version(void);
const option *get_option(void);

#endif /* not __DANGEN_OPTION_H__ */
