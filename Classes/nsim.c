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
 
 This file was modified by mba@mac.com, 05/2004
 
 Changes:
 - main() moved to main.c
 - removed setjmp/longjmp
 - removed debugger &c.
 - removed SIGINT handler
 - and many others...
 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "nsim.h"
#include "xio.h"
#include "rom.h"
#include "lcd.h"
#include "phineas.h"

#define USE_TIMER YES

#ifdef USE_TIMER
#include <signal.h>
#include <sys/time.h>
#endif /* USE_TIMER */

#define EXIT_SAVE 1
#define EXIT_NOSAVE 2

/* map from high opcode bits to register index */
int tmap [16] =
{ 3, 4, 5, 10, 8, 6, 11, -1, 2, 9, 7, 13, 1, 12, 0, -1 };

/* map from register index to high opcode bits */
int itmap [WSIZE] =
{ 0xe, 0xc, 0x8, 0x0, 0x1, 0x2, 0x5, 0xa, 0x4, 0x9, 0x3, 0x6, 0xd, 0xb };


#define P 0
#define Q 1

reg a, b, c, m, n;

digit p;
digit q;
digit *pt;

#define SSIZE 14
cbool s [SSIZE];
digit g [2];

uchar fo;

uchar arith_base;  /* 10 or 16 */
cbool carry, prev_carry;

cbool awake;

address pc, prev_pc;
#define STACK_DEPTH 4
address stack [STACK_DEPTH];

#ifdef AUTO_POWER_OFF
int display_timer;  /* display time out in 1/100 of seconds */
#define DISPLAY_TIMEOUT 66000 /* nominal 11 minutes */
#endif /* AUTO_POWER_OFF */

cbool key_flag;
uchar key_buf;

int io_count;

#define MAX_RAM 1024
int  ram_addr;
cbool ram_exists [MAX_RAM];
reg  ram [MAX_RAM];

int  pf_addr;
cbool pf_exists [MAX_PFAD];
void (* rd_n_fcn [MAX_PFAD])(int);
void (* wr_n_fcn [MAX_PFAD])(int);
void (* wr_fcn   [MAX_PFAD])(void);
void (* save_fcn [MAX_PFAD])(FILE *f, char *prefix);
cbool (* load_fcn [MAX_PFAD])(char *buf);

void (* op_fcn [1024])(int);

unsigned int cycle;


//#ifndef PATH_MAX
//#define PATH_MAX 1024
//#endif




/* generate fatal error message to stderr, doesn't return */

void fatal (int ret, char *format, ...)
{
	va_list ap;
	
	if (format)
    {
		fprintf (stderr, "fatal error: ");
    }
	exit (ret);
}

static char *newstr (char *orig)
{
	int len;
	char *r;
	
	len = strlen (orig);
	r = (char *) malloc (len + 1);
	
	if (! r)
		fatal (2, "memory allocation failed\n");
	
	memcpy (r, orig, len + 1);
	return (r);
}


void reg_copy (reg dest, reg src)
{
	int i;
	for (i = 0; i < WSIZE; i++)
		dest [i] = src [i];
}

void reg_zero (reg dest)
{
	int i;
	for (i = 0; i < WSIZE; i++)
		dest [i] = 0;
}

static void bad_op (int opcode)
{
	printf ("illegal opcode %02x at %05o\n", opcode, prev_pc);
}

static digit do_add (digit x, digit y)
{
	int res;
	
	res = x + y + carry;
	if (res >= arith_base)
    {
		res -= arith_base;
		carry = 1;
    }
	else
		carry = 0;
	return (res);
}

static digit do_sub (digit x, digit y)
{
	int res;
	
	res = (x - y) - carry;
	if (res < 0)
    {
		res += arith_base;
		carry = 1;
    }
	else
		carry = 0;
	return (res);
}

