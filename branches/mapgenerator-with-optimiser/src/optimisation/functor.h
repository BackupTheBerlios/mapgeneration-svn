/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef FUNCTOR_H
#define FUNCTOR_H

#include <cassert>

namespace mapgeneration_opt
{
	
	template<typename T_ReturnValue, typename T_Parameter>
	class GenericFunctor
	{
		
		public:
			
			virtual T_ReturnValue operator()(T_Parameter* pointer = 0) = 0;
			
	};
	
	
	template <typename T_Class, typename T_ReturnValue, typename T_Parameter>
	class Functor : public GenericFunctor<T_ReturnValue, T_Parameter>
	{
		
		public:
			
			Functor(T_Class* obj,
				T_ReturnValue (T_Class::* function)(T_Parameter* pointer) )
			: _function(function), _obj(obj)
			{
				assert(function && _obj);
			}
			
			
			inline virtual T_ReturnValue
			operator()(T_Parameter* pointer = 0)
			{
				return (_obj->*_function)(pointer);
			}
			
			
		private:
			
			T_ReturnValue (T_Class::* _function)(T_Parameter*);
			
			
			T_Class* _obj;
			
	};
	
} //namespace mapgeneration_opt

#endif //FUNCTOR_H
