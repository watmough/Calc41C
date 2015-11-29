//
//  Calc41CViewController.m
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

#include <assert.h>
#include <pthread.h>

#include "xio.h"
#import "Display.h"
#import "CalculatorButton.h"

// .h file
int init_nsim(int ram_size, char *rom_f, int trace);
int run_nsim(char *ram_f);

int key_pressed = KC_NONE;
BOOL key_click;
char disp_buf1 [40];
char disp_buf2 [40];

pthread_mutex_t mutex;

int check_keyboard (void) 
{	
	// Catch any key presses
	int key_to_return = key_pressed;
	
	return key_to_return;
}

void update_display (char *str1, char *str2) 
{
	// occasionally you'll see this be in the wrong place
	// sadly a factor of how the display is updated by the emulation
//	printf("update_display: %s\n", str1);
	
	if ((strcmp (disp_buf1, str1) != 0) || (strcmp (disp_buf2, str2) != 0)) {
		pthread_mutex_lock(&mutex);
		strcpy (disp_buf1, str1);
		strcpy (disp_buf2, str2);
		pthread_mutex_unlock(&mutex);
	}
}

#include <AudioToolbox/AudioToolbox.h>
#import "Calc41CViewController.h"
#import "CalculatorButton.h"

// Set up the pieces needed to play a sound.
SystemSoundID    ssidCli;
SystemSoundID    ssidIck;


#pragma mark -
#pragma mark Calc41CViewController Implementation 

@implementation Calc41CViewController

//--------------------------------------------------------------------------------
// dealloc
//--------------------------------------------------------------------------------
- (void)dealloc 
{
	[super dealloc];
}

//--------------------------------------------------------------------------------
// initialize
// called at startup. save key click status.
//--------------------------------------------------------------------------------
+ (void)initialize
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSDictionary *appDefaults = [NSDictionary
								 dictionaryWithObject:@"YES" forKey:@"KeyClick"];	
    [defaults registerDefaults:appDefaults];
}


//--------------------------------------------------------------------------------
// keyDown:
//--------------------------------------------------------------------------------
- (void)keyDown:(id)sender
{
    // Play the sound file.
	if( key_click ) {
		AudioServicesPlaySystemSound (ssidCli); 	
	}
	
	// redisplay key
	[sender setNeedsDisplay];
	
	// tag holds the key code
	key_pressed = [sender tag];
}

//--------------------------------------------------------------------------------
// keyUp:
// having distinct key up/dn events lets the emulator do it's thing
// with nulling out a key that is held down.
//--------------------------------------------------------------------------------
- (void)keyUp:(id)sender
{
	if( key_click ) {
//		AudioServicesPlaySystemSound (ssidIck); 	
	}
	
	[sender setNeedsDisplay];
	key_pressed = KC_NONE;
}

//--------------------------------------------------------------------------------
// makeButton
// create a standard calculator button.
//--------------------------------------------------------------------------------
- (CalculatorButton *)makeButton:(BOOL)orange
{
	// create a UIButton (UIButtonTypeRoundedRect)
	CalculatorButton * button = [[CalculatorButton alloc] initWithFrame:CGRectMake(0.0, 0.0, /*width*/30, /*height*/20) 
																 orange:orange];
	button.backgroundColor = [UIColor clearColor];
	[button addTarget:self action:@selector(keyDown:) forControlEvents:UIControlEventTouchDown];
	[button addTarget:self action:@selector(keyUp:) forControlEvents:UIControlEventTouchUpInside|UIControlEventTouchDragExit];
	return [button autorelease];
}

