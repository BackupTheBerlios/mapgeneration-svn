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
#include "edgecache.h"
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
					EdgeCache* edge_cache, TileCache* tile_cache);
					

			void
			edge_prefetched(unsigned int id);


			void
			on_paint(wxPaintEvent& wx_paint_event, wxDC& dc);
			
			
			void
			set_edge_cache(EdgeCache* edge_cache);
			
			
			void
			set_tile_cache(TileCache* tile_cache);


			void
			tile_prefetched(unsigned int id);
			

		private:
					
			EdgeCache*
			_edge_cache;
			
			
			pubsub::ClassCallSubscriber<MapDrawer, unsigned int> 
			_edge_prefetch_caller;
			
			
			pubsub::AsynchronousProxy<unsigned int>
			_edge_prefetch_proxy;
			
			
			GPSDraw* _gps_draw;
						

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
			

			std::vector< Node >
			construct_edge_vector(const Edge& edge);
			
						
			void
			update_used_tile_blocks();
			
	};
	

} // namespace mapgeneration_gui

#endif //MAPDRAWER_H
