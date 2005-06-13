/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/

#ifndef ARITHMETICSERVICE_H
#define ARITHMETICSERVICE_H

#include "servicesystem.h"

namespace pubsub
{

	template <typename T_Value>
	class ArithmeticService : public Service<T_Value>
	{

		public:
		
			
			/**
			 * @brief The constructor.
			 * 
			 * The name is used for the GenericService. The initial_value
			 * is stored and return when no publishers are registered and
			 * the publish method is invoked.
			 * 
			 * @param name The name of the service.
			 * @param initial_value The initial value.
			 */
			ArithmeticService(std::string name, T_Value initial_value,
				T_Value (*operation_pointer) (const T_Value&, const T_Value&) );
			
			
			T_Value
			publish();
			
			
			/**
			 * @brief Stores and redistributes the value.
			 * 
			 * The received value is stored and the receive()-method
			 * of all registered subscribers is called with this value.
			 */
			void
			receive(T_Value value);
			
			
			/**
			 * @brief Resets the value.
			 */
			void
			reset();
			
			
		protected:
			
			/**
			 * @brief Pointer to an arithmetic operation.
			 */
			T_Value
			(*_operation_pointer) (const T_Value&, const T_Value&);
			
			
	};
	
	
	template <typename T_Value>
	ArithmeticService<T_Value>::ArithmeticService(
		std::string name,
		T_Value initial_value,
		T_Value (*operation_pointer) (const T_Value&, const T_Value&)
	)
	: Service<T_Value>::Service(name, initial_value),
		_operation_pointer(operation_pointer)
	{
	}
	

	template <typename T_Value>
	T_Value
	ArithmeticService<T_Value>::publish()
	{
		return Service<T_Value>::_value;
	}

	
	template <typename T_Value>
	void
	ArithmeticService<T_Value>::receive(T_Value value)
	{
		Service<T_Value>::receive(
			_operation_pointer(value, Service<T_Value>::_value) );
	}
	
	
	template <typename T_Value>
	void
	ArithmeticService<T_Value>::reset()
	{
		Service<T_Value>::_value;
	}
	
	
} // namespace pubsub

#endif //ARITHMETICSERVICE_H
