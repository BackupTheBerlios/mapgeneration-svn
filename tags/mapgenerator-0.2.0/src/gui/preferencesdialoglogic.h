/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef PREFERENCESDIALOGLOGIC_H
#define PREFERENCESDIALOGLOGIC_H

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "preferencesdialoglogic.cpp"
#endif

#include <map>
#include <vector>
#include <utility>
#include <wx/splitter.h>
#include <wx/treectrl.h>

#include "dialoglogic.h"

namespace mapgeneration_gui
{
	
	class PreferencesDialogLogic : public DialogLogic
	{
		
		DECLARE_DYNAMIC_CLASS(PreferencesDialogLogic);
		
		
		public:
			
			PreferencesDialogLogic();
			
			
			void
			add_tree_item(wxTreeCtrl* tree_item);
			
			
			static wxTreeItemData*
			build_tree_item_data_from_window(wxWindow* window);
			
			
			static wxTreeCtrl*
			new_tree_item();


			void
			remove_tree_item(wxTreeCtrl* tree_item);
			
			
			static wxWindow*
			tree_item_data_parent();
			
			
		protected:
			
			bool
			intern_load_dialog();
			
			
		private:
			
			wxPanel* _content_panel;
			
			
			static int _parent_windows_counter;
			
			
			std::map<wxTreeCtrl*, wxTreeItemId> _trees;
			
			
			wxTreeCtrl* _tree_ctrl;

			
			static std::vector<wxDialog*> _tree_item_data_parent_windows;
			
			
			static std::vector<wxDialog*> _tree_item_parent_windows;

			
			void
			build_tree_recursively(
				wxTreeCtrl* input_tree,
				wxTreeCtrl* output_tree,
				const wxTreeItemId& input_tree_current_item_id,
				const wxTreeItemId& output_tree_current_parent_id
			);
			
			
			void
			on_tree_ctrl_selection_changed(wxTreeEvent& event);
			
			
			class TreeItemDataWithWindow : public wxTreeItemData
			{
				
				public:
					
					TreeItemDataWithWindow(wxWindow* window);
					
					
					wxWindow* _window;
					
			};
						
	};
	
} // namespace mapgeneration_gui

#endif // PREFERENCESDIALOGLOGIC_H
