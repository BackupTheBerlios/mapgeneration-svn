/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "mlog.h"

#include <iostream>

namespace mapgeneration_util
{
	
	MLog mlog;


	MLog::MLog()
	: std::streambuf(), std::ostream((std::streambuf*) this), _buffer()
	{
	}


	MLog::~MLog()
	{
	}


/*	std::ostream&
	MLog::operator<<(std::ostream& os)
	{
		std::cout << "(" << _level << ") " << _source << ": " << os << std::endl;
		
		return *this;
	}*/
	
	
	int MLog::overflow(int character)
	{		
		/** @todo Speed up this implementation!!!! */
		if (character == '\n' || character == EOF || !character)
		{
			std::cout << "(" << _level << ") " << _source << ": " << _buffer << "\n";
			_buffer = "";
		} else
		{
			_buffer += character;
		}
	}
	
} // namespace mapgeneration_util
