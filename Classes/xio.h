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

$Header: /home/yl2/eric/hpcalc/nasm/RCS/xio.h,v 1.6 1995/08/01 15:08:00 eric Exp eric $
*/

void init_user_io (int argc, char *argv[]);

void update_display (char *, char *);

#define KC_NONE -1
#define KC_EXIT -2

/*
 * returns KC_NONE if no key pressed,
 * returns KC_EXIT for exit,
 * otherwise logical keycode of (10 * row) + col
 * Normal keys have row and column numbers starting at one; on the HP-41
 * there are four special keys (ON, USER, PRGM, ALPHA) in logical row 0.
 * On the HP-1xC, there is a tenth column which is considered to be logical
 * column 0.
 * The enter key is considered to occupy two positions in the logical key
 * numbering.
 */
int check_keyboard (void);
