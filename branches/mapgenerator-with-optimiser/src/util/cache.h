/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef CACHE_H
#define CACHE_H

#include <deque>
#include <map>
#include <vector>

#include "util/controlledthread.h"
#include "util/mlog.h"
#include "util/pubsub/subscriber.h"

using ost::MutexLock;

namespace mapgeneration_util
{
	

	/**
	 * @brief This template class provides a cache
	 * for elements of one type.
	 * 
	 * Cache contains just the bare cache logic. The backend is 
	 * "specified" by the methods load, save,
	 * erase are virtual, that have to be implemented by
	 * subclasses.
	 */
	template <typename T_ID, typename T_Elem>
	class Cache : public ControlledThread
	{
		
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
				
				int _size;
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
			Cache(Strategy strategy, 
				unsigned int options, 
				int minimal_object_capacity,
				int hard_max_cached_size, int soft_max_cached_size);


			/**
			 * \brief The destructor.
			 * 
			 * The destructor does not flush the cache, because it cannot
			 * guarantee that the underlying save method still works.
			 */
			 virtual
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
			 * @brief Removes all entries from the prefetch queue.
			 */
			void
			clear_prefetch_queue();
			

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
			 * @brief Fast version of get. If the element is not cached,
			 * returns 0.
			 */
			Pointer
			get_if_in_cache(T_ID id);
			

			/**
			 * @brief Fast version of get. If the element
			 * is not cached, returns 0 and starts a prefetch for the element.
			 */
			Pointer
			get_or_prefetch(T_ID id, 
				pubsub::Subscriber<T_ID>* notifier = 0);
			
			
			/**
			 * @brief Returns a sorted vector of ids that contain an element.
			 * 
			 * @return Sorted list of used ids.
			 */
			std::vector<T_ID>
			get_used_ids();


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
			 * @brief Returns true if the prefetch queue is not empty
			 * or the thread is prefetching the last object.
			 * @return True if prefetching is active.
			 */
			bool
			is_prefetching();
			
			
			/**
			 * \brief Orders the cache to load the element.
			 * 
			 * @param id The id of the element to prefetch.
			 */
			void
			prefetch(T_ID id, pubsub::Subscriber<T_ID>* notifier = 0);
			
			
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
			persistent_load(T_ID id, int& size);
			

			/**
			 * \brief This virtual function has to be overloaded
			 * and should save the element elem.
			 */
			virtual void
			persistent_save(T_ID id, T_Elem* elem, int& size);
			

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
			 * We start with the MUTEX variable.
			 */

			/**
			 * @brief The mutex that protects everything at the moment.
			 */
			ost::Mutex _mutex;
			
			/**
			 * @brief Protect the prefetch queue.
			 */
			ost::Mutex _prefetch_queue_mutex;
			
			
			/**
			 * @brief The average object size, calculated from loaded objects
			 * sizes.
			 */
			double _average_object_size;
			
			
			/**
			 * @brief The counter to calculate the average object size;
			 */
			double _average_object_size_counter;
						
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
			 * @brief The minimal number of objects that fit into the cache.
			 */
			int _minimal_object_capacity;
			
				
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
			std::deque< std::pair <T_ID, pubsub::Subscriber<T_ID>* > >
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
			new_entry_pair(T_ID id, T_Elem* elem, bool dirty, int size);
			
			
			/**
			 * \brief Searches the request object in the cache.
			 */
			Entry*
			search_in_cache(T_ID id);
			
			
			/**
			 * @brief Updates the average object size variable with a new
			 * size information.
			 */
			void
			update_average_object_size(int size);
			
			
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
		if ((&p1)!=this)
		{
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
		int minimal_object_capacity, 
		int hard_max_cached_size, int soft_max_cached_size)
	: _cached_size(0), _cached_objects(0), 
		_thread_should_work_event(), _hard_max_cached_size(hard_max_cached_size), 
		_mutex(), _minimal_object_capacity(minimal_object_capacity),
		_objects(), _object_ids(), _options(options), _prefetches(),
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
			mlog(MLog::info, "Cache::Cache") << "minimal_object_capacity: " <<
				"unlimited" << "\n";
		} else
		{
			mlog(MLog::info, "Cache::Cache") << "soft_max_cached_size: " <<
				_soft_max_cached_size << "\n";
			mlog(MLog::info, "Cache::Cache") << "hard_max_cached_size: " <<
				_hard_max_cached_size << "\n";
			mlog(MLog::info, "Cache::Cache") << "minimal_object_capacity: " <<
				_minimal_object_capacity << "\n";
		}		

