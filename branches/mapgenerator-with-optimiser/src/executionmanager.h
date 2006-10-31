/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef EXECUTIONMANAGER_H
#define EXECUTIONMANAGER_H

#include "util/controlledthread.h"

/* Forward declarations ... */
namespace pubsub
{
	
	class ServiceList;
	
} // namespace pubsub

namespace mapgeneration
{
	
	class DBConnection;
	class TileCache;
	class TileManager;
	class TraceFilter;
	class TraceServer;
	
} // namespace mapgeneration
/* Forward declarations done! */


namespace mapgeneration
{

	/**
	 * @brief Starting process of this program.
	 * 
	 * <ul>
	 * <li>starts TraceServer and TileManager</li>
	 * <li>inits the DBConnection</li>
	 * <li>... (extensable!)</li>
	 * </ul>
	 *
	 * Use controlled_start() to use the ExecutionManager as thread.
	 * run_without_thread() does not make use of the thread properties. But emulates
	 * the same behaviour.
	 */
	class ExecutionManager : public mapgeneration_util::ControlledThread
	{
		
		public:
			
			/**
			 * @brief Empty constructor.
			 */
			ExecutionManager();
			
			
			virtual
			~ExecutionManager();
			
			
			bool
			get_delete_database() const;
			
			
			bool
			get_delete_tracelogs() const;
			
			
			bool
			get_single_trace_mode() const;
			
			
			bool
			get_start_trace_filter() const;
			
			
			bool
			get_start_trace_server() const;
			
			
			/**
			 * @brief The main method of the ExecutionManager.
			 * 
			 * Starts the other processes, waits for shutdown-condition
			 * and shuts everything down in the end.
			 *
			 * Use controlled_start() when the ExecutionManager should run
			 * as thread!
			 */
			void
			run_without_thread();
	
		
			pubsub::ServiceList*
			service_list();
			
			
			void
			set_delete_database(bool flag);
			
			
			void
			set_delete_tracelogs(bool flag);
			
			
			void
			set_single_trace_mode(bool flag);
			
			
			void
			set_start_trace_filter(bool flag);
			
			
			void
			set_start_trace_server(bool flag);
			
			
			TileManager*
			tile_manager();
			
			
		protected:
			
			virtual void
			thread_deinit();
			
			
			virtual void
			thread_init();
			
			
			virtual void
			thread_run();
			
			
		private:
	
			/**
			 * @brief Pointer to the DBConnection.
			 * 
			 * @see DBConnection
			 */
			DBConnection* _db_connection;
			
			
			/**
			 * @brief Pointer to the central (and only?) ServiceList.
			 * 
			 * @see pubsub::ServiceList
			 */
			pubsub::ServiceList* _service_list;
			
			
			/**
			 * @brief Pointer to the TraceFilter.
			 * 
			 * @see TraceFilter
			 */
			TraceFilter* _trace_filter;
	
		
			/**
			 * @brief Pointer to the TraceServer.
			 * 
			 * @see TraceServer
			 */
			TraceServer* _trace_server;
		
		
			/**
			 * @brief Pointer to the TileCache.
			 * 
			 * @see TileCache
			 */
			TileCache* _tile_cache;
		
		
			/**
			 * @brief Pointer to the TielManager.
			 * 
			 * @see TileManager
			 */
			TileManager* _tile_manager;
			
			
			unsigned int _status;
			static const int _DELETE_DATABASE =		1;
			static const int _DELETE_TRACELOGS =	2;
			static const int _START_TRACE_FILTER =	4;
			static const int _START_TRACE_SERVER =	8;
			static const int _SINGLE_TRACE_MODE =	16;
			
//			bool _delete_database;
//			bool _delete_tracelogs;
//			bool _start_trace_filter;
//			bool _start_trace_server;
			ost::Mutex _mutex;
		
	};
	
} // namespace mapgeneration

#endif //EXECUTIONMANAGER_H
