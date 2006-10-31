/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef EDGE_H
#define EDGE_H

#include "node.h"
#include "util/direction.h"
#include "util/serializer.h"

using namespace mapgeneration_util;


namespace mapgeneration
{
	/**
	 * @brief This class represent an edge.
	 */
	class Edge {
		
		public:
				
			/**
			 * @brief Empty Constructor.
			 */
			inline
			Edge();
			
			/**
			 * @brief Constructor which inits this edge with specified ID.
			 * 
			 * @param edgeID the id
			 */
			inline
			Edge(Node::Id next_node_id, Direction direction);
			
			
			/**
			 * @see mapgeneration_util::Serializer
			 */
			inline void
			deserialize(std::istream& i_stream);
			
			
			inline Direction
			get_direction() const;
			
			
			inline Node::Id
			get_next_node_id() const;
			
			
			/**
			 * @see mapgeneration_util::Serializer
			 */			
			inline void
			serialize (std::ostream& o_stream) const;
			
			
			inline void
			set_direction(Direction direction);
			
			
			inline void
			set_next_node_id(Node::Id next_node_id);
			
			
			/**
			 * @brief Returns the approx. size of this object.
			 * 
			 * @return the approx. size
			 */			
//			int
//			size_of();
			
			
		private:
			
			Node::Id _next_node_id;
			Direction _direction;
			
			
	};
	
	
	inline
	Edge::Edge()
	: _direction(), _next_node_id()
	{
	}
	
	
	inline
	Edge::Edge(Node::Id node_id, Direction direction)
	: _direction(direction), _next_node_id(node_id)
	{
	}
	
	
	inline void
	Edge::deserialize(std::istream& i_stream)
	{
		Serializer::deserialize(i_stream, _direction);
		Serializer::deserialize(i_stream, _next_node_id);
	}
	
	
	inline Direction
	Edge::get_direction() const
	{
		return _direction;
	}
	
	
	inline Node::Id
	Edge::get_next_node_id() const
	{
		return _next_node_id;
	}
	
	
	inline void
	Edge::serialize(std::ostream& o_stream) const
	{
		Serializer::serialize(o_stream, _direction);
		Serializer::serialize(o_stream, _next_node_id);
	}
	
	
	inline void
	Edge::set_direction(Direction direction)
	{
		_direction = direction;
	}
	
	
	inline void
	Edge::set_next_node_id(Node::Id next_node_id)
	{
		_next_node_id = next_node_id;
	}
	
} // namespace mapgeneration

#endif //EDGE_H
