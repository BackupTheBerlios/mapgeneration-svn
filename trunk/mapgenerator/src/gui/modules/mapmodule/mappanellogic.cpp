/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "mappanellogic.h"

#include <iostream>
#include <set>
#include "dbconnection/odbcdbconnection.h"
#include "util/mlog.h"


namespace mapgeneration_gui
{
	
	MapPanelLogic::MapPanelLogic
		(wxScrolledWindow* map_scrolled_window)
	: MapScrolledWindowLogic::MapScrolledWindowLogic(map_scrolled_window)
	{
		mlog(MLog::info, "MapPanelLogic") 
			<< "Initializing DBConnection and caches.\n";
		ODBCDBConnection* odbc_db_connection = new ODBCDBConnection;
		odbc_db_connection->set_parameters("MapGeneration", "mapgeneration", "mg");		
		_db_connection = odbc_db_connection;
		size_t tiles_table_id = _db_connection->register_table("tiles");
		_db_connection->connect();

		TileCache* tile_cache = new TileCache(_db_connection, tiles_table_id,
			TileCache::_FIFO, TileCache::_NO_WRITEBACK, 20000000, 18000000);
		tile_cache->start();

		set_tile_cache(tile_cache);
		
		mlog(MLog::info, "MapPanelLogic")
			<< "DBConnection and caches initialized.\n";
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
