/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef FIXEDSIZEQUEUE_H
#define FIXEDSIZEQUEUE_H

#include <deque>
#include <utility>

namespace mapgeneration_util
{

	template <typename T_ElemType>
	class FixedSizeQueue
	{
		
		public:
			
			typedef typename std::deque<T_ElemType>::const_iterator const_iterator;
			typedef typename std::deque<T_ElemType>::iterator iterator;
			typedef typename std::deque<T_ElemType>::size_type size_type;


			FixedSizeQueue(size_type capacity = 4,
				bool garantuee_uniquness = true, bool init_empty = true);
			
			
			FixedSizeQueue(const FixedSizeQueue& fixed_size_queue);


			const_iterator
			begin() const;
	
			
			size_type
			capacity() const;
			
			
			void
			clear();
			
			
			const_iterator
			end() const;
			
			
			const T_ElemType&
			front() const;
			
			
			bool
			get_garantuee_uniqueness() const;
			
			
			const T_ElemType&
			operator[](size_type pos) const;
			
			
			void
			push(const T_ElemType& element);


			void
			set_garantuee_uniqueness(bool garantuee_uniqueness);
			
			
			size_type
			size() const;

			
		protected:
			
			size_type _capacity;

			std::deque<T_ElemType> _fixed_size_queue;

			bool _garantuee_uniqueness;
	
	};


	/**
	 * Implementation of the FixedSizeQueue functions.
	 */

	template <typename T_ElemType>
	FixedSizeQueue<T_ElemType>::FixedSizeQueue(
		FixedSizeQueue<T_ElemType>::size_type capacity,
		bool garantuee_uniqueness, bool init_empty
	)
	: _capacity(capacity), _fixed_size_queue(),
		_garantuee_uniqueness(garantuee_uniqueness)
	{
		if (!init_empty)
			_fixed_size_queue.resize(_capacity);
	}
	
	
	template <typename T_ElemType>
	FixedSizeQueue<T_ElemType>::FixedSizeQueue(const FixedSizeQueue<T_ElemType>& fixed_size_queue)
	: _capacity(fixed_size_queue._capacity),
		_fixed_size_queue(fixed_size_queue._fixed_size_queue),
		_garantuee_uniqueness(fixed_size_queue._garantuee_uniqueness)
	{
	}
	
	
	template <typename T_ElemType>
	typename FixedSizeQueue<T_ElemType>::const_iterator
	FixedSizeQueue<T_ElemType>::begin() const
	{
		return _fixed_size_queue.begin();
	}
	
	
	template <typename T_ElemType>
	typename FixedSizeQueue<T_ElemType>::size_type
	FixedSizeQueue<T_ElemType>::capacity() const
	{
		return _capacity;
	}
	
	
	template <typename T_ElemType>
	void
	FixedSizeQueue<T_ElemType>::clear()
	{
		_fixed_size_queue.clear();
	}
	
	
	template <typename T_ElemType>
	typename FixedSizeQueue<T_ElemType>::const_iterator
	FixedSizeQueue<T_ElemType>::end() const
	{
		return _fixed_size_queue.end();
	}
	
	
	template <typename T_ElemType>
	const T_ElemType&
	FixedSizeQueue<T_ElemType>::front() const
	{
		return _fixed_size_queue.front();
	}
	
	
	template <typename T_ElemType>
	bool
	FixedSizeQueue<T_ElemType>::get_garantuee_uniqueness() const
	{
		return _garantuee_uniqueness;
	}
	
	
	template <typename T_ElemType>
	const T_ElemType&
	FixedSizeQueue<T_ElemType>::operator[](FixedSizeQueue<T_ElemType>::size_type pos) const
	{
		return _fixed_size_queue[pos];
	}
	
	
	template <typename T_ElemType>
	void
	FixedSizeQueue<T_ElemType>::push(const T_ElemType& element)
	{
		if (_garantuee_uniqueness)
		{
			/*
			 *  @todo: Why do we find find without std:: and why does it not
			 * work with std:: ????
			 */
			iterator find_iter 
				= find(_fixed_size_queue.begin(), _fixed_size_queue.end(), element);
		
			if (find_iter == _fixed_size_queue.end())
			{
				while (size() >= capacity())
				{
					_fixed_size_queue.pop_back();
				}
		
				_fixed_size_queue.push_front(element);
			} else
			{
				_fixed_size_queue.push_front(*find_iter);
				_fixed_size_queue.erase(find_iter);
			}
		} else
		{
			while(size() >= capacity())
			{
				_fixed_size_queue.pop_back();
			}
			
			_fixed_size_queue.push_front(element);
		}
	}
	
	
	template <typename T_ElemType>
	void
	FixedSizeQueue<T_ElemType>::set_garantuee_uniqueness(bool garantuee_uniqueness)
	{
		_garantuee_uniqueness = garantuee_uniqueness;
	}
	
	
	template <typename T_ElemType>
	typename FixedSizeQueue<T_ElemType>::size_type
	FixedSizeQueue<T_ElemType>::size() const
	{
		return _fixed_size_queue.size();
	}

} // namespace mapgeneration_util

#endif //FIXEDSIZEQUEUE_H
