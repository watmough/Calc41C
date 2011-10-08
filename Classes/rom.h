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

$Header: /home/yl2/eric/hpcalc/nasm/RCS/rom.h,v 1.1 1995/07/30 18:34:58 eric Exp eric $
*/


#define MAX_BANK 2
#define MAX_PAGE 16
#define PAGE_SIZE 4096
#define MAX_ROM 65536

/*
 * set breakpoints at every location so we know if we hit
 * uninitialized ROM
 */
void init_rom (void);

void set_ucode  (address a, int b, romword val);
void set_bpt    (address a, int b, cbool val);
void set_source (address a, int b, char *val);

romword get_ucode (address a);
cbool get_bpt      (address a);
char *get_source  (address a);

void select_bank (address a, int b);


