/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef CACHE_H
#define CACHE_H

#include <deque>
#include <list>
#include <map>
#include <cc++/thread.h>
#include <utility>
#include "util/controlledthread.h"
#include "util/mlog.h"
#include "util/pubsub/asynchronousproxy.h"


namespace mapgeneration_util
{
	

	/**
	 * @brief This template class provides a cache
	 * for elements of one type.
	 * 
	 * Cache contains just the bare cache logic. The backend is 
	 * "specified" by the methods load, save,
	 * erase and size_of are virtual, that have to be implemented by
	 * subclasses.
	 */
	template <typename T_ID, typename T_Elem>
	class Cache : public ControlledThread {
		
		public:
		
		
			/** 
			 * \brief Entries are the objects that are stored in the cache and provide 
			 * extra internal information beside the object itself.
			 */
			struct Entry
			{
				T_Elem* object;				/**< The object itself. */
				ost::AtomicCounter users;	/**< The current number of users of 
												this object. */
				bool dirty;					/**< The dirty flag. */
			};

		
			/**
			 * \brief Pointer provides a simple smart pointer for elements of 
			 * the cache.
			 * 
			 * This class helps the cache to figure out which of it's objects 
			 * are still used and which might have been changed. All functions 
			 * that return elements of the cache use this pointer as return 
			 * type. If a constant pointer is used the element cannot be changed
			 * and the elements does not have to be marked as dirty. If the 
			 * users request _and_ uses a non constant pointer the element is 
			 * marked as dirty. The Pointer also saves the number of users that 
			 * currently use a cached element and so only unused elements are 
			 * removed from the cache.
			 */
			class Pointer
			{
				
				public:

					Pointer();
				
					/**
					 * @brief Initialises the Pointer with the given entry and 
					 * increases the users-counter by one.
					 */
					Pointer(Entry* e);
										
					
					/**
					 * @brief Copy Constructor
					 */
					Pointer(const Pointer& p);

					
					/**
					 * @brief Deinitialises the Pointer and decreases the users-counter 
					 * by one.
					 */
					~Pointer();
					

					/**
					 * @brief Sets the contents of the Pointer to 
					 * the contents of the Pointer p.
					 */					
					inline Pointer&
					operator=(const Pointer& p);


					/**
					 * @brief Compares two Pointer. Two pointers are equal if they 
					 * point to the same entry.
					 */
					inline bool	
					operator==(const Pointer& p) const;
					
					
					/**
					 * @brief Compares two Pointer. Two pointers are equal if they 
					 * point to the same entry.
					 */
					inline bool	
					operator!=(const Pointer& p) const;


					/**
					 * @brief The standard dereferencing operator for pointers for the 
					 * use in const Pointers.This function does not set dirty to
					 * true, because the returned element is constant and cannot
					 * be changed.
					 */
					inline const T_Elem&
					operator*() const;


					/**
					 * @brief See operator*
					 */
					inline const T_Elem*
					operator->() const;


					inline T_Elem&
					write();


				private:
				
					/**
					 * @brief The entry the pointer points at.
					 * This is not returned, but the contained object!
					 */
					Entry* entry;
				
			};


			/**
			 * \brief The long list of support cache strategies. ;-)
			 */
			enum Strategy
			{
				_FIFO = 1
			};
			
			
			/**
			 * @brief The constants used for options.
			 */
			const static unsigned int _STANDARD_CACHE = 	0x00000000;
			const static unsigned int _NON_PERSISTENT = 	0x00000001;
			const static unsigned int _NO_WRITEBACK = 		0x00000002;
			const static unsigned int _NO_MEMORY_LIMIT = 	0x00000004;
			

			/**
			 * The constructor, the default strategy is random.
			 */
			Cache(Strategy strategy=_FIFO, 
				unsigned int options = _STANDARD_CACHE, 
				int hard_max_cached_size=0, int soft_max_cached_size=0);


			/**
			 * \brief The destructor.
			 * 
			 * The destructor does not flush the cache, because it cannot
			 * guarantee that the underlying save method still works.
			 */
			~Cache();
			
			
			/**
			 * \brief Returns the number of cached elements.
			 * @return Number of cached elements.
			 */
			inline int
			cached_objects();
			
			
			/**
			 * \brief Returns the sum of sizes of all cached elements.
			 * @return Sum of size() for all elements.
			 */
			inline int
			cached_size();
			

