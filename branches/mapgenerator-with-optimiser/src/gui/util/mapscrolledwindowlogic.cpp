/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "mapscrolledwindowlogic.h"


#include <iostream>
#include <set>

#include <wx/xrc/xmlres.h>

#include "mapgenerationdraw.h"



namespace mapgeneration_gui
{
	
	MapScrolledWindowLogic::MapScrolledWindowLogic
		(wxScrolledWindow* map_scrolled_window)
	:	_dragging(false), 
		_gps_draw(),
		_map_drawer(map_scrolled_window, &_gps_draw, 0),
		_map_scrolled_window(map_scrolled_window),
		_mouse_position_x(0),
		_mouse_position_y(0),
		_node_search_distance(30.0),
		_offset_x(0), 
		_offset_y(0), 
		_tile_cache(0),
		_zoom_factor(0.0001)
	{
		_map_scrolled_window->SetScrollRate(1, 1);
		_map_scrolled_window->EnableScrolling(true, true);
		
		update_window_setup();
		
		_map_drawer.controlled_start(false);
	}
	
	
	void
	MapScrolledWindowLogic::draw_map(wxPaintEvent& wx_paint_event, wxDC& dc)
	{
		dc.SetBackground(*wxWHITE_BRUSH);
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.Clear();
		_gps_draw.set(&dc, _offset_x, _offset_y, _zoom_factor);
		
		int pixels_per_unit_x, pixels_per_unit_y;
		_map_scrolled_window->GetScrollPixelsPerUnit(&pixels_per_unit_x, &pixels_per_unit_y);
		
		int view_start_x, view_start_y;
		_map_scrolled_window->GetViewStart(&view_start_x, &view_start_y);
		view_start_x *= pixels_per_unit_x;
		view_start_y *= pixels_per_unit_y;
		
		int client_size_x, client_size_y;
		_map_scrolled_window->GetClientSize(&client_size_x, &client_size_y);
		int view_end_x = view_start_x + client_size_x;
		int view_end_y = view_start_y + client_size_y;
		
		/*std::cout << "Viewmin: " << view_start_x << ", " << view_start_y << "\nViewmax:"
			<< view_end_x << ", " << view_end_y << "\n";*/
		
		_map_drawer.on_paint(wx_paint_event, dc);
		
		// draw _zoom_factor
		wxColour saved_text_foreground = dc.GetTextForeground();
		dc.SetTextForeground(wxColor(150, 150, 150));
		wxFont saved_font = dc.GetFont();
		dc.SetFont(wxFont(12, wxMODERN, wxNORMAL, wxNORMAL));
		wxPen saved_pen = dc.GetPen();
		dc.SetPen(wxPen(*wxBLACK, 2));
		
		wxString stringed_zoom_factor;
		stringed_zoom_factor << _zoom_factor;
		dc.DrawText(wxT("zoom:  ") + stringed_zoom_factor, view_start_x, view_start_y);
		wxString stringed_inverse_zoom_factor;
		stringed_inverse_zoom_factor << (1.0 / _zoom_factor);
		dc.DrawText(wxT("scale: 1px = ") + stringed_inverse_zoom_factor + wxT("m"),
			view_start_x, view_start_y + 20);
		
		double meter = 1.0;
		double scale = _zoom_factor;
		
		while (scale < 50.0)
		{
			meter *= 10.0;
			scale *= 10.0;
		}
		
		while (scale > 200.0)
		{
			meter /= 10.0;
			scale /= 10.0;
		}
		
		wxString stringed_meter;
		if (meter < 0.1)
		{
			stringed_meter << wxT(" cm");
			meter *= 100.0;
		} else if (meter > 1000.0)
		{
			stringed_meter << wxT(" km");
			meter /= 1000.0;
		} else
		{
			stringed_meter << wxT(" m");
		}
		
		wxString stringed_scale;
		stringed_scale << meter << stringed_meter;
		dc.DrawText(stringed_scale, view_start_x, view_start_y + 45);
		dc.DrawLine(view_start_x + 80, view_start_y + 52,
			view_start_x + 80, view_start_y + 58);
		dc.DrawLine(view_start_x + 80, view_start_y + 55,
			view_start_x + 80 + static_cast<int>(scale), view_start_y + 55);
		dc.DrawLine(view_start_x + 80 + static_cast<int>(scale), view_start_y + 52,
			view_start_x + 80 + static_cast<int>(scale), view_start_y + 58);
		
		dc.SetTextForeground(saved_text_foreground);
		dc.SetFont(saved_font);
		dc.SetPen(saved_pen);
		
//		if (_zoom_factor > 2.0)
//		{
//			dc.DrawCircle(_mouse_position_x, _mouse_position_y,
//				static_cast<int>(_node_search_distance/* / _zoom_factor*/));
//		}
	}
	
	
	wxPoint
	MapScrolledWindowLogic::get_center()
	{
		int view_start_x, view_start_y;
		_map_scrolled_window->GetViewStart(&view_start_x, &view_start_y);
		int client_size_x, client_size_y;
		_map_scrolled_window->GetClientSize(&client_size_x, &client_size_y);
		int pixels_per_unit_x, pixels_per_unit_y;
		_map_scrolled_window->GetScrollPixelsPerUnit(&pixels_per_unit_x, &pixels_per_unit_y);
		client_size_x /= pixels_per_unit_x;
		client_size_y /= pixels_per_unit_y;
		int view_middle_x = view_start_x + client_size_x / 2;
		int view_middle_y = view_start_y + client_size_y / 2;
		
		return wxPoint(view_middle_x, view_middle_y);
	}
	
	
	void
	MapScrolledWindowLogic::on_mouse_event(wxMouseEvent& wx_mouse_event)
	{
		long x, y;
		wx_mouse_event.GetPosition(&x, &y);
		
		// show node properties
		if (_zoom_factor > 2.0)
		{
			int pixels_per_unit_x, pixels_per_unit_y;
			_map_scrolled_window->GetScrollPixelsPerUnit(
				&pixels_per_unit_x, &pixels_per_unit_y);
			
			int view_start_x, view_start_y;
			_map_scrolled_window->GetViewStart(&view_start_x, &view_start_y);
			view_start_x *= pixels_per_unit_x;
			view_start_y *= pixels_per_unit_y;
				
			double latitude, longitude;
			_gps_draw.reproject(view_start_x + x, view_start_y + y,
				latitude, longitude);
			GeoCoordinate test_gc(latitude, longitude);
			double distance = _node_search_distance / _zoom_factor;
			std::vector<Node::Id> query_result;
			
			Tile::Id tile_id = Tile::get_tile_id_of(test_gc);
			TileCache::Pointer tile = _tile_cache->get(tile_id);
			
			if (tile != 0)
			{
				tile->fast_cluster_nodes_search(test_gc, distance, query_result);
			}
			
			_mouse_position_x = x;
			_mouse_position_y = y;
			if (query_result.size() == 1)
			{
				Tile::Id tile_id;
				Node::LocalId node_local_id;
				Node::split_id(query_result[0], tile_id, node_local_id);
				
				wxULongLong wx_tile_id(0, tile_id);
				wxULongLong wx_node_local_id(0, node_local_id);
				wxULongLong wx_node_id(tile_id, node_local_id);
				
				wxWindow* parent = _map_scrolled_window;
				wxStaticText* node_id_text = 0;
				while ( (node_id_text == 0) && (parent != 0) )
				{
					node_id_text = XRCCTRL(*parent, "node_id_text", wxStaticText);
					
					parent = parent->GetParent();
				}
				
				if (node_id_text != 0)
				{
					wxString label;
					label << wxT(" (") << _mouse_position_x
						<< wxT(", ") << _mouse_position_y << wxT(") ")
						<< wx_tile_id << wxT(", ") << wx_node_local_id
						<< wxT(" -> ") << wx_node_id;
					node_id_text->SetLabel(label);
				}
			} else
			{
				wxWindow* parent = _map_scrolled_window;
				wxStaticText* node_id_text = 0;
				while ( (node_id_text == 0) && (parent != 0) )
				{
					node_id_text = XRCCTRL(*parent, "node_id_text", wxStaticText);
					
					parent = parent->GetParent();
				}
				
				if (node_id_text != 0)
				{
					wxString label;
					label << wxT(" (") << _mouse_position_x
						<< wxT(", ") << _mouse_position_y << wxT(")");
					node_id_text->SetLabel(label);
				}
			}
			
			_mouse_position_x = x;
			_mouse_position_y = y;
			
			_map_scrolled_window->Refresh();
		}
		
		if (wx_mouse_event.LeftDown())
		{
			_dragging_last_x = x;
			_dragging_last_y = y;
			_dragging = true;
		}
		if (wx_mouse_event.Dragging() && wx_mouse_event.LeftIsDown() &&
			_dragging)
		{
			scroll(-(x - _dragging_last_x), -(y - _dragging_last_y));
			_dragging_last_x = x;
			_dragging_last_y = y;
		}
		if (wx_mouse_event.LeftUp() || wx_mouse_event.Leaving())
		{
			_dragging = false;
		}
		if (wx_mouse_event.GetWheelRotation()>0)
			zoom(1.2);
		else if (wx_mouse_event.GetWheelRotation()<0)
			zoom(1.0/1.2);
		if (wx_mouse_event.ButtonDClick(wxMOUSE_BTN_LEFT))
		{
			int view_start_x, view_start_y;
			_map_scrolled_window->GetViewStart(&view_start_x, &view_start_y);
			set_center(wxPoint(view_start_x + (int)x, view_start_y + (int)y));
		}
	}


