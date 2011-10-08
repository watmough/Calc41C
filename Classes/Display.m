//
//  Display.m
//  Calc41C
//
// Copyright 2011 Jonathan A. Watmough
//
// Calc41C is free software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License version 2 as published by the Free
// Software Foundation.  Note that I am not granting permission to redistribute
// or modify NSIM under the terms of any later version of the General Public
// License.
// 
// This program is distributed in the hope that it will be useful (or at least
// amusing), but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
// Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program (in the file "COPYING"); if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#import "Display.h"

#pragma mark -
#pragma mark Display Implementation

extern pthread_mutex_t mutex;
extern BOOL key_click;

@implementation Display

@synthesize glyphs;
@synthesize display;

//--------------------------------------------------------------------------------
// touchesEnded:withEvent:
// toggle key clicks on/off when user taps display.
//--------------------------------------------------------------------------------
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	if( [[NSUserDefaults standardUserDefaults] boolForKey:@"KeyClick"] )
		key_click=NO, [[NSUserDefaults standardUserDefaults] setObject:@"NO" forKey:@"KeyClick"];
	else
		key_click=YES, [[NSUserDefaults standardUserDefaults] setObject:@"YES" forKey:@"KeyClick"];
}

//--------------------------------------------------------------------------------
// initWithFrame:
// create display area and read in glyphs.
//--------------------------------------------------------------------------------
- (id)initWithFrame:(CGRect)frame {
    if (self = [super initWithFrame:frame]) {
        // Initialization code
		
		// read in glyphs
		NSMutableArray * g = [[[NSMutableArray alloc] init] autorelease];
		UIImage * blank = [UIImage imageNamed:@"000.png"];
		for( int i=0 ; i<128 ; ++i )
		{
			NSString * name = [NSString stringWithFormat:@"%03d.png",i];
			UIImage * image = [UIImage imageNamed:name];
			[g insertObject:(image ? image : blank) atIndex:i];
		}
		self.glyphs = g;
    }

    return self;
}


//--------------------------------------------------------------------------------
// dealloc
//--------------------------------------------------------------------------------
- (void)dealloc {
	
	// releases
	self.display = nil;
	self.ann = nil;
	self.glyphs = nil;
	
    [super dealloc];
}

//--------------------------------------------------------------------------------
// setDisplay:ann:
// set new strings for the display and mark for redraw.
//--------------------------------------------------------------------------------
- (void)setDisplay:(NSString *)theDisplay ann:(NSString *)theAnn
{
	if (display!=theDisplay) {
		[display release];
		display = [theDisplay retain];
	}

	if (ann!=theAnn) {
		[ann release];
		ann = [theAnn retain];
	}
	
	[self setNeedsDisplay];
}

//--------------------------------------------------------------------------------
// drawRect:
// draw the actual display.
//--------------------------------------------------------------------------------
- (void)drawRect:(CGRect)rect 
{
    // Drawing code
	const char * string = display ? [display UTF8String] : "";
	float pos = 0;
	float step = 0;
	for( int i=0 ; i<strlen(string) ; ++i )
	{
		if( i%2==0 )
		{
			// digit / letter
			step = 16.0;
			[[glyphs objectAtIndex:*(string+i)] drawInRect:CGRectMake(pos, 0, step, 53.0*step/40.0)];
			pos += step;
		}
		else
		{
			// punctuation
			step = 15.0*16/40;
			[[glyphs objectAtIndex:*(string+i)] drawInRect:CGRectMake(pos, 0, step, 70.0*step/15.0)];
			pos += step;
		}
	}
	
	// draw
	static const char * anns[12] = {"BAT","USER","GRAD","RAD","SHIFT","0","1","2","3","4","PRGM","ALPHA"};
	static const int poss[12] =    {0    ,60    ,95   ,100  ,130, 170,175,180,185,190   ,200   ,240};
	const char * ccann = [ann UTF8String];
	for( int i=0 ; i<12 ; ++i )
	{
		if( ccann && strstr(ccann, anns[i]) )
		{
			[[NSString stringWithCString:anns[i] encoding:NSUTF8StringEncoding] drawAtPoint:CGPointMake(poss[i],23) withFont:[UIFont systemFontOfSize:10]];
			if( i==2 )
				++i;
		}
	}
	
	// draw note to denote key click
	if( key_click )
		[@"♪" drawAtPoint:CGPointMake(0,21) withFont:[UIFont systemFontOfSize:13]];
}

@end





