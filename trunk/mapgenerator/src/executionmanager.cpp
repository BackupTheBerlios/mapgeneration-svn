/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "executionmanager.h"

#include "traceserver.h"
#include "util/configuration.h"
#include "util/mlog.h"


using namespace mapgeneration_util;

namespace mapgeneration
{

	ExecutionManager::ExecutionManager()
	{
	}
	
	
	void
	ExecutionManager::run()
	{
		bool stop = false;
		bool delete_db = false;
		
		mlog(MLog::info, "ExecutionManager") << "Running.\n";
				
		mlog(MLog::info, "ExecutionManager") << "Initializing ServiceList.\n";
		_service_list = new pubsub::ServiceList();
		mlog(MLog::info, "ExecutionManager") << "ServiceList initialized.\n";
		
		mlog(MLog::info, "ExecutionManager") << "Loading configuration.\n";
		DefaultConfiguration default_configuration;
		std::vector< Configuration::Parameter > dc = 
			default_configuration.get();
		Configuration configuration(CONFIGURATION_PATH, _service_list,
			&dc);
		configuration.read_configuration();
		mlog(MLog::info, "ExecutionManager") << "Configuration loaded.\n";
		
		mlog(MLog::info, "ExecutionManager") << "Initializing DBConnection.\n";
		_db_connection = new DBConnection();
		size_t tiles_table_id = _db_connection->register_table("tiles");		
		_db_connection->init();
		_db_connection->connect("MapGeneration", "mapgeneration", "mg", true);
		mlog(MLog::info, "ExecutionManager") << "DBConnection initialized.\n";
		
		mlog(MLog::info, "ExecutionManager") << "Starting TileCache.\n";
		_tile_cache = new TileCache(_db_connection, tiles_table_id,
			TileCache::_FIFO, TileCache::_STANDARD_CACHE, 12000000, 10000000);
		_tile_cache->controlled_start();
		mlog(MLog::info, "ExecutionManager") << "TileCache started.\n";

		mlog(MLog::info, "ExecutionManager") << "Starting TileManager.\n";
		_tile_manager = new TileManager(_service_list, _tile_cache);
		_tile_manager->controlled_start();
		mlog(MLog::info, "ExecutionManager") << "TileManager started.\n";
		
		mlog(MLog::info, "ExecutionManager") << "Starting TraceFilter.\n";
		_trace_filter = new TraceFilter(_service_list, _tile_manager);
		_trace_filter->controlled_start();
		mlog(MLog::info, "ExecutionManager") << "TraceFilter started.\n";
		
		mlog(MLog::info, "ExecutionManager") << "Starting TraceServer.\n";
		/** @todo change parameters here: TileManager -> TraceFilter */
		_trace_server = new TraceServer(_service_list, _trace_filter);
		_trace_server->controlled_start();
		mlog(MLog::info, "ExecutionManager") << "TraceServer started.\n";


		while (!stop)
		{
			char c = getchar();
			
			if (c == 'x') stop = true;
			
			if (c == 'y') {
				stop = true;
				delete_db= true;
			}
		}


		mlog(MLog::info, "ExecutionManager") << "MapGenerator shutting down! :-(\n";

		
		mlog(MLog::info, "ExecutionManager") << "Stopping TraceServer...\n";
		_trace_server->controlled_stop();
		delete _trace_server;
		mlog(MLog::info, "ExecutionManager") << "TraceServer stopped.\n";

		mlog(MLog::info, "ExecutionManager") << "Stopping TraceFilter...\n";
		_trace_filter->controlled_stop();
		delete _trace_filter;
		mlog(MLog::info, "ExecutionManager") << "TraceFilter stopped.\n";

		mlog(MLog::info, "ExecutionManager") << "Stopping TileManager...\n";
		_tile_manager->controlled_stop();
		delete _tile_manager;
		mlog(MLog::info, "ExecutionManager") << "TileManager stopped.\n";
		
		mlog(MLog::info, "ExecutionManager") << "Stopping TileCache...\n";
		_tile_cache->controlled_stop();
		delete _tile_cache;
		mlog(MLog::info, "ExecutionManager") << "TileCache stopped.\n";
		
		#ifdef DEBUG
			if (delete_db == true)
			{
				mlog(MLog::info, "ExecutionManager") << "Deleting DB ...\n";
				_db_connection->dropTables();
				mlog(MLog::info, "ExecutionManager") << "DB deleted.\n";
			}
		#endif

		mlog(MLog::info, "ExecutionManager") << "Disconnecting DBConnection...\n";
		_db_connection->disconnect();
		_db_connection->destroy();
		delete _db_connection;
		mlog(MLog::info, "ExecutionManager") << "DBConnection deleted.\n";
		
		mlog(MLog::info, "ExecutionManager") << "Deleting ServiceList...\n";
		delete _service_list;
		mlog(MLog::info, "ExecutionManager") << "ServiceList deleted.\n";
	}

} // namespace mapgeneration
