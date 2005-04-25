/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef MAPPANELLOGIC_H
#define MAPPANELLOGIC_H

#include <bitset>
#include <ctime>
#include <vector>
#include <wx/wx.h>

#include "util/pubsub/servicesystem.h"
#include "gui/util/mapscrolledwindowlogic.h"


using namespace mapgeneration;


namespace mapgeneration_gui
{
	
	class MapPanelLogic : public MapScrolledWindowLogic
	{

		public:
		
			MapPanelLogic
				(wxScrolledWindow* map_scrolled_window, 
					pubsub::ServiceList* service_list);
				
			~MapPanelLogic();


			void
			on_paint(wxPaintEvent& wx_paint_event);
						
			
		private:
		
			DBConnection* _db_connection;
						
	};
	

} // namespace mapgeneration_gui

#endif //MAPPANELLOGIC_H
