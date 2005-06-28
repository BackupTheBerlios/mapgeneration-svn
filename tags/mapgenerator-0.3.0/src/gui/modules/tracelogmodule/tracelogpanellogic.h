/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef TRACELOGPANELLOGIC_H
#define TRACELOGPANELLOGIC_H

#include <bitset>
#include <ctime>
#include <vector>
#include <wx/wx.h>

#include "tracelog.h"
#include "gui/util/mapscrolledwindowlogic.h"


using namespace mapgeneration;


namespace mapgeneration_gui
{
	
	class TraceLogPanelLogic : public MapScrolledWindowLogic
	{

		public:
		
			TraceLogPanelLogic
				(wxScrolledWindow* map_scrolled_window);
			
			
			void
			on_backward_button(wxEvent& event);
		
			
			void
			on_forward_button(wxEvent& event);
					
			
			void
			on_load(wxEvent& event);


			void
			on_paint(wxPaintEvent& wx_paint_event);
						
			
		private:
		
			TraceLog _trace_log;
			
	};
	

} // namespace mapgeneration_gui

#endif //MAPPANELLOGIC_H
