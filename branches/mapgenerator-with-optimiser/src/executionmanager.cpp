/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/

#include "executionmanager.h"

#include "config.h"
#include "defaultconfiguration.h"
#include "filteredtrace.h"
#include "tilecache.h"
#include "tilemanager.h"
#include "tracefilter.h"
#include "traceserver.h"

#include "dbconnection/filedbconnection.h"
#include "util/configuration.h"
#include "util/mlog.h"

#ifdef HAVE_ODBC
	#include "dbconnection/odbcdbconnection.h"
#endif


using namespace mapgeneration_util;

namespace mapgeneration
{

	ExecutionManager::ExecutionManager()
	: ControlledThread(),
		_db_connection(0), _service_list(0), _trace_filter(0), _trace_server(0),
		_tile_cache(0), _tile_manager(0), _mutex()
	{
		set_delete_database(false);
		set_delete_tracelogs(true);
		set_single_trace_mode(false);
		set_start_trace_filter(true);
		set_start_trace_server(true);
	}
	
	
	ExecutionManager::~ExecutionManager()
	{
	}
	
	
	bool
	ExecutionManager::get_delete_database() const
	{
		return _status & _DELETE_DATABASE;
	}
	
	
	bool
	ExecutionManager::get_delete_tracelogs() const
	{
		return _status & _DELETE_TRACELOGS;
	}
	
	
	bool
	ExecutionManager::get_single_trace_mode() const
	{
		return _status & _SINGLE_TRACE_MODE;
	}
	
	bool
	ExecutionManager::get_start_trace_filter() const
	{
		return _status & _START_TRACE_FILTER;
	}
	
	
	bool
	ExecutionManager::get_start_trace_server() const
	{
		return _status & _START_TRACE_SERVER;
	}
	
	
	void
	ExecutionManager::run_without_thread()
	{
		thread_init();
		
		mlog(MLog::info, "ExecutionManager") << "Use \"o\" to stop the MapGenerator.\n";
		mlog(MLog::info, "ExecutionManager") << "Use \"d\" to delete the database afterwards. (ONLY works in DEBUG mode!)\n";
		mlog(MLog::info, "ExecutionManager") << "Use \"t\" to delete the tracelogs afterwards.\n";
		mlog(MLog::info, "ExecutionManager") << "Use \"a\" to delete both afterwards.\n";
		mlog(MLog::info, "ExecutionManager") << "Use \"c\" to continue the processing of the traces.\n";
		
		bool stop = false;		
		while (!stop)
		{
			char c = getchar();
			
			if (c == 'o')
			{
				stop = true;
				set_delete_database(false);
				set_delete_tracelogs(false);
			}
			
			if (c == 'd')
			{
				stop = true;
				set_delete_database(true);
				set_delete_tracelogs(false);
			}
			
			if (c == 't')
			{
				stop = true;
				set_delete_database(false);
				set_delete_tracelogs(true);
			}
			
			if (c == 'a')
			{
				stop = true;
				set_delete_database(true);
				set_delete_tracelogs(true);
			}
			
			if (c == 'c')
			{
				_tile_manager->continue_processing();
			}
		}
		
		thread_deinit();
	}
	
