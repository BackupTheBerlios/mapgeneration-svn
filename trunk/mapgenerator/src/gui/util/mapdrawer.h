/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef MAPDRAWER_H
#define MAPDRAWER_H

#include <bitset>
#include <ctime>
#include <vector>
#include <wx/wx.h>
#include <wx/event.h>

#include "gpsdraw.h"
#include "geocoordinate.h"
#include "dbconnection.h"
#include "tilecache.h"
#include "util/pubsub/asynchronousproxy.h"
#include "util/pubsub/classcallsubscriber.h"


using namespace mapgeneration;


namespace mapgeneration_gui
{
	
	class MapDrawer
	{

		public:
		
			MapDrawer
				(wxScrolledWindow* map_scrolled_window, GPSDraw* gps_draw,
					TileCache* tile_cache);


			void
			on_paint(wxPaintEvent& wx_paint_event, wxDC& dc);
			
			
			void
			set_tile_cache(TileCache* tile_cache);


			void
			tile_prefetched(unsigned int id);
			

		private:			
			
			GPSDraw* _gps_draw;
			
			
			int
			_current_draw_id;
						

			time_t
			_last_paint_seconds;

			
			wxScrolledWindow*
			_map_scrolled_window;
			
			
			TileCache* 
			_tile_cache;
						
			
			pubsub::ClassCallSubscriber<MapDrawer, unsigned int> 
			_tile_prefetch_caller;
			
			
			pubsub::AsynchronousProxy<unsigned int>
			_tile_prefetch_proxy;
			
			
			std::vector< std::bitset<360> >
			_used_tile_blocks_50;
			
			
			std::vector< std::bitset<3600> >
			_used_tile_blocks_5;
			
			
			bool
			_use_prefetch;
			
			
			void
			draw_tile(unsigned int tile_id,
				unsigned int min_tile_id_northing, unsigned int min_tile_id_easting, 
				unsigned int max_tile_id_northing, unsigned int max_tile_id_easting);
			
			
			void
			draw_tiles(
				unsigned int min_tile_id_northing, unsigned int min_tile_id_easting, 
				unsigned int max_tile_id_northing, unsigned int max_tile_id_easting);
			
						
			void
			update_used_tile_blocks();
			
	};
	

} // namespace mapgeneration_gui

#endif //MAPDRAWER_H
