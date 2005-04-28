/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/

/**
 * You have to include this file to use services and service lists. This is
 * needed because of circular references in the methods below. These could
 * not be compiled with the methods directly in the header files since
 * gcc 3.4.x.
 */

#ifndef SERVICESYSTEM_H
#define SERVICESYSTEM_H
#define SERVICESYSTEM_INIT

#include "servicelist.h"
#include "service.h"

#undef SERVICESYSTEM_INIT

namespace pubsub
{
	
	template <typename T_Value>			
	Service<T_Value>*
	ServiceList::find_service(std::string name)
	{
		GenericService* generic_service = find_generic_service(name);
		if (!generic_service)
			return 0;

		return (dynamic_cast<Service<T_Value>*>(generic_service));
	}


	template <typename T_Value>
	bool
	ServiceList::get_service_value(std::string name, T_Value& value)
	{
		Service<T_Value>* service = find_service<T_Value>(name);
		if (!service)
			return false;
			
		value = service->publish();
		
		return true;
	}
	
}


/*std::ostream& operator<<(std::ostream& out, const ServiceList& service_list)
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
			out << (dynamic_cast<Service<int>* const>(iter->second))->publish();
		if (iter->second->get_data_type_name() == typeid(double).name())
			out << (dynamic_cast<Service<double>*>(iter->second))->publish();
		if (iter->second->get_data_type_name() == typeid(std::string).name())
			out << (dynamic_cast<Service<std::string>*>(iter->second))->publish();

		out << "\"" << std::endl;
	}
	
	out << std::endl;
	
	return out;
}*/


#endif // SERVICESYSTEM_H