//--------------------------------------------------------------------------------
// buttonText:
// return appropriate HP41C keycap text by keyboard coordinate.
//--------------------------------------------------------------------------------
- (NSString *)buttonText:(int)pos
{
	// Get row and col
	int row = pos / 5;
	int col = pos % 5;
	
	if( row==0 )
	{
		switch (col) {
			case 0:	return @"on"; break;
			case 1:	return @"user"; break;
			case 2: return @"prgm"; break;
			case 3:	return @"alpha"; break;
			default: break;
		}
	}
	if( row==1 )
	{
		switch (col) {
			case 0:	return @"∑+"; break;
			case 1:	return @"1/x"; break;
			case 2: return @"√x"; break;
			case 3:	return @"log"; break;
			case 4:	return @"ln"; break;
			default: break;
		}
	}
	if( row==2 )
	{
		switch (col) {
			case 0:	return @"x<>y"; break;
			case 1:	return @"rv"; break;
			case 2: return @"sin"; break;
			case 3:	return @"cos"; break;
			case 4:	return @"tan"; break;
			default: break;
		}
	}
	if( row==3 )
	{
		switch (col) {
			case 0:	return @"shift"; break;
			case 1:	return @"xeq"; break;
			case 2: return @"sto"; break;
			case 3:	return @"rcl"; break;
			case 4:	return @"sst"; break;
			default: break;
		}
	}
	if( row==4 )
	{
		switch (col) {
			case 0:	return @"Enter"; break;
			case 1:	return @"♘"; break;
			case 2: return @"chs"; break;
			case 3:	return @"eex"; break;
			case 4:	return @"←"; break;
			default: break;
		}
	}
	if( row==5 )
	{
		switch (col) {
			case 0:	return @"-"; break;
			case 1:	return @"7"; break;
			case 2: return @"8"; break;
			case 3:	return @"9"; break;
			default: break;
		}
	}
	if( row==6 )
	{
		switch (col) {
			case 0:	return @"+"; break;
			case 1:	return @"4"; break;
			case 2: return @"5"; break;
			case 3:	return @"6"; break;
			default: break;
		}
	}
	if( row==7 )
	{
		switch (col) {
			case 0:	return @"x"; break;
			case 1:	return @"1"; break;
			case 2: return @"2"; break;
			case 3:	return @"3"; break;
			default: break;
		}
	}
	if( row==8 )
	{
		switch (col) {
			case 0:	return @"÷"; break;
			case 1:	return @"0"; break;
			case 2: return @"."; break;
			case 3:	return @"r/s"; break;
			default: break;
		}
	}
	
	return @"";
}

//--------------------------------------------------------------------------------
// buttonText:
// return appropriate HP41C keycap text by keyboard coordinate.
//--------------------------------------------------------------------------------
- (NSString *)buttonTextTop:(int)pos
{
	// Get row and col
	int row = pos / 5;
	int col = pos % 5;
	
	if( row==0 )
	{
		switch (col) {
			case 0:	return NULL; break;
			case 1:	return NULL; break;
			case 2: return NULL; break;
			case 3:	return NULL; break;
			default: break;
		}
	}
	if( row==1 )
	{
		switch (col) {
			case 0:	return @"∑-"; break;
			case 1:	return @"y^x"; break;
			case 2: return @"x^2"; break;
			case 3:	return @"10^x"; break;
			case 4:	return @"e^x"; break;
			default: break;
		}
	}
	if( row==2 )
	{
		switch (col) {
			case 0:	return @"CL∑"; break;
			case 1:	return @"%"; break;
			case 2: return @"sin-1"; break;
			case 3:	return @"cos-1"; break;
			case 4:	return @"tan-1"; break;
			default: break;
		}
	}
	if( row==3 )
	{
		switch (col) {
			case 0:	return NULL; break;
			case 1:	return @"asn"; break;
			case 2: return @"lbl"; break;
			case 3:	return @"gto"; break;
			case 4:	return @"bst"; break;
			default: break;
		}
	}
	if( row==4 )
	{
		switch (col) {
			case 0:	return @"catalog"; break;
			case 1:	return NULL; break;
			case 2: return @"isg"; break;
			case 3:	return @"rtn"; break;
			case 4:	return @"clϰ/a"; break;
			default: break;
		}
	}
	if( row==5 )
	{
		switch (col) {
			case 0:	return @"x=y?"; break;
			case 1:	return @"sf"; break;
			case 2: return @"cf"; break;
			case 3:	return @"fs?"; break;
			default: break;
		}
	}
	if( row==6 )
	{
		switch (col) {
			case 0:	return @"x<=y?"; break;
			case 1:	return @"beep"; break;
			case 2: return @"p->r"; break;
			case 3:	return @"r->p"; break;
			default: break;
		}
	}
	if( row==7 )
	{
		switch (col) {
			case 0:	return @"x>y?"; break;
			case 1:	return @"fix"; break;
			case 2: return @"sci"; break;
			case 3:	return @"eng"; break;
			default: break;
		}
	}
	if( row==8 )
	{
		switch (col) {
			case 0:	return @"x=0?"; break;
			case 1:	return @"pi"; break;
			case 2: return @"last ϰ"; break;
			case 3:	return @"view"; break;
			default: break;
		}
	}
	
	return @"";
}

