/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "mapmodule.h"

#include "gui/eventmacros.h"

#include <wx/xrc/xmlres.h>


namespace mapgeneration_gui
{
	
	MapModule::MapModule()
	: _map_panel_logic(0)
	{
	}
	
	
	MapModule::~MapModule()
	{
		delete _map_panel_logic;
	}
	
	
	bool
	MapModule::intern_load_content_panel()
	{
		if(wxXmlResource::Get()->LoadPanel(_content_panel, 
			_content_panel->GetParent(), wxT("map_viewer_panel")) == false)
		{
			mlog(MLog::error, "MapModule") << "Could not load MapPanel!\n";
			return false;
		}
		
		label() = wxT("MapModule");
				
		wxScrolledWindow* map_panel = XRCCTRL(*_content_panel, "map_panel",
			wxScrolledWindow);
		
		_map_panel_logic = new MapPanelLogic(map_panel, _service_list);

		MGG_XRC_EVT_PAINT(_content_panel, "map_panel",
			MapPanelLogic::on_paint, _map_panel_logic);
		
		MGG_XRC_EVT_BUTTON(_content_panel, "zoom_in_button",
			MapPanelLogic::on_zoom_in_button, _map_panel_logic);
			
		MGG_XRC_EVT_BUTTON(_content_panel, "zoom_out_button",
			MapPanelLogic::on_zoom_out_button, _map_panel_logic);
		
		MGG_XRC_EVT_BUTTON(_content_panel, "reload_button",
			MapPanelLogic::on_reload_button, _map_panel_logic);
		
		MGG_XRC_EVT(wxEVT_LEFT_DOWN, _content_panel, "map_panel",
			MapPanelLogic::on_mouse_event, _map_panel_logic);
		MGG_XRC_EVT(wxEVT_LEFT_UP, _content_panel, "map_panel",
			MapPanelLogic::on_mouse_event, _map_panel_logic);
		MGG_XRC_EVT(wxEVT_MOTION, _content_panel, "map_panel",
			MapPanelLogic::on_mouse_event, _map_panel_logic);
		MGG_XRC_EVT(wxEVT_LEAVE_WINDOW, _content_panel, "map_panel",
			MapPanelLogic::on_mouse_event, _map_panel_logic);				
		MGG_XRC_EVT(wxEVT_MOUSEWHEEL, _content_panel, "map_panel",
			MapPanelLogic::on_mouse_event, _map_panel_logic);
		MGG_XRC_EVT(wxEVT_LEFT_DCLICK, _content_panel, "map_panel",
			MapPanelLogic::on_mouse_event, _map_panel_logic);
			
		MGG_XRC_EVT(wxEVT_SCROLL_THUMBTRACK, _content_panel, "map_panel",
			MapPanelLogic::on_scroll_event, _map_panel_logic);
		MGG_XRC_EVT(wxEVT_SCROLL_THUMBRELEASE, _content_panel, "map_panel",
			MapPanelLogic::on_scroll_event, _map_panel_logic);
			
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
	MapModule::intern_load_preferences_tree_item()
	{
		return false;
	}
	
} // namespace mapgeneration_util

