/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef MAINFRAMELOGIC_H
#define MAINFRAMELOGIC_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "mainframelogic.cpp"
#endif

namespace mapgeneration_gui
{
	class Module;
}

/** @todo think about a better system! */
#define MIN_ID 1000000
#define MAX_ID 1000999

#define _CONFIGURATION_FILE ("mgg.conf")

#include <wx/event.h>
#include <wx/frame.h>
#include <wx/notebook.h>
#include <wx/string.h>

#include "modulemanager.h"
#include "openconnectiondialoglogic.h"
#include "preferencesdialoglogic.h"
#include "../util/fixedsizequeue.h"
#include "../util/pubsub/servicelist.h"

namespace mapgeneration_gui
{
	
	class MainFrameLogic : public wxFrame
	{
			
		DECLARE_DYNAMIC_CLASS(MainFrameLogic);
		DECLARE_EVENT_TABLE();
		
		public:
			
			MainFrameLogic();
			
			
			~MainFrameLogic();
			
			
			bool
			load_frame();
			
			
/*			ModuleManager&
			module_manager();*/
			
			
			bool
			register_module(Module* module);
			
			
/*			pubsub::ServiceList&
			service_list();*/
			
			
			bool
			show_frame();
			
			
			bool
			unregister_module(Module* module);
			
			
		private:
			
			
			OpenConnectionDialogLogic _dialog_open_connection_logic;
			
			
			PreferencesDialogLogic _dialog_preferences_logic;
			
			
			mapgeneration_util::FixedSizeQueue<wxString> _last_connections;
			
			
			ModuleManager _module_manager;
			
			
			std::vector<Module*> _registered_modules;
			
			
			pubsub::ServiceList _service_list;
			
					
			int _next_id;


			wxNotebook* _notebook;

			
			void
			increment_next_id();

			
			void
			on_exit_click(wxCommandEvent& event);

			
			void
			on_generated_menuitem_lastconnections_click(wxCommandEvent& event);

			
			void
			on_notebook_page_changed(wxNotebookEvent& event);
			
			
			void
			on_openconnection_click(wxCommandEvent& event);

			
			void
			on_preferences_click(wxCommandEvent& event);

			
			void
			reorganize_lastconnections_submenu();

	};
	
} // namespace mapgeneration_gui

#endif // MAINFRAMELOGIC_H