//--------------------------------------------------------------------------------
// buttonText:
// return appropriate HP41C keycap text by keyboard coordinate.
//--------------------------------------------------------------------------------
- (NSString *)buttonTextLower:(int)pos
{
	// Get row and col
	int row = pos / 5;
	int col = pos % 5;
	
	if( row==0 )
	{
		switch (col) {
			case 0:	return NULL; break;
			case 1:	return NULL; break;
			case 2: return NULL; break;
			case 3:	return NULL; break;
			default: break;
		}
	}
	if( row==1 )
	{
		switch (col) {
			case 0:	return @"A"; break;
			case 1:	return @"B"; break;
			case 2: return @"C"; break;
			case 3:	return @"D"; break;
			case 4:	return @"E"; break;
			default: break;
		}
	}
	if( row==2 )
	{
		switch (col) {
			case 0:	return @"F"; break;
			case 1:	return @"G"; break;
			case 2: return @"H"; break;
			case 3:	return @"I"; break;
			case 4:	return @"J"; break;
			default: break;
		}
	}
	if( row==3 )
	{
		switch (col) {
			case 0:	return NULL; break;
			case 1:	return @"K"; break;
			case 2: return @"L"; break;
			case 3:	return @"M"; break;
			case 4:	return NULL; break;
			default: break;
		}
	}
	if( row==4 )
	{
		switch (col) {
			case 0:	return @"N"; break;
			case 1:	return NULL; break;
			case 2: return @"O"; break;
			case 3:	return @"P"; break;
			case 4:	return NULL; break;
			default: break;
		}
	}
	if( row==5 )
	{
		switch (col) {
			case 0:	return @"Q"; break;
			case 1:	return @"R"; break;
			case 2: return @"S"; break;
			case 3:	return @"T"; break;
			default: break;
		}
	}
	if( row==6 )
	{
		switch (col) {
			case 0:	return @"U"; break;
			case 1:	return @"V"; break;
			case 2: return @"W"; break;
			case 3:	return @"X"; break;
			default: break;
		}
	}
	if( row==7 )
	{
		switch (col) {
			case 0:	return @"Y"; break;
			case 1:	return @"Z"; break;
			case 2: return @"="; break;
			case 3:	return @"?"; break;
			default: break;
		}
	}
	if( row==8 )
	{
		switch (col) {
			case 0:	return @":"; break;
			case 1:	return @"space"; break;
			case 2: return @","; break;
			case 3:	return NULL; break;
			default: break;
		}
	}
	
	return @"";
}


pthread_attr_t  attr;
pthread_t       posixThreadID;
int             returnVal;
pid_t			thread;

//--------------------------------------------------------------------------------
// myAction:
// ??? some kind of watchdog timer ??? 
// not super sure what I was thinking here.
//--------------------------------------------------------------------------------
- (void)myAction:(NSTimer*)timer 
{
	// update the display
	Display * display = (Display *)[self.view viewWithTag:(int)-1];
	if( display )
	{
		pthread_mutex_lock(&mutex);
//		display.text = [NSString stringWithFormat:@"Button %d was pressed.", [sender tag]];
		[display setDisplay:[NSString stringWithFormat:@"%s", disp_buf1] ann:[NSString stringWithFormat:@"%s", disp_buf2]];
		pthread_mutex_unlock(&mutex);
	}
	
	// bang the nsim thread just in case...
	// since it seems to lose SIGALRM events. I should really be tracking a counter or
	// something instead of this.
	kill(thread, SIGALRM);
}

