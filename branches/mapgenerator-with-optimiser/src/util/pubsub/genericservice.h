/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef GENERICSERVICE_H
#define GENERICSERVICE_H

#include <string>
#include <typeinfo>


namespace pubsub
{

	/**
	 * @brief GenericService is a type independent superclass of a Service and
	 * can be registered in a ServiceList.
	 * 
	 * GenericService provides a name and a data type name for the entries
	 * in a ServiceList. The data type name should always be the return
	 * value of typeid(T_Value).name() for the T_Value of the subclass.
	 * 
	 * @see pubsub, ServiceList, Service
	 */
	class GenericService {

		public:
					
			typedef unsigned int D_Id;
			
			
			/**
			 * @brief The constructor.
			 * 
			 * @param name The name of the service.
			 * @param data_type_name The return value of typeid(T_Value).name()
			 * of the subclasses' T_Value.
			 */
			GenericService(std::string name, std::string data_type_name);
			
			
			/**
			 * @brief Returns the data type name.
			 * 
			 * @return The data type name, that should be the return value
			 * of typeid(T_Value).name() for the subclasses' T_Value.
			 */
			inline std::string
			get_data_type_name() const;
			
			
			inline D_Id
			get_id() const;
			
			
			/**
			 * @brief Returns the name of the service.
			 * 
			 * @return Name of the service.
			 */
			virtual inline std::string
			get_name() const;


		protected:
			
			
		private:
		
			static unsigned int _next_id;
		
			D_Id _id;
			std::string _name;
			
			std::string _data_type_name;

	};
	
	
	inline std::string
	GenericService::get_data_type_name() const
	{
		return _data_type_name;
	}
	
	
	inline GenericService::D_Id
	GenericService::get_id() const
	{
		return _id;
	}
	
	
	inline std::string
	GenericService::get_name() const
	{
		return _name;
	}
		

} // namespace pubsub

#endif //GENERICSERVICE_H
