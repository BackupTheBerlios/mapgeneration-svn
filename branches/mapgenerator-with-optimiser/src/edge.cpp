/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "edge.h"

/*
#include "util/mlog.h"

namespace mapgeneration
{
	
	Edge::Edge(int edge_id)
	: _id(edge_id), _node_ids(), _times()
	{
	}
	
	
	void
	Edge::change_time_after(std::list<double>::iterator& current_time_iter, double new_time_after)
	{
		*current_time_iter = new_time_after;
	}
			
			
	void
	Edge::change_time_before(std::list<double>::iterator& current_time_iter, double new_time_before)
	{
		if (current_time_iter != _times.begin())
		{
			--current_time_iter;
			*current_time_iter = new_time_before;
			++current_time_iter;
		}
	}
	
	
	void
	Edge::change_times(std::list<double>::iterator& current_time_iter, double new_previous_time, double new_time)
	{
		if (current_time_iter != _times.begin())
		{
			--current_time_iter;
			*current_time_iter = new_previous_time;
			++current_time_iter;
		}
		
		*current_time_iter = new_time;
	}
	
	
	void
	Edge::deserialize(std::istream& i_stream)
	{
		Serializer::deserialize(i_stream, _id);
		Serializer::deserialize(i_stream, _node_ids);
		Serializer::deserialize(i_stream, _times);
	}
	
	
	void
	Edge::init(std::pair<unsigned int, unsigned int> first_node_id, std::pair<unsigned int, unsigned int> second_node_id, double time)
	{
		if (_node_ids.size() == 0)
		{
			_node_ids.push_back(first_node_id);
			_node_ids.push_back(second_node_id);
			_times.push_back(time);
//			std::cout.setf(std::ios::
			std::cout << "First: " << first_node_id.first << ", " << first_node_id.second
				<< "\nSecond: " << second_node_id.first << ", " << second_node_id.second
				<< "\nTime: " << time << "\n";
		} else
		{
			mlog(MLog::error, "Edge::init") << "Edge already initialized!\n";
		}
	}
	
	
	void
	Edge::extend(std::pair<unsigned int, unsigned int> next_to_node_id, std::pair<unsigned int, unsigned int> new_node_id, double time)
	{
		if ((_node_ids.front().first == next_to_node_id.first) && (_node_ids.front().second == next_to_node_id.second))
		{
			_node_ids.push_front(new_node_id);
			_times.push_front(time);
		} else if ((_node_ids.back().first == next_to_node_id.first) && (_node_ids.back().second == next_to_node_id.second))
		{
			_node_ids.push_back(new_node_id);
			_times.push_back(time);
		} else
		{
			mlog(MLog::error, "Edge::extend") << "Could not extend edge, next_to_node_id not  found!\n";
		}
	}
	
	
	std::set<unsigned int>
	Edge::get_tile_ids()
	{		
		std::set<unsigned int> set_tile_ids;
		std::list< std::pair<unsigned int, unsigned int> >::iterator iter = _node_ids.begin();
		std::list< std::pair<unsigned int, unsigned int> >::iterator iter_end = _node_ids.end();
		for (; iter != iter_end ; ++iter)
		{
			unsigned int tile_id= iter->first;
			set_tile_ids.insert(tile_id);
		}
		return set_tile_ids;
	}
	

	std::list< std::pair<unsigned int, unsigned int> >&
	Edge::node_ids()
	{
		return _node_ids;
	}
	
	
	const std::list< std::pair<unsigned int, unsigned int> >&
	Edge::node_ids() const
	{
		return _node_ids;
	}


	bool
	Edge::node_is_at_end(const std::pair<unsigned int, unsigned int> node_id) const
	{
		if ((_node_ids.back().first == node_id.first) && (_node_ids.back().second == node_id.second))
			return true;
		
		return false;
	}


	bool
	Edge::node_is_at_start(const std::pair<unsigned int, unsigned int> node_id) const
	{
		if ((_node_ids.front().first == node_id.first) && (_node_ids.front().second == node_id.second))
			return true;
		
		return false;
	}


	/** @TODO REICHT DAS??? *
	bool
	Edge::operator==(Edge& compare_to) const
	{
		if (_id == compare_to._id) return true;
		
		return false;
	}

	void
	Edge::serialize(std::ostream& o_stream) const
	{
		Serializer::serialize(o_stream, _id);
		Serializer::serialize(o_stream, _node_ids);
		Serializer::serialize(o_stream, _times);
	}
	
	
	int
	Edge::size_of()
	{
		return ((_node_ids.size() * sizeof(int) * 2) + (_times.size() * sizeof(double))
			+ sizeof(Edge));
	}

}*/