			/**
			 * \brief Saves all unwritten data.
			 */
			int
			flush();


			/**
			 * \brief Returns a Pointer to the elements or 0 if the
			 * element cannot be loaded.
			 * 
			 * @param id The id of the requested element.
			 */

			Pointer
			get(T_ID id);
			

			/**
			 * \brief Returns a complete entry and will be removed soon!
			 */
			Entry*
			get_entry(T_ID id);
			

			/**
			 * @brief Fast version of get_const. If the element
			 * is not cached, returns 0 and starts a prefetch for the element.
			 */
			Pointer
			get_or_prefetch(T_ID id, 
				pubsub::AsynchronousProxy<T_ID>* notifier = 0);
			
			
			/**
			 * @brief Returns a sorted vector of ids that contain an element.
			 * 
			 * @return Sorted list of used ids.
			 */
			std::vector<T_ID>
			Cache<T_ID, T_Elem>::get_used_ids();


			/**
			 * @brief Returns the hard limit for the size of all cached elements.
			 * @return Hard limit for the size of all cached elements.
			 */
			inline int
			hard_max_cached_size() const;			


			/**
			 * \brief Inserts a new element into the cache.
			 * 
			 * @return True if object could be inserted, false otherwise.
			 * @param id The id for the element.
			 * @param elem The new element.
			 */
			bool
			insert(T_ID id, T_Elem* elem);
			
			
			bool
			insert(T_ID* id, T_Elem* elem);			
			
			
			/**
			 * \brief Returns the dirty flag of the object.
			 * @return True if the object is dirty, false otherwise.
			 * @param id
			 */
			bool
			is_dirty(T_ID id);
			
			
			/**
			 * \brief Orders the cache to load the element.
			 * 
			 * @param id The id of the element to prefetch.
			 */
			void
			prefetch(T_ID id, pubsub::AsynchronousProxy<T_ID>* notifier = 0);
			
			
			/**
			 * \brief Immediatly removes the element from the cache and the 
			 * underlying storage!
			 * 
			 * @param id
			 */
			bool
			remove(T_ID id);


			/**
			 * @brief Returns the soft limit for the size of all cached elements.
			 * @return Soft limit for the size of all cached elements.
			 */
			inline int
			soft_max_cached_size() const;


			/**
			 * Writes all dirty elements back to the underlying storage and 
			 * returns the number of written elements.
			 * @return Number of written elements.
			 */
			int
			write_back();
		
		
		protected:
		
			/**
			 * @brief This virtual function has to be overloaded
			 * and should erase the element with the given id.
			 * 
			 * @param id Id of the element to erase.
			 * @return True if successfull, false otherwise.
			 */
			virtual bool
			persistent_erase(T_ID id);
			
			
			virtual std::vector<T_ID>
			persistent_get_free_ids();
			
			
			/**
			 * @brief This virtual function may be overloaded to return
			 * a vector of used ids. The default implementation returns
			 * an empty vector!
			 * 
			 * @return Vector with used ids.
			 */
			virtual std::vector<T_ID>
			persistent_get_used_ids();
			

			/**
			 * \brief This virtual function has to be overloaded
			 * and should load the requested element.
			 * 
			 * @return Pointer to the loaded element or 0 if element could not
			 * be loaded.
			 */
			virtual T_Elem*
			persistent_load(T_ID id);
			

			/**
			 * \brief This virtual function has to be overloaded
			 * and should save the element elem.
			 */
			virtual void
			persistent_save(T_ID id, T_Elem* elem);


			/**
			 * \brief This virtual function has to be overloaded.
			 * 
			 * @return Size of the element in any unit. The functions has to be
			 * able to handle 0 pointers!
			 */
			virtual int
			persistent_size_of(T_Elem* elem);


			/**
			 * \brief The overloaded function for thread deinitialisation.
			 */
			void
			thread_deinit();


			/**
			 * \brief The overloaded function for thread initialisation.
			 */
			void
			thread_init();
			
	
			/**
			 * \brief The Cache's thread method.
			 */
			void
			thread_run();
		
		
		private:
						
			/**
			 * @brief The current size of all cached objects.
			 */
			int _cached_size;
			
