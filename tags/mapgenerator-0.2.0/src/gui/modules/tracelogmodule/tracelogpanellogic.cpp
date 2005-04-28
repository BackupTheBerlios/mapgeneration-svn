/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "tracelogpanellogic.h"


#include <fstream>
#include <set>


namespace mapgeneration_gui
{
	
	TraceLogPanelLogic::TraceLogPanelLogic
		(wxScrolledWindow* map_scrolled_window)
	: MapScrolledWindowLogic::MapScrolledWindowLogic(map_scrolled_window)
	{
	}
	
	
	void
	TraceLogPanelLogic::on_backward_button(wxEvent& event)
	{
	}
		
			
	void
	TraceLogPanelLogic::on_forward_button(wxEvent& event)
	{
		std::cout << "Step forward!\n";
		_trace_log.step_forward(1);
	}
	
	
	
	void
	TraceLogPanelLogic::on_load(wxEvent& event)
	{
		wxFileDialog wx_file_dialog(_map_scrolled_window, wxT("Choose a file"), 
			"", "", "*", wxOPEN | wxFILE_MUST_EXIST, wxDefaultPosition);
		
		int dialog_result = wx_file_dialog.ShowModal();
		
		if (dialog_result != wxID_OK) return;
		
		std::ifstream _file_stream(wx_file_dialog.GetDirectory()+"/"+
			wx_file_dialog.GetFilename(), std::ifstream::in | 
			std::ifstream::binary);
			
		if (_file_stream.good())
		{
			_trace_log.load(_file_stream);
			set_tile_cache(_trace_log.tile_cache());
			_map_scrolled_window->Refresh();
		} else
		{
			wxMessageDialog(_map_scrolled_window, wxT("Could not open file!"),
			"Error!", wxOK, wxDefaultPosition).ShowModal();
			
		}
		
		_file_stream.close();
	}

	
	void
	TraceLogPanelLogic::on_paint(wxPaintEvent& wx_paint_event)
	{		
		wxPaintDC dc(_map_scrolled_window);
		_map_scrolled_window->PrepareDC(dc);
		
		draw_map(wx_paint_event, dc);
	}

} // namespace mapgeneration_gui
