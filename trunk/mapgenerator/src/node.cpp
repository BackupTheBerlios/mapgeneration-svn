/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "node.h"
#include "util/constants.h"
#include "util/mlog.h"

using namespace mapgeneration_util;


namespace mapgeneration
{

	Node::Node()
	: Direction(), GeoCoordinate(), _edge_ids(), _weight(1)
	{
	}
	
	
	Node::Node(const Node& node)
	: Direction(node), GeoCoordinate(node), _edge_ids(node._edge_ids), 
		_weight(node._weight)
	{
	}
	
			
	Node::Node(const GPSPoint& gps_point)
	: Direction(gps_point), GeoCoordinate(gps_point), _edge_ids(), _weight(1)
	{
	}
	
	
	bool
	Node::is_on_edge_id(unsigned int edge_id) const
	{
		std::vector<unsigned int>::const_iterator iter = _edge_ids.begin();
		std::vector<unsigned int>::const_iterator iter_end = _edge_ids.end();
		
		for (; iter != iter_end; ++iter)
		{
			if (*iter == edge_id)
				return true;
		}
		
		return false;
	}
	
	
	void
	Node::merge(GPSPoint& gps_point)
	{
		set_latitude((get_latitude() * (double)_weight + gps_point.get_latitude()) / (_weight +1));
		set_longitude((get_longitude() * (double)_weight + gps_point.get_longitude()) / (_weight +1));
		set_altitude((get_altitude() * (double)_weight + gps_point.get_altitude()) / (_weight +1));

		double old_direction = get_direction();
		double merge_direction = gps_point.get_direction();
		double difference = merge_direction - old_direction;			

		if (difference > PI) merge_direction -= 2 * PI;
		else if (difference < -PI) old_direction -= 2 * PI;

		if ((old_direction < 0) || (merge_direction < 0))
		{
			old_direction += 2 * PI;
			merge_direction += 2 * PI;
		}

		double new_direction = (old_direction * (double)_weight + merge_direction) / (_weight + 1);
		if (new_direction >= 2 * PI) new_direction -= 2 * PI;
		else if (new_direction < 0) new_direction += 2 * PI;
		set_direction(new_direction);

		++_weight;
	}
	
	
	Node&
	Node::operator=(const Node& node)
	{
		GeoCoordinate::operator=(node);
		Direction::operator=(node);

		_edge_ids = node._edge_ids;

		_weight = node._weight;
		
		return *this;
	}


	void
	Node::renumber(unsigned int from_id, unsigned int to_id)
	{
		std::vector<unsigned int>::iterator iter = std::find(_edge_ids.begin(), _edge_ids.end(), from_id);
		if (iter == _edge_ids.end())
		{
			mlog(MLog::error, "Node::renumber") << "Cannot find edge from_id!\n!";
		} else
		{
			*iter = to_id;
		}
	}

}