			/**
			 * @brief The current number of cached objects.
			 */
			int _cached_objects;
						
			
			/**
			 * @brief Should be called when the cache is over it's soft limit.
			 */
			ost::Event _thread_should_work_event;
						
			/**
			 * @brief The hard limit of the cached_size.
			 */
			int _hard_max_cached_size;
			
			/**
			 * \brief The mutex that protects everything at the moment.
			 */
			ost::Mutex _mutex;
			
			/**
			 * @brief The data of the cache.
			 */
			std::map<T_ID, Entry> _objects;
			
			
			/**
			 * @brief Ids that may be in the cache.
			 */
			std::deque<T_ID> _object_ids;
			
			
			/**
			 * @brief The integer for the option flags.
			 */
			unsigned int _options;
			
			
			/**
			 * @brief Contains all object ids that should be prefetched.
			 */
			std::deque< std::pair <T_ID, pubsub::AsynchronousProxy<T_ID>* > >
				_prefetches;


			/**
			 * @brief The soft limit of the cached_size.
			 */
			int _soft_max_cached_size;
			
			
			/**
			 * @brief Vector of unused ids in the cache.
			 * 
			 * The last value is the maximum id plus 1.
			 */
			std::deque<T_ID> _unused_ids;
			
			
			void
			free_cache_down_to(int max_size);



			/**
			 * \brief Flushes (writes back and removes from cache) a single 
			 * cache element
			 * 
			 * @return
			 */
			inline bool
			flush(typename std::map<T_ID, Entry>::iterator);
			

			/**
			 * \brief Loades an element into the cache.
			 */
			Entry*
			load_into_cache(T_ID id);
			

			/**
			 * \brief Creates a new entry.
			 */
			std::pair<T_ID, Entry>
			new_entry_pair(T_ID id, T_Elem* elem, bool dirty);
			
			
			/**
			 * \brief Searches the request object in the cache.
			 */
			Entry*
			search_in_cache(T_ID id);
			
			
			/**
			 * @brief Wrapper for persistent_erase.
			 * 
			 * @return True if successfull, false otherwise.
			 */
			inline bool
			wrapper_erase(T_ID id);
			
			
			inline std::vector<T_ID>
			wrapper_get_free_ids();
			
			
			/**
			 * @brief Wrapper for persistent_get_used_ids.
			 * 
			 * @return Vector with used ids.
			 */
			inline std::vector<T_ID>
			wrapper_get_used_ids();
			

			/**
			 * @brief Wrapper for persistent_load.
			 * 
			 * @return Pointer to the loaded element or 0 if element could not
			 * be loaded.
			 */
			inline T_Elem*
			wrapper_load(T_ID id);
			

			/**
			 * @brief Wrapper for persistent_save.
			 */
			inline void
			wrapper_save(T_ID id, T_Elem* elem);


			/**
			 * @brief Wrapper for persistent_size_of.
			 * 
			 * @return Size of the element in any unit. The functions has to be
			 * able to handle 0 pointers!
			 */
			inline int
			wrapper_size_of(T_Elem* elem);
			
			
			/**
			 * \brief Writes back an element to the storage. The element
			 * is not removed from the cache.
			 */
			inline bool
			write_back(typename std::map<T_ID, Entry>::iterator);

	};

	
	/*
	 * Implementation of the Pointer functions
	 */
	 
	 
 	template <typename T_ID, typename T_Elem>
	Cache<T_ID, T_Elem>::Pointer::Pointer()
	: entry(0)
	{
	}


