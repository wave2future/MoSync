//
// GeoPoint.cpp
//
// Author: Lars Ake Vinberg
//

#include "MapConfig.h"
#include "MemoryMgr.h"
#include "GeoPoint.h"
#include "PixelCoordinate.h"

namespace MAP
{
	//-------------------------------------------------------------------------
	GeoPoint::GeoPoint( ) :
	//-------------------------------------------------------------------------
		mLocation( ),
		mCachedWorldPixelLocation( ),
		mTitle( ),
		mDescription( ),
		mMarkerSize( ),
		mTimestamp( )
	{
	}

	//-------------------------------------------------------------------------
	GeoPoint::GeoPoint( LonLat location, const char* title, const char* description, int markerSize, const DateTime timestamp ) :
	//-------------------------------------------------------------------------
		mLocation( location ),
		mCachedWorldPixelLocation( ),
		mTitle( title ),
		mDescription( description ),
		mMarkerSize( markerSize ),
		mTimestamp( timestamp )
	{
	}

	//-------------------------------------------------------------------------
	GeoPoint::~GeoPoint( )
	//-------------------------------------------------------------------------
	{
	}
}
