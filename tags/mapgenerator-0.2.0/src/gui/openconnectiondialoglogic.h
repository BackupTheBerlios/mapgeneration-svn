/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef OPENCONNECTIONDIALOGLOGIC_H
#define OPENCONNECTIONDIALOGLOGIC_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "openconnectiondialoglogic.cpp"
#endif

#include <wx/event.h>
#include <wx/listbox.h>
#include <wx/string.h>

#include "dialoglogic.h"

namespace mapgeneration_gui
{
	
	class OpenConnectionDialogLogic : public DialogLogic
	{
		
		DECLARE_DYNAMIC_CLASS(OpenConnectionDialogLogic);
		DECLARE_EVENT_TABLE();
		
		public:
			
			OpenConnectionDialogLogic();
			

			const wxString&
			current_host() const;

			
			const wxString&
			current_port() const;
			
			
		protected:
			
			bool
			intern_load_dialog();
			
			
		private:
			
			wxString _current_host;


			wxString _current_port;


			wxListBox* _listbox_savedsessions;


			wxArrayString _saved_sessions; /** @todo change to std::vector!!! */

			
			void
			on_button_cancel_click(wxCommandEvent& event);

			
			void
			on_button_connect_click(wxCommandEvent& event);
			

			void
			on_button_delete_click(wxCommandEvent& event);
			

			void
			on_button_load_click(wxCommandEvent& event);
			

			void
			on_button_save_click(wxCommandEvent& event);
			

			void
			on_close_window(wxCloseEvent& event);
			

			void
			on_listbox_savedsessions_double_clicked(wxCommandEvent& event);
			

			void
			on_listbox_savedsessions_selected(wxCommandEvent& event);
			
	};
	
} // namespace mapgeneration_gui

#endif // OPENCONNECTIONDIALOGLOGIC_H
