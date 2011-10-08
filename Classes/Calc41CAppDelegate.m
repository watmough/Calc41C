//
//  Calc41CAppDelegate.m
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

#import "Calc41CAppDelegate.h"
#import "Calc41CViewController.h"

extern char * ram_file_name;
extern int write_ram_file (char *ramfn);

#pragma mark -
#pragma mark Calc41CAppDelegate Implementation 

@implementation Calc41CAppDelegate

@synthesize window;
@synthesize viewController;

//--------------------------------------------------------------------------------
// applicationDidFinishlaunching:
//--------------------------------------------------------------------------------
- (void)applicationDidFinishLaunching:(UIApplication *)application {    
    
    // Override point for customization after app launch    
    [window addSubview:viewController.view];
    [window makeKeyAndVisible];
}

//--------------------------------------------------------------------------------
// applicationWillResignActive:
//--------------------------------------------------------------------------------
- (void)applicationWillResignActive:(UIApplication *)application
{
	// save memory
	if( ram_file_name) {
		if (! write_ram_file (ram_file_name))
		{
			fprintf (stderr, "Error writing RAM file '%s'\n", ram_file_name);
		}
	}
	
}

//--------------------------------------------------------------------------------
// applicationWillTerminate:
//--------------------------------------------------------------------------------
- (void)applicationWillTerminate:(UIApplication *)app
{
	// save memory
	if( ram_file_name) {
		if (! write_ram_file (ram_file_name))
		{
			fprintf (stderr, "Error writing RAM file '%s'\n", ram_file_name);
		}
	}
	
}

//--------------------------------------------------------------------------------
// dealloc
//--------------------------------------------------------------------------------
- (void)dealloc
{
    [viewController release];
    [window release];

    [super dealloc];
}

@end







