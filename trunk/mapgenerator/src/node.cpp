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
	: Direction(), GeoCoordinate(), _weight(1)
	{
	}
	
	
	Node::Node(const Node& node)
	: Direction(node), GeoCoordinate(node), _next_node_ids(node._next_node_ids), 
		_weight(node._weight)
	{
	}
	
			
	Node::Node(const GPSPoint& gps_point)
	: Direction(gps_point), GeoCoordinate(gps_point), _weight(1)
	{
	}
	
	
	void
	Node::add_next_node_id(std::pair<unsigned int, unsigned int> next_node_id)
	{
		std::vector< std::pair<unsigned int, unsigned int> >::iterator iter=
			_next_node_ids.begin();
		for (; iter != _next_node_ids.end() && 
			(iter->first!=next_node_id.first || 
				iter->second!=next_node_id.second);
			iter++)
		{}
		
		Node::Id new_node_id = std::make_pair(next_node_id.first, next_node_id.second);
		if (iter == _next_node_ids.end())
			_next_node_ids.push_back(new_node_id);
		std::cout << "_next_node_ids.size(): " << _next_node_ids.size() << "\n";
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
	
	
	bool
	Node::is_reachable(Id node_id) const
	{
		std::vector< std::pair<unsigned int, unsigned int> >::const_iterator 
			iter=_next_node_ids.begin();
		for (; iter != _next_node_ids.end() && 
			(iter->first!=node_id.first || 
				iter->second!=node_id.second);
			iter++)
		{}
		
		if (iter == _next_node_ids.end())
			return false;
			
		return true;
	}
	
	
	Node&
	Node::operator=(const Node& node)
	{
		GeoCoordinate::operator=(node);
		Direction::operator=(node);
		_next_node_ids = node._next_node_ids;
		_weight = node._weight;
		
		return *this;
	}


}
