/*
	carbonio.c
	
	carbon gui for nsim by Maciej Bartosiak 
 
	This source is distributed under the terms of GNU Public License (GPL) 
	see www.gnu.org for more info.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xio.h"

char quit_flag= 0;

int win_x = 50, win_y = 50;


#pragma mark -

static void draw_display (char *str1, char *str2) {
	printf("%s  %s", str1, str2);

}

static void draw_calc (void) {
}



unsigned int key_to_kode(unsigned int key) {
	switch( key ) {
		case 'O': return 1;
		case 'U': return 2;
		case 'P': return 3;
		case 'A': return 4;
		
		case 'a': return 11;
		case 'b': return 12;
		case 'c': return 13;
		case 'd': return 14;
		case 'e': return 15;
		
		case 'f': return 21;
		case 'g': case 0x1f: return 22;
		case 'h': return 23;
		case 'i': return 24;
		case 'j': return 25;
		
		case 'G': return 31;
		case 'k': return 32;
		case 'l': return 33;
		case 'm': return 34;
		case 'S': return 35;
		
		case 'n': case 0x0d: case 0x03: return 41;
		case 'o': return 43;
		case 'p': return 44;
		case 0x08: return 45;
		
		case 'q': case '-': return 51;
		case 'r': case '7': return 52;
		case 's': case '8': return 53;
		case 't': case '9': return 54;
		
		case 'u': case '+': return 61;
		case 'v': case '4': return 62;
		case 'w': case '5': return 63;
		case 'x': case '6': return 64;
		
		case 'y': case '*': return 71;
		case 'z': case '1': return 72;
		case '=': case '2': return 73;
		case '?': case '3': return 74;
		
		case ':': case '/': return 81;
		case ' ': case '0': return 82;
		case ',': case '.': return 83;
		case 'R': return 84;
	}
	return KC_NONE;
}


int handle_events()
{
}

#pragma mark -
#pragma mark required by nsim

// required by xio.h
void init_user_io (int argc, char *argv[]) {
}

// required by xio.h

// required by xio.h