	template <typename T_ID, typename T_Elem>
	Cache<T_ID, T_Elem>::Pointer::Pointer(Entry* e)
	{
		if (e!=0 && e->object!=0)		
			entry = e;
		else
			entry = 0;

		if (entry != 0)
			++(entry->users);
	}
	
	
	template <typename T_ID, typename T_Elem>
	Cache<T_ID, T_Elem>::Pointer::Pointer(const Pointer& pointer)
	{
		entry = pointer.entry;
		if (entry != 0)
			++(entry->users);
	}
	
	 
	template <typename T_ID, typename T_Elem>
	Cache<T_ID, T_Elem>::Pointer::~Pointer()
	{
		if (entry != 0)
			--(entry->users);
	}
	
	
	template <typename T_ID, typename T_Elem>
	inline typename Cache<T_ID, T_Elem>::Pointer&
	Cache<T_ID, T_Elem>::Pointer::
		operator=(const typename Cache<T_ID, T_Elem>::Pointer& p1)
	{
		if ((&p1)!=this) {
			if (entry != 0)
				--(entry->users);
			entry = p1.entry;
			if (entry != 0)
				++(entry->users);
		}
		
		return *this;
	}
	
	
	template <typename T_ID, typename T_Elem>
	inline bool
	Cache<T_ID, T_Elem>::Pointer::operator==(const typename Cache<T_ID, T_Elem>::Pointer& p1) const
	{
		if (entry == p1.entry)
			return true;
		if (entry == 0 || p1.entry == 0)
			 return false;
			 
		return ((*entry).object == (*(p1.entry)).object);
	}
	
	
	template <typename T_ID, typename T_Elem>
	inline bool
	Cache<T_ID, T_Elem>::Pointer::operator!=(const typename Cache<T_ID, T_Elem>::Pointer& p1) const
	{
		return (operator==(p1) ? false : true);
	}
	

	template <typename T_ID, typename T_Elem>
	inline const T_Elem&
	Cache<T_ID, T_Elem>::Pointer::operator*() const
	{
		return *(entry->object);
	}
	

	template <typename T_ID, typename T_Elem>
	inline const T_Elem*
	Cache<T_ID, T_Elem>::Pointer::operator->() const
	{
		return entry->object;
	}	
	
	
	template <typename T_ID, typename T_Elem>
	inline T_Elem&
	Cache<T_ID, T_Elem>::Pointer::write()
	{
		entry->dirty = true;
		return *(entry->object);		
	}
	


	/*
	 * Implementation of the public functions of Cache
	 */	
	 
 	template <typename T_ID, typename T_Elem>
	Cache<T_ID, T_Elem>::Cache(Strategy strategy, unsigned int options,
		int hard_max_cached_size, int soft_max_cached_size)
	: _cached_size(0), _cached_objects(0), _thread_should_work_event(),
		_hard_max_cached_size(hard_max_cached_size), _mutex(), _objects(),
		_object_ids(), _options(options), _prefetches(),
		_soft_max_cached_size(soft_max_cached_size),
		_unused_ids()
	{
		mlog(MLog::info, "Cache::Cache") << "Options:" << "\n";
		if (_options & _NON_PERSISTENT)
			mlog(MLog::info, "Cache::Cache") << "_NON_PERSISTENT\n";
		if (_options & _NO_WRITEBACK)
			mlog(MLog::info, "Cache::Cache") << "_NO_WRITEBACK\n";
		if (_options & _NO_MEMORY_LIMIT)
			mlog(MLog::info, "Cache::Cache") << "_NO_MEMORY_LIMIT\n";
		
		if (!(_options & _NO_MEMORY_LIMIT) &&
			(_soft_max_cached_size > _hard_max_cached_size || 
				_soft_max_cached_size == 0))
		{
			_soft_max_cached_size = (int)((double)_hard_max_cached_size * 0.8);
			mlog(MLog::info, "Cache::Cache") << "Corrected _soft_max_cached_size to "
				<< _soft_max_cached_size << " !\n";
		}
		
		if (_options & _NO_MEMORY_LIMIT)
		{
			_soft_max_cached_size = 0;
			_hard_max_cached_size = 0;
			mlog(MLog::info, "Cache::Cache") << "soft_max_cached_size: " <<
				"unlimited" << "\n";
			mlog(MLog::info, "Cache::Cache") << "hard_max_cached_size: " <<
				"unlimited" << "\n";
		} else
		{
			mlog(MLog::info, "Cache::Cache") << "soft_max_cached_size: " <<
				_soft_max_cached_size << "\n";
			mlog(MLog::info, "Cache::Cache") << "hard_max_cached_size: " <<
				_hard_max_cached_size << "\n";
		}
		
	}


	template <typename T_ID, typename T_Elem>
	Cache<T_ID, T_Elem>::~Cache()
	{
		/** @todo Implement correct check for unflushed caches! */
	}
	
	
	template <typename T_ID, typename T_Elem>
	inline int
	Cache<T_ID, T_Elem>::cached_objects()
	{
		_mutex.enter();
		int cached_objects = _objects.size();
		_mutex.leave();
		return cached_objects;
	}


