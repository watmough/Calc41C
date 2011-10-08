//
//  Calc41CAppDelegate.h
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

#import <UIKit/UIKit.h>

@class Calc41CViewController;

@interface Calc41CAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    Calc41CViewController *viewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet Calc41CViewController *viewController;

@end