static void op_arith (int opcode)
{
	uchar op, field;
	int first=0, last=0;
	int temp;
	int i;
	reg t;
	
	op = opcode >> 5;
	field = (opcode >> 2) & 7;
	
	switch (field)
    {
		case 0:  /* p  */  first = (*pt);      last = (*pt);      break;
		case 1:  /* x  */  first = 0;          last = 2;          break;
		case 2:  /* wp */  first = 0;          last = (*pt);      break;
		case 3:  /* w  */  first = 0;          last = WSIZE - 1;  break;
		case 4:  /* pq */  first = p;          last = q;
			if (first > last)
				last = WSIZE - 1;
				break;
		case 5:  /* xs */  first = 2;          last =  2;            break;
		case 6:  /* m  */  first = 3;          last = WSIZE - 2;     break;
		case 7:  /* s  */  first = WSIZE - 1;  last = WSIZE - 1;     break;
    }
	
	switch (op)
    {
		case 0x00:  /* a=0 */
			for (i = first; i <= last; i++)
				a [i] = 0;
			break;
		case 0x01:  /* b=0 */
			for (i = first; i <= last; i++)
				b [i] = 0;
			break;
		case 0x02:  /* c=0 */
			for (i = first; i <= last; i++)
				c [i] = 0;
			break;
		case 0x03:  /* ab ex */
			for (i = first; i <= last; i++)
			{ temp = b [i]; b [i] = a [i]; a [i] = temp; }
			break;
		case 0x04:  /* b=a */
			for (i = first; i <= last; i++)
				b [i] = a [i];
			break;
		case 0x05:  /* ac ex */
			for (i = first; i <= last; i++)
			{ temp = c [i]; c [i] = a [i]; a [i] = temp; }
			break;
		case 0x06:  /* c=b */
			for (i = first; i <= last; i++)
				c [i] = b [i];
			break;
		case 0x07:  /* bc ex */
			for (i = first; i <= last; i++)
			{ temp = c [i]; c [i] = b [i]; b [i] = temp; }
			break;
		case 0x08:  /* a=c */
			for (i = first; i <= last; i++)
				a [i] = c [i];
			break;
		case 0x09:  /* a=a+b */
			for (i = first; i <= last; i++)
				a [i] = do_add (a [i], b [i]);
			break;
		case 0x0a:  /* a=a+c */
			for (i = first; i <= last; i++)
				a [i] = do_add (a [i], c [i]);
			break;
		case 0x0b:  /* a=a+1 */
			carry = 1;
			for (i = first; i <= last; i++)
				a [i] = do_add (a [i], 0);
				break;
		case 0x0c:  /* a=a-b */
			for (i = first; i <= last; i++)
				a [i] = do_sub (a [i], b [i]);
			break;
		case 0x0d:  /* a=a-1 */
			carry = 1;
			for (i = first; i <= last; i++)
				a [i] = do_sub (a [i], 0);
				break;
		case 0x0e:  /* a=a-c */
			for (i = first; i <= last; i++)
				a [i] = do_sub (a [i], c [i]);
			break;
		case 0x0f:  /* c=c+c */
			for (i = first; i <= last; i++)
				c [i] = do_add (c [i], c [i]);
			break;
		case 0x10:  /* c=a+c */
			for (i = first; i <= last; i++)
				c [i] = do_add (a [i], c [i]);
			break;
		case 0x11:  /* c=c+1 */
			carry = 1;
			for (i = first; i <= last; i++)
				c [i] = do_add (c [i], 0);
				break;
		case 0x12:  /* c=a-c */
			for (i = first; i <= last; i++)
				c [i] = do_sub (a [i], c [i]);
			break;
		case 0x13:  /* c=c-1 */
			carry = 1;
			for (i = first; i <= last; i++)
				c [i] = do_sub (c [i], 0);
				break;
		case 0x14:  /* c=-c */
			for (i = first; i <= last; i++)
				c [i] = do_sub (0, c [i]);
			break;
		case 0x15:  /* c=-c-1 */
			carry = 1;
			for (i = first; i <= last; i++)
				c [i] = do_sub (0, c [i]);
				break;
		case 0x16:  /* ? b<>0 */
			for (i = first; i <= last; i++)
				carry |= (b [i] != 0);
			break;
		case 0x17:  /* ? c<>0 */
			for (i = first; i <= last; i++)
				carry |= (c [i] != 0);
			break;
		case 0x18:  /* ? a<c */
			for (i = first; i <= last; i++)
				t [i] = do_sub (a [i], c [i]);
			break;
		case 0x19:  /* ? a<b */
			for (i = first; i <= last; i++)
				t [i] = do_sub (a [i], b [i]);
			break;
		case 0x1a:  /* ? a<>0 */
			for (i = first; i <= last; i++)
				carry |= (a [i] != 0);
			break;
		case 0x1b:  /* ? a<>c */
			for (i = first; i <= last; i++)
				carry |= (a [i] != c [i]);
			break;
		case 0x1c:  /* a sr */
			for (i = first; i <= last; i++)
				a [i] = (i == last) ? 0 : a [i+1];
			break;
		case 0x1d:  /* b sr */
			for (i = first; i <= last; i++)
				b [i] = (i == last) ? 0 : b [i+1];
			break;
		case 0x1e:  /* a sr */
			for (i = first; i <= last; i++)
				c [i] = (i == last) ? 0 : c [i+1];
			break;
		case 0x1f:  /* a sl */
			for (i = last; i >= first; i--)
				a [i] = (i == first) ? 0 : a [i-1];
			break;
    }
}

/*
 * stack operations
 */

static address pop (void)
{
	int i;
	address ret;
	
	ret = stack [0];
	for (i = 0; i < STACK_DEPTH - 1; i++)
		stack [i] = stack [i + 1];
	stack [STACK_DEPTH - 1] = 0;
	return (ret);
}

static void push (address a)
{
	int i;
	for (i = STACK_DEPTH - 1; i > 0; i--)
		stack [i] = stack [i - 1];
	stack [0] = a;
}

static void op_return (int opcode)
{
	pc = pop ();
}

static void op_return_if_carry (int opcode)
{
	if (prev_carry)
		pc = pop ();
}

static void op_return_if_no_carry (int opcode)
{
	if (! prev_carry)
		pc = pop ();
}

static void op_pop (int opcode)
{
	address a;
	a = pop ();
}

static void op_pop_c (int opcode)
{
	address a;
	
	a = pop ();
	c [6] = a >> 12;
	c [5] = (a >> 8) & 0x0f;
	c [4] = (a >> 4) & 0x0f;
	c [3] = a & 0x0f;
}

static void op_push_c (int opcode)
{
	push ((c [6] << 12) | (c [5] << 8) | (c [4] << 4) | c [3]);
}

/*
 * branch operations
 */