char * ram_file_name = 0;
//- (void)startNSIM:(id)obj
//{
//	// need to pass in ram file
//	run_nsim(ram_file_name);
//}

//--------------------------------------------------------------------------------
// PosixThreadMainRoutine
//--------------------------------------------------------------------------------
void* PosixThreadMainRoutine(void* data)
{
	// cache the thread PID, and start up nsim
	thread = getpid();
	run_nsim(ram_file_name);
	
    return NULL;
}

//--------------------------------------------------------------------------------
// LaunchThread
//--------------------------------------------------------------------------------
void LaunchThread()
{
	// block sig alarm on this thread
//	sigset_t blocked;
//	sigemptyset(&blocked);
//	sigaddset(&blocked, SIGALRM);
//	pthread_sigmask(SIG_BLOCK, &blocked, NULL);
    
	// Create the thread using POSIX routines.
    returnVal = pthread_attr_init(&attr);
    assert(!returnVal);
    returnVal = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    assert(!returnVal);
	
    int     threadError = pthread_create(&posixThreadID, &attr, &PosixThreadMainRoutine, NULL);
	
    returnVal = pthread_attr_destroy(&attr);
    assert(!returnVal);
    if (threadError != 0)
    {
		// Report an error.
    }
}


/*
- (BOOL)prefersStatusBarHidden
{
    return YES;
}
*/

