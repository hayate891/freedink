/**
 * Dialog choices

 * Copyright (C) 2014  Sylvain Beucler

 * This file is part of GNU FreeDink

 * GNU FreeDink is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.

 * GNU FreeDink is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef _TALK_H
#define _TALK_H

#include "SDL.h"

/* Talk choices */
#define TALK_TITLE_BUFSIZ 3000
#define TALK_LINE_BUFSIZ 101
struct talk_struct
{
  char line[21][TALK_LINE_BUFSIZ];  /* dialog choices */
  int line_return[21]; /* return this number if chosen */
  char buffer[TALK_TITLE_BUFSIZ];   /* title */
  int cur;
  int last;
  /*bool*/int active;
  int cur_view;
  int cur_view_end;
  int page;
  int script;
  int offset;
  int newy;
  int color;
  int curf;
  Uint32 timer;
};
extern struct talk_struct talk;
extern void talk_start(int script, int nb_choices);
extern void talk_clear();
extern void talk_process();

#endif