static void op_short_branch (int opcode)
{
	int offset;
	
	offset = (opcode >> 3) & 0x3f;
	if (opcode & 0x200)
		offset = offset - 64;
	
	if (((opcode >> 2) & 1) == prev_carry)
		pc = pc + offset - 1;
}

static void op_long_branch (int opcode)
{
	romword word2;
	address target;
	
	word2 = get_ucode (pc++);
	target = (opcode >> 2) | ((word2 & 0x3fc) << 6);
	
	if ((word2 & 0x001) == prev_carry)
    {
		if (word2 & 0x002)
			pc = target;
		else
		{
			push (pc);
			pc = target;
			if (get_ucode (pc) == 0)
				pc = pop ();
		}
    }
}

static void op_goto_c (int opcode)
{
	pc = (c [6] << 12) | (c [5] << 8) | (c [4] << 4) | c [3];
}

/*
 * Bank selection used in 41CX, Advantage ROM, and perhaps others
 */
static void op_enbank1 (int opcode)
{
	select_bank (prev_pc, 0);
}

static void op_enbank2 (int opcode)
{
	select_bank (prev_pc, 1);
}

/*
 * m operations
 */

static void op_c_to_m (int opcode)
{
	int i;
	for (i = 0; i < WSIZE; i++)
		m [i] = c [i];
}

static void op_m_to_c (int opcode)
{
	int i;
	for (i = 0; i < WSIZE; i++)
		c [i] = m [i];
}

static void op_c_exch_m (int opcode)
{
	int i, t;
	for (i = 0; i < WSIZE; i++)
    {
		t = c [i]; c [i] = m [i]; m [i] = t;
    }
}

/*
 * n operations
 */

static void op_c_to_n (int opcode)
{
	int i;
	for (i = 0; i < WSIZE; i++)
		n [i] = c [i];
}

static void op_n_to_c (int opcode)
{
	int i;
	for (i = 0; i < WSIZE; i++)
		c [i] = n [i];
}

static void op_c_exch_n (int opcode)
{
	int i, t;
	for (i = 0; i < WSIZE; i++)
    {
		t = c [i]; c [i] = n [i]; n [i] = t;
    }
}

/*
 * RAM and peripheral operations
 */

static void op_c_to_dadd (int opcode)
{
	ram_addr = ((c [2] << 8) | (c [1] << 4) | c [0]) & 0x3ff;
}

static void op_c_to_pfad (int opcode)
{
	pf_addr = (c [1] << 4) | c [0];
}

static void op_read_reg_n (int opcode)
{
	int i;
	int is_ram, is_pf;
	
	if ((opcode >> 6) != 0)
		ram_addr = (ram_addr & ~0x0f) | (opcode >> 6);
	is_ram = ram_exists [ram_addr];
	is_pf  = pf_exists  [pf_addr];
	
	if (is_ram && is_pf)
    {
		printf ("warning: conflicting read RAM %03x PF %02x reg %01x\n",
				ram_addr, pf_addr, opcode >> 6);
    }
	if (is_ram)
    {
		for (i = 0; i < WSIZE; i++)
			c [i] = ram [ram_addr][i];
    }
	else if (is_pf)
    {
		if (rd_n_fcn [pf_addr])
			(*rd_n_fcn [pf_addr]) (opcode >> 6);
    }
	else
    {
		printf ("warning: stray read RAM %03x PF %02x reg %01x\n",
				ram_addr, pf_addr, opcode >> 6);
		for (i = 0; i < WSIZE; i++)
			c [i] = 0;
    }
}

static void op_write_reg_n (int opcode)
{
	int i;
	int is_ram, is_pf;
	
	ram_addr = (ram_addr & ~0x0f) | (opcode >> 6);
	is_ram = ram_exists [ram_addr];
	is_pf  = pf_exists  [pf_addr];
	
	if (is_ram && is_pf)
    {
		printf ("warning: conflicting write RAM %03x PF %02x reg %01x\n",
				ram_addr, pf_addr, opcode >> 6);
    }
	else if ((! is_ram) && (! is_pf))
    {
#ifdef WARN_STRAY_WRITE
		printf ("warning: stray write RAM %03x PF %02x reg %01x\n",
				ram_addr, pf_addr, opcode >> 6);
#endif
    }
	if (is_ram)
    {
		for (i = 0; i < WSIZE; i++)
			ram [ram_addr][i] = c [i];
    }
	if (is_pf)
    {
		if (wr_n_fcn [pf_addr])
			(*wr_n_fcn [pf_addr]) (opcode >> 6);
    }
}

static void op_c_to_data (int opcode)
{
	int i;
	int is_ram, is_pf;
	
	is_ram = ram_exists [ram_addr];
	is_pf  = pf_exists  [pf_addr];
	
	if (is_ram && is_pf)
    {
		printf ("warning: conflicting write RAM %03x PF %02x\n",
				ram_addr, pf_addr);
    }
	else if ((! is_ram) && (! is_pf))
    {
#ifdef WARN_STRAY_WRITE
		printf ("warning: stray write RAM %03x PF %02x\n",
				ram_addr, pf_addr);
#endif
    }
	if (is_ram)
    {
		for (i = 0; i < WSIZE; i++)
			ram [ram_addr][i] = c [i];
    }
	if (is_pf)
    {
		if (wr_fcn [pf_addr])
			(* wr_fcn [pf_addr]) ();
    }
}

