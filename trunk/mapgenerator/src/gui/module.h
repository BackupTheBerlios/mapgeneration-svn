/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef MODULE_H
#define MODULE_H

#include <wx/icon.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/string.h>
#include <wx/treectrl.h>

#include "mainframelogic.h"

namespace mapgeneration_gui
{
		
	class Module
	{
		
		friend bool MainFrameLogic::register_module(Module* module);
		friend bool MainFrameLogic::unregister_module(Module* module);
		friend void MainFrameLogic::on_notebook_page_changed (wxNotebookEvent& event);
		
		public:
			
			typedef std::vector<
					std::pair<void (wxEvtHandler::*)(wxCommandEvent&), wxEvtHandler*>
				> D_IconsEventHandlersVector;
			
			
			static const size_t INVALID_NOTEBOOK_ID = 1000000; /** @todo works that?! */


			static const int INVALID_TOOLBAR_ID = -1;
			

			Module();
			
			
			Module(const wxString& label);
			

			~Module();
			
			
			wxPanel*
			content_panel();
			
			
			std::vector<wxIcon*>&
			icons();
			
			
			D_IconsEventHandlersVector&
			icons_event_handlers();
			
			
			wxString&
			label();
			

			bool
			load_content_panel(wxWindow* parent);
			
			
			bool
			load_preferences_tree_item(pubsub::ServiceList* service_list);
			
			
			bool
			operator==(const Module& module);
			
			
			wxTreeCtrl*
			preferences_tree_item();
			
		protected:
		
			wxPanel* _content_panel;
			
			
			std::vector<wxIcon*> _icons;
			
			
			D_IconsEventHandlersVector _icons_event_handlers;
			
			
			wxString _label;
			
			
			wxTreeCtrl* _preferences_tree_item;
			
			
			pubsub::ServiceList* _service_list;
			
			
			virtual bool
			intern_load_content_panel();
			
			
			virtual bool
			intern_load_preferences_tree_item();
			
			
			bool
			connect_component_to_service
				(wxWindow* component, pubsub::GenericService* service);
			
			
		private:
			
			
			bool _content_panel_loaded;
			
			
			bool _gave_away_content_panel;
			
			
			bool _gave_away_preferences_tree_item;
			
			
			size_t _notebook_id;
			
			
			bool _preferences_tree_item_loaded;


			std::vector<int> _toolbar_ids;
			
			
			size_t
			get_notebook_id();
			
			
			std::vector<int>&
			toolbar_ids();
			
			
			void
			set_notebook_id(size_t id);
			
	};
	
} // namespace mapgeneration_util

#endif //MODULE_H
