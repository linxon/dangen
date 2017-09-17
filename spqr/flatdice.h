/* $Id: flatdice.h,v 1.5 2005/07/06 12:35:11 oohara Exp $ */

#ifndef __FLATDICE_H__
#define __FLATDICE_H__

struct _flatdice
{
  int side;
  int randomness;
  int repeat;
  int *stock;
  int last;
};
typedef struct _flatdice flatdice;

flatdice *flatdice_new(int side, int randomness, int repeat);
void flatdice_delete(flatdice *p);
int flatdice_valid(flatdice *p, int quiet);
int flatdice_next(flatdice *p);
int flatdice_reset(flatdice *p);

#endif /* not __FLATDICE_H__ */
