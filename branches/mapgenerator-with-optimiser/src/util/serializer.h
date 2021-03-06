/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef SERIALIZER_H
#define SERIALIZER_H

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#ifdef HAVE_STDINT_H
	#include "stdint.h"
#endif

#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <utility>
#include <vector>

namespace mapgeneration_util
{

	/**
	 * @brief This class contains some static functions to serialize
	 * and deserialize standard objects. Be aware: Serialization 
	 * is not machine-independent at the moment!
	 * 
	 * Serializer uses stream to serialize and deserialize data. All 
	 * deserializer and all serializer functions have the same basic form. 
	 * Functions for most basic data types are provided in this class, other
	 * classes have to implement a serialize and deserialize function that has
	 * the same parameters as the functions in this class.
	 * 
	 * Besides the standard functions there are some wrapper functions to
	 * directly deserialize from/serialize to strings.
	 */
	class Serializer{
	public:
		
		/**
		 * @brief Deserializes an object by calling it's serialize method.
		 * 
		 * This is the most generic variant of deserialize and tries to call
		 * the object deserialize method.
		 */
		template <typename T_Obj>
		inline static void
		deserialize(std::istream& i_stream, T_Obj& obj);
		
		
		inline static void
		deserialize(std::istream& i_stream, bool& boolean);		
		
		inline static void
		deserialize(std::istream& i_stream, double& doub);
		
		inline static void
		deserialize(std::istream& i_stream, float& floa);
		
//		inline static void
//		deserialize(std::istream& i_stream, int& integer);
		
//		inline static void
//		deserialize(std::istream& i_stream, unsigned int& integer);
		
//		inline static void
//		deserialize(std::istream& i_stream, long& lon);
				
//		inline static void
//		deserialize(std::istream& i_stream, long unsigned int& lon);
		
		inline static void
		deserialize(std::istream& i_stream, int32_t& value);
		
		inline static void
		deserialize(std::istream& i_stream, uint32_t& value);
		
		inline static void
		deserialize(std::istream& i_stream, int64_t& value);
		
		inline static void
		deserialize(std::istream& i_stream, uint64_t& value);
		
		
		template <typename T_1, typename T_2>
		inline static void
		deserialize(std::istream& i_stream, std::pair<T_1, T_2>& p); 
		
		
		inline static void
		deserialize(std::istream& i_stream, std::string& str);
		
		
		template <typename T_Elem>
		static void
		deserialize(std::istream& i_stream, std::list<T_Elem>& list);

		
		template <typename T_Elem>
		static void
		deserialize(std::istream& i_stream, std::vector<T_Elem>& container);
		
		
		template < typename T_Key, typename T_Elem,
			typename T_Comp >
		static void
		deserialize(std::istream& i_stream,
			std::multimap<T_Key, T_Elem, T_Comp>& constainer);
		
		
		/**
		 * @brief Deserializes an object from a string.
		 * 
		 * This is just a wrapper that creates a stream from the
		 * string and calles another deserializer.
		 */
		template <typename T_Obj>
		static void
		deserialize(std::string& str, T_Obj& obj);


		/**
		 * @brief Deserializes an object from a string. The return type
		 * has to be specified explicitly (deserialize<TYPE>(...)).
		 * 
		 * This is just a wrapper that creates the object and calls
		 * deserialize(std::string&, T_Obj&).
		 */
		template <typename T_Obj>
		static T_Obj
		deserialize(std::string& str);
		
		
		/**
		 * @brief Deserializes an object from an iostream. The return type
		 * has to be specified explicitly (deserialize<TYPE>(...)).
		 * 
		 * This is just a wrapper that creates the object and calls
		 * deserialize(std::iostream&, T_Obj&).
		 */
		template <typename T_Obj>
		static T_Obj
		deserialize(std::istream& i_stream);
		
		
		/**
		 * @brief Serializes obj into iostream by calling obj.serialize.
		 * 
		 * This is the most generic variant of serialize and tries to call
		 * the object serialize method.
		 */
		template <typename T_Obj>
		inline static void
		serialize(std::ostream& o_stream, const T_Obj& obj);
		
		
		inline static void
		serialize(std::ostream& o_stream, const bool boolean);
		
