/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include <iostream>
#include <vector>
#include "util/cache.h"

using namespace std;
using namespace mapgeneration_util;


class TestObject
{
	public:
		
		TestObject(int size)
		: _size(size)
		{
		}
		
		
		int _size;
};


vector<TestObject*> data;


class TestCache : public Cache<int, TestObject>
{
	
	public:
	
		TestCache()
		: Cache<int, TestObject>::Cache(Cache<int, TestObject>::_FIFO, 
			Cache<int, TestObject>::_STANDARD_CACHE, 100000, 1000, 800)
		{
		}
		
		
	protected:
		
		bool
		persistent_erase(int id)
		{
			if (data[id] != 0)
			{
				delete data[id];
				data[id] = 0;
				return true;
			}
			
			return false;
		}
		
		
		TestObject*
		persistent_load(int id)
		{
			if (id < data.size() && data[id] != 0)
			{
				return new TestObject(*data[id]);
			}
			
			return 0;
		}
		
		
		void
		persistent_save(int id, TestObject* elem)
		{
			if (data[id] != 0)
			{
				delete data[id];
			}
			data[id] = new TestObject(*elem);
		}
		
		
		int
		persistent_size_of(TestObject* elem)
		{
			if (elem)
				return elem->_size;
			else
				return sizeof(TestObject*);
		}
};


class TestThread : public ControlledThread
{
	
	public:
	
		TestThread(TestCache* cache)
		: ControlledThread::ControlledThread(), _cache(cache), _counter(0),
			_go_event()
		{
		}
		
	
		void thread_run()
		{
			cout << "Thread started!\n";
			_go_event.wait(10000);
			sleep(1);
			while (!should_stop())
			{
				int random_number = (int) ((double)data.size() * ((double)std::rand() / (double)RAND_MAX));
				_cache->get(random_number);
				_counter++;
			}
			cout << "Thread stopping!\n";
		}
		
		TestCache* _cache;
		
		int _counter;
		
		ost::Event _go_event;

};


void
show_cache_stats(TestCache& cache)
{
	cout << "Cache stats:\n";
	cout << "  Cached objects: " << cache.cached_objects() << "\n";
	cout << "  Cached size   : " << cache.cached_size() << " / (" 
		<< cache.soft_max_cached_size() << " / " 
		<< cache.hard_max_cached_size() << ")\n";
	cout << "\n";
}


int main()
{
	cout << "\nTesting the cache!\n\n";
	
	cout << "We'll start with initializing a data base.\nIn this test we will"
		<< "use a vector. The objects 0-10000 simulate the size 1, the objects "
		<< "20000-30000 simulate the size 10.\n";
	int i;
	for (i=0; i<10000; i++)
		data.push_back(new TestObject(1));
	for (; i<20000; i++)
		data.push_back(0);
	for (; i<30000; i++)
		data.push_back(new TestObject(10));
	cout << "OK!\n\n";

	cout << "Now we have to initialize a cache:\n";
	TestCache cache;
	cache.controlled_start();
	show_cache_stats(cache);
	cout << "OK!\n\n";
	
	cout << "Let's fill our cache by using elements 20000 to 20100 for only a short time:\n";
	for (i=20000; i<20100; i++)
		cache.get(i);
	sleep(1);
	show_cache_stats(cache);
	cout << "OK!\n\n";
	
	cout << "Let's use elements 0 to 100:\n";
	for (i=0; i<100; i++)
		cache.get(i);
	sleep(1);
	show_cache_stats(cache);
	cout << "OK!\n\n";
	
	cout << "Now we test the get_or_prefetch functions. First we try to "
		<< "get_or_prefetch element 0 until its returned (which should the "
		<< "the case immidiatly:\n";
	while (cache.get_or_prefetch(0) == 0)
	{
		cout << "Not yet in cache.\n";
	}
	cout << "In cache.\n";
	cout << "OK!\n\n";
	
	cout << "The same with element 500 which should be loaded after a short "
		<< " time:\n";
	while (cache.get_or_prefetch(500) == 0)
	{
		cout << "Not yet in cache.\n";
	}
	cout << "In cache.\n";
	cout << "OK!\n\n";	
	
	cout << "Let's random access our cache 10000 times:\n";
	for (i=0; i<10; i++)
	{
		show_cache_stats(cache);
		for (int j=0; j<1000; j++)
		{
			int random_number = (int) ((double)data.size() * ((double)std::rand() / (double)RAND_MAX));
			cache.get(random_number);
		}
	}
	cout << "OK!!\n\n";
	
	cout << "Ok, that is boring. Let's access our cache with 10 threads:\n";
	TestThread tt1(&cache); TestThread tt2(&cache);
	TestThread tt3(&cache);	TestThread tt4(&cache);
	TestThread tt5(&cache);	TestThread tt6(&cache);
	TestThread tt7(&cache);	TestThread tt8(&cache);
	TestThread tt9(&cache);	TestThread tt10(&cache);
	tt1.controlled_start(true); tt2.controlled_start(true);
	tt3.controlled_start(true); tt4.controlled_start(true);
	tt5.controlled_start(true); tt6.controlled_start(true);
	tt7.controlled_start(true); tt8.controlled_start(true);
	tt9.controlled_start(true); tt10.controlled_start(true);
	tt1._go_event.signal(); tt2._go_event.signal();
	tt3._go_event.signal(); tt4._go_event.signal();
	tt5._go_event.signal(); tt6._go_event.signal();
	tt7._go_event.signal(); tt8._go_event.signal();
	tt9._go_event.signal(); tt10._go_event.signal();
	for (i=0; i<10; i++)
	{
		sleep(1);
		show_cache_stats(cache);
	}
	tt1.controlled_stop(true); tt2.controlled_stop(true);
	tt3.controlled_stop(true); tt4.controlled_stop(true);
	tt5.controlled_stop(true); tt6.controlled_stop(true);
	tt7.controlled_stop(true); tt8.controlled_stop(true);
	tt9.controlled_stop(true); tt10.controlled_stop(true);	
	cout << "OK! Counted " 
		<< tt1._counter + tt2._counter + tt3._counter + tt4._counter 
			+ tt5._counter + tt6._counter + tt7._counter + tt8._counter 
			+ tt9._counter + tt10._counter 
		<< " accesses in about 10 seconds.\n\n";
	
	cout << "Cleaning used memory: ";
	int cleaned_objects = 0;
	for (i=0; i<data.size(); i++)
		if (data[i]!=0)
		{
			cleaned_objects++;
			delete data[i];
		}
	cout << " deleted " << cleaned_objects << " objects. OK!\n\n";
	
	cout << "Stopping cache:\n";
	cache.controlled_stop();
	cout << "OK!\n\n";
	
	return 0;
}
