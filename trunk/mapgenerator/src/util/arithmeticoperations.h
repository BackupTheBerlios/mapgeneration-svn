/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef ARITHMETICOPERATIONS_H
#define ARITHMETICOPERATIONS_H

namespace mapgeneration_util
{
	
	template <typename T_Value>
	inline T_Value
	add(const T_Value& v1, const T_Value& v2)
	{
		return v1 + v2;
	}
	
	
	template <typename T_Value>
	inline T_Value
	sub(const T_Value& v1, const T_Value& v2)
	{
		return v1 - v2;
	}
	
	
	template <typename T_Value>
	inline T_Value
	mult(const T_Value& v1, const T_Value& v2)
	{
		return v1 * v2;
	}
	
	
	template <typename T_Value>
	inline T_Value
	div(const T_Value& v1, const T_Value& v2)
	{
		return v1 / v2;
	}
	
	
	template <typename T_Value>
	inline T_Value
	mod(const T_Value& v1, const T_Value& v2)
	{
		return v1 % v2;
	}
	
}

#endif //ARITHMETICOPERATIONS_H
