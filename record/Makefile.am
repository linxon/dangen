## Makefile.am for dangen/record
## process this file with automake --foreign to generate Makefile.in
# $Id: Makefile.am,v 1.12 2011/08/23 20:55:11 oohara Exp $

noinst_LIBRARIES = librecord.a

# automake doesn't know that bison generates .h from .y,
# so we must add .h to SOURCES and BUILT_SOURCES
librecord_a_SOURCES = record_data.c \
  record_data.h \
  record_io.c \
  record_io.h \
  record_lexical.h \
  record_lexical.l \
  record_parser.h \
  record_parser.y \
  record_parser_private.h \
  record_parser_public.h \
  record_util.c \
  record_util.h

BUILT_SOURCES = record_parser.h

AM_YFLAGS = -d
# -olex.yy.c is a workaround for automake braindamage
AM_LFLAGS = -Precord_ -olex.yy.c -Cfe -8

#CC_WARNINGS = -Wall
# the overkill warnings
# I don't use -Wtraditional because I use string concatenation
# use -Wno-long-long because -pedantic implies -Wlong-long
CC_WARNINGS = -Wall -Wshadow -Wpointer-arith -Wcast-qual \
  -Wcast-align -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes \
  -Wmissing-declarations -Wnested-externs -Winline -pedantic -Wno-long-long

# -g -O2 are in CFLAGS by default, don't use @CFLAGS@ here
AM_CFLAGS = $(CC_WARNINGS)

AM_CPPFLAGS = -I$(top_srcdir) \
  -I$(top_srcdir)/spqr
