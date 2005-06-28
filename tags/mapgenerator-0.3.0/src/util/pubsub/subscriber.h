/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H


namespace pubsub
{
	template <typename T_Value>	
	class Subscriber;
}


#include <bitset>

#include "publisher.h"


namespace pubsub
{

	/**
	 * @brief Subscriber is the subscriber interface of the 
	 * Publish/Subscribe-System.
	 * 
	 * For a generic introduction to this system have a look at the namespace
	 * description for pubsub.
	 * 
	 * @see pubsub
	 */
	template <typename T_Value>
	class Subscriber {

		public:
		
			/**
			 * @brief Typedef for the receive mode.
			 */
			typedef std::bitset<16> D_ReceiveMode;
		
			static const int PUSH = 0;
			static const int PULL = 1;

		
			/**
			 * @brief The constructor.
			 * 
			 * If a valid pointer to a publisher (not 0) is given, then that
			 * publisher is registered and the publishers
			 * register_subscriber()-method is invoked to register the 
			 * subscriber to the publisher.
			 * 
			 * @param publisher The publisher to register to.
			 * @param receive_mode The receive mode.
			 */
			Subscriber(Publisher<T_Value>* publisher, D_ReceiveMode receive_mode);


			/**
			 * @brief The destructor.
			 * 
			 * If a Publisher is know the Publishers remove_subscriber()-method
			 * is called.
			 */
			~Subscriber();
			
			
			/**
			 * @brief Returns areference to the receive mode.
			 * 
			 * @return A reference to the receive mode.
			 */
			inline const D_ReceiveMode&
			reveice_mode() const;
			
			
			/**
			 * @brief Accepts a new value.
			 * 
			 * This virtual method must accept a value that is by this way
			 * send to the subscriber. The action taken must at least make some
			 * sense if the subscribers receive_mode contains the push-flag.
			 */
			virtual void
			receive(T_Value value) = 0;
			
			
			/**
			 * @brief Saves the publisher as only publisher to this subscriber.
			 * 
			 * @param publisher
			 */
			virtual void
			register_publisher(Publisher<T_Value>* publisher);
			
			
			/**
			 * @brief Is called when the publisher wants to disconnect.
			 * 
			 * There is no default implementation given, because the subscribers
			 * behaviour may vary. Many standard subscribers delete themselfes,
			 * for details see the subscribers documentation.
			 * 
			 * @param publisher The publisher to remove.
			 */
			virtual void
			remove_publisher(Publisher<T_Value>* publisher) = 0;


		protected:			
		
			/**
			 * @brief The only publisher.
			 */
			Publisher<T_Value>* _publisher;
			
			
			/**
			 * @brief The receive mode.
			 */
			D_ReceiveMode _receive_mode;

	};
	
	
	template <typename T_Value>
	Subscriber<T_Value>::Subscriber(Publisher<T_Value>* publisher, 
		D_ReceiveMode receive_mode)
	: _publisher(publisher), _receive_mode(receive_mode)
	{
		if (_publisher != 0) _publisher->register_subscriber(this);
	}
	
	
	template <typename T_Value>
	Subscriber<T_Value>::~Subscriber()
	{
		if (_publisher != 0) _publisher->remove_subscriber(this);
	}
	
	
	/* The return type should be
	 * inline const Subscriber<T_Value>::D_ReceiveMode&
	 * , but that seems not to work with gcc 3.4....
	 */
	template <typename T_Value>
	inline const std::bitset<16>&
	Subscriber<T_Value>::reveice_mode() const
	{
		return _receive_mode;
	}

	
	template <typename T_Value>
	void
	Subscriber<T_Value>::register_publisher(Publisher<T_Value>* publisher)
	{
		_publisher = publisher;
	}
	

} // namespace pubsub

#endif //SUBSCRIBER_H
