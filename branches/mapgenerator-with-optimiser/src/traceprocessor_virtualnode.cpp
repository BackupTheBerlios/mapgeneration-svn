/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "traceprocessor.h"

namespace mapgeneration
{
	
	TraceProcessor::VirtualNode::VirtualNode()
	: GPSPoint(), _position_on_trace()
	{
	}
	
	
	TraceProcessor::VirtualNode::VirtualNode(const GPSPoint& point)
	: GPSPoint(point), _position_on_trace()
	{
	}
	
	
	TraceProcessor::VirtualNode&
	TraceProcessor::VirtualNode::operator=(const GeoCoordinate& point)
	{
		GeoCoordinate::operator=(point);
		
		return *this;
	}
	
	
	TraceProcessor::VirtualNode&
	TraceProcessor::VirtualNode::operator=(const GPSPoint& point)
	{
		GPSPoint::operator=(point);
		
		return *this;
	}
		
} // namespace mapgeneration
