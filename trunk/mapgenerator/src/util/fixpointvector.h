/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef FIXPOINTVECTOR_H
#define FIXPOINTVECTOR_H

#include <utility>
#include <vector>
#include "serializer.h"

namespace mapgeneration_util
{


	/** 
	 * \brief
	 * An extension of the standard vector class. Deleted elements leave a hole,
	 * that is filled with the next inserted element.
	 * 
	 * The FixpointVector-Class does not store the template-type in the vector, but
	 * pairs of <bool, template-type>. The bool is true if the position is currently
	 * used and false otherwise. The store functions need just a template-type as 
	 * input, the pairs are created internally. The output functions return the
	 * pairs! The user has to check if the returned element is valid.*/
	template <typename T_ElemType>
	class FixpointVector : protected std::vector< std::pair<bool, T_ElemType> >
	{
	
		public:
	
			/**
			 * \brief A stl compatible iterator.
			 */
			class iterator
			{
				
				friend class FixpointVector<T_ElemType>;
				
				public:
					iterator (std::pair<bool, T_ElemType>* start_position, int size);
					
					iterator next_pair();
				
					inline bool operator==(const iterator& iter);
					inline bool operator!=(const iterator& iter);
					iterator operator++();
					inline std::pair<bool, T_ElemType>& operator*();
					inline std::pair<bool, T_ElemType>* operator->();
				
					inline int position_number();
			
				private:
					std::pair<bool, T_ElemType>* _start_position;
					std::pair<bool, T_ElemType>* _end_position;
					std::pair<bool, T_ElemType>* _position;				
			};
			

			/**
			 * \brief A stl compatible const iterator.
			 */
			class const_iterator
			{
				public:
					const_iterator (const std::pair<bool, T_ElemType>* start_position, int size);
					
					const_iterator next_pair();
				
					inline bool operator==(const const_iterator& iter) const;
					inline bool operator!=(const const_iterator& iter) const;
					const_iterator operator++();
					inline const std::pair<bool, T_ElemType>& operator*() const;
					inline const std::pair<bool, T_ElemType>* operator->() const;
				
					inline int position_number();
			
				private:
					std::pair<bool, T_ElemType>* _start_position;
					std::pair<bool, T_ElemType>* _end_position;
					std::pair<bool, T_ElemType>* _position;				
			};
	

			/**
			 * \brief The standard constructor creates an empty FixpointVector.
			 */
			FixpointVector();

			
			/**
			 * \brief Returns an iterator pointing at the first element.
			 * @return An iterator pointing at the first element.
			 */
			inline iterator
			begin();
			
			
			/**
			 * \brief Returns an const_iterator pointing at the first element.
			 * @return An const_iterator pointing at the first element.
			 */
			inline const_iterator
			begin() const;
	
			
			/**
			 * \brief Returns the current capacity of the FixpointVector.
			 * @return The current capacity of the FixpointVector.
			 */
			inline size_t
			capacity() const;
			
			
			/**
			 * \brief Removes all elements from the FixpointVector.
			 */
			inline void
			clear();
			
			
			/**
			 * \brief Uses the deserialize functions to deserialize the
			 * FixpointVector.
			 * @param io_stream The iostream to deserialize from.
			 */
			void
			deserialize(std::istream& i_stream);

			
			/**
			 * \brief Returns an iterator that points "behind" the last element.
			 * @return A pointer that points "behind" the last element.
			 */
			inline iterator
			end();


			/**
			 * \brief Returns an iterator that points "behind" the last element.
			 * @return A pointer that points "behind" the last element.
			 */			
			inline const_iterator
			end() const;
			
			
			/**
			 * \brief Erases an element from the vector and returns an iterator
			 * that points to the next element behind the erased one.
			 * @return Iterator that points to the next element behind the 
			 * erased one.
			 * @param i An iterator pointing at the element to erase.
			 */
			iterator
			erase(iterator& i);
			
			
			/**
			 * @brief Erases the element from the vector and returns an iterator
			 * that points to the next element behind the erased one.
			 * @return Iterator that points to the next element behind the 
			 * erased one.
			 * @param index The index pointing at the element to erase.
			 */
			iterator
			erase(int index);


