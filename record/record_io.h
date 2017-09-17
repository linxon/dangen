/* $Id: record_io.h,v 1.2 2005/06/30 15:29:15 oohara Exp $ */

#ifndef __DANGEN_RECORD_IO_H__
#define __DANGEN_RECORD_IO_H__

#include "record_data.h"

game_record *game_record_load(void);
int game_record_save(game_record *p);

#endif /*not __DANGEN_RECORD_IO_H__ */
