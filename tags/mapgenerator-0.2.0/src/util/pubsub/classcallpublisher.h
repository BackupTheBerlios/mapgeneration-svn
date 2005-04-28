/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef CLASSCALLPUBLISHER_H
#define CLASSCALLPUBLISHER_H


#include <string>
#include "publisher.h"


namespace pubsub
{

	/**
	 * @brief A Publisher that calls a function inside a class when a value
	 * is request by publish().
	 * 
	 * In addition to the default T_Value another parameter is needed that
	 * gives the type of the class in which the function is located. The
	 * constructor need three parameter: The typcical subscriber (or 0), 
	 * a pointer to the instance of the class and a pointer to a function inside
	 * that class.
	 * 
	 * @see Publisher
	 */
	template <typename T_Class, typename T_Value>
	class ClassCallPublisher : public Publisher<T_Value>
	{
		
		public:
		
			ClassCallPublisher(Subscriber<T_Value>* subscriber, T_Class* class_pointer, 
				T_Value (T_Class::*publish_pointer) ());


			ClassCallPublisher(Subscriber<T_Value>* subscriber, T_Class* class_pointer, 
				T_Value (T_Class::*const_publish_pointer) () const);


			T_Value
			publish();
			
			
			void
			remove_subscriber(Subscriber<T_Value>* subscriber);
		
		
		private:
		
			T_Class* _class_pointer;
		
			T_Value
			(T_Class::*_publish_pointer) ();
			
			T_Value
			(T_Class::*_const_publish_pointer) () const;
		
	};
	
	
	template <typename T_Class, typename T_Value>
	ClassCallPublisher<T_Class, T_Value>::ClassCallPublisher(
		Subscriber<T_Value>* subscriber, T_Class* class_pointer, 
		T_Value (T_Class::*publish_pointer) ())
	: Publisher<T_Value>::Publisher(subscriber, Publisher<T_Value>::PULL), 
		_class_pointer(class_pointer), _publish_pointer(publish_pointer), 
		_const_publish_pointer(0)
	{
	}
	
	
	template <typename T_Class, typename T_Value>
	ClassCallPublisher<T_Class, T_Value>::ClassCallPublisher(
		Subscriber<T_Value>* subscriber, T_Class* class_pointer, 
		T_Value (T_Class::*const_publish_pointer) () const)
	: Publisher<T_Value>::Publisher(subscriber, Publisher<T_Value>::PULL), 
		_class_pointer(class_pointer), _publish_pointer(0), 
		_const_publish_pointer(const_publish_pointer)
	{
	}


	template <typename T_Class, typename T_Value>
	T_Value
	ClassCallPublisher<T_Class, T_Value>::publish()
	{
		if (_publish_pointer != 0)		
			return (_class_pointer->*_publish_pointer)();

		return (_class_pointer->*_const_publish_pointer)();
	}
	
	
	template <typename T_Class, typename T_Value>
	void
	ClassCallPublisher<T_Class, T_Value>::remove_subscriber(Subscriber<T_Value>* subscriber)
	{
		delete this;
	}

	

} // namespace pubsub

#endif //CALLPUBLISHER_H