			/**
			 * \brief Inserts an element.
			 * @return Position of the inserted element.
			 * @param elem The element to insert.
			 */
			int
			insert(const T_ElemType& elem);


			/**
			 * \brief Returns an "element pair".
			 * @return (true, T_ElemType) if element is valid, 
			 * (false, T_ElemType) otherwise
			 * @param pos The position of the element to return.
			 */
			inline std::pair<bool, T_ElemType>&
			operator[] (int pos);


			/**
			 * \brief Returns a const "element pair".
			 * @param pos The position of the element to return.
			 * @return (true, T_ElemType) if element is valid, 
			 * (false, T_ElemType) otherwise
			 */
			inline const std::pair<bool, T_ElemType>&
			operator[] (int pos) const;


			/**
			 * \brief Serializes the whole FixpointVector and its elements.
			 * @param io_stream The iostream to serialize to.
			 */
			void
			serialize(std::ostream& o_stream) const;


			/**
			 * \brief Returns the current number of elements in the 
			 * FixpointVector.
			 * @return Current number of elements.
			 */
			inline size_t 
			size() const;

		
			/**
			 * \brief Returns the current number of elements plus the number
			 * of holes in the vector.
			 * @return Size including holes.
			 */
			inline size_t
			size_including_holes() const;
			
			
			/**
			 * \brief Returns the memory needed in bytes. Results are not 
			 * accurate.
			 * @return Memory usage in byte.
			 */
			inline int
			size_of() const;

	
		protected:

		private:
		
			/**
			 * \brief The vector of free positions in the data vector.
			 * 
			 * This vector contains all free positions in the vector. The data
			 * is stored in the FixpointVector itself,it is a subclass of
			 * vector.
			 */
			std::vector<int> _free_positions;
	};


	/**
	 * Implementation of the iterator functions
	 */

	template <typename T_ElemType>
	FixpointVector<T_ElemType>::iterator::iterator
		(std::pair<bool, T_ElemType>* start_position, int size)
	: _start_position(start_position), _end_position(start_position + size), 
		_position(start_position)
	{
	}


	template <typename T_ElemType>
	inline typename FixpointVector<T_ElemType>::iterator
	FixpointVector<T_ElemType>::iterator::next_pair()
	{
		++_position;
		
		return *this;
	}


	template <typename T_ElemType>
	inline bool 
	FixpointVector<T_ElemType>::iterator::operator==(const iterator& a)
	{
		if (a._position == _position)
			return true;

		return false;
	}


	template <typename T_ElemType>
	inline bool 
	FixpointVector<T_ElemType>::iterator::operator!=(const iterator& a)
	{
		return !operator==(a);
	}


	template <typename T_ElemType>
	typename FixpointVector<T_ElemType>::iterator
	FixpointVector<T_ElemType>::iterator::operator++()
	{
		/*do
		{
			++_position;
		} while (_position >= _start_position && _position < _end_position && 
			(*_position).first == false);*/
		
		do
		{
			++_position;
		} while (_position < _end_position && (*_position).first == false);
	
		return *this;
	}


	template <typename T_ElemType>
	inline std::pair<bool, T_ElemType>&
	FixpointVector<T_ElemType>::iterator::operator*()
	{
		return *_position;
	}
	
	
	template <typename T_ElemType>
	inline std::pair<bool, T_ElemType>*
	FixpointVector<T_ElemType>::iterator::operator->()
	{
		return _position;
	}


	template <typename T_ElemType>
	inline int
	FixpointVector<T_ElemType>::iterator::position_number()
	{
		return _position - _start_position;
	}
	
	
	
	
	/**
	 * Implementation of the const_iterator functions
	 */

