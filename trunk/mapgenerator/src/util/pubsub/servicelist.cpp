/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include <iostream>
#include "servicelist.h"


namespace pubsub
{
	
	std::list<ServiceList*> ServiceList::_service_lists;
	
	
	ServiceList::ServiceList()
	: _next_id(0), _services_by_name()
	{
		_service_lists.push_back(this);
	}
	
	
	ServiceList::~ServiceList()
	{
		_service_lists.remove(this);
	}
	
	
	void
	ServiceList::add(GenericService* generic_service)
	{
		_services_by_name.insert(std::make_pair(generic_service->get_name(), generic_service));
	}
	
	
	GenericService*
	ServiceList::find_generic_service(std::string name)
	{
		std::map<std::string, GenericService*>::iterator by_name_iter 
			= _services_by_name.find(name);
		if (by_name_iter == _services_by_name.end())
			return 0;

		return (by_name_iter->second);
	}
	
	
	void
	ServiceList::remove_service_from_list(std::string name)
	{	
		std::map<std::string, GenericService*>::iterator by_name_iter 
			= _services_by_name.find(name);
		if (by_name_iter != _services_by_name.end())
		{
			_services_by_name.erase(by_name_iter);
		}
	}
	
	
	void
	ServiceList::remove_service_from_all_lists(std::string name)
	{
		std::list<ServiceList*>::iterator iter = _service_lists.begin();
		for (; iter != _service_lists.end(); iter++)
			(*iter)->remove_service_from_list(name);
	}


	/* ************************************
	 *  Output operator
	 * ***********************************/
	std::ostream& operator<<(std::ostream& out, const ServiceList& service_list)
	{
		out << std::endl;
		out << "ServiceList:" << std::endl;
		out << "==================================" << std::endl;
		
		std::map<std::string, GenericService*>::const_iterator iter
			= service_list.services_by_name().begin();
		std::map<std::string, GenericService*>::const_iterator iter_end
			= service_list.services_by_name().end();
		for (; iter != iter_end; ++iter)
		{
			out << "\"" << iter->first << "\"\t\t\"";
			
			if (iter->second->get_data_type_name() == typeid(int).name())
				out << (dynamic_cast<Service<int>*>(iter->second))->publish();
			if (iter->second->get_data_type_name() == typeid(double).name())
				out << (dynamic_cast<Service<double>*>(iter->second))->publish();
			if (iter->second->get_data_type_name() == typeid(std::string).name())
				out << (dynamic_cast<Service<std::string>*>(iter->second))->publish();
	
			out << "\"" << std::endl;
		}
		
		out << std::endl;
		
		return out;
	}
	/* ************************************
	 *  Output operator
	 * ***********************************/
	
	
} // namespace pubsub
