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
	: GeoCoordinate(), _directions(), _next_node_ids(), _weight(1)
	{
	}
	
	
	Node::Node(const Node& node)
	: GeoCoordinate(node), _directions(node._directions),
		_next_node_ids(node._next_node_ids), _weight(node._weight)
	{
	}
	
			
	Node::Node(const GeoCoordinate& geo_coordinate)
	: GeoCoordinate(geo_coordinate), _directions(), 
		_next_node_ids(), _weight(1)
	{
	}
	
	
	Node::Node(const GPSPoint& gps_point)
	: GeoCoordinate(gps_point), _directions(), 
		_next_node_ids(), _weight(1)
	{
		_directions.push_back(gps_point.get_direction());
	}
	
	
	void
	Node::add_direction(double direction)
	{
		_directions.push_back(Direction(direction));
	}
	
	
	void
	Node::add_next_node(Id node_id, double direction)
	{
		std::vector<Id>::iterator iter = _next_node_ids.begin();
		for (; iter != _next_node_ids.end() && *iter!=node_id; iter++)
		{}
				
		if (iter == _next_node_ids.end())
		{
			_next_node_ids.push_back(node_id);
			_directions.push_back(Direction(direction));
		}
	}
	
	
	int
	Node::connected_nodes() const
	{
		return _next_node_ids.size();
	}
	
	
	void
	Node::merge(const GPSPoint& gps_point)
	{
		set_latitude((get_latitude() * (double)_weight + gps_point.get_latitude()) / (_weight +1));
		set_longitude((get_longitude() * (double)_weight + gps_point.get_longitude()) / (_weight +1));
		set_altitude((get_altitude() * (double)_weight + gps_point.get_altitude()) / (_weight +1));

/*		double old_direction = get_direction();
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
		set_direction(new_direction);*/

		++_weight;
	}
	
	
	double
	Node::minimal_direction_difference_to(const Direction& direction) const
	{
		std::vector<Direction>::const_iterator iter = _directions.begin();
		std::vector<Direction>::const_iterator iter_end = _directions.end();
		
		double minimal_difference = PI;
		
		for (; iter != iter_end; iter++)
		{
			double difference = iter->angle_difference(direction);
			if (difference < minimal_difference)
				minimal_difference = difference;
		}
		
		return minimal_difference;
	}
	
	
	double
	Node::minimal_direction_difference_to(const Node& node) const
	{
		std::vector<Direction>::const_iterator iter = _directions.begin();
		std::vector<Direction>::const_iterator iter_end = _directions.end();
		
		double minimal_difference = PI;
		
		for (; iter != iter_end; iter++)
		{
			std::vector<Direction>::const_iterator inner_iter = 
				node._directions.begin();
			std::vector<Direction>::const_iterator inner_iter_end = 
				node._directions.end();

			for (; inner_iter != inner_iter_end; ++inner_iter)
			{
				double difference = iter->angle_difference(*inner_iter);
				if (difference < minimal_difference)
					minimal_difference = difference;
			}
			return minimal_difference;			
		}
		
		return minimal_difference;
	}
	
	
	bool
	Node::is_reachable(Id node_id) const
	{
		std::vector<Id>::const_iterator iter = _next_node_ids.begin();
		for (; (iter != _next_node_ids.end()) && (*iter != node_id); iter++)
		{}
		
		if (iter == _next_node_ids.end())
			return false;
			
		return true;
	}
	
	
	Node&
	Node::operator=(const Node& node)
	{
		GeoCoordinate::operator=(node);
		_directions = node._directions;
		_next_node_ids = node._next_node_ids;
		_weight = node._weight;
		
		return *this;
	}


}
