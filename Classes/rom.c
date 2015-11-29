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

$Header: /home/yl2/eric/hpcalc/nasm/RCS/rom.c,v 1.2 1995/08/02 22:16:45 eric Exp eric $
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "nsim.h"
#include "rom.h"

#ifndef NULL
#define NULL ((void *) 0)
#endif

romword *ucode [MAX_PAGE][MAX_BANK];
cbool *bpt      [MAX_PAGE][MAX_BANK];
char **source  [MAX_PAGE][MAX_BANK];
int active_bank   [MAX_PAGE];

void init_rom (void)
{
  int b, p;
  for (p = 0; p < MAX_PAGE; p++)
    {
      active_bank [p] = 0;  /* all ROMs start with bank 0 selected */
      for (b = 0; b < MAX_BANK; b++)
	{
	  ucode  [p][b] = NULL;
	  bpt    [p][b] = NULL;
	  source [p][b] = NULL;
	}
    }
}

static void alloc_page (int page, int bank)
{
  int o;

  ucode  [page][bank] = (romword *) calloc (PAGE_SIZE, sizeof (romword));
  bpt    [page][bank] = (cbool *) calloc (PAGE_SIZE, sizeof (cbool));
  source [page][bank] = (char **) calloc (PAGE_SIZE, sizeof (char *));

  if (! (ucode [page][bank] && bpt [page][bank] && source [page][bank]))
    fatal (2, "memory allocation failed\n");

  for (o = 0; o < PAGE_SIZE; o++)
    bpt [page][bank][o] = 1;
}

void set_ucode (address a, int b, romword val)
{
  int p = a / PAGE_SIZE;  /* counting on the compiler to do this efficiently */
  if (! ucode [p][b])
    alloc_page (p, b);
  ucode [p][b][a % PAGE_SIZE] = val;
}

void set_bpt    (address a, int b, cbool val)
{
  int p = a / PAGE_SIZE;  /* counting on the compiler to do this efficiently */
  if (! bpt [p][b])
    alloc_page (p, b);
  bpt [p][b][a % PAGE_SIZE] = val;
}

void set_source (address a, int b, char *val)
{
  int p = a / PAGE_SIZE;  /* counting on the compiler to do this efficiently */
  if (! source [p][b])
    alloc_page (p, b);
  source [p][b][a % PAGE_SIZE] = val;
}

romword get_ucode (address a)
{
  int p = a / PAGE_SIZE;  /* counting on the compiler to do this efficiently */
  romword *page = ucode [p][active_bank [p]];
  if (page)
    return (page [a % PAGE_SIZE]);
  else
    return (0);
}

cbool get_bpt (address a)
{
  int p = a / PAGE_SIZE;  /* counting on the compiler to do this efficiently */
  cbool *page = bpt [p][active_bank [p]];
  if (page)
    return (page [a % PAGE_SIZE]);
  else
    return (1);
}

char *get_source (address a)
{
  int p = a / PAGE_SIZE;  /* counting on the compiler to do this efficiently */
  char **page = source [p][active_bank [p]];
  if (page)
    return (page [a % PAGE_SIZE]);
  else
    return (NULL);
}

void select_bank (address a, int b)
{
  /* change active bank only if requested bank actually exists */
  int p = a / PAGE_SIZE;  /* counting on the compiler to do this efficiently */
  if (ucode [p][b])
    active_bank [p] = b;
}