static void op_test_ext_flag (int opcode)
{
	carry = 0;  /* no periphs yet */
}

/*
 * s operations
 */

static void op_set_s (int opcode)
{
	s [tmap [opcode >> 6]] = 1;
}

static void op_clr_s (int opcode)
{
	s [tmap [opcode >> 6]] = 0;
}

static void op_test_s (int opcode)
{
	carry = s [tmap [opcode >> 6]];
}

static int get_s_bits (int first, int count)
{
	int i;
	int mask = 1;
	int r = 0;
	for (i = first; i < first + count; i++)
    {
		if (s [i])
			r = r + mask;
		mask <<= 1;
    }
	return (r);
}

static void set_s_bits (int first, int count, int a)
{
	int i;
	int mask = 1;
	for (i = first; i < first + count; i++)
    {
		s [i] = (a & mask) != 0;
		mask <<= 1;
    }
}

static void op_clear_all_s (int opcode)
{
	set_s_bits (0, 8, 0);
}

static void op_c_to_s (int opcode)
{
	set_s_bits (0, 4, c [0]);
	set_s_bits (4, 4, c [1]);
}

static void op_s_to_c (int opcode)
{
	c [0] = get_s_bits (0, 4);
	c [1] = get_s_bits (4, 4);
}

static void op_c_exch_s (int opcode)
{
	int t;
	t = get_s_bits (0, 4);
	set_s_bits (0, 4, c [0]);
	c [0] = t;
	t = get_s_bits (4, 4);
	set_s_bits (4, 4, c [1]);
	c [1] = t;
}

static void op_sb_to_f (int opcode)
{
	fo = get_s_bits (0, 8);
}

static void op_f_to_sb (int opcode)
{
	set_s_bits (0, 8, fo);
}

static void op_f_exch_sb (int opcode)
{
	int t;
	t = get_s_bits (0, 8);
	set_s_bits (0, 8, fo);
	fo = t;
}

/*
 * pointer operations
 */

static void op_dec_pt (int opcode)
{
	(*pt)--;
	if ((*pt) >= WSIZE)  /* can't be negative because it is unsigned */
		(*pt) = WSIZE - 1;
}

static void op_inc_pt (int opcode)
{
	(*pt)++;
	if ((*pt) >= WSIZE)
		(*pt) = 0;
}

static void op_set_pt (int opcode)
{
	(*pt) = tmap [opcode >> 6];
}

static void op_test_pt (int opcode)
{
	carry = ((*pt) == tmap [opcode >> 6]);
}

static void op_sel_p (int opcode)
{
	pt = & p;
}

static void op_sel_q (int opcode)
{
	pt = & q;
}

static void op_test_pq (int opcode)
{
	if (p == q)
		carry = 1;
}

static void op_lc (int opcode)
{
	c [(*pt)--] = opcode >> 6;
	if ((*pt) >= WSIZE)  /* unsigned, can't be negative */
		(*pt) = WSIZE - 1;
}

static void op_c_to_g (int opcode)
{
	g [0] = c [(*pt)];
	if ((*pt) == (WSIZE - 1))
    {
		g [1] = 0;
#ifdef WARNING_G
		fprintf (stderr, "warning: c to g transfer with pt=13\n");
#endif
    }
	else
		g [1] = c [(*pt) + 1];
}

static void op_g_to_c (int opcode)
{
	c [(*pt)] = g [0];
	if ((*pt) == (WSIZE - 1))
    {
		;
#ifdef WARNING_G
		fprintf (stderr, "warning: g to c transfer with pt=13\n");
#endif
    }
	else
    {
		c [(*pt) + 1] = g [1];
    }
    
}

static void op_c_exch_g (int opcode)
{
	int t;
	t = g [0];
	g [0] = c [(*pt)];
	c [(*pt)] = t;
	if ((*pt) == (WSIZE - 1))
    {
		g [1] = 0;
#ifdef WARNING_G
		fprintf (stderr, "warning: c exchange g with pt=13\n");
#endif
    }
	else
    {
		t = g [1];
		g [1] = c [(*pt) + 1];
		c [(*pt) + 1] = t;
    }
}

/*
 * keyboard operations
 */

/*
 * This table maps logical keycodes from the I/O system into the actual
 * keycodes returned by the c=key microinstruction.  -1 entries are
 * invalid key codes.
 */
static int keymap [100] =
{
	/* col       0,    1,    2,    3,    4,    5,    6,    7,    8,    9 */
	/* row 0 */ -1, 0x18, 0xc6, 0xc5, 0xc4,   -1,   -1,   -1,   -1,   -1,
	/* row 1 */ -1, 0x10, 0x30, 0x70, 0x80, 0xc0,   -1,   -1,   -1,   -1,
	/* row 2 */ -1, 0x11, 0x31, 0x71, 0x81, 0xc1,   -1,   -1,   -1,   -1,
	/* row 3 */ -1, 0x12, 0x32, 0x72, 0x82, 0xc2,   -1,   -1,   -1,   -1,
	/* row 4 */ -1, 0x13,   -1, 0x73, 0x83, 0xc3,   -1,   -1,   -1,   -1,
	/* row 5 */ -1, 0x14, 0x34, 0x74, 0x84,   -1,   -1,   -1,   -1,   -1,
	/* row 6 */ -1, 0x15, 0x35, 0x75, 0x85,   -1,   -1,   -1,   -1,   -1,
	/* row 7 */ -1, 0x16, 0x36, 0x76, 0x86,   -1,   -1,   -1,   -1,   -1,
	/* row 8 */ -1, 0x17, 0x37, 0x77, 0x87,   -1,   -1,   -1,   -1,   -1,
	/* row 9 */ -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1
};


