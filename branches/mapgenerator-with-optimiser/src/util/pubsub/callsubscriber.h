/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef CALLSUBSCRIBER_H
#define CALLSUBSCRIBER_H


#include <string>
#include "subscriber.h"

namespace pubsub
{

	/**
	 * @brief A Subscriber that calls a function when its receive()-method is
	 * called.
	 * 
	 * The constructor needs a pointer to the function as its second parameter.
	 * 
	 * @see Subscriber
	 */
	template <typename T_Value>
	class CallSubscriber : public  Subscriber<T_Value>
	{
		
		public:
		
			CallSubscriber(Publisher<T_Value>* publisher, 
				void (*receive_pointer) (T_Value));
	
	
			void
			receive(T_Value value);
			
			
			void
			remove_publisher(Publisher<T_Value>* publisher);
		
		
		private:
		
			void
			(*_receive_pointer) (T_Value);
		
	};
	
	
	template <typename T_Value>
	CallSubscriber<T_Value>::CallSubscriber(Publisher<T_Value>* publisher,
		void (*receive_pointer) (T_Value))
	: Subscriber<T_Value>::Subscriber(publisher, Subscriber<T_Value>::PUSH), 
		_receive_pointer(receive_pointer)
	{
	}


	template <typename T_Value>
	void
	CallSubscriber<T_Value>::receive(T_Value value)
	{
		_receive_pointer(value);
	}
	
	
	template <typename T_Value>
	void
	CallSubscriber<T_Value>::remove_publisher(Publisher<T_Value>* publisher)
	{
		delete this;
	}

	

} // namespace pubsub

#endif //CALLSUBSCRIBER_H