	template <typename T_ElemType>
	FixpointVector<T_ElemType>::const_iterator::const_iterator
		(const std::pair<bool, T_ElemType>* start_position, int size)
	{
		_start_position = const_cast< std::pair<bool, T_ElemType>* >(start_position);
		_position = _start_position;
		_end_position = _start_position + size;
	}


	template <typename T_ElemType>
	inline typename FixpointVector<T_ElemType>::const_iterator
	FixpointVector<T_ElemType>::const_iterator::next_pair()
	{
		++_position;
		
		return *this;
	}


	template <typename T_ElemType>
	inline bool 
	FixpointVector<T_ElemType>::const_iterator::operator==(
		const const_iterator& a) const
	{
		if (a._position == _position)
			return true;

		return false;
	}


	template <typename T_ElemType>
	inline bool 
	FixpointVector<T_ElemType>::const_iterator::operator!=(
		const const_iterator& a) const
	{
		return !operator==(a);
	}


	template <typename T_ElemType>
	typename FixpointVector<T_ElemType>::const_iterator
	FixpointVector<T_ElemType>::const_iterator::operator++()
	{
		/*do
		{
			++_position;
		} while (_position >= _start_position && _position < _end_position && 
			(*_position).first == false);*/
			
		do
		{
			++_position;
		} while (_position < _end_position && (*_position).first == false);
		
		return *this;
	}


	template <typename T_ElemType>
	inline const std::pair<bool, T_ElemType>&
	FixpointVector<T_ElemType>::const_iterator::operator*() const
	{
		return *_position;
	}
	
	
	template <typename T_ElemType>
	inline const std::pair<bool, T_ElemType>*
	FixpointVector<T_ElemType>::const_iterator::operator->() const
	{
		return _position;
	}


	template <typename T_ElemType>
	inline int
	FixpointVector<T_ElemType>::const_iterator::position_number()
	{
		return _position - _start_position;
	}

	
	
	/**
	 * Implementation of the FixpointVector functions.
	 */

	template <typename T_ElemType>
	FixpointVector<T_ElemType>::FixpointVector ()
	: _free_positions()
	{
	}


	template <typename T_ElemType>
	inline typename FixpointVector<T_ElemType>::iterator
	FixpointVector<T_ElemType>::begin()
	{
		std::pair<bool, T_ElemType>* first_element
			= &*std::vector< std::pair<bool, T_ElemType> >::begin();
		return iterator(first_element, size_including_holes());
	}
	
	
	template <typename T_ElemType>
	inline typename FixpointVector<T_ElemType>::const_iterator
	FixpointVector<T_ElemType>::begin() const
	{
		const std::pair<bool, T_ElemType>* first_element
			= &*std::vector< std::pair<bool, T_ElemType> >::begin();
		return const_iterator(first_element, size_including_holes());
	}


	template <typename T_ElemType>
	inline size_t FixpointVector<T_ElemType>::capacity() const
	{
		/** @todo Check if this works correctly!!! */
		return std::vector< std::pair<bool, T_ElemType> >::capacity() 
			+ _free_positions.size();
	}
	
	
	template <typename T_ElemType>
	inline void
	FixpointVector<T_ElemType>::clear()
	{
		std::vector< std::pair<bool, T_ElemType> >::clear();
		_free_positions.clear();
	}
	
	
	template <typename T_ElemType>
	void
	FixpointVector<T_ElemType>::deserialize(std::istream& i_stream)
	{
		clear();

		int vector_size;
		Serializer::deserialize(i_stream, vector_size);

		for (int i=0; i<vector_size; ++i)
		{
			bool used;
			Serializer::deserialize(i_stream, used);
			
			std::pair<bool, T_ElemType> p;
			p.first = used;
			
			if (used)
			{
				T_ElemType elem;
				Serializer::deserialize(i_stream, elem);
				p.second = elem;
			}
			
			push_back(p);
		}
		
		
		//Serializer::deserialize(io_stream, *static_cast< std::vector< std::pair<bool, T_Elem> >* >(this));
		Serializer::deserialize(i_stream, _free_positions);
	}


