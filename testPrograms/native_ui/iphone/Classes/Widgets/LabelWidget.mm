//
//  LabelWidget.mm
//  nativeuitest
//
//  Created by Niklas Nummelin on 11/26/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "LabelWidget.h"


@implementation LabelWidget

- (id)init {	
	UILabel* label = [[[UILabel alloc] initWithFrame:CGRectMake(0, 0, 100, 60)] retain];
	label.opaque = NO;
	view = label;		
	
	return [super init];	
}

- (void)addChild: (IWidget*)child {
	[super addChild:child];
}

- (void)removeChild: (IWidget*)child {
}

- (int)setPropertyWithKey: (NSString*)key toValue: (NSString*)value {
	if([key isEqualToString:@"text"]) {
		UILabel* label = (UILabel*) view;
		[label setText: value];
		[label sizeToFit];
	} 
	else if([key isEqualToString:@"horizontalAlignment"]) {
		UILabel* label = (UILabel*) view;
		if([value isEqualToString:@"left"]) {
			label.textAlignment = UITextAlignmentLeft;
		}
		else if([value isEqualToString:@"center"]) {
			label.textAlignment = UITextAlignmentCenter;
		}
		else if([value isEqualToString:@"right"]) {
			label.textAlignment = UITextAlignmentRight;
		}		
	}
	else if([key isEqualToString:@"verticalAlignment"]) {
		// This isn't trivial on iphone.
		
		//UILabel* label = (UILabel*) view;
		//if([value isEqualToString:@"center"]) {
		//}
	}	
	else if([key isEqualToString:@"fontColor"]) {
		UILabel* label = (UILabel*) view;	
		label.textColor = [UIColor colorWithHexString:value];
	}	
	else if([key isEqualToString:@"fontSize"]) {
		UILabel* label = (UILabel*) view;	
		float fontSize = [value floatValue];
		label.font = [UIFont boldSystemFontOfSize:fontSize];
	}
	else {
		return [super setPropertyWithKey:key toValue:value];
	}
	return MA_WIDGET_OK;	
}

- (NSString*)getPropertyWithKey: (NSString*)key {
	
	return [super getPropertyWithKey:key];
}

@end
