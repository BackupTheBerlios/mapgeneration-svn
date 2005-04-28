/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef PUBLISHER_H
#define PUBLISHER_H


namespace pubsub
{
	template <typename T_Value>	
	class Publisher;
}


#include <bitset>

#include "subscriber.h"


namespace pubsub
{

	/**
	 * @brief Publisher is the publisher interface of the 
	 * Publish/Subscribe-System.
	 * 
	 * For a generic introduction to this system have a look at the namespace
	 * description for pubsub.
	 * 
	 * @see pubsub
	 */
	template <typename T_Value>
	class Publisher {

		public:
		
			/**
			 * @brief Typedef for the mode.
			 */
			typedef std::bitset<16> D_PublishMode;

			static const int PUSH = 0;
			static const int PULL = 1;


			/**
			 * @brief The contructor.
			 * 
			 * If a valid pointer to a subscriber (not 0) is given, then that
			 * subscriber is registered and the subscribers 
			 * register_publisher()-method is invoked to register the publisher
			 * to the subscriber.
			 * 
			 * @param subscriber A subscriber to register and to register to.
			 * @param publish_mode The used/supported publish_mode.
			 */
			Publisher(Subscriber<T_Value>* subscriber, D_PublishMode publish_mode);
			
			
			/**
			 * @brief The destructor.
			 * 
			 * If a subscriber is known the subscribers
			 * remove_publisher()-method is called.
			 */
			~Publisher();
			
			
			/**
			 * @brief Returns the publish mode.
			 */
			inline const D_PublishMode&
			publish_mode() const;

			
			/**
			 * @brief Virtual function that should return the value that the 
			 * Publisher wants to publish.
			 * 
			 * This function can be called to retrieve the value this
			 * Publisher want to publisher. The return value should be at least
			 * ok, if the publish mode has the pull-flag set.
			 * 
			 * @return The publishers value.
			 */
			virtual T_Value
			publish() = 0;
			
			
			/**
			 * @brief Saves the given subscriber as the only subscriber to
			 * the publisher.
			 * 
			 * @param subscriber The subscriber.
			 */
			virtual void
			register_subscriber(Subscriber<T_Value>* subscriber);
			
			
			/**
			 * @brief Removes the given subscriber.
			 * 
			 * There is no default implementation for this case, because there
			 * is no sure behaviour for this case. Many standard Publishers
			 * will delete themselfes after this call, but users Publishers
			 * may often want another behaviour.
			 * 
			 * @param subscriber The subscriber to remove.
			 */
			virtual void
			remove_subscriber(Subscriber<T_Value>* subscriber) = 0;


		protected:
		
			/**
			 * @brief The mode of the publisher.
			 */
			D_PublishMode _publish_mode;
						
			/**
			 * @brief The only subscriber.
			 */
			Subscriber<T_Value>* _subscriber;						

	};

		
	template <typename T_Value>
	Publisher<T_Value>::Publisher(Subscriber<T_Value>* subscriber, 
		D_PublishMode publish_mode)
	: _subscriber(), _publish_mode(publish_mode)
	{
		if (subscriber != 0)
		{
			register_subscriber(subscriber);			
		}
		
		if (_subscriber != 0)
		{
			_subscriber->register_publisher(this);
		}
	}
	
	
	template <typename T_Value>
	Publisher<T_Value>::~Publisher()
	{
		if (_subscriber != 0) _subscriber->remove_publisher(this);
	}


	/* The return type should be
	 * inline const Publisher<T_Value>::D_PublishMode&
	 * , but that seems not to work with gcc 3.4....
	 */
	template <typename T_Value>
	inline const std::bitset<16>&
	Publisher<T_Value>::publish_mode() const
	{
		return _publish_mode;
	}


	template <typename T_Value>
	void
	Publisher<T_Value>::register_subscriber(Subscriber<T_Value>* subscriber)
	{
		_subscriber = subscriber;
	}

} // namespace publisher

#endif //PUBLISHER_H