	template <typename T_ID, typename T_Elem>
	inline int
	Cache<T_ID, T_Elem>::cached_size()
	{
		_mutex.enter();
		int cached_size = _cached_size;
		_mutex.leave();
		return cached_size;
	}


	template <typename T_ID, typename T_Elem>
	int
	Cache<T_ID, T_Elem>::flush()
	{
		int result = 0;

		_mutex.enter();
		typename std::map<T_ID, Entry>::iterator iter = _objects.begin();
		typename std::map<T_ID, Entry>::iterator iter_end = _objects.end();
		for (; iter != iter_end; ++iter)
			if (flush(iter)) result++;			
		_mutex.leave();

		return result;
	}
	
	
 	template <typename T_ID, typename T_Elem>
	typename Cache<T_ID, T_Elem>::Pointer
	Cache<T_ID, T_Elem>::get(T_ID id)
	{		
		_mutex.enter();

		Entry* entry = search_in_cache(id);
		if (!entry)
			entry = load_into_cache(id);

		if (entry)
		{
			typename Cache<T_ID, T_Elem>::Pointer pointer(entry);
			_mutex.leave();
			return pointer;
		}

		_mutex.leave();
		return typename Cache<T_ID, T_Elem>::Pointer(0);
	}
	
	
	template <typename T_ID, typename T_Elem>
	typename Cache<T_ID, T_Elem>::Entry*
	Cache<T_ID, T_Elem>::get_entry(T_ID id)
	{
		_mutex.enter();
		Entry* entry = search_in_cache(id);
		_mutex.leave();
		
		return entry;
	}
	
	
	template <typename T_ID, typename T_Elem>
	typename Cache<T_ID, T_Elem>::Pointer
	Cache<T_ID, T_Elem>::get_or_prefetch(T_ID id, 
		pubsub::AsynchronousProxy<T_ID>* notifier)
	{
		_mutex.enter();

		Entry* entry = search_in_cache(id);
		if (!entry)
		{
			prefetch(id, notifier);
		} else
		{
			typename Cache<T_ID, T_Elem>::Pointer pointer(entry);
			_mutex.leave();
			return pointer;
		}

		_mutex.leave();
		return typename Cache<T_ID, T_Elem>::Pointer(0);
	}
	
	
	template <typename T_ID, typename T_Elem>
	std::vector<T_ID>
	Cache<T_ID, T_Elem>::get_used_ids()
	{
		std::vector<T_ID> result;
		
		_mutex.enter();
		result = wrapper_get_used_ids();
		std::sort(result.begin(), result.end());
		
		typename std::map<T_ID, Entry>::const_iterator iter = _objects.begin();
		typename std::map<T_ID, Entry>::const_iterator iter_end = _objects.end();
		for (; iter != iter_end; ++iter)
		{
			if (iter->second.object!=0 && iter->second.dirty &&
				!std::binary_search(result.begin(), result.end(), iter->first))
			{
				result.push_back(iter->first);
			}
		}
		_mutex.leave();
	
		std::sort(result.begin(), result.end());
		
		return result;
	}
	
	
	template <typename T_ID, typename T_Elem>
	inline int
	Cache<T_ID, T_Elem>::hard_max_cached_size() const
	{
		return _hard_max_cached_size;
	}


	template <typename T_ID, typename T_Elem>
	bool
	Cache<T_ID, T_Elem>::insert(T_ID id, T_Elem* elem)
	{
		_mutex.enter();
		Entry* search_result = search_in_cache(id);
		if ((search_result && search_result->object!=0) || 
			(!search_result && load_into_cache(id)->object!=0))
		{
			_mutex.leave();
			return false;
		}

		search_result = search_in_cache(id);
		if (search_result && search_result->object==0)
		{
			typename std::map<T_ID, typename Cache<T_ID, T_Elem>::Entry >::iterator
				search_result_2 = _objects.find(id);
			if (search_result_2 != _objects.end() && search_result_2->second.users == 0)		
				_objects.erase(search_result_2);
			else
				mlog(MLog::error, "Cache::insert") << "Could not erase "
					<< "element that should be empty!!!!\n";
		}

		std::pair<typename std::map<T_ID, typename Cache<T_ID, T_Elem>::Entry >::iterator, bool> result = 
			_objects.insert(new_entry_pair(id, elem, true));
		
		_mutex.leave();
		
		if (!result.second)
			mlog(MLog::error) << "Could not insert element that should be "
				<< "insertable!\n";

		return result.second;
	}
	