/*static void op_keys_to_rom_addr (int opcode)
{
	pc = (pc & 0xff00) | key_buf;
}*/

static void op_keys_to_c (int opcode)
{
	c [4] = key_buf >> 4;
	c [3] = key_buf & 0x0f;
}

static void op_test_kb (int opcode)
{
	carry = key_flag;
}

static int handle_keyboard (void)
{
	int i;
	
	i = check_keyboard ();
	if (i == KC_EXIT)
		return KC_EXIT;
	if (i == KC_NONE)
		return 0;
	if (keymap [i] < 0)
		return 0;
	if ((! awake) && (! display_enable) && (keymap [i] != 0x18))
		return 0;
	key_flag = 1;
	key_buf = keymap [i];
	awake = 1;
	return 0;
}

static void op_reset_kb (int opcode)
{
	key_flag = 0;
	handle_keyboard ();  /* see if key still down */
}


/*
 * misc. operations
 */

static void op_nop (int opcode)
{
}

static void op_set_hex (int operand)
{
	arith_base = 16;
}

static void op_set_dec (int operand)
{
	arith_base = 10;
}

static void op_rom_to_c (int operand)
{
	address addr;
	addr = (c [6] << 12) | (c [5] << 8) | (c [4] << 4) | c [3];
	operand = get_ucode (addr);
	c [2] = operand >> 8;
	c [1] = (operand >> 4) & 0x0f;
	c [0] = operand & 0x0f;
}

static void op_clear_abc (int opcode)
{
	int i;
	for (i = 0; i < WSIZE; i++)
		a [i] = b [i] = c [i] = 0;
}

static void op_ldi (int opcode)
{
	opcode = get_ucode (pc++);
	c [2] = opcode >> 8;
	c [1] = (opcode >> 4) & 0x0f;
	c [0] = opcode & 0x00f;
}

static void op_or (int opcode)
{
	int i;
	for (i = 0; i < WSIZE; i++)
		c [i] = c [i] | a [i];
}

static void op_and (int opcode)
{
	int i;
	for (i = 0; i < WSIZE; i++)
		c [i] = c [i] & a [i];
}

static void op_rcr (int opcode)
{
	int count, i, j;
	reg t;
	count = tmap [opcode >> 6];
	for (i = 0; i < WSIZE; i++)
    {
		j = (i + count) % WSIZE;
		t [i] = c [j];
    }
	for (i = 0; i < WSIZE; i++)
		c [i] = t [i];
}

static void op_lld (int opcode)
{
	carry = 0;  /* "batteries" are fine */
}

static void op_powoff (int opcode)
{
//	awake = 0;
	pc = 0;
	if (display_enable)
    {
		/* going to light sleep */
#ifdef AUTO_POWER_OFF
		/* start display timer if LCD chip is selected */
		if (pf_addr == LCD_DISPLAY)
			display_timer = DISPLAY_TIMEOUT;
#endif /* AUTO_POWER_OFF */
    }
	else
		/* going to deep sleep */
		carry = 1;
}

/*
 * initialization
 */

