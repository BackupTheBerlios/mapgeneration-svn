/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/

#ifndef SERVICESYSTEM_INIT
#error Please do not include service.h, use servicesystem.h instead.
#else

#ifndef SERVICELIST_H
#define SERVICELIST_H


namespace pubsub
{
	template <typename T_Value>
	class Service;
}


#include <ostream>
#include <list>
#include <map>
#include <string>
#include "genericservice.h"

namespace pubsub
{

	/**
	 * @brief Stores a list of services.
	 * 
	 * ServiceList is part of the Publish/Subscribe-System. For a generic 
	 * description have a look at the documentation for the 
	 * pubsub namespace.
	 * 
	 * @see pubsub
	 */
	class ServiceList {

		public:
		
			/**
			 * @brief The constructor initializes an empty ServiceList.
			 */
			ServiceList();
			
			
			/** 
			 * @brief The destructor removes the ServiceList from the 
			 * internal list of ServiceLists.
			 */
			~ServiceList();
			
			
			/**
			 * @brief Adds a service to the list.
			 */
			void
			add(GenericService* generic_service);
			
			
			/**
			 * @brief Searches for a service and returns a pointer to the
			 * Service if it's found and can be converted to the specified
			 * type.
			 * 
			 * @param name The name of the service to search for.
			 * @return A pointer to the Service or 0 if the service could
			 * not e found or has another type.
			 */
			template <typename T_Value>			
			Service<T_Value>*
			find_service(std::string name);


			/**
			 * @brief Tries to fill the reference "value" with the value of the
			 * service.
			 * 
			 * The method uses find to find the service and tries to convert it
			 * to the type of value.
			 * 
			 * @param name The name of the service in which the value is stored.
			 * @param value A reference to the variable in which the value 
			 * should be stored.
			 * @return True if successfull, false otherwise.
			 */
			template <typename T_Value>
			bool
			get_service_value(std::string name, T_Value& value);


			/**
			 * @brief Searches for a service and returns a pointer to the
			 * GenericService if it's found.
			 *
			 * @param name The name of the service to search for.
			 * @return A pointer to the GenericService or 0 if the service could
			 * not be found.
			 */
			GenericService*
			find_generic_service(std::string name);
						
			
			/**
			 * @brief Returns the number of known services.
			 * 
			 * @return Number of services in the list.
			 */
			inline int
			number_of_services() const;
			
			
			/**
			 * @brief Removes the Service with the given name from 
			 * the ServiceList.
			 * 
			 * @param name Name of the service to remove.
			 */
			void
			remove_service_from_list(std::string name);
			
			
			/**
			 * @brief Removes the Service with the given name from all
			 * ServiceLists.
			 * 
			 * The internal static list of all ServiceLists is used for this.
			 * 
			 * @param name Name of the service to remove.
			 */
			static void
			remove_service_from_all_lists(std::string name);
			
						
			/**
			 * @brief Returns a reference to the internal map of Services.
			 * 
			 * @return The map of services.
			 */
			inline const std::map<std::string, GenericService*>&
			services_by_name() const;


		private:
		
			unsigned int _next_id;


			/**
			 * @brief The map in which the services are stored.
			 */
			std::map<std::string, GenericService*> _services_by_name;


			/**
			 * @brief The static list of all ServiceLists.
			 */
			static std::list<ServiceList*> _service_lists;

	};
	

	/* ************************************
	 *  Output operator
	 * ***********************************/
	std::ostream& operator<<(std::ostream& out, const ServiceList& service_list);
	/* ************************************
	 *  Output operator
	 * ***********************************/


	inline int
	ServiceList::number_of_services() const
	{
		return _services_by_name.size();
	}
	
	
	inline const std::map<std::string, GenericService*>&
	ServiceList::services_by_name() const
	{
		return _services_by_name;
	}
	

} // namespace pubsub


#endif //SERVICELIST_H

#endif //SERVICESYSTEM_H