	template <typename T_ID, typename T_Elem>	
	bool
	Cache<T_ID, T_Elem>::insert(T_ID* id, T_Elem* elem)
	{
		_mutex.enter();		
		if (_unused_ids.size() == 0)
		{
			std::vector<T_ID> unused_ids_vector = wrapper_get_free_ids();
			
			if (unused_ids_vector.size() == 0)
			{
				_mutex.leave();
				return false;
			}
				
			_unused_ids.insert(_unused_ids.end(),
				unused_ids_vector.begin(), unused_ids_vector.end());
		}
	
		*id = _unused_ids.front();
		_unused_ids.pop_front();
		
		if (_unused_ids.size() == 0)
			_unused_ids.push_back(*id + 1);
			
		std::cout << "Inserting as id: " << *id << "\n";
		
		bool insert_result = insert(*id, elem);		
		_mutex.leave();
		
		return insert_result;
	}
	
	
	template <typename T_ID, typename T_Elem>
	bool
	Cache<T_ID, T_Elem>::is_dirty(T_ID id)
	{
		_mutex.enter();
		Entry* entry = search_in_cache(id);
		
		bool dirty;
		if (!entry) dirty = false;
			else dirty = entry.dirty;
		_mutex.leave();
		
		return dirty;
	}


	template <typename T_ID, typename T_Elem>
	void
	Cache<T_ID, T_Elem>::prefetch(T_ID id, 
		pubsub::AsynchronousProxy<T_ID>* notifier)
	{
		_mutex.enter();
		_prefetches.push_back(
			std::pair<T_ID, pubsub::AsynchronousProxy<T_ID>* >(
				id,
				notifier
			)
		);
		_mutex.leave();
		
		_thread_should_work_event.signal();
	}


	template <typename T_ID, typename T_Elem>
	bool
	Cache<T_ID, T_Elem>::remove(T_ID id)
	{
		_mutex.enter();
		/** @todo Change search_in_cache so that this function can also use it...*/
		typename std::map<T_ID, typename Cache<T_ID, T_Elem>::Entry >::iterator search_result = 
			_objects.find(id);
		
		if (search_result != _objects.end() && search_result->second.users == 0)
			_objects.erase(search_result);

		wrapper_erase(id);
		
		typename std::deque<T_ID>::iterator unused_ids_iter =
			std::find(_unused_ids.begin(), _unused_ids.end(), id);
		if (unused_ids_iter == _unused_ids.end())
			_unused_ids.push_front(id);
		_mutex.leave();
		
		return true;
	}


	template <typename T_ID, typename T_Elem>
	inline int
	Cache<T_ID, T_Elem>::soft_max_cached_size() const
	{
		return _soft_max_cached_size;
	}


	template <typename T_ID, typename T_Elem>
	int
	Cache<T_ID, T_Elem>::write_back()
	{
		int counter = 0;
		
		_mutex.enter();
		typename std::map<T_ID, Entry>::iterator iter = _objects.begin();
		typename std::map<T_ID, Entry>::iterator iter_end = _objects.end();
		for (; iter != iter_end; ++iter)
			if (write_back(iter)) counter++;
		_mutex.leave();
			
		return counter;
	}
	
	
	/*
	 * Implementation of the protected functions of Cache.
	 */	 

	template <typename T_ID, typename T_Elem>
 	bool
	Cache<T_ID, T_Elem>::persistent_erase(T_ID id)
	{
		return false;
	}
	

	template <typename T_ID, typename T_Elem>
	std::vector<T_ID>
	Cache<T_ID, T_Elem>::persistent_get_free_ids()
	{
		std::vector<T_ID> result;
		return result;
	}
	 
	 
	template <typename T_ID, typename T_Elem>
	std::vector<T_ID>
	Cache<T_ID, T_Elem>::persistent_get_used_ids()
	{		
		std::vector<T_ID> result;
		return result;
	}
	