		inline static void
		serialize(std::ostream& o_stream, const double doub);
		
		inline static void
		serialize(std::ostream& o_stream, const float floa);
		
//		inline static void
//		serialize(std::ostream& o_stream, const int integer);
		
//		inline static void
//		serialize(std::ostream& o_stream, const unsigned int integer);
		
//		inline static void
//		serialize(std::ostream& o_stream, const long lon);
		
//		inline static void
//		serialize(std::ostream& o_stream, const long unsigned int lon);
		
		inline static void
		serialize(std::ostream& o_stream, const int32_t value);
		
		inline static void
		serialize(std::ostream& o_stream, const uint32_t value);
		
		inline static void
		serialize(std::ostream& o_stream, const int64_t value);
		
		inline static void
		serialize(std::ostream& o_stream, const uint64_t value);		


		template <typename T_1, typename T_2>
		inline static void
		serialize(std::ostream& o_stream, const std::pair<T_1, T_2>& p);
		
		
		inline static void
		serialize(std::ostream& o_stream, const std::string& str);


		template <typename T_ElemType>
		static void
		serialize(std::ostream& o_stream, const std::list<T_ElemType>& list);


		template <typename T_ElemType>
		static void
		serialize(std::ostream& o_stream, const std::vector<T_ElemType>& vec);
		
		
		template < typename T_Key, typename T_Elem,
			typename T_Comp >
		static void
		serialize(std::ostream& o_stream,
			const std::multimap< T_Key, T_Elem, T_Comp>& constainer);
		
		
		/**
		 * @brief Serializes the object into a string.
		 * 
		 * This is just a wrapper that uses a call to 
		 * Serializer::serialize(iostream, obj) to serialize the object and
		 * returns the result as a string.
		 * @return The resulting string.
		 */
		template <typename T_Obj>
		static std::string
		serialize(const T_Obj& obj);
		
	};
	
	
	template <typename T_Obj>
	inline void
	Serializer::deserialize(std::istream& i_stream, T_Obj& obj)
	{
		obj.deserialize(i_stream);
	}


	inline void
	Serializer::deserialize(std::istream& i_stream, bool& boolean)
	{
		i_stream.read(reinterpret_cast<char*>(&boolean), sizeof(boolean));
	}
		
	
	inline void
	Serializer::deserialize(std::istream& i_stream, double& doub)
	{
		i_stream.read(reinterpret_cast<char*>(&doub), sizeof(double));
	}
	
	
	inline void
	Serializer::deserialize(std::istream& i_stream, float& floa)
	{
		i_stream.read(reinterpret_cast<char*>(&floa), sizeof(float));
	}
	
	
/*	
	inline void
	Serializer::deserialize(std::istream& i_stream, int& integer)
	{
		i_stream.read(reinterpret_cast<char*>(&integer), sizeof(int));
	}
	
	
	inline void
	Serializer::deserialize(std::istream& i_stream, unsigned int& integer)
	{
		i_stream.read(reinterpret_cast<char*>(&integer), sizeof(unsigned int));
	}
	
	
	inline void
	Serializer::deserialize(std::istream& i_stream, long& lon)
	{
		i_stream.read(reinterpret_cast<char*>(&lon), sizeof(long));
	}
	
	
	inline void
	Serializer::deserialize(std::istream& i_stream, long unsigned int& lon)
	{
		i_stream.read(reinterpret_cast<char*>(&lon), sizeof(long unsigned int));
	}
*/
	
	
	inline void
	Serializer::deserialize(std::istream& i_stream, int32_t& value)
	{
		i_stream.read(reinterpret_cast<char*>(&value), sizeof(int32_t));
	}
	
	
	inline void
	Serializer::deserialize(std::istream& i_stream, uint32_t& value)
	{
		i_stream.read(reinterpret_cast<char*>(&value), sizeof(uint32_t));
	}
	
	
	inline void
	Serializer::deserialize(std::istream& i_stream, int64_t& value)
	{
		i_stream.read(reinterpret_cast<char*>(&value), sizeof(int64_t));
	}
	
	
	inline void
	Serializer::deserialize(std::istream& i_stream, uint64_t& value)
	{
		i_stream.read(reinterpret_cast<char*>(&value), sizeof(uint64_t));
	}
	
	
	
