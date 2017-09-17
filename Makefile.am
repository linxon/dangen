## Makefile.am for dangen
## process this file with automake --foreign to generate Makefile.in
## automake version 1.11 is required
# $Id: Makefile.am,v 1.97 2011/08/26 14:02:35 oohara Exp $

SUBDIRS = libtenm record spqr

bin_PROGRAMS = dangen
dangen_SOURCES = background.c \
  background.h \
  chain.c \
  chain.h \
  const.h \
  esc-ok.c \
  esc-ok.h \
  info.c \
  info.h \
  loop.c \
  loop.h \
  main.c \
  option.c \
  option.h \
  pause.c \
  pause.h \
  score.c \
  score.h \
  ship.c \
  ship.h \
  slow.c \
  slow.h \
  stage.c \
  stage.h \
  util.c \
  util.h

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
  -I$(top_srcdir)/libtenm \
  -I$(top_srcdir)/record \
  -I$(top_srcdir)/spqr

# there is no AM_LDADD
# don't use $(top_srcdir) here
dangen_LDADD =  libtenm/libtenm.a \
  record/librecord.a \
  spqr/libspqr.a

man_MANS = dangen.6

EXTRA_DIST = Artistic-2 \
  GPL-2 \
  LICENSE \
  README-ja-utf8 \
  README-ja-sjis \
  dangen.6

BUILT_SOURCES = README-ja-sjis

README-ja-sjis: README-ja-utf8
	sed "s/\n/\r\n/g" $< | \
  iconv -f UTF-8 -t SJIS -o $@