	template <typename T_ElemType>
	inline typename FixpointVector<T_ElemType>::iterator
	FixpointVector<T_ElemType>::end()
	{
		std::pair<bool, T_ElemType>* last_element
			= &*std::vector< std::pair<bool, T_ElemType> >::end();
		return iterator(last_element, 0);
	}
	
	
	template <typename T_ElemType>
	inline typename FixpointVector<T_ElemType>::const_iterator
	FixpointVector<T_ElemType>::end() const
	{
		const std::pair<bool, T_ElemType>* last_element
			= &*std::vector< std::pair<bool, T_ElemType> >::end();
		return const_iterator(last_element, 0);
	}
	


	template <typename T_ElemType>
	typename FixpointVector<T_ElemType>::iterator
	FixpointVector<T_ElemType>::erase
		(typename FixpointVector<T_ElemType>::iterator& iter)
	{
		if (iter.position_number() == size_including_holes()-1)
		{
			std::vector< std::pair<bool, T_ElemType> >::pop_back();
		} else
		{
			_free_positions.push_back (iter.position_number());
			(*iter).first = false;
			++iter;
		}

		return iter;
	}
	
	
	template<typename T_ElemType>
	typename FixpointVector<T_ElemType>::iterator
	FixpointVector<T_ElemType>::erase(int index)
	{
		if (index > size_including_holes() - 1)
		{
			return end();
		} else
		{
			iterator iter = begin();
			iter._position = &(operator[](index));
			
			return erase(iter);
		}
	}
	
	
	template <typename T_ElemType>
	int
	FixpointVector<T_ElemType>::insert (const T_ElemType& elem)
	{
		if (!_free_positions.size())
		{
			push_back (std::pair<bool, T_ElemType>(true, elem));

			return size()-1;
		}
		else 
		{
			int free_position = _free_positions.back();
			_free_positions.pop_back();
			std::vector< std::pair<bool, T_ElemType> >::operator[](free_position) 
				= std::pair<bool, T_ElemType>(true, elem);

			return free_position;
		}
	}

	
	template <typename T_Elem>
	inline std::pair<bool, T_Elem>&
	FixpointVector<T_Elem>::operator[] (int pos)
	{
		return std::vector< std::pair<bool, T_Elem> >::operator[](pos);
	}


	template <typename T_Elem>
	inline const std::pair<bool, T_Elem>&
	FixpointVector<T_Elem>::operator[] (int pos) const
	{
		return std::vector< std::pair<bool, T_Elem> >::operator[](pos);
	}


	template <typename T_Elem>
	void
	FixpointVector<T_Elem>::serialize(std::ostream& o_stream) const
	{
		int vector_size = size_including_holes();
		Serializer::serialize(o_stream, vector_size);

		typename FixpointVector<T_Elem>::const_iterator iter = begin();
		typename FixpointVector<T_Elem>::const_iterator iter_end = end();
		for (; iter != iter_end; iter.next_pair())
		{
			bool used = (*iter).first;
			Serializer::serialize(o_stream, used);
			
			if (used)
				Serializer::serialize(o_stream, (*iter).second);
		}
	
		//Serializer::serialize(io_stream, *static_cast< std::vector< std::pair<bool, T_Elem> >* >(this));
		Serializer::serialize(o_stream, _free_positions);
	}
	

	template <typename T_ElemType>
	inline size_t
	FixpointVector<T_ElemType>::size() const
	{
		return std::vector<std::pair<bool, T_ElemType> >::size() 
			- _free_positions.size();
	}


	template <typename T_ElemType>
	inline size_t
	FixpointVector<T_ElemType>::size_including_holes() const
	{
		return std::vector<std::pair<bool, T_ElemType> >::size();
	}
	
	
	template <typename T_ElemType>
	inline int
	FixpointVector<T_ElemType>::size_of() const
	{
		return (this->size() * sizeof(T_ElemType)) + (_free_positions.size() * sizeof(int)) + sizeof(FixpointVector<T_ElemType>);
	}


} // namespace mapgeneration_util

#endif //FIXPOINTVECTOR_H