	void
	MapScrolledWindowLogic::on_reload_button(wxCommandEvent& event)
	{
		reload();
	}
	
	
	void
	MapScrolledWindowLogic::on_scroll_event(wxScrollEvent& event)
	{
		std::cout << "X";
		
	}
	
	
	void
	MapScrolledWindowLogic::on_zoom_in_button(wxCommandEvent& event)
	{
		zoom(10.0);
	}
	
	
	void
	MapScrolledWindowLogic::on_zoom_out_button(wxCommandEvent& event)
	{
		zoom(1.0/10.0);
	}
	
	
	void
	MapScrolledWindowLogic::reload()
	{
		_map_drawer.reload();
		_map_scrolled_window->Refresh();
	}
	
	
	void
	MapScrolledWindowLogic::scroll(int offset_x, int offset_y)
	{
		wxPoint center = get_center();
		set_center(wxPoint(center.x + offset_x, center.y + offset_y));
	}
	
	
	void
	MapScrolledWindowLogic::set_center(wxPoint wx_point)
	{
		int client_size_x, client_size_y;
		_map_scrolled_window->GetClientSize(&client_size_x, &client_size_y);
		int pixels_per_unit_x, pixels_per_unit_y;
		_map_scrolled_window->GetScrollPixelsPerUnit(&pixels_per_unit_x, &pixels_per_unit_y);
		client_size_x /= pixels_per_unit_x;
		client_size_y /= pixels_per_unit_y;
		
		int view_start_x = wx_point.x - client_size_x / 2;
		int view_start_y = wx_point.y - client_size_y / 2;
		_map_scrolled_window->Scroll(view_start_x, view_start_y);
		
		_map_scrolled_window->Refresh();
	}
	
	
	void
	MapScrolledWindowLogic::set_tile_cache(TileCache* tile_cache)
	{
		_tile_cache = tile_cache;
		_map_drawer.set_tile_cache(_tile_cache);
	}
	
	
	void
	MapScrolledWindowLogic::update_window_setup()
	{
		_gps_draw.set(0, _offset_x, _offset_y, _zoom_factor);
		double minx, maxx, miny, maxy, dummy;
		
		_gps_draw.gps_project(0.0, -179.99, minx, dummy);
		_gps_draw.gps_project(0.0, +179.99, maxx, dummy);
		_gps_draw.gps_project(-75.0, 0.0, dummy, miny);
		_gps_draw.gps_project(+75.0, 0.0, dummy, maxy);
		
		std::cout << "Minx: " << minx << " Maxx: " << maxx << " Miny: " << miny << " Maxy: " << maxy << "\n";
		
		_offset_x = -minx;
		_offset_y = -maxy;
		
		double width = maxx - minx;
		double height = maxy - miny;
		
		std::cout << "Width: " << width << " Height: " << height << "\n";
		std::cout << "Width: " << width*_zoom_factor << " Height: " << height*_zoom_factor << "\n";
		
		_map_scrolled_window->SetVirtualSize((int)(width*_zoom_factor), 
			(int)(height*_zoom_factor));
	}
		
	
	void
	MapScrolledWindowLogic::zoom(double factor)
	{			
		wxPoint center = get_center();
		
		_zoom_factor *= factor;
		
		int width, height;
		_map_scrolled_window->GetVirtualSize(&width, &height);
		_map_scrolled_window->SetVirtualSize((int)(width*factor), (int)(height*factor));
		
		set_center(wxPoint((int)(center.x*factor), (int)(center.y*factor)));
		
		_map_scrolled_window->Refresh();
	}	

} // namespace mapgeneration_gui