	template <typename T_ID, typename T_Elem>
	T_Elem*
	Cache<T_ID, T_Elem>::persistent_load(T_ID id)
	{
		return 0;
	}
	
	
	template <typename T_ID, typename T_Elem>
	void
	Cache<T_ID, T_Elem>::persistent_save(T_ID id, T_Elem* elem)
	{
	}


	template <typename T_ID, typename T_Elem>
	int
	Cache<T_ID, T_Elem>::persistent_size_of(T_Elem* elem)
	{
		return sizeof(T_Elem);
	}
	

	template <typename T_ID, typename T_Elem>
	void
	Cache<T_ID, T_Elem>::thread_deinit()
	{
		mlog(MLog::info, "Cache") << "Shutting down...\n";
		flush();
		mlog(MLog::info, "Cache") << "Stopped.\n";
	}
		
		
	template <typename T_ID, typename T_Elem>
	void
	Cache<T_ID, T_Elem>::thread_init()
	{
		mlog(MLog::info, "Cache") << "Initializing...\n";
	}

	
	template <typename T_ID, typename T_Elem>
	void
	Cache<T_ID, T_Elem>::thread_run()
	{
		mlog(MLog::info, "Cache") << "Running.\n";
		
		while (!should_stop())
		{
			if (cached_size() > soft_max_cached_size() &&
				!(_options & _NO_MEMORY_LIMIT))
			{				
				_mutex.enter();
				free_cache_down_to(soft_max_cached_size());
				_mutex.leave();
			}
			
			while(_prefetches.size() > 0)
			{
				_mutex.enter();
				T_ID id = _prefetches.front().first;
				if (!search_in_cache(id))
					load_into_cache(id);				
				
				pubsub::AsynchronousProxy<T_ID>* notifier = 
					_prefetches.front().second;
				_mutex.leave();
				
				if (notifier)
					notifier->receive(id);

				_mutex.enter();					
				_prefetches.pop_front();
				if (cached_size() > soft_max_cached_size())
					free_cache_down_to(soft_max_cached_size());
				_mutex.leave();
			}

			_thread_should_work_event.reset();
			_thread_should_work_event.wait(100);
		}
	}
	


	/*
	 * Implementation of the private functions of Cache.
	 */
	 
	template <typename T_ID, typename T_Elem>
	void
	Cache<T_ID, T_Elem>::free_cache_down_to(int max_size)
	{
		_mutex.enter();
	 	typename std::map<T_ID, Entry>::iterator iter;
	 	typename std::deque<T_ID>::iterator ids_iter = _object_ids.begin();
	 	typename std::deque<T_ID>::iterator ids_iter_end = _object_ids.end();
		while (cached_size() > max_size && _object_ids.size() && 
			ids_iter != ids_iter_end)
		{
			T_ID delete_element_id = *ids_iter;			
			
			bool flush_result = true;
			iter = _objects.find(delete_element_id);
			if (iter != _objects.end())
				flush_result = flush(iter);
			
			if (flush_result)
				_object_ids.erase(ids_iter);
			
			++ids_iter;
		}
		_mutex.leave();
	}


	template <typename T_ID, typename T_Elem>
	inline bool
	Cache<T_ID, T_Elem>::flush(typename std::map<T_ID, Entry>::iterator entry)
	{
		_mutex.enter();
		T_ID id = entry->first;
		T_Elem* elem = entry->second.object;
		
		if (entry->second.dirty)
		{
			wrapper_save(id, elem);
		}
			
		if (entry->second.users == 0)
		{
			int size_of_object = wrapper_size_of(elem);
			delete elem;
			_objects.erase(entry);
			_cached_size -= size_of_object;
			_mutex.leave();
			return true;
		}
		
		_mutex.leave();
		return false;
	}
	