static void init_ops (void)
{
	int i;
	for (i = 0; i < 1024; i += 4)
    {
		op_fcn [i + 0] = bad_op;
		op_fcn [i + 1] = op_long_branch;
		op_fcn [i + 2] = op_arith;
		op_fcn [i + 3] = op_short_branch;
    }
	
	op_fcn [0x000] = op_nop;
	
	/*  op_fcn [0x040] = op_write_mldl; */
	
	op_fcn [0x100] = op_enbank1;
	op_fcn [0x180] = op_enbank2;
	
	/*  for (i = 0; i < 8; i++)
		op_fcn [0x200 + (i << 6)] = op_write_pil; */
	
	for (i = 0; i < WSIZE; i++)
    {
		op_fcn [0x004 + (itmap [i] << 6)] = op_clr_s;
		op_fcn [0x008 + (itmap [i] << 6)] = op_set_s;
		op_fcn [0x00c + (itmap [i] << 6)] = op_test_s;
		op_fcn [0x014 + (itmap [i] << 6)] = op_test_pt;
		op_fcn [0x01c + (itmap [i] << 6)] = op_set_pt;
		op_fcn [0x02c + (itmap [i] << 6)] = op_test_ext_flag;
		op_fcn [0x03c + (itmap [i] << 6)] = op_rcr;
    }
	op_fcn [0x3c4] = op_clear_all_s;
	op_fcn [0x3c8] = op_reset_kb;
	op_fcn [0x3cc] = op_test_kb;
	op_fcn [0x3d4] = op_dec_pt;
	op_fcn [0x3dc] = op_inc_pt;
	
	for (i = 0; i < 16; i++)
    {
		op_fcn [0x010 + (i << 6)] = op_lc;
		/* op_fcn [0x024 + (i << 6)] = op_selprf; */
		op_fcn [0x028 + (i << 6)] = op_write_reg_n;
		op_fcn [0x038 + (i << 6)] = op_read_reg_n;
    }
	
	op_fcn [0x058] = op_c_to_g;
	op_fcn [0x098] = op_g_to_c;
	op_fcn [0x0d8] = op_c_exch_g;
	
	op_fcn [0x158] = op_c_to_m;
	op_fcn [0x198] = op_m_to_c;
	op_fcn [0x1d8] = op_c_exch_m;
	
	op_fcn [0x258] = op_sb_to_f;
	op_fcn [0x298] = op_f_to_sb;
	op_fcn [0x2d8] = op_f_exch_sb;
	
	op_fcn [0x358] = op_c_to_s;
	op_fcn [0x398] = op_s_to_c;
	op_fcn [0x3d8] = op_c_exch_s;
	
	op_fcn [0x020] = op_pop;
	op_fcn [0x060] = op_powoff;
	op_fcn [0x0a0] = op_sel_p;
	op_fcn [0x0e0] = op_sel_q;
	op_fcn [0x120] = op_test_pq;
	op_fcn [0x160] = op_lld;
	op_fcn [0x1a0] = op_clear_abc;
	op_fcn [0x1e0] = op_goto_c;
	op_fcn [0x220] = op_keys_to_c;
	op_fcn [0x260] = op_set_hex;
	op_fcn [0x2a0] = op_set_dec;
	op_fcn [0x360] = op_return_if_carry;
	op_fcn [0x3a0] = op_return_if_no_carry;
	op_fcn [0x3e0] = op_return;
	
	op_fcn [0x070] = op_c_to_n;
	op_fcn [0x0b0] = op_n_to_c;
	op_fcn [0x0f0] = op_c_exch_n;
	op_fcn [0x130] = op_ldi;
	op_fcn [0x170] = op_push_c;
	op_fcn [0x1b0] = op_pop_c;
	/* op_fcn [0x230] = op_keys_to_rom_addr; */
	op_fcn [0x270] = op_c_to_dadd;
	/* op_fcn [0x2b0] = op_clear_regs; */
	op_fcn [0x2f0] = op_c_to_data;
	op_fcn [0x330] = op_rom_to_c;
	op_fcn [0x370] = op_or;
	op_fcn [0x3b0] = op_and;
	op_fcn [0x3f0] = op_c_to_pfad;
}

/*static void disassemble_instruction (int p, int opcode)
{
	printf ("%04x: %03x  ", p, opcode);
}*/


static void init_ram (void)
{
	int i;
	for (i = 0; i < MAX_RAM; i++)
		ram_exists [i] = 0;
}

static void create_ram (int first, int last)
{
	int i, j;
	for (i = first; i <= last; i++)
    {
		ram_exists [i] = 1;
		for (j = 0; j < WSIZE; j++)
			ram [i][j] = 0;
    }
}

cbool parse_hex (char **buf, int *v, int d)
{
	int i, t, n;
	char *s;
	
	s = strchr (* buf, ':');
	if (s)
		* buf = s + 1;
	
	while (**buf == ' ')
		(* buf)++;
	
	if (strlen (* buf) < d)
		return (0);
	
	if (sscanf (* buf, "%x%n", & t, & n) == 1)
    {
		* v = t;
		(* buf) += n;
		return (1);
    }
	return (0);
}

cbool parse_reg (char *buf, digit *r, int d)
{
	int i, t;
	char *s;
	
	s = strchr (buf, ':');
	if (s)
		buf = s + 1;
	
	while (*buf == ' ')
		buf++;
	
	if (strlen (buf) < d)
		return (0);
	
	for (i = 0; i < d; i++)
    {
		if (sscanf (buf, "%1x", & t) != 1)
			return (0);
		r [(d - 1) - i] = t;
		buf++;
    }
	
	return (1);
}

static int parse_flags (char *buf)
{
	char *s;
	
	awake = 0;
	carry = 0;
	arith_base = 16;
	pt = & p;
	
	s = strchr (buf, ':');
	if (s)
		buf = s + 1;
	
	while (*buf)
		switch (*buf++)
		{
			case ' ':
			case '\n': break;
			case 'a': awake = 1; break;
			case 'c': carry = 1; break;
			case 'd': arith_base = 10;
			case 'q': pt = & q;
			default:
				return (0);
		}
			return (1);
}

static cbool parse_stack (char *buf)
{
	int i, t;
	
	for (i = 0; i < STACK_DEPTH; i++)
    {
		if (! parse_hex (& buf, & t, 4))
			return (0);
		stack [i] = t;
    }
	
	return (1);
}

static cbool parse_status (char *buf)
{
	int i;
	char *p;
	
	p = strchr (buf, ':');
	if (p)
		buf = p + 1;
	
	while (*buf == ' ')
		buf++;
	
	if (strlen (buf) < SSIZE)
		return (0);
	
	for (i = SSIZE - 1; i >= 0; i--)
    {
		switch (*buf)
		{
			case '*':
				s [i] = 1;
				break;
			case '.':
				s [i] = 0;
				break;
			case ' ':
			case '\n':
				break;
			default:
				return (0);
		}
		buf++;
    }
	
	return (1);
}

