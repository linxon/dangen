## Makefile.am for dangen/spqr
## process this file with automake --foreign to generate Makefile.in
# $Id: Makefile.am,v 1.91 2011/08/23 20:55:49 oohara Exp $

SUBDIRS = image

noinst_LIBRARIES = libspqr.a

libspqr_a_SOURCES = afterdeath.c \
  afterdeath.h \
  brilliance.c \
  brilliance.h \
  cat-tail-grep.c \
  cat-tail-grep.h \
  cat-tail.c \
  cat-tail.h \
  empty-wind.c \
  empty-wind.h \
  explosion.c \
  explosion.h \
  flatdice.c \
  flatdice.h \
  fragment.c \
  fragment.h \
  gosanpachi.c \
  gosanpachi.h \
  hatsuda.c \
  hatsuda.h \
  hugin.c \
  hugin.h \
  insane-hand.c \
  insane-hand.h \
  laser.c \
  laser.h \
  last-boss.c \
  last-boss.h \
  mankanshoku.c \
  mankanshoku.h \
  negation-engine.c \
  negation-engine.h \
  net-can-howl.c \
  net-can-howl.h \
  nexus.c \
  nexus.h \
  normal-enemy.c \
  normal-enemy.h \
  normal-shot.c \
  normal-shot.h \
  p-can.c \
  p-can.h \
  perpeki.c \
  perpeki.h \
  plan-0.c \
  plan-0.h \
  plan-1.c \
  plan-1.h \
  plan-2.c \
  plan-2.h \
  plan-3.c \
  plan-3.h \
  plan-4.c \
  plan-4.h \
  plan-5.c \
  plan-5.h \
  plan-6.c \
  plan-6.h \
  plan-7.c \
  plan-7.h \
  plan-8.c \
  plan-8.h \
  plan-9.c \
  plan-9.h \
  plan-10.c \
  plan-10.h \
  plan-11.c \
  plan-11.h \
  plan-12.c \
  plan-12.h \
  plan-13.c \
  plan-13.h \
  plan-14.c \
  plan-14.h \
  plan-15.c \
  plan-15.h \
  plan-16.c \
  plan-16.h \
  plan-17.c \
  plan-17.h \
  plan-18.c \
  plan-18.h \
  plan-19.c \
  plan-19.h \
  plan-20.c \
  plan-20.h \
  player-shot.c \
  player-shot.h \
  player.c \
  player.h \
  respiration.c \
  respiration.h \
  result.c \
  result.h \
  scheduler.c \
  scheduler.h \
  seiron-fake.c \
  seiron-fake.h \
  seiron.c \
  seiron.h \
  show-record.c \
  show-record.h \
  silver-chimera.c \
  silver-chimera.h \
  spellbook.c \
  spellbook.h \
  stage-clear.c \
  stage-clear.h \
  stage-list.c \
  stage-list.h \
  stage-select.c \
  stage-select.h \
  stage-title.c \
  stage-title.h \
  strikers.c \
  strikers.h \
  tadashi.c \
  tadashi.h \
  tenmado.c \
  tenmado.h \
  theorem-weapon.c \
  theorem-weapon.h \
  title.c \
  title.h \
  tutor.c \
  tutor.h \
  w-ko.c \
  w-ko.h \
  wall-0.c \
  wall-0.h \
  wall-4.c \
  wall-4.h \
  wall-8.c \
  wall-8.h \
  wall-11.c \
  wall-11.h \
  wall-13.c \
  wall-13.h \
  warning.c \
  warning.h \
  watcher-below.c \
  watcher-below.h

#CC_WARNINGS = -Wall
# the overkill warnings
# I don't use -Wtraditional because I use string concatenation
# use -Wno-long-long because -pedantic implies -Wlong-long
CC_WARNINGS = -Wall -Wshadow -Wpointer-arith -Wcast-qual \
  -Wcast-align -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes \
  -Wmissing-declarations -Wnested-externs -Winline -pedantic -Wno-long-long

# let's hardcode SDL flags here
SDL_CFLAGS=`sdl-config --cflags`
# sdl-config --libs is added to LIBS in configure

# -g -O2 are in CFLAGS by default, don't use @CFLAGS@ here
AM_CFLAGS = $(SDL_CFLAGS) $(CC_WARNINGS)

AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" \
  -DLOCALSTATEDIR=\"$(localstatedir)\" \
  -I$(top_srcdir) \
  -I$(top_srcdir)/libtenm \
  -I$(top_srcdir)/record
