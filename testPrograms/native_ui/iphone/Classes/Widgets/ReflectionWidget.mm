//
//  WindowWidget.mm
//  nativeuitest
//
//  Created by Niklas Nummelin on 11/26/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "ReflectionWidget.h"
#import <objc/runtime.h>

@implementation ReflectionWidget

- (id)initWithName:(NSString*)name {
	widgetClass = NSClassFromString(name);
	if(widgetClass == nil) {
		[self dealloc];
		return nil;
	}
	
	view = [[widgetClass alloc] init];
	
	return [super init];
}

- (void)addChild: (IWidget*)child {
	[super addChild:child];	
}

- (void)removeChild: (IWidget*)child {
}

- (int)setPropertyWithKey: (NSString*)key toValue: (NSString*)value {
	Ivar nameVar = class_getInstanceVariable(widgetClass, [key UTF8String]);
	if(nameVar != nil) {
		object_setIvar(widgetClass, nameVar, value);
	} else {
		return MA_WIDGET_ERROR;
	}
	
	return MA_WIDGET_OK;	
}

- (NSString*)getPropertyWithKey: (NSString*)key {
	
	return [super getPropertyWithKey:key];
}

@end
