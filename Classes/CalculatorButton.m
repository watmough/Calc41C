//
//  CalculatorButton.m
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

#import "CalculatorButton.h"

#pragma mark -
#pragma mark CalculatorButton implementation

@implementation CalculatorButton

//--------------------------------------------------------------------------------
// initWithFrame:orange:
// create a button.
//--------------------------------------------------------------------------------
- (id)initWithFrame:(CGRect)frame orange:(BOOL)orange {
	if (self = [super initWithFrame:frame]) {

		// Initialization code
		[super setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
		[super setTitleColor:[UIColor blueColor] forState:UIControlStateHighlighted];

		if( orange )
			[self setBackgroundImage:[UIImage imageNamed:@"button60orange.png"] forState:UIControlStateNormal];
		else
			[self setBackgroundImage:[UIImage imageNamed:@"button60.png"] forState:UIControlStateNormal];
	}
	return self;
}

//--------------------------------------------------------------------------------
// dealloc
//--------------------------------------------------------------------------------
- (void)dealloc {
	[super dealloc];
}

//--------------------------------------------------------------------------------
// setTop:
// set top title of key
//--------------------------------------------------------------------------------
- (void) setTop:(NSString *)top; {
	
	// Set orange key text
	topTitle = top;
}

//--------------------------------------------------------------------------------
// setMain:
// set main title of key
//--------------------------------------------------------------------------------
- (void) setMain:(NSString *)mainText; {
	
	// Set white key text
	mainTitle = mainText;
}

//--------------------------------------------------------------------------------
// setLower:
// set lower title of key
//--------------------------------------------------------------------------------
- (void) setLower:(NSString *)lower; {
	
	// Set blue key text
	lowerTitle = lower;
}

//--------------------------------------------------------------------------------
// addLabels:
// create labels and add within key view.
//--------------------------------------------------------------------------------
- (void)addLabels
{
	// get fonts
	UIFont * topFont = [UIFont fontWithName:@"Helvetica-Bold" size:14];
	UIFont * mainFont = [UIFont fontWithName:@"Helvetica-Bold" size:14];
	UIFont * lowerFont = [UIFont fontWithName:@"Helvetica-Bold" size:11];

	// add orange text as label
	CGRect b = [self bounds];
	float yt = b.size.height*0.06;
	UILabel * tlabel = [[UILabel alloc] initWithFrame:CGRectMake(0, yt, b.size.width, 14)];
	tlabel.font = topFont;
	tlabel.text = topTitle;
	tlabel.backgroundColor = [UIColor clearColor];
	tlabel.textColor = [UIColor orangeColor];
	tlabel.textAlignment = UITextAlignmentCenter;
	[self addSubview:tlabel];
	
	// add main text as label
	float ym = b.size.height/2.7;
	UILabel * mlabel = [[UILabel alloc] initWithFrame:CGRectMake(0, ym, b.size.width, 16)];
	mlabel.font = mainFont;
	mlabel.text = mainTitle;
	mlabel.backgroundColor = [UIColor clearColor];
	mlabel.textColor = [UIColor whiteColor];
	mlabel.textAlignment = UITextAlignmentCenter;
	[self addSubview:mlabel];
	
	// add blue text as label
	float yl = b.size.height*0.66;
	UILabel * llabel = [[UILabel alloc] initWithFrame:CGRectMake(0, yl, b.size.width, 14)];
	llabel.font = lowerFont;
	llabel.text = lowerTitle;
	llabel.backgroundColor = [UIColor clearColor];
	llabel.textColor = [UIColor cyanColor];
	llabel.textAlignment = UITextAlignmentCenter;
	[self addSubview:llabel];
}

//--------------------------------------------------------------------------------
// drawRect:
// draw key in different selection states.
//--------------------------------------------------------------------------------
- (void)drawRect:(CGRect)rect {
	// Drawing code
	if( !labels ) {
		labels=YES;
		[self addLabels];
	}
}

@end





