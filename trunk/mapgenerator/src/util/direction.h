/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef DIRECTION_H
#define DIRECTION_H

#include <iostream>

#include "util/constants.h"
#include "util/mlog.h"
#include "util/serializer.h"


namespace mapgeneration_util
{

	/**
	 * @brief Direction implements a direction/heading baased on radiant values.
	 */
	class Direction
	{
		
		public:
			
			/**
			 * @brief Empty constructor.
			 */
			inline 
			Direction();
			

			/**
			 * @brief Constructor that inits the Direction with the specified value.
			 * 
			 * @param direction the value
			 */
			inline
			Direction(double direction);
			
			
			/**
			 * @brief Calculates the angle difference to the specified Direction.
			 * 
			 * @param direction the Direction
			 * @return the angle difference
			 */
			inline double
			angle_difference(const Direction& direction) const;
			
			
			/**
			 * @see Serializer
			 */
			inline void
			deserialize(std::istream& i_stream);
			
			
			/**
			 * @return the direction
			 */
			inline double
			get_direction() const;
			
			
			/**
			 * @brief Assignment operator.
			 * 
			 * Assigns a Direction to this.
			 * 
			 * @param direction a reference to a Direction
			 * @return (new) this
			 */
			inline Direction&
			operator=(const Direction& direction);
			
			
			/**
			 * @see Serializer
			 */
			inline void
			serialize(std::ostream& o_stream) const;
			
			
			/**
			 * @param direction the value of the direction
			 */
			inline void
			set_direction(double direction);
		
			
		private:
			
			double _direction;	/** < radient You know what I mean.... */

	};
	
	
	inline 
	Direction::Direction()
	: _direction(0)
	{
	}
	
	
	inline
	Direction::Direction(double direction)
	: _direction(direction)
	{
		if (direction < 0 || direction >= 2 * PI)
			throw ("Direction not in [0; 2*PI)");
	}
	
	
	inline double
	Direction::angle_difference(const Direction& direction) const
	{
		double difference = direction._direction - _direction;
		if (difference < 0) difference *= -1;
		if (difference >= PI) difference = (2 * PI) - difference;
		
		return difference;
	}
	
	
	inline void
	Direction::deserialize(std::istream& i_stream)
	{
		Serializer::deserialize(i_stream, _direction);
	}
				
	
	inline double
	Direction::get_direction() const
	{
		return _direction;
	}
	
	
	inline Direction&
	Direction::operator=(const Direction& direction)
	{
		_direction = direction._direction;
	}
	
	
	
	inline void
	Direction::serialize(std::ostream& o_stream) const
	{
		Serializer::serialize(o_stream, _direction);
	}
	
	
	inline void
	Direction::set_direction(double direction)
	{
		if (direction < 0 || direction >= 2 * PI)
			throw ("Direction not in [0; 2*PI)");
		_direction = direction;
	}


} // namespace mapgeneration_util

#endif //DIRECTION_H
