/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef EXECUTIONMANAGER_H
#define EXECUTIONMANAGER_H

#include <cc++/thread.h>

#include "defaultconfiguration.h"
#include "tilecache.h"
#include "tilemanager.h"
#include "tracefilter.h"
#include "traceserver.h"
#include "util/pubsub/servicesystem.h"


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
 */
class ExecutionManager {
	public:
		
		/**
		 * @brief Empty constructor.
		 * 
		 * Probably it will do nothing.
		 */
		ExecutionManager();
	
	
		/**
		 * @brief The main method of the ExecutionManager.
		 * 
		 * Starts the other processes, waits for shutdown-condition
		 * and shuts everything down in the end.
		 */
		void
		run();

	
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
	
};

} // namespace mapgeneration

#endif //EXECUTIONMANAGER_H
