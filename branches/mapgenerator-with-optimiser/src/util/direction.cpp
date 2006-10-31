/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "direction.h"

namespace mapgeneration_util
{
	
	double
	Direction::interpolate(double direction_1,
		double direction_2, const double weight_on_first, int dummy)
	{
		if (direction_1 >= direction_2)
		{
			if (direction_1 - direction_2 <= PI)
			{
				return weight_on_first * direction_1
					+ (1.0 - weight_on_first) * direction_2;
				
			} else
			{
				double direction = interpolate(direction_1,
					direction_2 + 2.0 * PI, weight_on_first, dummy);
				if (direction >= 2.0 * PI)
					direction -= 2.0 * PI;
				
				return direction;
			}
		} else
		{
			return interpolate(direction_2, direction_1, weight_on_first, dummy);
		}
	}
	
} // namespace mapgeneration_util
