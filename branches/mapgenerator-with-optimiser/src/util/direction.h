/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef DIRECTION_H
#define DIRECTION_H

#include <cmath>

#include "util/constants.h"
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
			
			
			inline
			operator double();
			
			
			/**
			 * @brief Calculates the angle difference to the specified Direction.
			 * 
			 * @param direction the Direction
			 * @return the angle difference
			 */
			inline double
			angle_difference(const Direction& direction) const;
			
			
			inline double
			angle_difference_signed(const Direction& direction) const;
			
			
			inline double
			angle_difference_unsigned(const Direction& direction) const;
			
			
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
			
			
			static Direction
			interpolate(Direction direction_1, Direction direction_2,
				const double weight_on_first)
			{
				return interpolate(direction_1, direction_2, weight_on_first, 0);
			}
			
			
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
			
			
			inline bool
			operator==(const Direction& direction) const;
			
			
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
			
			
			static double
			interpolate(double direction_1, double direction_2,
				const double weight_on_first, int dummy);

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
	
	
	inline
	Direction::operator double()
	{
		return _direction;
	}
	
	
	inline double
	Direction::angle_difference(const Direction& direction) const
	{
		return angle_difference_unsigned(direction);
	}
	
	
	inline double
	Direction::angle_difference_signed(const Direction& direction) const
	{
		double difference = direction._direction - _direction;
		
		double sign = 1.0;
		if (difference < 0)
		{
			difference *= -1;
			sign *= -1.0;
		}
		
		if (difference > PI)
		{
			difference = (2.0 * PI) - difference;
			sign *= -1.0;
		}
		
		return sign * difference;
	}
	
	inline double
	Direction::angle_difference_unsigned(const Direction& direction) const
	{
		return fabs(angle_difference_signed(direction));
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
	
	
	inline bool
	Direction::operator==(const Direction& direction) const
	{
		return _direction == direction._direction;
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
