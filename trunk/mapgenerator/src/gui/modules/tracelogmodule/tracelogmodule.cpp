/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "tracelogmodule.h"

#include "gui/eventmacros.h"

#include <wx/xrc/xmlres.h>


namespace mapgeneration_gui
{
	
	TraceLogModule::TraceLogModule()
	{			
	}
	
	
	TraceLogModule::~TraceLogModule()
	{
		delete _trace_log_panel_logic;
	}
	
	
	bool
	TraceLogModule::intern_load_content_panel()
	{
		if(wxXmlResource::Get()->LoadPanel(_content_panel, 
			_content_panel->GetParent(), wxT("trace_log_viewer_panel")) == false)
		{
			mlog(MLog::error, "TraceLogModule") << "Could not load TraceLogPanel!\n";
			return false;
		}
		
		label() = "TraceLogModule";
				
		wxScrolledWindow* map_panel = XRCCTRL(*_content_panel, "trace_log_panel",
			wxScrolledWindow);
		
		_trace_log_panel_logic = new TraceLogPanelLogic(map_panel);

		MGG_XRC_EVT_PAINT(_content_panel, "trace_log_panel",
			TraceLogPanelLogic::on_paint, _trace_log_panel_logic);
		
		MGG_XRC_EVT_BUTTON(_content_panel, "zoom_in_button",
			TraceLogPanelLogic::on_zoom_in_button, _trace_log_panel_logic);
		MGG_XRC_EVT_BUTTON(_content_panel, "zoom_out_button",
			TraceLogPanelLogic::on_zoom_out_button, _trace_log_panel_logic);
		MGG_XRC_EVT(wxEVT_LEFT_DOWN, _content_panel, "trace_log_panel",
			TraceLogPanelLogic::on_mouse_event, _trace_log_panel_logic);
		MGG_XRC_EVT(wxEVT_LEFT_UP, _content_panel, "trace_log_panel",
			TraceLogPanelLogic::on_mouse_event, _trace_log_panel_logic);
		MGG_XRC_EVT(wxEVT_MOTION, _content_panel, "trace_log_panel",
			TraceLogPanelLogic::on_mouse_event, _trace_log_panel_logic);
		MGG_XRC_EVT(wxEVT_LEAVE_WINDOW, _content_panel, "trace_log_panel",
			TraceLogPanelLogic::on_mouse_event, _trace_log_panel_logic);				
		MGG_XRC_EVT(wxEVT_MOUSEWHEEL, _content_panel, "trace_log_panel",
			TraceLogPanelLogic::on_mouse_event, _trace_log_panel_logic);
		MGG_XRC_EVT(wxEVT_LEFT_DCLICK, _content_panel, "trace_log_panel",
			TraceLogPanelLogic::on_mouse_event, _trace_log_panel_logic);
		
		MGG_XRC_EVT_BUTTON(_content_panel, "load_button",
			TraceLogPanelLogic::on_load, _trace_log_panel_logic);
			
		MGG_XRC_EVT_BUTTON(_content_panel, "backward_button",
			TraceLogPanelLogic::on_backward_button, _trace_log_panel_logic);
		MGG_XRC_EVT_BUTTON(_content_panel, "forward_button",
			TraceLogPanelLogic::on_forward_button, _trace_log_panel_logic);
			
		/*Mouse events:
		 * DECLARE_EVENT_TYPE(wxEVT_LEFT_DOWN, 100)
		 * DECLARE_EVENT_TYPE(wxEVT_LEFT_UP, 101)
		 * DECLARE_EVENT_TYPE(wxEVT_MIDDLE_DOWN, 102)
		 * DECLARE_EVENT_TYPE(wxEVT_MIDDLE_UP, 103)
		 * DECLARE_EVENT_TYPE(wxEVT_RIGHT_DOWN, 104)
		 * DECLARE_EVENT_TYPE(wxEVT_RIGHT_UP, 105)
		 * DECLARE_EVENT_TYPE(wxEVT_MOTION, 106)
		 * DECLARE_EVENT_TYPE(wxEVT_ENTER_WINDOW, 107)
		 * DECLARE_EVENT_TYPE(wxEVT_LEAVE_WINDOW, 108)
		 * DECLARE_EVENT_TYPE(wxEVT_LEFT_DCLICK, 109)
		 * DECLARE_EVENT_TYPE(wxEVT_MIDDLE_DCLICK, 110)
		 * DECLARE_EVENT_TYPE(wxEVT_RIGHT_DCLICK, 111)
		 * DECLARE_EVENT_TYPE(wxEVT_SET_FOCUS, 112)
		 * DECLARE_EVENT_TYPE(wxEVT_KILL_FOCUS, 113)
		 * DECLARE_EVENT_TYPE(wxEVT_CHILD_FOCUS, 114)
		 * DECLARE_EVENT_TYPE(wxEVT_MOUSEWHEEL, 115)*/

		return true;
	}
	
	
	bool
	TraceLogModule::intern_load_preferences_tree_item()
	{
		return false;
	}
	
} // namespace mapgeneration_util