	pubsub::ServiceList*
	ExecutionManager::service_list()
	{
		ost::MutexLock lock(_mutex);
		
		// return the service list if already loaded. Null pointer else.
		return _service_list;
	}
	
	
	void
	ExecutionManager::set_delete_database(bool flag)
	{
		if (flag)
			_status = _status | _DELETE_DATABASE;
		else
			_status = _status & (!_DELETE_DATABASE);
	}
	
	
	void
	ExecutionManager::set_delete_tracelogs(bool flag)
	{
		if (flag)
			_status = _status | _DELETE_TRACELOGS;
		else
			_status = _status & (!_DELETE_TRACELOGS);
	}
	
	
	void
	ExecutionManager::set_single_trace_mode(bool flag)
	{
		if (flag)
			_status = _status | _SINGLE_TRACE_MODE;
		else
			_status = _status & (!_SINGLE_TRACE_MODE);
	}
	
	
	void
	ExecutionManager::set_start_trace_filter(bool flag)
	{
		if (flag)
			_status = _status | _START_TRACE_FILTER;
		else
			_status = _status & (!_START_TRACE_FILTER);
	}
	
	
	void
	ExecutionManager::set_start_trace_server(bool flag)
	{
		if (flag)
			_status = _status | _START_TRACE_SERVER;
		else
			_status = _status & (!_START_TRACE_SERVER);
	}
	
	
	void
	ExecutionManager::thread_deinit()
	{
		ost::MutexLock lock(_mutex);
		
		mlog(MLog::info, "ExecutionManager") << "MapGenerator shutting down! :-(\n";
		
		mlog(MLog::info, "ExecutionManager") << "Stopping TraceServer...\n";
		if (_trace_server == 0)
		{
			mlog(MLog::info, "ExecutionManager") << "TraceServer was not running.\n";
		} else
		{
			_trace_server->controlled_stop();
			delete _trace_server;
			mlog(MLog::info, "ExecutionManager") << "TraceServer stopped.\n";
		}

		mlog(MLog::info, "ExecutionManager") << "Stopping TraceFilter...\n";
		if (_trace_filter == 0)
		{
			mlog(MLog::info, "ExecutionManager") << "TraceServer was not running.\n";
		} else
		{
			_trace_filter->controlled_stop();
			delete _trace_filter;
			mlog(MLog::info, "ExecutionManager") << "TraceFilter stopped.\n";
		}

		mlog(MLog::info, "ExecutionManager") << "Stopping TileManager...\n";
		_tile_manager->controlled_stop();
		delete _tile_manager;
		mlog(MLog::info, "ExecutionManager") << "TileManager stopped.\n";
		
		mlog(MLog::info, "ExecutionManager") << "Stopping TileCache...\n";
		_tile_cache->controlled_stop();
		delete _tile_cache;
		mlog(MLog::info, "ExecutionManager") << "TileCache stopped.\n";
		
		if (get_delete_database())
		{
			mlog(MLog::info, "ExecutionManager") << "Deleting DB ...\n";
			_db_connection->drop_tables();
			mlog(MLog::info, "ExecutionManager") << "DB deleted.\n";
		}

		mlog(MLog::info, "ExecutionManager") << "Disconnecting DBConnection...\n";
		_db_connection->disconnect();		
		delete _db_connection;
		mlog(MLog::info, "ExecutionManager") << "DBConnection deleted.\n";
		
		mlog(MLog::info, "ExecutionManager") << "Deleting ServiceList...\n";
		_service_list->delete_all_services();
		delete _service_list;
		mlog(MLog::info, "ExecutionManager") << "ServiceList deleted.\n";
		
		if (get_delete_tracelogs())
		{
			mlog(MLog::info, "ExecutionManager") << "Deleting Tracelogs...\n";
			
			std::string directory_name = "traceprocessor_logs/";
			
			DIR *dir;
			struct dirent *direntp;
			dir = opendir (directory_name.c_str());
			if (dir != NULL)
			{
				readdir(dir); readdir(dir);
				while ((direntp = readdir(dir)) != NULL)
				{
					std::string file_name = directory_name;
					file_name += "/";
					file_name += direntp->d_name;
					unlink(file_name.c_str());
				}
				closedir (dir);
			}
			
			rmdir(directory_name.c_str());
			
			mlog(MLog::info, "ExecutionManager") << "Tracelogs deleted.\n";
		}

	}
	
	
	void
	ExecutionManager::thread_init()
	{
		MutexLock lock(_mutex);
		
		mlog(MLog::info, "ExecutionManager") << "Running.\n";
		
		mlog(MLog::info, "ExecutionManager") << "Creating Tracelogs directory...\n";
		std::string directory_name = "traceprocessor_logs/";
		
		DIR *dir;
		struct dirent *direntp;
		dir = opendir (directory_name.c_str());
		if (dir == NULL)
		{
			mkdir(directory_name.c_str(), 0700);
		}
		mlog(MLog::info, "ExecutionManager") << "TraceLogs directory created.\n";
		
		mlog(MLog::info, "ExecutionManager") << "Initializing ServiceList.\n";
		_service_list = new pubsub::ServiceList();
		mlog(MLog::info, "ExecutionManager") << "ServiceList initialized.\n";
		
		mlog(MLog::info, "ExecutionManager") << "Loading configuration.\n";
		DefaultConfiguration default_configuration;
		std::vector< Configuration::Parameter > dc = 
			default_configuration.get();
		Configuration configuration(CONFIGURATION_PATH, _service_list, "",
			&dc);
		configuration.read_configuration();
		mlog(MLog::info, "ExecutionManager") << "Configuration loaded.\n";
		
		mlog(MLog::info, "ExecutionManager") << 
			"Checking licence acceptance.\n";
		std::string licence_accepted;
		if (!_service_list->get_service_value("accepted_licence", 
			licence_accepted) ||
			(licence_accepted != "yes"))
		{
			std::cout << "\nYou did not yet accept the licence.\n"
			          << "This program is licenced under the Academic Free License version 2.1.\n"
			          << "The complete licence text is available in the LICENCE file and at\n"
			          << "http://www.opensource.org/licenses/afl-2.1.php\n"
			          << "\n"
			          << "If you understand and accept the licence terms enter 'y' to continue, \n"
			          << "otherwise press enter to abort program execution.\n"
			          << "If you are the only user you may also change the accepted_licence\n"
			          << "value in the mapgenerator.conf file to 'yes' to accept the licence.\n\n";
			char c = getchar();
			if (c != 'y' && c != 'Y')
			{
				mlog(MLog::info, "ExecutionManager") << 
					"Licence not accepted.\n";
				throw("Licence not accepted!");
			}
		}
		mlog(MLog::info, "ExecutionManager") << "Licence accepted.\n";
		
		
		mlog(MLog::info, "ExecutionManager") << "Initializing DBConnection.\n";
		
		std::string db_type;
		if (!_service_list->get_service_value("db.type", db_type))
			throw("Configuration for db type not found.");
		if (db_type == "file")
		{
			FileDBConnection* file_db_connection = new FileDBConnection();
			std::string db_directory;
			if (!_service_list->get_service_value("db.file.directory",
				db_directory))
				throw("DB directory not configured!");
			file_db_connection->set_parameters(db_directory);
			_db_connection = file_db_connection;
		} else if (db_type == "odbc")
		{
#ifdef HAVE_ODBC
			ODBCDBConnection* odbc_db_connection = new ODBCDBConnection();
			std::string dns, user, password;
			if (!_service_list->get_service_value("db.odbc.dns", dns) ||
				!_service_list->get_service_value("db.odbc.user", user) ||
				!_service_list->get_service_value("db.odbc.password", password))
				throw("Missing parameters for db connection!");
			odbc_db_connection->set_parameters(dns, user, password, true);
			_db_connection = odbc_db_connection;
#else
			mlog(MLog::error, "ExecutionManager") << "ODBC support not " <<
				"compiled. Please set db type to file in mapgenerator.conf.\n";
			throw("ODBC not supported.");			
#endif
		} else
			throw("Unknown db type!");
			
		size_t tiles_table_id = _db_connection->register_table("tiles");
		_db_connection->connect();
		mlog(MLog::info, "ExecutionManager") << "DBConnection initialized.\n";
		
		mlog(MLog::info, "ExecutionManager") << "Starting TileCache.\n";
		int min_object_capacity, hard_max_size, soft_max_size;
		if (!_service_list->get_service_value("tilecache.min_object_capacity",
				min_object_capacity) ||
			!_service_list->get_service_value("tilecache.hard_max_size",
				hard_max_size) ||
			!_service_list->get_service_value("tilecache.soft_max_size",
				soft_max_size))
			throw("Missing parameters for tilecache!");
		_tile_cache = new TileCache(_db_connection, tiles_table_id,
			TileCache::_FIFO, TileCache::_STANDARD_CACHE, min_object_capacity,
			hard_max_size, soft_max_size);
		_tile_cache->controlled_start();
		mlog(MLog::info, "ExecutionManager") << "TileCache started.\n";

		mlog(MLog::info, "ExecutionManager") << "Starting TileManager.\n";
		_tile_manager = new TileManager(_service_list, _tile_cache);
		_tile_manager->set_single_trace_mode(get_single_trace_mode());
		_tile_manager->controlled_start();
		mlog(MLog::info, "ExecutionManager") << "TileManager started.\n";
		
		// take a look at the _start_* parameter and decide which module
		// have to be ran
		if (get_start_trace_server() || get_start_trace_filter())
		{
			mlog(MLog::info, "ExecutionManager") << "Starting TraceFilter.\n";
			_trace_filter = new TraceFilter(_service_list, _tile_manager);
			_trace_filter->controlled_start();
			mlog(MLog::info, "ExecutionManager") << "TraceFilter started.\n";
		}
		
		if (get_start_trace_server())
		{
			mlog(MLog::info, "ExecutionManager") << "Starting TraceServer.\n";
			/** @todo change parameters here: TileManager -> TraceFilter */
			_trace_server = new TraceServer(_service_list, _trace_filter);
			_trace_server->controlled_start();
			mlog(MLog::info, "ExecutionManager") << "TraceServer started.\n";
		}
	}
	
	
	void
	ExecutionManager::thread_run()
	{
		_should_stop_event.wait();
	}
	
	
	TileManager*
	ExecutionManager::tile_manager()
	{
		MutexLock lock(_mutex);
		
		// return the tile manager if already loaded. Null pointer else.
		return _tile_manager;
	}
	
} // namespace mapgeneration
