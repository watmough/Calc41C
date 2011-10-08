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

$Header: /home/yl2/eric/hpcalc/nasm/RCS/lcd.c,v 1.3 1995/08/03 01:53:59 eric Exp eric $
*/

#include <stdio.h>

#include "nsim.h"
#include "lcd.h"

#define DSIZE 12

cbool display_enable;
digit lcd_a [DSIZE];
digit lcd_b [DSIZE];
digit lcd_c [DSIZE];
int lcd_ann;

static void op_display_off (int opcode)
{
  display_enable = 0;
  io_count = 2;
  /*
   * Don't immediately turn off display because the very next instruction
   * might be a display_toggle to turn it on.  This happens in the HP-45
   * stopwatch.
   */
}

static void op_display_toggle (int opcode)
{
  display_enable = ! display_enable;
  io_count = 0;  /* force immediate display update */
}

#define A 1
#define B 2
#define C 4
#define AB 3
#define ABC 7

#define LEFT (DSIZE - 1)
#define RIGHT 0

static void lcd_rot (digit *reg, int dir)
{
  int i, t;
  if (dir == LEFT)
    {
      t = reg [DSIZE - 1];
      for (i = DSIZE - 1; i > 0; i--)
	reg [i] = reg [i - 1];
      reg [0] = t;
    }
  else
    {
      t = reg [0];
      for (i = 0; i < DSIZE - 1; i++)
	reg [i] = reg [i + 1];
      reg [DSIZE - 1] = t;
    }
	
	io_count = 20;
}

static void lcd_rd (int reg, int chars, int dir)
{
  int i, j;
  j = 0;
  for (i = 0; i < chars; i++)
    {
      if (reg & A)
	{
	  c [j++] = lcd_a [dir];
	  lcd_rot (& lcd_a [0], dir);
	}
      if (reg & B)
	{
	  c [j++] = lcd_b [dir];
	  lcd_rot (& lcd_b [0], dir);
	}
      if (reg & C)
	{
	  c [j++] = lcd_c [dir];
	  lcd_rot (& lcd_c [0], dir);
	}
    }
}

static void lcd_wr (int reg, int chars, int dir)
{
  int i, j;
  j = 0;
  for (i = 0; i < chars; i++)
    {
      if (reg & A)
	{
	  lcd_rot (& lcd_a [0], LEFT - dir);
	  lcd_a [dir] = c [j++];
	}
      if (reg & B)
	{
	  lcd_rot (& lcd_b [0], LEFT - dir);
	  lcd_b [dir] = c [j++];
	}
      if (reg & C)
	{
	  lcd_rot (& lcd_c [0], LEFT - dir);
	  lcd_c [dir] = c [j++] & 1;
	}
    }
}

static void lcd_rd_ann (void)
{
  c [2] = lcd_ann >> 8;
  c [1] = (lcd_ann >> 4) & 0x0f;
  c [0] = lcd_ann & 0x0f;
}

static void lcd_wr_ann (void)
{
  lcd_ann = (c [2] << 8) | (c [1] << 4) | c [0];
}

static void display_rd_n (int n)
{
  switch (n)
    {
    case 0x0:  lcd_rd (A,   12, LEFT);  break;
    case 0x1:  lcd_rd (B,   12, LEFT);  break;
    case 0x2:  lcd_rd (C,   12, LEFT);  break;
    case 0x3:  lcd_rd (AB,   6, LEFT);  break;
    case 0x4:  lcd_rd (ABC,  4, LEFT);  break;
    case 0x5:  lcd_rd_ann ();           break;
    case 0x6:  lcd_rd (C,    1, LEFT);  break;
    case 0x7:  lcd_rd (A,    1, RIGHT); break;
    case 0x8:  lcd_rd (B,    1, RIGHT); break;
    case 0x9:  lcd_rd (C,    1, RIGHT); break;
    case 0xa:  lcd_rd (A,    1, LEFT);  break;
    case 0xb:  lcd_rd (B,    1, LEFT);  break;
    case 0xc:  lcd_rd (AB,   1, RIGHT); break;
    case 0xd:  lcd_rd (AB,   1, LEFT);  break;
    case 0xe:  lcd_rd (ABC,  1, RIGHT); break;
    case 0xf:  lcd_rd (ABC,  1, LEFT);  break;
    }
}

static void display_wr_n (int n)
{
  switch (n)
    {
    case 0x0:  lcd_wr (A,   12, LEFT);  break;
    case 0x1:  lcd_wr (B,   12, LEFT);  break;
    case 0x2:  lcd_wr (C,   12, LEFT);  break;
    case 0x3:  lcd_wr (AB,   6, LEFT);  break;
    case 0x4:  lcd_wr (ABC,  4, LEFT);  break;
    case 0x5:  lcd_wr (AB,   6, RIGHT); break;
    case 0x6:  lcd_wr (ABC,  4, RIGHT); break;
    case 0x7:  lcd_wr (A,    1, LEFT);  break;
    case 0x8:  lcd_wr (B,    1, LEFT);  break;
    case 0x9:  lcd_wr (C,    1, LEFT);  break;
    case 0xa:  lcd_wr (A,    1, RIGHT); break;
    case 0xb:  lcd_wr (B,    1, RIGHT); break;
    case 0xc:  lcd_wr (C,    1, RIGHT); break;
    case 0xd:  lcd_wr (AB,   1, RIGHT); break;
    case 0xe:  lcd_wr (ABC,  1, LEFT);  break;
    case 0xf:  lcd_wr (ABC,  1, RIGHT); break;
    }
}

