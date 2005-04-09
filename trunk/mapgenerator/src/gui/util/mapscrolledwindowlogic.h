/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef MAPSCROLLEDWINDOWLOGIC_H
#define MAPSCROLLEDWINDOWLOGIC_H

#include <bitset>
#include <ctime>
#include <vector>
#include <wx/wx.h>

#include "gui/util/gpsdraw.h"
#include "mapdrawer.h"
#include "geocoordinate.h"
#include "dbconnection.h"
#include "tilecache.h"
#include "util/pubsub/asynchronousproxy.h"
#include "util/pubsub/classcallsubscriber.h"


using namespace mapgeneration;


namespace mapgeneration_gui
{
	
	class MapScrolledWindowLogic : public wxEvtHandler
	{

		public:
		
			MapScrolledWindowLogic
				(wxScrolledWindow* map_scrolled_window);

					
			void
			draw_map(wxPaintEvent& wx_paint_event, wxDC& dc);
			
			
			wxPoint
			get_center();
			
						
			void
			on_mouse_event(wxMouseEvent& wx_mouse_event);
			
			
			void
			on_reload_button(wxCommandEvent& event);
			
			
			void
			on_scroll_event(wxScrollEvent& event);			
						
			
			void
			on_zoom_in_button(wxCommandEvent& event);
	
	
			void
			on_zoom_out_button(wxCommandEvent& event);
						
			
			void
			reload();
			
			
			void
			scroll(int offset_x, int offset_y);
			
			
			void
			set_center(wxPoint wx_point);
			
			
			void
			set_tile_cache(TileCache* tile_cache);			
						
			
			void
			update_window_setup();
						
			
			void
			zoom(double factor);
			

		protected:

			GPSDraw _gps_draw;
			
			wxScrolledWindow* _map_scrolled_window;

			TileCache* _tile_cache;


		private:
		
			bool _dragging;
			
			int _dragging_last_x, _dragging_last_y;
			
			MapDrawer _map_drawer;
			
			double _offset_x;
			double _offset_y;
						
			double _zoom_factor;
			
	};
	

} // namespace mapgeneration_gui

#endif //MAPSCROLLEDWINDOWLOGIC_H
