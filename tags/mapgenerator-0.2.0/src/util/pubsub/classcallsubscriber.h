/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef CLASSCALLSUBSCRIBER_H
#define CLASSCALLSUBSCRIBER_H


#include <string>
#include "subscriber.h"


namespace pubsub
{

	/**
	 * @brief A Subscriber that calls a function inside a class when it's
	 * receive method is called.
	 * 
	 * The template has two parameters: The type of the class in which the
	 * function is and the typical T_Value. The contructor needs three
	 * parameter: The publisher, a pointer to the class in which the function
	 * to call is located and a pointer to the function inside the class.
	 * 
	 * @see Subscriber
	 */
	template <typename T_Class, typename T_Value>
	class ClassCallSubscriber : public Subscriber<T_Value>
	{
		
		public:
		
			ClassCallSubscriber(Publisher<T_Value>* publisher, T_Class* class_pointer, 
				void (T_Class::*receive_pointer) (T_Value));


			ClassCallSubscriber(Publisher<T_Value>* publisher, T_Class* class_pointer, 
				void (T_Class::*const_receive_pointer) (T_Value) const);

			
			void
			receive(T_Value value);
			
			
			void
			remove_publisher(Publisher<T_Value>* publisher);
		
		
		private:
		
			T_Class* _class_pointer;
		
			void
			(T_Class::*_receive_pointer) (T_Value);
			
			void
			(T_Class::*_const_receive_pointer) (T_Value) const;
		
	};
	
	
	template <typename T_Class, typename T_Value>
	ClassCallSubscriber<T_Class, T_Value>::ClassCallSubscriber(
		Publisher<T_Value>* publisher, T_Class* class_pointer, 
		void (T_Class::*receive_pointer) (T_Value))
	: Subscriber<T_Value>::Subscriber(publisher, Subscriber<T_Value>::PUSH),
		_class_pointer(class_pointer), 
		_receive_pointer(receive_pointer), _const_receive_pointer(0)
	{
	}
	
	
	template <typename T_Class, typename T_Value>
	ClassCallSubscriber<T_Class, T_Value>::ClassCallSubscriber(
		Publisher<T_Value>* publisher, T_Class* class_pointer, 
		void (T_Class::*const_receive_pointer) (T_Value) const)
	: Subscriber<T_Value>::Subscriber(publisher, Subscriber<T_Value>::PUSH),
		_class_pointer(class_pointer), _receive_pointer(0), 
		_const_receive_pointer(const_receive_pointer)
	{
	}


	template <typename T_Class, typename T_Value>
	void
	ClassCallSubscriber<T_Class, T_Value>::receive(T_Value value)
	{
		if (_receive_pointer != 0)
			(_class_pointer->*_receive_pointer)(value);
		else
			(_class_pointer->*_const_receive_pointer)(value);
	}
	
	
	template <typename T_Class, typename T_Value>
	void
	ClassCallSubscriber<T_Class, T_Value>::remove_publisher(Publisher<T_Value>* publisher)
	{
		delete this;
	}

	

} // namespace pubsub

#endif //CALLPUBLISHER_H