static int read_ram_file (char *ramfn)
{
	FILE *f;
	int addr;
	int t;
	int ok;
	char buf [81];
	
	f = fopen (ramfn, "r");
	if (! f)
    {
		fprintf (stderr, "Error opening RAM file\n");
		return (0);
    }
	while (fgets (buf, sizeof (buf), f))
    {
		ok = 0;
		switch (buf [0])
		{
			case '0': case '1': case '2': case '3':
				if (sscanf (buf, "%3x: ", & addr) != 1)
					fprintf (stderr, "badly formatted line\n");
				else if ((addr < 0) || (addr >= MAX_RAM) || (! ram_exists [addr]))
					fprintf (stderr, "Non-existent RAM address %03x\n", addr);
				else
					ok = parse_reg (buf, ram [addr], WSIZE);
				break;
			case 'f':
				ok = parse_flags (buf);
				break;
			case 'g': ok = parse_reg (buf, g, 2); break;
			case 'p': ok = parse_reg (buf, & p, 1); break;
			case 'q': ok = parse_reg (buf, & p, 1); break;
			case 's': ok = parse_status (buf); break;
			case 'P':
				if (sscanf (buf, "P: %4x", & t) == 1)
				{
					pc = t;
					ok = 1;
				}
				break;
			case 'S':
				ok = parse_stack (buf);
				break;
			case 'a': ok = parse_reg (buf, a, WSIZE); break;
			case 'b': ok = parse_reg (buf, b, WSIZE); break;
			case 'c': ok = parse_reg (buf, c, WSIZE); break;
			case 'm': ok = parse_reg (buf, m, WSIZE); break;
			case 'n': ok = parse_reg (buf, n, WSIZE); break;
			case 'x':
				if (sscanf (buf, "x%2x-", & addr) == 1)
					ok = load_fcn [addr] (buf + 4);
				break;
		}
		if (! ok)
			fprintf (stderr, "error parsing '%s'\n", buf);
    }
	fclose (f);
	return (1);
}

void write_reg (FILE *f, digit *r, int d)
{
	int i;
	
	for (i = d - 1; i >= 0; i--)
		fprintf (f, "%x", r [i]);
	fprintf (f, "\n");
}

static void write_flags (FILE *f)
{
	if (awake)
		fprintf (f, "a");
	if (carry)
		fprintf (f, "c");
	if (arith_base == 10)
		fprintf (f, "d");
	if (pt == & q)
		fprintf (f, "q");
	fprintf (f, "\n");
}

static void write_stack (FILE *f)
{
	int i;
	
	for (i = 0; i < STACK_DEPTH; i++)
		fprintf (f, " %04x", stack [i]);
	fprintf (f, "\n");
}

static void write_status (FILE *f)
{
	int i;
	
	fprintf (f, "s: ");
	for (i = SSIZE - 1; i >= 0; i--)
		fprintf (f, s [i] ? "*" : ".");
	fprintf (f, "\n");
}

int write_ram_file (char *ramfn)
{
	FILE *f;
	int addr;
	char buf [10];
	
	f = fopen (ramfn, "w");
	if (!f)
		return (0);
	
	fprintf (f, "f: "); write_flags (f);
	fprintf (f, "a: "); write_reg (f, a, WSIZE);
	fprintf (f, "b: "); write_reg (f, b, WSIZE);
	fprintf (f, "c: "); write_reg (f, c, WSIZE);
	fprintf (f, "m: "); write_reg (f, m, WSIZE);
	fprintf (f, "n: "); write_reg (f, n, WSIZE);
	fprintf (f, "g: "); write_reg (f, g, 2);
	fprintf (f, "p: "); write_reg (f, & p, 1);
	fprintf (f, "q: "); write_reg (f, & q, 1);
	write_status (f);
	fprintf (f, "P: %04x\n", pc);
	fprintf (f, "S:");  write_stack (f);
	for (addr = 0; addr < MAX_PFAD; addr++)
		if (pf_exists [addr] && save_fcn [addr])
		{
			sprintf (buf, "x%02x-", addr);
			save_fcn [addr] (f, buf);
		}
			for (addr = 0; addr < MAX_RAM; addr++)
				if (ram_exists [addr])
				{
					fprintf (f, "%03x: ", addr);
					write_reg (f, ram [addr], WSIZE);
				}
					fclose (f);
	return (1);
}

static void init_periphs (void)
{
	int i;
	for (i = 0; i < MAX_PFAD; i++)
		pf_exists [i] = 0;
}


static int read_rom_file (char *objfn, int keep_src)
{
	int i, x1, x2;
	address p;
	romword opcode;
	int count = 0;
	int verbose = 0;
	FILE *f;
	char buf [80];
	
	if (verbose)
		fprintf (stderr, "ROM file '%s'\n", objfn);
	
	f = fopen (objfn, "r");
	if (! f)
		return (0);
	
	while (fgets (buf, sizeof (buf), f))
    {
		if (sscanf (buf, "%x %x", & x1, &x2) == 2)
		{
			p = x1;
			opcode = x2;
			if (! get_bpt (p))
			{
				fprintf (stderr, "duplicate listing line for address %04x\n", p);
				fprintf (stderr, "orig: %s\n", get_source (p));
				fprintf (stderr, "dup:  %s\n", buf);
			}
			else
			{
				set_ucode (p, 0, opcode);
				set_bpt   (p, 0, 0);
				if (keep_src)
					set_source (p, 0, newstr (& buf [0]));
				count ++;
			}
		}
    }
	
	if (verbose)
		fprintf (stderr, "read %d words from '%s'\n", count, objfn);
	
	fclose (f);
	return (1);
}



