/*
NSIM is a simulator for the processor used in the HP-41 (Nut) and in the HP
Series 10 (Voyager) calculators.

Copyright 1995 Eric L. Smith

NSIM is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License version 2 as published by the Free
Software Foundation.  Note that I am not granting permission to redistribute
or modify NSIM under the terms of any later version of the General Public
License.

This program is distributed in the hope that it will be useful (or at least
amusing), but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with
this program (in the file "COPYING"); if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

$Header: /home/yl2/eric/hpcalc/nasm/RCS/lcd.h,v 1.1 1995/07/30 19:00:31 eric Exp eric $
*/

#define LCD_DISPLAY 0xfd
#define HALFNUT     0x10

extern cbool display_enable;

void init_display (void);

char *ann_to_buf (char *buf);
char *display_to_buf (char *buf);

void show_display (void);
