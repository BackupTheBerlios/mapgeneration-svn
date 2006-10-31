/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "genericservice.h"


namespace pubsub
{
	
	GenericService::D_Id GenericService::_next_id = 0;
	
	GenericService::GenericService(std::string name, std::string data_type_name)
	: _name(name), _data_type_name(data_type_name)
	{
		_id = _next_id;
		_next_id++;
	}


} // namespace pubsub