static void handle_display (void)
{
	char buf1 [40];
	char buf2 [40];
	
	display_to_buf (buf1);
	ann_to_buf (buf2);
	
	update_display (buf1, buf2);
}


#ifdef USE_TIMER
static void alarm_handler (int signo)
{
//	signal (SIGALRM, alarm_handler);  /* resinstall the signal handler */
}
#endif /* USE_TIMER */

static void sim_inst (void)
{
	int opcode;
	
	opcode = get_ucode (pc);
	
	prev_pc = pc;
	prev_carry = carry;
	carry = 0;
	pc++;
	(* op_fcn [opcode]) (opcode);
	cycle++;
}

#ifdef USE_TIMER
void init_timer(void) {
	struct itimerval itv;
	
	signal (SIGALRM, alarm_handler);
	itv.it_interval.tv_sec	= 0;
	itv.it_interval.tv_usec = 10000;
	itv.it_value.tv_sec		= 0;
	itv.it_value.tv_usec	= 10000;
	setitimer (ITIMER_REAL, &itv, NULL);
}
#endif /* USE_TIMER */

static void init_cpu_registers (void)
{
	int i;
	
	cycle = 0;
	
	/* wake from deep sleep */
	awake = 1;
	pc = 0;
	carry = 1;
	
	for (i = 0; i < WSIZE; i++)
		a [i] = b [i] = c [i] = m [i] = n [i] = 0;
	g [0] = 0;
	g [1] = 0;
	fo = 0;
	op_clear_all_s (0);
	
	p = q = 0;
	pt = & p;
	
	arith_base = 16;
}

int init_nsim(int ram_size, char *rom_f, int trace) {
	
	init_rom ();
	if (! read_rom_file (rom_f, trace)) {
		printf( "unable to read ROM file '%s'\n", rom_f);
		return -1;
	}
	
	init_ops ();
	init_ram ();
	create_ram (0x000, 0x00f);  /* status registers */
	create_ram (0x0c0, 0x0c0 + ram_size - 1);  /* main RAM */
	create_ram (0x040, 0x0bf);  /* extended function module */
	create_ram (0x201, 0x2ef);  /* extended memory module 1 */
	create_ram (0x301, 0x3ef);  /* extended memory module 2 */
	
	init_periphs ();
	init_display ();
	init_phineas ();
	
	init_cpu_registers ();
	
	return 0;
}

typedef unsigned long long int u_time_t; /* in microsecs */
static int handler_flag; 

static void my_utilities_dummy_handler (int sig, siginfo_t *siginfo,
										void *context) {
	handler_flag++;
	return ;
}

int isleep(u_time_t time, int return_on_any_signal) {
	struct itimerval newtv;
	
	sigset_t sigset;
	struct sigaction sigact;
	
	if ( time == 0)
		return 0;
	
	/* block SIGALRM */
	
	sigemptyset (&sigset);
	sigaddset (&sigset, SIGALRM);
	sigprocmask (SIG_BLOCK, &sigset, NULL);
	
	/* set up our handler */
	sigact.sa_sigaction = my_utilities_dummy_handler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = SA_SIGINFO;
	sigaction (SIGALRM, &sigact, NULL);
	
	newtv.it_interval.tv_sec = 0;
	newtv.it_interval.tv_usec = 0;
	newtv.it_value.tv_sec = time / 1000000;
	newtv.it_value.tv_usec = time % 1000000;
	
	if ( setitimer(ITIMER_REAL,&newtv,NULL) < 0 ) {
		perror("setitimer(set)");
		return 1;
	}
	
	sigemptyset (&sigset);
	sigsuspend (&sigset);
	return 0;
	
} 

int run_nsim(char *ram_f) {
    io_count = 10000;
	key_flag = 0;
	
	if (! read_ram_file (ram_f))
		fprintf (stderr, "unable to read RAM file '%s'\n", ram_f);
	
#ifdef USE_TIMER
//	init_timer();
#endif /* USE_TIMER */
	
	while (1) {
		if (awake)
			sim_inst ();
		else if (display_enable) {
			/* light sleep */

//#ifdef AUTO_POWER_OFF
//			/* run display timer if LCD chip is enabled */
//			if ((pf_addr == LCD_DISPLAY) && (--display_timer == 0)) {
//				/* go to deep sleep */
//				display_enable = 0;
//				carry = 1;
//			}
//#endif /* AUTO_POWER_OFF */

			io_count = 0;
		}
		else {
			/* deep sleep */
			io_count = 0;
		}
		io_count--;

		if (io_count <= 0) 
		{
			handle_display ();
			if (handle_keyboard() == KC_EXIT && pc!=0x27f8)     // ??? This pc doesn't recover/restart
			{
				if (ram_f) 
					if (! write_ram_file (ram_f))
						fprintf (stderr, "Error writing RAM file '%s'\n", ram_f);
				return 0;
			}
			
            io_count = 67;      //67;  /* nominal 380 KHz oscillator */
//			printf("pc: %x\n",pc);
#ifdef USE_TIMER
//			pause ();
			isleep(10000,0);
#endif
		}
	}

	return 0;
}

