/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "mappanellogic.h"

#include <iostream>
#include <set>
#include "dbconnection/filedbconnection.h"
#include "dbconnection/odbcdbconnection.h"
#include "util/mlog.h"


namespace mapgeneration_gui
{
	
	MapPanelLogic::MapPanelLogic
		(wxScrolledWindow* map_scrolled_window, 
		pubsub::ServiceList* service_list)
	: MapScrolledWindowLogic::MapScrolledWindowLogic(map_scrolled_window)
	{
		
		mlog(MLog::info, "MapPanelLogic") << "Initializing DBConnection.\n";
		
		std::string db_type;
		if (!service_list->get_service_value("db.type", db_type))
			throw("Configuration for db type not found.");
		if (db_type == "file")
		{
			FileDBConnection* file_db_connection = new FileDBConnection();
			std::string db_directory;
			if (!service_list->get_service_value("db.file.directory",
				db_directory))
				throw("DB directory not configured!");
			file_db_connection->set_parameters(db_directory);
			_db_connection = file_db_connection;
		} else if (db_type == "odbc")
		{
			ODBCDBConnection* odbc_db_connection = new ODBCDBConnection();
			std::string dns, user, password;
			if (!service_list->get_service_value("db.odbc.dns",
				dns) ||
				!service_list->get_service_value("db.odbc.user",
				user) ||
				!service_list->get_service_value("db.odbc.password",
				password))
				throw("Missing parameters for db connection!");
			odbc_db_connection->set_parameters(dns, user, password, true);
			_db_connection = odbc_db_connection;
		} else
			throw("Unknown db type!");
			
		size_t tiles_table_id = _db_connection->register_table("tiles");
		_db_connection->connect();
		mlog(MLog::info, "MapPanelLogic") << "DBConnection initialized.\n";
		
/*		ODBCDBConnection* odbc_db_connection = new ODBCDBConnection;
		odbc_db_connection->set_parameters("MapGeneration", "mapgeneration", "mg");		*/
//		FileDBConnection* file_db_connection = new FileDBConnection();
//		file_db_connection->set_parameters("filedb");
//		_db_connection = file_db_connection;
//		_db_connection = odbc_db_connection;
//		size_t tiles_table_id = _db_connection->register_table("tiles");
//		_db_connection->connect();
		
		mlog(MLog::info, "MapPanelLogic") << "Initializing cache.\n";

		TileCache* tile_cache = new TileCache(_db_connection, tiles_table_id,
			TileCache::_FIFO, TileCache::_NO_WRITEBACK, 20000000, 18000000);
		tile_cache->start();

		set_tile_cache(tile_cache);
		
		mlog(MLog::info, "MapPanelLogic") << "Cache initialized.\n";
	}
	
	
	MapPanelLogic::~MapPanelLogic()
	{
		delete _tile_cache;
		
		_db_connection->disconnect();
		delete _db_connection;
	}

	
	void
	MapPanelLogic::on_paint(wxPaintEvent& wx_paint_event)
	{		
		wxPaintDC dc(_map_scrolled_window);
		_map_scrolled_window->PrepareDC(dc);
		
		draw_map(wx_paint_event, dc);		
	}

} // namespace mapgeneration_gui
