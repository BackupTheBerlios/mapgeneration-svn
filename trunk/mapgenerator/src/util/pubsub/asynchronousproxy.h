/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef ASYNCHRONOUSPROXY_H
#define ASYNCHRONOUSPROXY_H


#include "publisher.h"
#include "subscriber.h"
#include "../controlledthread.h"


namespace pubsub
{

	/**
	 * @brief 
	 * 
	 * @see Publisher, Subscriber
	 */
	template <typename T_Value>
	class AsynchronousProxy : public Publisher<T_Value>, 
		public Subscriber<T_Value>//, public mapgeneration_util::ControlledThread
	{
		
		public:
		
			AsynchronousProxy(Publisher<T_Value>* publisher = 0,
				Subscriber<T_Value>* subscriber = 0);
			
			
			~AsynchronousProxy();
	
	
			inline T_Value
			publish();
			
			
			inline void
			receive(T_Value value);
			
			
			void
			remove_publisher(Publisher<T_Value>* publisher);
			
			
			void
			remove_subscriber(Subscriber<T_Value>* subscriber);
		
		
		private:
		
	};
	
	
	template <typename T_Value>
	AsynchronousProxy<T_Value>::AsynchronousProxy(Publisher<T_Value>* publisher,
		Subscriber<T_Value>* subscriber)
	: Publisher<T_Value>::Publisher(subscriber, 
			Publisher<T_Value>::PULL | Publisher<T_Value>::PUSH), 
		Subscriber<T_Value>::Subscriber(publisher, 
			Subscriber<T_Value>::PULL | Publisher<T_Value>::PUSH)
	{
	}
	
	
	template <typename T_Value>
	AsynchronousProxy<T_Value>::~AsynchronousProxy()
	{
		if (_publisher)
			_publisher->remove_subscriber(this);
		if (_subscriber)
			_subscriber->remove_publisher(this);
	}


	template <typename T_Value>
	inline T_Value
	AsynchronousProxy<T_Value>::publish()
	{
		if (_publisher != 0)
			return _publisher->publish();
		
		return T_Value();
	}
	
	
	template <typename T_Value>
	inline void
	AsynchronousProxy<T_Value>::receive(T_Value value)
	{
		if (_subscriber != 0)
			_subscriber->receive(value);
	}
	
	
	template <typename T_Value>
	void
	AsynchronousProxy<T_Value>::remove_publisher(Publisher<T_Value>* publisher)
	{
		if (_publisher == publisher)
		{
			_publisher = 0;
			if (_subscriber == 0)
				delete this;
		}
	}
	
	
	template <typename T_Value>
	void
	AsynchronousProxy<T_Value>::remove_subscriber(Subscriber<T_Value>* subscriber)
	{
		if (_subscriber == subscriber)
		{
			_subscriber = 0;
			if (_publisher == 0)
				delete this;
		}
	}

	

} // namespace pubsub

#endif //ASYNCHRONOUSPROXY_H