		if (_minimal_object_capacity == 0)
			_average_object_size = _soft_max_cached_size;
		else
			_average_object_size = 	_soft_max_cached_size / 
				_minimal_object_capacity;
		_average_object_size_counter = 0;		
		mlog(MLog::info, "Cache::Cache") << "Start for average object size: " <<
			_average_object_size << "\n";
		
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
		MutexLock lock(_mutex);
//		_mutex.enterMutex();
		int cached_objects = _objects.size();
//		_mutex.leaveMutex();
		return cached_objects;
	}


	template <typename T_ID, typename T_Elem>
	inline int
	Cache<T_ID, T_Elem>::cached_size()
	{
		MutexLock lock(_mutex);
//		_mutex.enterMutex();
		int cached_size = _cached_size;
//		_mutex.leaveMutex();
		return cached_size;
	}
	

	template <typename T_ID, typename T_Elem>	
	void
	Cache<T_ID, T_Elem>::clear_prefetch_queue()
	{
		MutexLock lock(_prefetch_queue_mutex);
//		_prefetch_queue_mutex.enterMutex();
		_prefetches.clear();
//		_prefetch_queue_mutex.leaveMutex();
	}


	template <typename T_ID, typename T_Elem>
	int
	Cache<T_ID, T_Elem>::flush()
	{
		MutexLock lock(_mutex);
		
		int result = 0;

//		_mutex.enterMutex();
		typename std::map<T_ID, Entry>::iterator iter = _objects.begin();
		typename std::map<T_ID, Entry>::iterator iter_end = _objects.end();
		for (; iter != iter_end; ++iter)
			if (flush(iter)) result++;			
//		_mutex.leaveMutex();

		return result;
	}
	
	
 	template <typename T_ID, typename T_Elem>
	typename Cache<T_ID, T_Elem>::Pointer
	Cache<T_ID, T_Elem>::get(T_ID id)
	{		
		MutexLock lock(_mutex);

//		_mutex.enterMutex();

		Entry* entry = search_in_cache(id);
		if (!entry)
			entry = load_into_cache(id);

		if (entry)
		{
			typename Cache<T_ID, T_Elem>::Pointer pointer(entry);
			
//			_mutex.leaveMutex();
			return pointer;
		}

//		_mutex.leaveMutex();
		return typename Cache<T_ID, T_Elem>::Pointer(0);
	}
	
	
	template <typename T_ID, typename T_Elem>
	typename Cache<T_ID, T_Elem>::Entry*
	Cache<T_ID, T_Elem>::get_entry(T_ID id)
	{
		MutexLock lock(_mutex);

//		_mutex.enterMutex();
		Entry* entry = search_in_cache(id);
//		_mutex.leaveMutex();
		
		return entry;
	}
	
	
	template <typename T_ID, typename T_Elem>
	typename Cache<T_ID, T_Elem>::Pointer
	Cache<T_ID, T_Elem>::get_if_in_cache(T_ID id)
	{
		MutexLock lock(_mutex);

//		_mutex.enterMutex();

		Entry* entry = search_in_cache(id);
		if (entry)
		{
			typename Cache<T_ID, T_Elem>::Pointer pointer(entry);
//			_mutex.leaveMutex();
			return pointer;
		}

//		_mutex.leaveMutex();
		return typename Cache<T_ID, T_Elem>::Pointer(0);
	}
	
	
	template <typename T_ID, typename T_Elem>
	typename Cache<T_ID, T_Elem>::Pointer
	Cache<T_ID, T_Elem>::get_or_prefetch(T_ID id, 
		pubsub::Subscriber<T_ID>* notifier)
	{
		MutexLock lock(_mutex);

//		_mutex.enterMutex();

		Entry* entry = search_in_cache(id);
		if (!entry)
		{
			prefetch(id, notifier);
		} else
		{
			typename Cache<T_ID, T_Elem>::Pointer pointer(entry);
//			_mutex.leaveMutex();
			return pointer;
		}

//		_mutex.leaveMutex();
		return typename Cache<T_ID, T_Elem>::Pointer(0);
	}
	
	
	template <typename T_ID, typename T_Elem>
	std::vector<T_ID>
	Cache<T_ID, T_Elem>::get_used_ids()
	{
		MutexLock lock(_mutex);

		std::vector<T_ID> result;
		
//		_mutex.enterMutex();
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
//		_mutex.leaveMutex();
	
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
		MutexLock lock(_mutex);

//		_mutex.enterMutex();
		Entry* search_result = search_in_cache(id);
		if ((search_result && search_result->object!=0) || 
			(!search_result && load_into_cache(id)->object!=0))
		{
//			_mutex.leaveMutex();
			return false;
		}

		search_result = search_in_cache(id);
		if (search_result && search_result->object==0)
		{
			typename std::map<T_ID, typename Cache<T_ID, T_Elem>::Entry >::iterator
				search_result_2 = _objects.find(id);
			if (search_result_2 != _objects.end() && search_result_2->second.users == 0)
			{
				int size = search_result_2->second._size;
				_objects.erase(search_result_2);
				_cached_size -= size;
			}
			else
				mlog(MLog::error, "Cache::insert") << "Could not erase "
					<< "element that should be empty!!!!\n";
		}

		std::pair<typename std::map<T_ID, typename Cache<T_ID, T_Elem>::Entry >::iterator, bool> result = 
			_objects.insert(new_entry_pair(id, elem, true, 
				(int)_average_object_size));
		_cached_size += (int)_average_object_size;
		
//		_mutex.leaveMutex();
		
		if (!result.second)
			mlog(MLog::error) << "Could not insert element that should be "
				<< "insertable!\n";

		return result.second;
	}
	

	template <typename T_ID, typename T_Elem>	
	bool
	Cache<T_ID, T_Elem>::insert(T_ID* id, T_Elem* elem)
	{
		MutexLock lock(_mutex);

//		_mutex.enterMutex();		
		if (_unused_ids.size() == 0)
		{
			std::vector<T_ID> unused_ids_vector = wrapper_get_free_ids();
			
			if (unused_ids_vector.size() == 0)
			{
//				_mutex.leaveMutex();
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
//		_mutex.leaveMutex();
		
		return insert_result;
	}
	
	
	template <typename T_ID, typename T_Elem>
	bool
	Cache<T_ID, T_Elem>::is_dirty(T_ID id)
	{
		MutexLock lock(_mutex);

//		_mutex.enterMutex();
		Entry* entry = search_in_cache(id);
		
		bool dirty;
		if (!entry) dirty = false;
			else dirty = entry.dirty;
//		_mutex.leaveMutex();
		
		return dirty;
	}
	
	
	template <typename T_ID, typename T_Elem>
	bool
	Cache<T_ID, T_Elem>::is_prefetching()
	{
		MutexLock lock(_prefetch_queue_mutex);

		bool result = false;
		
//		_prefetch_queue_mutex.enterMutex();
		result = !_prefetches.empty();
//		_prefetch_queue_mutex.leaveMutex();
		
		return result;
	}


	template <typename T_ID, typename T_Elem>
	void
	Cache<T_ID, T_Elem>::prefetch(T_ID id, 
		pubsub::Subscriber<T_ID>* notifier)
	{
		MutexLock lock(_prefetch_queue_mutex);

//		_prefetch_queue_mutex.enterMutex();
		_prefetches.push_back(
			std::pair<T_ID, pubsub::Subscriber<T_ID>* >(
				id,
				notifier
			)
		);
//		_prefetch_queue_mutex.leaveMutex();
		
		_thread_should_work_event.signal();
	}


	template <typename T_ID, typename T_Elem>
	bool
	Cache<T_ID, T_Elem>::remove(T_ID id)
	{
		MutexLock lock(_mutex);

//		_mutex.enterMutex();
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
//		_mutex.leaveMutex();
		
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
		MutexLock lock(_mutex);

		int counter = 0;
		
//		_mutex.enterMutex();
		typename std::map<T_ID, Entry>::iterator iter = _objects.begin();
		typename std::map<T_ID, Entry>::iterator iter_end = _objects.end();
		for (; iter != iter_end; ++iter)
			if (write_back(iter)) counter++;
//		_mutex.leaveMutex();
			
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
	Cache<T_ID, T_Elem>::persistent_load(T_ID id, int& size)
	{
		size = 1;
		return 0;
	}
	
	
	template <typename T_ID, typename T_Elem>
	void
	Cache<T_ID, T_Elem>::persistent_save(T_ID id, T_Elem* elem, int& size)
	{
		size = 1;
	}
	

	template <typename T_ID, typename T_Elem>
	void
	Cache<T_ID, T_Elem>::thread_deinit()
	{
		mlog(MLog::info, "Cache") << "Shutting down...\n";
		mlog(MLog::info, "Cache") << "Average object size was " << 
			_average_object_size << "\n";
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
			write_back();
		/*	_mutex.enterMutex();
		 	typename std::map<T_ID, Entry>::iterator iter;
		 	typename std::deque<T_ID>::iterator ids_iter = _object_ids.begin();
		 	typename std::deque<T_ID>::iterator ids_iter_end = _object_ids.end();
			int write_backs = 0;
			while (ids_iter != ids_iter_end && write_backs < 10)
			{
				T_ID delete_element_id = *ids_iter;
				bool flush_result = true;
				iter = _objects.find(delete_element_id);
				if (write_back(iter)) ++write_backs;

				++ids_iter;
			}
			_mutex.leaveMutex();*/
			
			if (cached_size() > soft_max_cached_size() &&
				_objects.size() > _minimal_object_capacity &&
				!(_options & _NO_MEMORY_LIMIT))
			{				
				_mutex.enterMutex();
				free_cache_down_to(soft_max_cached_size());
				_mutex.leaveMutex();
			}
			
			bool end = false;
			while(!end)
			{
				
				_prefetch_queue_mutex.enterMutex();
				if (_prefetches.empty())
					end = true;
				else
				{
					T_ID id = _prefetches.front().first;
					pubsub::Subscriber<T_ID>* notifier = 
						_prefetches.front().second;
					
					_mutex.enterMutex();				
					if (!search_in_cache(id))
						load_into_cache(id);
					_mutex.leaveMutex();
	
					if (notifier)
						notifier->receive(id);
					_prefetches.pop_front();					
				}
				_prefetch_queue_mutex.leaveMutex();
				
				_mutex.enterMutex();
				if (cached_size() > soft_max_cached_size() &&
					_objects.size() > _minimal_object_capacity)
					free_cache_down_to(soft_max_cached_size());				
				_mutex.leaveMutex();
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
		MutexLock lock(_mutex);
		
//		_mutex.enterMutex();
	 	typename std::map<T_ID, Entry>::iterator iter;
	 	typename std::deque<T_ID>::iterator ids_iter = _object_ids.begin();
	 	typename std::deque<T_ID>::iterator ids_iter_end = _object_ids.end();
		while (cached_size() > max_size && 
			_object_ids.size() > _minimal_object_capacity && 
			ids_iter != ids_iter_end)
		{
			T_ID delete_element_id = *ids_iter;			
			
			bool flush_result = true;
			iter = _objects.find(delete_element_id);
			if (iter != _objects.end())
				flush_result = flush(iter);
			
			if (flush_result)
			{
				ids_iter = _object_ids.erase(ids_iter);
			} else
			{
				++ids_iter;
			}
		}
//		_mutex.leaveMutex();
	}


	template <typename T_ID, typename T_Elem>
	inline bool
	Cache<T_ID, T_Elem>::flush(typename std::map<T_ID, Entry>::iterator entry)
	{
		#warning That will/does(?) not work. The same mutex is used in the method above!
		MutexLock lock(_mutex);

//		_mutex.enterMutex();
		T_ID id = entry->first;
		T_Elem* elem = entry->second.object;
		
		if (entry->second.dirty)
		{
			if(entry->second.users > 0)
				std::cout << "Mmh. ALERT! Cache is flushing data that is in use!" << std::endl;
			
			wrapper_save(id, elem);
		}
			
		if (entry->second.users == 0)
		{
			int size_of_object = entry->second._size;
			delete elem;
			_objects.erase(entry);
			_cached_size -= size_of_object;
//			_mutex.leaveMutex();
			return true;
		}
		
//		_mutex.leaveMutex();
		return false;
	}
	

	template <typename T_ID, typename T_Elem>
	typename Cache<T_ID, T_Elem>::Entry*
	Cache<T_ID, T_Elem>::load_into_cache(T_ID id)
	{
		MutexLock lock(_mutex);

//		_mutex.enterMutex();
		if (cached_size() >= hard_max_cached_size() && 
			(_objects.size() + 1) > _minimal_object_capacity &&
			!(_options & _NO_MEMORY_LIMIT))
			free_cache_down_to(hard_max_cached_size());
	
		if (cached_size() >= soft_max_cached_size() &&
			!(_options & _NO_MEMORY_LIMIT))
		{
			_thread_should_work_event.signal();
		}
		
		int size;
		T_Elem* elem = wrapper_load(id);
		
		std::pair<typename std::map<T_ID, typename Cache<T_ID, T_Elem>::Entry>::iterator, bool>
			result = _objects.insert(new_entry_pair(id, elem, false, size));
		
		if (!result.second)
		{
			mlog(MLog::error, "Cache::load_into_cache") 
				<< "Wanted to insert a tile into cache, but map says it's already inserted... :-(\n";
//			_mutex.leaveMutex();
			return 0;
		} else
		{
			_cached_size += size;
			_object_ids.push_back(id);
//			_mutex.leaveMutex();
			return &((result.first)->second);
		}

//		_mutex.leaveMutex();			// This should be unreachable....
		
	}
	
	
	template <typename T_ID, typename T_Elem>
	std::pair<T_ID, typename Cache<T_ID, T_Elem>::Entry>
	Cache<T_ID, T_Elem>::new_entry_pair(T_ID id, T_Elem* elem, bool dirty, 
		int size)
	{
		std::pair<T_ID, typename Cache<T_ID, T_Elem>::Entry> new_pair;
		new_pair.first = id;
		new_pair.second.object = elem;
		new_pair.second.users = 0;
		new_pair.second.dirty = dirty;
		new_pair.second._size = size;

		return new_pair;
	}
	
	
	template <typename T_ID, typename T_Elem>
	typename Cache<T_ID, T_Elem>::Entry*
	Cache<T_ID, T_Elem>::search_in_cache(T_ID id)
	{
		MutexLock lock(_mutex);
		
//		_mutex.enterMutex();
		typename std::map<T_ID, typename Cache<T_ID, T_Elem>::Entry >::iterator search_result = 
			_objects.find(id);
			
		if (search_result != _objects.end())
		{
//			_mutex.leaveMutex();
			return &(*search_result).second;
		}

//		_mutex.leaveMutex();
		return 0;
	}
	
	
	template <typename T_ID, typename T_Elem>
	void
	Cache<T_ID, T_Elem>::update_average_object_size(int size)
	{
		_average_object_size = 
			((_average_object_size * _average_object_size_counter) + (double)size) /
			(_average_object_size_counter + 1.0);
		_average_object_size_counter += 1.0;
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
			int size;
			T_Elem* elem = persistent_load(id, size);
			if (elem)
				update_average_object_size(size);
			return elem;
		} else
			return 0;
	}
	

	template <typename T_ID, typename T_Elem>
	inline void
	Cache<T_ID, T_Elem>::wrapper_save(T_ID id, T_Elem* elem)
	{
		if (!(_options & _NON_PERSISTENT) && !(_options & _NO_WRITEBACK))
		{
			int size;
			persistent_save(id, elem, size);
			update_average_object_size(size);
		}
	}
	
	
	template <typename T_ID, typename T_Elem>
	inline bool
	Cache<T_ID, T_Elem>::write_back(typename std::map<T_ID, Entry>::iterator iter)
	{
		MutexLock lock(_mutex);

//		_mutex.enterMutex();
		if (iter->second.dirty && iter->second.users == 0)
		{
			T_ID id = iter->first;
			T_Elem* elem = iter->second.object;
			wrapper_save(id, elem);
			iter->second.dirty = false;

//			_mutex.leaveMutex();
			return true;
		}

//		_mutex.leaveMutex();
		return false;
	}
	
	

} // namespace mapgeneration_util

#endif // CACHE_H