	template <typename T_1, typename T_2>
	inline void
	Serializer::deserialize(std::istream& i_stream, std::pair<T_1, T_2>& p)
	{
		Serializer::deserialize(i_stream, p.first);
		Serializer::deserialize(i_stream, p.second);
	}


	inline void
	Serializer::deserialize(std::istream& i_stream, std::string& str)
	{
		uint64_t string_size;
		Serializer::deserialize(i_stream, string_size);
		
		char* buffer = new char[string_size];
		i_stream.read(buffer, string_size);
		str = "";
		str.append(buffer, string_size);
		delete [] buffer;
	}
	
	
	template <typename T_Elem>
	void
	Serializer::deserialize(std::istream& i_stream, std::list<T_Elem>& list)
	{
		list.clear();
		
		uint64_t list_size;
		Serializer::deserialize(i_stream, list_size);

		for (uint64_t i=0; i<list_size; ++i)
		{
			T_Elem elem;
			Serializer::deserialize(i_stream, elem);
			list.push_back(elem);
		}
	}
	


	template <typename T_Elem>
	void
	Serializer::deserialize(std::istream& i_stream, std::vector<T_Elem>& container)
	{
		container.clear();
		
		uint64_t container_size;
		Serializer::deserialize(i_stream, container_size);

		for (uint64_t i=0; i<container_size; ++i)
		{
			T_Elem elem;
			Serializer::deserialize(i_stream, elem);
			container.push_back(elem);
		}
	}
	
	
	template <typename T_Key, typename T_Elem, typename T_Comp>
	void
	Serializer::deserialize(std::istream& i_stream,
		std::multimap<T_Key, T_Elem, T_Comp>& container)
	{
		container.clear();
		
		uint64_t container_size;
		Serializer::deserialize(i_stream, container_size);

		for (uint64_t i=0; i<container_size; ++i)
		{
			T_Key key;
			Serializer::deserialize(i_stream, key);
			T_Elem elem;
			Serializer::deserialize(i_stream, elem);
			
			container.insert(std::make_pair(key, elem));
		}
	}


	template <typename T_Obj>
	void
	Serializer::deserialize(std::string& str, T_Obj& obj)
	{
		std::stringstream string_stream(str, std::stringstream::in | 
			std::stringstream::out | std::stringstream::binary);
		Serializer::deserialize(string_stream, obj);
	}
	
	
	template <typename T_Obj>
	T_Obj
	Serializer::deserialize(std::string& str)
	{
		T_Obj obj;
		Serializer::deserialize(str, obj);
		
		return obj;
	}
	
	
	template <typename T_Obj>
	T_Obj
	Serializer::deserialize(std::istream& i_stream)
	{
		T_Obj obj;
		Serializer::deserialize(i_stream, obj);
		
		return obj;
	}
	

