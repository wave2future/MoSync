/* Copyright (C) 2011 MoSync AB
 
 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License, version 2, as published by
 the Free Software Foundation.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; see the file COPYING.  If not, write to the Free
 Software Foundation, 59 Temple Place - Suite 330, Boston, MA
 02111-1307, USA.
 */

#import "LabelWidget.h"
#include "Platform.h"
#include <helpers/cpp_defs.h>
#include <helpers/CPP_IX_WIDGET.h>
#include <base/Syscall.h>
#include "UIColor-Expanded.h"

typedef enum VerticalAlignment {
    VerticalAlignmentTop,
    VerticalAlignmentMiddle,
    VerticalAlignmentBottom,
} VerticalAlignment;

@interface UILabelWithVerticalAlignment : UILabel {
@private
    VerticalAlignment verticalAlignment_;
}

@property (nonatomic, assign) VerticalAlignment verticalAlignment;

@end

@implementation UILabelWithVerticalAlignment

@synthesize verticalAlignment = verticalAlignment_;

- (id)initWithFrame:(CGRect)frame {
    if (self = [super initWithFrame:frame]) {
        self.verticalAlignment = VerticalAlignmentMiddle;
    }
    return self;
}

- (void)setVerticalAlignment:(VerticalAlignment)verticalAlignment {
    verticalAlignment_ = verticalAlignment;
    [self setNeedsDisplay];
}

- (CGRect)textRectForBounds:(CGRect)bounds limitedToNumberOfLines:(NSInteger)numberOfLines {
    CGRect textRect = [super textRectForBounds:bounds limitedToNumberOfLines:numberOfLines];
    switch (self.verticalAlignment) {
        case VerticalAlignmentTop:
            textRect.origin.y = bounds.origin.y;
            break;
        case VerticalAlignmentBottom:
            textRect.origin.y = bounds.origin.y + bounds.size.height - textRect.size.height;
            break;
        case VerticalAlignmentMiddle:
            // Fall through.
        default:
            textRect.origin.y = bounds.origin.y + (bounds.size.height - textRect.size.height) / 2.0;
    }
    return textRect;
}

-(CGSize)sizeThatFits:(CGSize)_size {
	CGFloat frameWidth = self.frame.size.width;
	CGSize size = [self.text sizeWithFont:self.font constrainedToSize:CGSizeMake(frameWidth,FLT_MAX) lineBreakMode:self.lineBreakMode ];
	return size;
}

-(void)drawTextInRect:(CGRect)requestedRect {
    CGRect actualRect = [self textRectForBounds:requestedRect limitedToNumberOfLines:self.numberOfLines];
    [super drawTextInRect:actualRect];
}
@end


@implementation LabelWidget

- (id)init {	
	UILabel* label = [[[UILabelWithVerticalAlignment alloc] initWithFrame:CGRectMake(0, 0, 200, 60)] retain];
	label.opaque = NO;
	view = label;
	//label.numberOfLines = 0;
	id ret = [super init];	
	[self setAutoSizeParamX:WRAP_CONTENT andY:WRAP_CONTENT];
	return ret;
}

- (int)setPropertyWithKey: (NSString*)key toValue: (NSString*)value {
	if([key isEqualToString:@"text"]) {
		UILabel* label = (UILabel*) view;
		[label setText: value];
		[self layout];
		//[label sizeToFit];
	} 
	else if([key isEqualToString:@"numberOfLines"]) {
		UILabel* label = (UILabel*) view;
		label.numberOfLines = [value intValue];
		[self layout];
	} 
	else if([key isEqualToString:@"textHorizontalAlignment"]) {
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
	else if([key isEqualToString:@"textVerticalAlignment"]) {
		UILabelWithVerticalAlignment* label = (UILabelWithVerticalAlignment*) view;
		if([value isEqualToString:@"top"]) {
			[label setVerticalAlignment:VerticalAlignmentTop];
		}
		else if([value isEqualToString:@"center"]) {
			[label setVerticalAlignment:VerticalAlignmentMiddle];
		}
		else if([value isEqualToString:@"bottom"]) {
			[label setVerticalAlignment:VerticalAlignmentBottom];
		}		
	}	
	else if([key isEqualToString:@"fontColor"]) {
		UILabel* label = (UILabel*) view;
		UIColor* color = [UIColor colorWithHexString:value];
		if(!color) return MAW_RES_INVALID_PROPERTY_VALUE;
		label.textColor = color;
	}	
	else if([key isEqualToString:@"fontSize"]) {
		UILabel* label = (UILabel*) view;	
		float fontSize = [value floatValue];
		label.font = [UIFont boldSystemFontOfSize:fontSize];
		[self layout];		
	}
	else {
		return [super setPropertyWithKey:key toValue:value];
	}
	return MAW_RES_OK;	
}

- (NSString*)getPropertyWithKey: (NSString*)key {
	if([key isEqualToString:@"text"]) {
		UILabel* label = (UILabel*) view;		
		return label.text;
	} else {	
		return [super getPropertyWithKey:key];
	}
}

@end
