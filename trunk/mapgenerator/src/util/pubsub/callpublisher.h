/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef CALLPUBLISHER_H
#define CALLPUBLISHER_H


#include <string>
#include "publisher.h"


namespace pubsub
{

	/**
	 * @brief CallPublisher is a Publisher that calls a function that is given 
	 * by a function pointer to get values when needed.
	 * 
	 * The CallPublisher is initialized with a function pointer. The given
	 * function is called when the CallPublishers publish()-method is called.
	 * 
	 * @see Publisher
	 */
	template <typename T_Value>
	class CallPublisher : public  Publisher<T_Value>
	{
		
		public:
		
			CallPublisher(Subscriber<T_Value>* subscriber, T_Value (*publish_pointer) ());
	
	
			T_Value
			publish();
			
			
			void
			remove_subscriber(Subscriber<T_Value>* subscriber);
		
		
		private:
		
			T_Value
			(*_publish_pointer) ();
		
	};
	
	
	template <typename T_Value>
	CallPublisher<T_Value>::CallPublisher(Subscriber<T_Value>* subscriber, 
		T_Value (*publish_pointer) ())
	: Publisher<T_Value>::Publisher(subscriber, Publisher<T_Value>::PULL), 
		_publish_pointer(publish_pointer)
	{
	}


	template <typename T_Value>
	T_Value
	CallPublisher<T_Value>::publish()
	{
		return _publish_pointer();
	}
	
	
	template <typename T_Value>
	void
	CallPublisher<T_Value>::remove_subscriber(Subscriber<T_Value>* subscriber)
	{
		delete this;
	}

	

} // namespace pubsub

#endif //CALLPUBLISHER_H