//--------------------------------------------------------------------------------
// viewDidLoad
//--------------------------------------------------------------------------------
- (void)viewDidLoad
{
/*  if ([self respondsToSelector:@selector(setNeedsStatusBarAppearanceUpdate)]) {
        // iOS 7
        [self prefersStatusBarHidden];
        [self performSelector:@selector(setNeedsStatusBarAppearanceUpdate)];
    }
*/    
	// init access to mutex used for display data
	pthread_mutex_init(&mutex, NULL);
	
	[super viewDidLoad];
	
	// get key click from prefs
	key_click = [[NSUserDefaults standardUserDefaults] boolForKey:@"KeyClick"];
	
	// Init display buffers
	disp_buf1 [0] ='\0';
	disp_buf2 [0] ='\0';

	// load rom
	NSString *romPath;
	NSBundle *thisBundle = [NSBundle bundleForClass:[self class]];
	if (!(romPath = [thisBundle pathForResource:@"nsim" ofType:@"rom"]))  {
		NSLog(@"Calc41C: unable to find rom");
	}

	// Get URL to the clicks
	CFURLRef urlRefCli = CFURLCreateWithFileSystemPath (
						 kCFAllocatorDefault,
						 (CFStringRef)[thisBundle pathForResource:@"hp65_4_1" ofType:@"aiff"],
						 kCFURLPOSIXPathStyle,
						 FALSE);
	CFURLRef urlRefIck = CFURLCreateWithFileSystemPath (
						 kCFAllocatorDefault,
						 (CFStringRef)[thisBundle pathForResource:@"hp65_4_2" ofType:@"aiff"],
						 kCFURLPOSIXPathStyle,
						 FALSE);
	
	// create a system sound ID to represent the sound file
	AudioServicesCreateSystemSoundID (urlRefCli, &ssidCli);
	AudioServicesCreateSystemSoundID (urlRefIck, &ssidIck);
	
	CFRelease(urlRefCli);
	CFRelease(urlRefIck);

	// Initialize nsim
	int ram_size = 320;
	init_nsim(ram_size, [romPath cStringUsingEncoding:NSASCIIStringEncoding] , 0/*trace*/);

    // get path to saved ram file
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    NSString *path = [documentsDirectory stringByAppendingPathComponent:@"savedram"];
    const char * pathChars = [path UTF8String];
    ram_file_name = (char *)malloc(PATH_MAX);
    strncpy(ram_file_name, pathChars, strlen(pathChars)+1);
    
    // start up nsim thread
    LaunchThread();
    
    // resize view to screen size
    self.view.frame = CGRectMake(0, 0, [[UIScreen mainScreen] bounds].size.width, [[UIScreen mainScreen] bounds].size.height);
    
    // get bounds
    CGRect b = self.view.bounds;
    NSLog(@"Bounds: xy %f %f  wh %f %f\n",b.origin.x,b.origin.y,b.size.width,b.size.height);
    
    // calculate start and height of button rows
    float sy = 64;
    float hy = (b.size.height-sy) / 9;
    
	// create UI
	for( int row = 0 ; row < 9 ; ++row )
	{
		for( int col = 0 ; (row<5 && col< 5) || (row>4 && col<4) ; ++col )
		{
			// Skip last button of first row
			if( row==0 && col==4 )
				continue;
			
			// Skip second part of enter key before we can waste memory on it
			if( row==4 && col==1 )
				continue;
			
			// Create a button
			CalculatorButton * button = [self makeButton:(row==3 && col==0)];
			button.tag = 10*(row+0)+col+1;
			[button setMain:[self buttonText:(5*row+col)]];
			[button setTop:[self buttonTextTop:(5*row+col)]];
			[button setLower:[self buttonTextLower:(5*row+col)]];

			// Button size
			button.frame = CGRectMake(0.0, 0.0, /*width*/54, /*height*/44);

			// Bigger number keys
			if( row>= 5 ) {
				button.frame = CGRectMake(0.0, 0.0, /*width*/60, /*height*/48);
			}
/*
            // Move it to the right spot
            if( row==0 && col<2 )
                button.center = CGPointMake(40+col*60, 53);
            else if( row==0 )
                button.center = CGPointMake(110+col*60, 53);
            else if( row<5 )
                button.center = CGPointMake(40+col*60, 53+row*47);
            else
                button.center = CGPointMake(40+col*80, 242+(row-4)*47);
*/
            // Move it to the right spot
            if( row==0 && col<2 )
                button.center = CGPointMake(40+col*60, sy+hy/4);
            else if( row==0 )
                button.center = CGPointMake(110+col*60, sy+hy/4);
            else if( row<5 )
                button.center = CGPointMake(40+col*60, sy+row*hy+hy/4);
            else
                button.center = CGPointMake(40+col*80, sy+row*hy+hy/4);
            
			// Double width enter key
			if( row==4 && col ==0 )
			{
				button.frame = CGRectMake(0.0, 0.0, /*width*/105, /*height*/44);
				button.center = CGPointMake(60, sy+row*hy+hy/4);
			}

			// Add button as subview
			[[self view] addSubview:button];
		}
	}
	
	[[self view] setUserInteractionEnabled:TRUE];
	
	// Kick off a timer that fires at 1/10th sec intervals
	[NSTimer scheduledTimerWithTimeInterval:0.1 target:self selector:@selector(myAction:) userInfo:NULL repeats:TRUE];

	// create display (renders custom glyphs)
	Display * mainDisplay = [[Display alloc] initWithFrame:CGRectMake(20.0, 0.0, 280, 40.0)];
	mainDisplay.center = CGPointMake(160, 40);
	mainDisplay.backgroundColor = [UIColor clearColor];
	mainDisplay.tag = -1;

    [[self view] addSubview:mainDisplay];
	[[self view] setNeedsDisplay];
    
    key_pressed = KC_NONE;
}

//--------------------------------------------------------------------------------
// shouldAutorotateToInterfaceOrientation:
//--------------------------------------------------------------------------------
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
	// Return YES for supported orientations
	return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

//--------------------------------------------------------------------------------
// didReceiveMemoryWarning
//--------------------------------------------------------------------------------
- (void)didReceiveMemoryWarning 
{
	[super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview

	// Release anything that's not essential, such as cached data
}

@end
