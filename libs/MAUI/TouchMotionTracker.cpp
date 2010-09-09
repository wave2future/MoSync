/* Copyright (C) 2010 MoSync AB

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

#include "TouchMotionTracker.h"
#include <madmath.h>

namespace MAUI {

TouchMotionTracker::TouchMotionTracker() : mStarted(false) {
}

void TouchMotionTracker::reset() {
	mStarted = false;
}

void normalize(double &x, double &y) {
	double len = sqrt(x*x+y*y);
	if(len==0)len=0.0001;
	double len_recip = 1.0/len;
	x*=len_recip;
	y*=len_recip;
}

void TouchMotionTracker::addPoint(MAPoint2d p) {
	if(!mStarted) {
		mLast = mStart = p;
		mStartTime = maGetMilliSecondCount();
		mDirx = 0;
		mDiry = 0;
	} else {
		double dx = p.x-mLast.x;
		double dy = p.y-mLast.y;
		mDirx+=dx;
		mDiry+=dy;
		mLast = p;
	}
}

void TouchMotionTracker::calculateVelocity(double &directionX, double &directionY, double &velocityX, double velocityY) {

	// take unit into acount ? (pixels per second or ms).
	double time = maGetMilliSecondCount()-mStartTime;

	float friction = pow(0.9, time/(1000.0/30.0));

	mDirx*=friction;
	mDiry*=friction;

	double dx = mDirx;
	double dy = mDiry;

	// normalize directions
	double len = sqrt(dx*dx+dy*dy);

	if(len == 0.0) {
		velocityX = velocityY = 0.0;
		directionX = directionY = 0.0;
		return;
	}

	double len_recip = 1.0/len;
	dx*=len_recip;
	dy*=len_recip;

	directionX = dx;
	directionY = dy;

	if(time == 0) {
		velocityX = velocityY = 0.0;
		return;
	}

	velocityX = mDirx/time;
	velocityY = mDiry/time;
}


} // namespace MAUI