	template <typename T_Obj>
	void
	Serializer::serialize(std::ostream& o_stream, const T_Obj& obj)
	{
		obj.serialize(o_stream);
	}
	
	
	inline void
	Serializer::serialize(std::ostream& o_stream, const bool boolean)
	{
		o_stream.write(reinterpret_cast<const char*>(&boolean), sizeof(bool));
	}
	
	
	inline void
	Serializer::serialize(std::ostream& o_stream, const double doub)
	{
		o_stream.write(reinterpret_cast<const char*>(&doub), sizeof(double));
	}
	
	
	inline void
	Serializer::serialize(std::ostream& o_stream, const float floa)
	{
		o_stream.write(reinterpret_cast<const char*>(&floa), sizeof(float));
	}
	
	
/*	
	inline void
	Serializer::serialize(std::ostream& o_stream, const int integer)
	{
		o_stream.write(reinterpret_cast<const char*>(&integer), sizeof(int));
	}
	
	
	inline void
	Serializer::serialize(std::ostream& o_stream, const unsigned int integer)
	{
		o_stream.write(reinterpret_cast<const char*>(&integer), sizeof(unsigned int));
	}
	
	
	inline void
	Serializer::serialize(std::ostream& o_stream, const long lon)
	{
		o_stream.write(reinterpret_cast<const char*>(&lon), sizeof(long));
	}
	
	
	inline void
	Serializer::serialize(std::ostream& o_stream, const long unsigned int lon)
	{
		o_stream.write(reinterpret_cast<const char*>(&lon), sizeof(long unsigned int));
	}
*/
	
	
	inline void
	Serializer::serialize(std::ostream& o_stream, const int32_t value)
	{
		o_stream.write(reinterpret_cast<const char*>(&value), sizeof(int32_t));
	}
	
	
	inline void
	Serializer::serialize(std::ostream& o_stream, const uint32_t value)
	{
		o_stream.write(reinterpret_cast<const char*>(&value), sizeof(uint32_t));
	}
	
	
	inline void
	Serializer::serialize(std::ostream& o_stream, const int64_t value)
	{
		o_stream.write(reinterpret_cast<const char*>(&value), sizeof(int64_t));
	}
	
	
	inline void
	Serializer::serialize(std::ostream& o_stream, const uint64_t value)
	{
		o_stream.write(reinterpret_cast<const char*>(&value), sizeof(uint64_t));
	}
	
	
	template <typename T_1, typename T_2>
	inline void
	Serializer::serialize(std::ostream& o_stream, const std::pair<T_1, T_2>& p)
	{
		Serializer::serialize(o_stream, p.first);
		Serializer::serialize(o_stream, p.second);
	}


	inline void
	Serializer::serialize(std::ostream& o_stream, const std::string& str)
	{
		/** @todo Casting from size_t to int might be dangerous... */
		uint64_t string_size = str.size();
		Serializer::serialize(o_stream, string_size);
		
		const char* buffer = str.c_str();
		o_stream.write(buffer, string_size);
	}


	template <typename T_Elem>
	void
	Serializer::serialize(std::ostream& o_stream, const std::list<T_Elem>& list)
	{
		uint64_t list_size = list.size();
		Serializer::serialize(o_stream, list_size);
		
		typename std::list<T_Elem>::const_iterator iter = list.begin();
		typename std::list<T_Elem>::const_iterator end = list.end();
		for (; iter != end; ++iter)
		{
				Serializer::serialize(o_stream, *iter);
		}
	}


	template <typename T_Elem>
	void
	Serializer::serialize(std::ostream& o_stream, const std::vector<T_Elem>& vec)
	{
		uint64_t vector_size = vec.size();
		Serializer::serialize(o_stream, vector_size);
		
		typename std::vector<T_Elem>::const_iterator iter = vec.begin();
		typename std::vector<T_Elem>::const_iterator end = vec.end();
		for (; iter != end; ++iter)
		{
				Serializer::serialize(o_stream, *iter);
		}
	}
	
	
	template <typename T_Key, typename T_Elem, typename T_Comp>
	void
	Serializer::serialize(std::ostream& o_stream,
		const std::multimap<T_Key, T_Elem, T_Comp>& container)
	{
		uint64_t container_size = container.size();
		Serializer::serialize(o_stream, container_size);
		
		typename std::multimap<T_Key, T_Elem, T_Comp>::const_iterator iter
			= container.begin();
		typename std::multimap<T_Key, T_Elem, T_Comp>::const_iterator iter_end
			= container.end();
		for (; iter != iter_end; ++ iter)
		{
			Serializer::serialize(o_stream, iter->first);
			Serializer::serialize(o_stream, iter->second);
		}
	}
	
	
	template <typename T_Obj>
	std::string
	Serializer::serialize(const T_Obj& obj)
	{
		std::stringstream str_stream(std::stringstream::in |
			std::stringstream::out | std::stringstream::binary);
		Serializer::serialize(str_stream, obj);

		return str_stream.rdbuf()->str();
	}

} // namespace mapgeneration_util

#endif // SERIALIZER_H
