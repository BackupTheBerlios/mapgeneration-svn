/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef SERVICE_H
#define SERVICE_H


/** 
 * @brief Pubsub contains a Publish/Subscribe-System.
 * 
 * The core classes of this system are Publisher and Subscriber. A Publisher
 * contains a method T_Value publish() that returns a value and methods to 
 * register and remove a subscriber.
 * 
 * A Subscriber on the other hand has a method receive(T_Value) that accepts a
 * value and methods to register and remove publishers.
 * Publisher and Subscriber build a pair that can push or pull values from
 * publisher to subscriber.
 * 
 * To make the system more usable some basic implementations of Publishers
 * and Subscribers are availabe. The most important is the Service.
 * A Service is a subclass of both, Publisher and Subscriber, and of another
 * class called GenericService. By beeing Publisher and Subscriber a class
 * can be used in the middle of another Publisher and Subscriber. It is
 * possible to build long "pipes" this way.
 * 
 * What makes the service so special is, that it is a subclass of a 
 * GenericService has and supports multiple Publishers and Subscribers. By 
 * beeing a subclass of GenericService a Service has a name and can be 
 * registered in the ServiceList. GenericService is not a template class, 
 * but stores the typeid().name() of it's subclass. So it's possible to
 * get a complete list of Services from a ServiceList.
 * 
 * For details of all these functions have a look at the class descriptions.
 * 
 * @see Publisher, Subscriber, Service, GenericService, ServiceList
 */
namespace pubsub
{
	template <typename T_Value>	
	class Publisher;
	
	
	class ServiceList;
	
	template <typename T_Value>	
	class Subscriber;
}


#include <list>
#include <string>
#include <typeinfo>
#include "publisher.h"
#include "subscriber.h"


namespace pubsub
{

	/**
	 * @brief The Service<T_Value> is a subclass of GenericService, 
	 * Publisher<T_Value> and Subscriber<T_Value> and therefore a part
	 * of the pubsub-System. It can be placed in the 
	 * middle of other publishers and subscribers and registered in a 
	 * ServiceList.
	 * 
	 * On the publisher side multiple publisher can be registered. All are
	 * informed when the Service is deleted, but only the firsts registered
	 * publishers' publish() is called to receive values when the
	 * Services publish() is called. If no Publishers are registered
	 * the Service returns a the last received or initial value.
	 * 
	 * On the subsciber side multiple subscribers can be registered. All are
	 * informed when the Service is deleted. When the Services receive()
	 * is called the value is stored internaly and a copy is distributed
	 * to all registered subscribers.
	 * 
	 * For a generic description have a look at the documentation for the 
	 * pubsub namespace.
	 * 
	 * @see pubsub
	 */
	template <typename T_Value>
	class Service : public GenericService, public Publisher<T_Value>, 
		public Subscriber<T_Value> {

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
			Service(std::string name, T_Value initial_value);
			
			
			/**
			 * @brief The destructor.
			 * 
			 * The destructor invokes remove_subscriber in all
			 * registered publisher and remove_publisher in all registered
			 * subscribers.
			 */
			~Service();
			
			
			/**
			 * @brief Return a value.
			 * 
			 * Publish() returns the result of publish() from the first
			 * registered publisher and stores that value. If no publisher is
			 * registered the stored value is returned.
			 */
			T_Value
			publish();
			
			
			/**
			 * @brief Registers a publisher.
			 * 
			 * Adds a publisher to the list of registered publishers.
			 */
			void
			register_publisher(Publisher<T_Value>* publisher);
			
			
			/**
			 * @brief Registers a subscriber.
			 * 
			 * Adds a subscriber to the list of registered subscribers.
			 */
			void
			register_subscriber(Subscriber<T_Value>* subscriber);
			
			
			/**
			 * @brief Removes a publisher.
			 * 
			 * Removes the given publisher from the list of publishers.
			 */
			void
			remove_publisher(Publisher<T_Value>* publisher);
			
			
			/**
			 * @brief Removes a subscriber.
			 * 
			 * Removes the given subscriber from the list of subscribers.
			 */
			void
			remove_subscriber(Subscriber<T_Value>* subscriber);
			
			
			/**
			 * @brief Stores and redistributes the value.
			 * 
			 * The received value is stored and the receive()-method
			 * of all registered subscribers is called with this value.
			 */
			void
			receive(T_Value value);


		protected:
		
			/**
			 * @brief The stored value.
			 */
			T_Value _value;

			/**
			 * @brief The list of registered publishers.
			 */
			std::list< Publisher<T_Value>* > _publishers;			

			/**
			 * @brief The list of registered subscribers.
			 */
			std::list< Subscriber<T_Value>* > _subscribers;

	};
	
	
	template <typename T_Value>
	Service<T_Value>::Service(std::string name, T_Value initial_value)
	: GenericService::GenericService(name, typeid(initial_value).name()), 
		Publisher<T_Value>::Publisher(0, 
			Publisher<T_Value>::PUSH && Publisher<T_Value>::PULL),
		Subscriber<T_Value>::Subscriber(0, 
			Subscriber<T_Value>::PUSH && Subscriber<T_Value>::PULL),
		_value(initial_value), _publishers(), _subscribers()
	{
	}
	

	template <typename T_Value>
	Service<T_Value>::~Service()
	{		
		ServiceList::remove_service_from_all_lists(get_name());

		while (_publishers.size() > 0)
		{
			Publisher<T_Value>* publisher = _publishers.front();
			publisher->remove_subscriber(this);
			_publishers.remove(publisher);			
		}
		
		while (_subscribers.size() > 0)
		{
			Subscriber<T_Value>* subscriber = _subscribers.front();
			_subscribers.front()->remove_publisher(this);
			_subscribers.remove(subscriber);
		}
	}
	
	
	template <typename T_Value>
	T_Value
	Service<T_Value>::publish()
	{
		if (_publishers.size() > 0)
			_value = _publishers.front()->publish();
		
		return _value;
	}

	
	template <typename T_Value>	
	void
	Service<T_Value>::register_publisher(Publisher<T_Value>* publisher)
	{
		_publishers.push_back(publisher);
	}
	
	
	template <typename T_Value>
	void
	Service<T_Value>::register_subscriber(Subscriber<T_Value>* subscriber)
	{
		_subscribers.push_back(subscriber);
	}
	
	
	template <typename T_Value>
	void
	Service<T_Value>::remove_publisher(Publisher<T_Value>* publisher)
	{
		_publishers.remove(publisher);
	}
	
	
	template <typename T_Value>
	void
	Service<T_Value>::remove_subscriber(Subscriber<T_Value>* subscriber)
	{
		_subscribers.remove(subscriber);
	}	
	
	
	template <typename T_Value>
	void
	Service<T_Value>::receive(T_Value value)
	{
		_value = value;
		
		if (_subscribers.size() > 0)
		{
			typename std::list< Subscriber<T_Value>* >::iterator iter = _subscribers.begin();
			for (; iter != _subscribers.end(); iter++)
				(*iter)->receive(_value);
		}
	}
	
	
} // namespace pubsub

#endif //SERVICE_H