	template <typename T_ID, typename T_Elem>
	typename Cache<T_ID, T_Elem>::Entry*
	Cache<T_ID, T_Elem>::load_into_cache(T_ID id)
	{
		_mutex.enter();
		if (cached_size() >= hard_max_cached_size() && 
			!(_options & _NO_MEMORY_LIMIT))
			free_cache_down_to(hard_max_cached_size());
	
		if (cached_size() >= soft_max_cached_size() &&
			!(_options & _NO_MEMORY_LIMIT))
		{
			_thread_should_work_event.signal();
		}

		T_Elem* elem = wrapper_load(id);
		
		std::pair<typename std::map<T_ID, typename Cache<T_ID, T_Elem>::Entry>::iterator, bool>
			result = _objects.insert(new_entry_pair(id, elem, false));
		
		if (!result.second)
		{
			mlog(MLog::error, "Cache::load_into_cache") 
				<< "Wanted to insert a tile into cache, but map says it's already inserted... :-(\n";
			_mutex.leave();
			return 0;
		} else
		{
			_cached_size += wrapper_size_of(elem);
			_object_ids.push_back(id);
			_mutex.leave();
			return &((result.first)->second);
		}

		_mutex.leave();			// This should be unreachable....
		
	}
	
	
	template <typename T_ID, typename T_Elem>
	std::pair<T_ID, typename Cache<T_ID, T_Elem>::Entry>
	Cache<T_ID, T_Elem>::new_entry_pair(T_ID id, T_Elem* elem, bool dirty)
	{
		std::pair<T_ID, typename Cache<T_ID, T_Elem>::Entry> new_pair;
		new_pair.first = id;
		new_pair.second.object = elem;
		new_pair.second.users = 0;
		new_pair.second.dirty = dirty;

		return new_pair;
	}
	
	
	template <typename T_ID, typename T_Elem>
	typename Cache<T_ID, T_Elem>::Entry*
	Cache<T_ID, T_Elem>::search_in_cache(T_ID id)
	{
		_mutex.enter();
		typename std::map<T_ID, typename Cache<T_ID, T_Elem>::Entry >::iterator search_result = 
			_objects.find(id);
			
		if (search_result != _objects.end())
		{
			_mutex.leave();
			return &(*search_result).second;
		}

		_mutex.leave();
		return 0;
	}
	
	
	template <typename T_ID, typename T_Elem>
	inline bool
	Cache<T_ID, T_Elem>::wrapper_erase(T_ID id)
	{
		if (!(_options & _NON_PERSISTENT) && !(_options & _NO_WRITEBACK))
		{
			return persistent_erase(id);
		} else
			return false;
	}
	
	
	template <typename T_ID, typename T_Elem>
	inline std::vector<T_ID>
	Cache<T_ID, T_Elem>::wrapper_get_free_ids()
	{
		if (!(_options & _NON_PERSISTENT))
		{
			return persistent_get_free_ids();
		} else
		{
			std::vector<T_ID> result;
			return result;
		}
	}
	
	
	template <typename T_ID, typename T_Elem>
	inline std::vector<T_ID>
	Cache<T_ID, T_Elem>::wrapper_get_used_ids()
	{
		if (!(_options & _NON_PERSISTENT))
		{
			return persistent_get_used_ids();
		} else
		{
			std::vector<T_ID> result;
			return result;
		}
	}
	

	template <typename T_ID, typename T_Elem>
	inline T_Elem*
	Cache<T_ID, T_Elem>::wrapper_load(T_ID id)
	{
		if (!(_options & _NON_PERSISTENT))
		{
			return persistent_load(id);
		} else
			return 0;
	}
	

	template <typename T_ID, typename T_Elem>
	inline void
	Cache<T_ID, T_Elem>::wrapper_save(T_ID id, T_Elem* elem)
	{
		if (!(_options & _NON_PERSISTENT) && !(_options & _NO_WRITEBACK))
		{
			persistent_save(id, elem);
		}
	}


	template <typename T_ID, typename T_Elem>
	inline int
	Cache<T_ID, T_Elem>::wrapper_size_of(T_Elem* elem)
	{
		return persistent_size_of(elem);
	}
	
	
	template <typename T_ID, typename T_Elem>
	inline bool
	Cache<T_ID, T_Elem>::write_back(typename std::map<T_ID, Entry>::iterator iter)
	{
		_mutex.enter();
		if (iter->second.dirty && iter->second.users == 0)
		{
			T_ID id = iter->first;
			T_Elem* elem = iter->second.object;
			wrapper_save(id, elem);
			iter->second.dirty = false;

			_mutex.leave();
			return true;
		}

		_mutex.leave();
		return false;
	}
	
	

} // namespace mapgeneration_util

#endif // CACHE_H
