/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "node.h"

#include "edge.h"

namespace mapgeneration
{
	
	std::ostream&
	operator<<(std::ostream& o_stream, const Node& node)
	{
		o_stream << dynamic_cast<const GeoCoordinate&>(node);
		o_stream << ", pred=" << node.predecessors().size();
		o_stream << ", succ=" << node.successors().size();
		
		return o_stream;
	}

	Node::Node()
	: GeoCoordinate(), _predecessors(), _successors(), _weight(1)
	{
	}
	
	
	Node::Node(const Node& node)
	: GeoCoordinate(node), _predecessors(node._predecessors),
		_successors(node._successors), _weight(node._weight)
	{
	}
	
			
	Node::Node(const GeoCoordinate& geo_coordinate)
	: GeoCoordinate(geo_coordinate), _predecessors(), _successors(), _weight(1)
	{
	}
	
	
	void
	Node::add_predecessor(Id node_id, double direction)
	{
		if (!has_predecessor(node_id))
		{
			Edge new_edge(node_id, direction);
			_predecessors.push_back(new_edge);
		}
		
		assert(has_predecessor(node_id));
	}
	void
	Node::add_successor(Id node_id, double direction)
	{
		if (!has_successor(node_id))
		{
			Edge new_edge(node_id, direction);
			_successors.push_back(new_edge);
		}
		
		assert(has_successor(node_id));
	}
		
		
/*	void
	Node::add_next_node(Id node_id, double direction)
	{
		D_Edges::const_iterator iter = _edges.begin();
		D_Edges::const_iterator iter_end = _edges.begin();
		while ( (iter->get_next_node_id() != node_id) && (iter != iter_end) )
			++iter;
				
		if (iter == iter_end)
		{
			Edge new_edge(node_id, direction);
			_edges.push_back(new_edge);
		}
	}*/
	
	
	Node::D_Edges::const_iterator
	Node::edge_iterator(Id node_id, EdgeKind search_in_kind) const
	{
		assert(search_in_kind != _NONE);
		
		D_Edges::const_iterator iter;
		D_Edges::const_iterator iter_end;
		if (search_in_kind == _PREDECESSOR)
		{
			iter = _predecessors.begin();
			iter_end = _predecessors.end();
		} else
		{
			iter = _successors.begin();
			iter_end = _successors.end();
		}
		
		while (iter != iter_end)
		{
			if (iter->get_next_node_id() == node_id)
				return iter;
			else
				++iter;
		}
		
		return iter;
	}
	
	
	Node::D_Edges::iterator
	Node::edge_iterator(Id node_id, EdgeKind search_in_kind)
	{
		// exactly the same method as above. EXACTLY!
		
		assert(search_in_kind != _NONE);
		
		D_Edges::iterator iter;
		D_Edges::iterator iter_end;
		if (search_in_kind == _PREDECESSOR)
		{
			iter = _predecessors.begin();
			iter_end = _predecessors.end();
		} else
		{
			iter = _successors.begin();
			iter_end = _successors.end();
		}
		
		while (iter != iter_end)
		{
			if (iter->get_next_node_id() == node_id)
				return iter;
			else
				++iter;
		}
		
		return iter;
	}
	
	
	bool
	Node::has_predecessor(Id node_id) const
	{
		return (edge_iterator(node_id, _PREDECESSOR) != _predecessors.end());
	}
	
	bool
	Node::is_reachable(Id node_id) const
	{
		return (has_predecessor(node_id) || has_successor(node_id));
	}
	
	
	bool
	Node::has_successor(Id node_id) const
	{
		return (edge_iterator(node_id, _SUCCESSOR) != _successors.end());
	}		
	
	
	double
	Node::minimal_direction_difference_to(const Direction& direction) const
	{
		// consequence: never inserted into path directly! But
		// fortunately we have TraceProcessor::insert_extra_entries_into_path.
		if (_successors.empty())
			return PI;
		
		D_Edges::const_iterator iter = _successors.begin();
		D_Edges::const_iterator iter_end = _successors.end();
		
		double minimal_difference = PI;
		
		for (; iter != iter_end; iter++)
		{
			double difference = iter->get_direction().angle_difference(direction);
			if (difference < minimal_difference)
				minimal_difference = difference;
		}
		
		return minimal_difference;
	}
	
	
} //namespace mapgeneration