static void display_wr (void)
{
  lcd_wr_ann ();
}

static cbool display_load (char *buf)
{
  cbool ok = 1;

  switch (buf [0])
    {
    case 'e': 
      display_enable = (buf [3] == 'e'); break;
    case 'a': ok = parse_reg (buf, lcd_a, DSIZE); break;
    case 'b': ok = parse_reg (buf, lcd_b, DSIZE); break;
    case 'c': ok = parse_reg (buf, lcd_c, DSIZE); break;
    case 'f': ok = parse_hex (& buf, & lcd_ann, 3); break;
    default: ok = 0;
    }
  return (1);
}

static void display_save (FILE *f, char *prefix)
{
  fprintf (f, "%se: %c\n", prefix, display_enable ? 'e' : 'd');
  fprintf (f, "%sa: ", prefix); write_reg (f, lcd_a, DSIZE);
  fprintf (f, "%sb: ", prefix); write_reg (f, lcd_b, DSIZE);
  fprintf (f, "%sc: ", prefix); write_reg (f, lcd_c, DSIZE);
  fprintf (f, "%sf: %03x\n", prefix, lcd_ann);
}


static void halfnut_rd_n (int n)
{
}

static void halfnut_wr_n (int n)
{
}

static void halfnut_wr (void)
{
}

static cbool halfnut_load (char *buf)
{
  return (0);
}

static void halfnut_save (FILE *f, char *prefix)
{
}


void init_display (void)
{
  int i;

  display_enable = 0;
  op_fcn [0x2e0] = op_display_off;
  op_fcn [0x320] = op_display_toggle;

  pf_exists [LCD_DISPLAY] = 1;
  rd_n_fcn [LCD_DISPLAY] = & display_rd_n;
  wr_n_fcn [LCD_DISPLAY] = & display_wr_n;
  wr_fcn   [LCD_DISPLAY] = & display_wr;
  save_fcn [LCD_DISPLAY] = & display_save;
  load_fcn [LCD_DISPLAY] = & display_load;

  pf_exists [HALFNUT] = 1;
  rd_n_fcn  [HALFNUT] = & halfnut_rd_n;
  wr_n_fcn  [HALFNUT] = & halfnut_wr_n;
  wr_fcn    [HALFNUT] = & halfnut_wr;
  save_fcn  [HALFNUT] = & halfnut_save;
  load_fcn  [HALFNUT] = & halfnut_load;

  for (i = 0; i < DSIZE; i++)
    lcd_a [i] = lcd_b [i] = lcd_c [i] = 0;
  lcd_ann = 0;
}

static int row_map [8] = { 0x40, 0x50, 0x20, 0x30, 0x60, -1, -1, -1 };
static int punct_map [4] = { ' ', '.', ':', ',' };

static char *append_ann (char *s, int b, char *s2)
{
  while (*s2)
    {
      *s++ = b ? *s2 : ' ';
      s2++;
    }
  return (s);
}

char *ann_to_buf (char *s)
{
  if (display_enable)
    {
      s = append_ann (s, lcd_ann & 0x800, "BAT ");
      s = append_ann (s, lcd_ann & 0x400, "USER ");
      s = append_ann (s, lcd_ann & 0x200, "G");
      s = append_ann (s, lcd_ann & 0x100, "RAD ");
      s = append_ann (s, lcd_ann & 0x080, "SHIFT ");
      s = append_ann (s, lcd_ann & 0x040, "0");
      s = append_ann (s, lcd_ann & 0x020, "1");
      s = append_ann (s, lcd_ann & 0x010, "2");
      s = append_ann (s, lcd_ann & 0x008, "3");
      s = append_ann (s, lcd_ann & 0x004, "4");
      s = append_ann (s, lcd_ann & 0x002, " PRGM");
      s = append_ann (s, lcd_ann & 0x001, " ALPHA");
    }
  *s++ = '\0';
  return (s);
}

char *display_to_buf (char *buf)
{
  int i;
  int row;
  char *s = buf;
  if (display_enable)
    {
      for (i = DSIZE - 1; i >= 0; i--)
	{
	  row = (lcd_b [i] & 3) | (lcd_c [i] << 2);
	  if (row_map [row] == -1)
	    *s++ = ' ';
	  else
	    *s++ = row_map [row] + lcd_a [i];
	  *s++ = punct_map [lcd_b [i] >> 2];
	}
    }
  *s++ = '\0';
  return (buf);
}

void show_display (void)
{
  char buf [80];

  printf ("LCD %s: ", display_enable ? "on" : "off");
  printf ("'%s'", display_to_buf (buf));
  printf ("ann: '%s'\n", ann_to_buf (buf));
}

