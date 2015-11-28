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

$Header: /home/yl2/eric/hpcalc/nasm/RCS/nsim.h,v 1.4 1995/08/03 01:53:29 eric Exp eric $
*/

typedef char cbool;               /* 1-bit value */
typedef unsigned char uchar;     /* 8-bit unsigned */
typedef unsigned short romword;  /* 10-bit unsigned */
typedef unsigned short address;  /* 16-bit unsigned */

#define WSIZE 14
typedef uchar digit;
typedef digit reg [WSIZE];

extern reg c;

extern void (* op_fcn [1024])(int);

#define MAX_PFAD 256

extern cbool pf_exists [MAX_PFAD];
extern void (* rd_n_fcn [MAX_PFAD])(int);
extern void (* wr_n_fcn [MAX_PFAD])(int);
extern void (* wr_fcn   [MAX_PFAD])(void);
extern void (* save_fcn [MAX_PFAD])(FILE *f, char *prefix);
extern cbool (* load_fcn [MAX_PFAD])(char *buf);

extern int io_count;

/* utility functions */

void fatal (int ret, char *format, ...);

void reg_copy (reg dest, reg src);
void reg_zero (reg dest);

cbool parse_hex (char **buf, int *v, int d);
cbool parse_reg (char *buf, digit *r, int d);
void write_reg (FILE *f, digit *r, int d);
