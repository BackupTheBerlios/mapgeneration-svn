/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "preferencesdialoglogic.h"
#endif

#include "preferencesdialoglogic.h"

#include <wx/button.h>
#include <wx/dcclient.h>
#include <wx/sizer.h>
#include <wx/xrc/xmlres.h>
#include <iostream>

#include "eventmacros.h"
#include "../util/mlog.h"
using mapgeneration_util::MLog;

namespace mapgeneration_gui
{

	IMPLEMENT_DYNAMIC_CLASS(PreferencesDialogLogic, DialogLogic)
	
	int PreferencesDialogLogic::_parent_windows_counter = -100;
	std::vector<wxDialog*> PreferencesDialogLogic::_tree_item_parent_windows;
	std::vector<wxDialog*> PreferencesDialogLogic::_tree_item_data_parent_windows;
	
	PreferencesDialogLogic::PreferencesDialogLogic()
	: _trees()
	{
		set_modal(true);
	}
	
	
	void
	PreferencesDialogLogic::add_tree_item(wxTreeCtrl* tree_item)
	{
		if (tree_item == NULL)
		{
			return;
		}
		
		build_tree_recursively(
			tree_item,
			_tree_ctrl,
			tree_item->GetRootItem(),
			_tree_ctrl->GetRootItem()
		);
		
		/** @todo when implementation of build_tree_recursively changes this will
		 * 	not work! */
		_trees.insert(std::make_pair(
			tree_item, _tree_ctrl->GetLastChild(_tree_ctrl->GetRootItem())
		));
		
		
		std::vector<wxDialog*>::iterator find_iter
			= std::find(_tree_item_parent_windows.begin(),
				_tree_item_parent_windows.end(), tree_item->GetParent());
		if (find_iter != _tree_item_parent_windows.end())
		{
			if ((*find_iter)->Destroy())
			{
//				mlog(MLog::debug, "PreferencesDialog::add_tree_item")
//					<< "tree_item_parent (a dialog) destroyed!\n";
				tree_item = NULL;
			} else
			{
				mlog(MLog::error, "PreferencesDialog::add_tree_item")
					<< "tree_item_parent (a dialog) NOT destroyed!\n";
			}
				
			_tree_item_parent_windows.erase(find_iter);
		}
	}
	
	
	wxTreeItemData*
	PreferencesDialogLogic::build_tree_item_data_from_window(wxWindow* window)
	{
		return new TreeItemDataWithWindow::TreeItemDataWithWindow(window);
	}
	
	
	void
	PreferencesDialogLogic::build_tree_recursively(
		wxTreeCtrl* input_tree,
		wxTreeCtrl* output_tree,
		const wxTreeItemId& input_tree_current_item_id,
		const wxTreeItemId& output_tree_current_parent_id
	)
	{
		/** @todo references?! And how to do? */
		wxTreeItemId current_sibling = input_tree_current_item_id;
		while (current_sibling.IsOk())
		{
			wxTreeItemId new_item_id = output_tree->AppendItem(
				output_tree_current_parent_id,
				input_tree->GetItemText(current_sibling),
				-1,
				-1,
				input_tree->GetItemData(current_sibling)
			);
			input_tree->SetItemData(current_sibling, NULL);
			
			if (output_tree->GetItemData(new_item_id) != NULL)
			{
				TreeItemDataWithWindow* data
					= dynamic_cast<TreeItemDataWithWindow*>(output_tree->GetItemData(new_item_id));
				if (data != NULL)
				{
					/* This search MUST stand BEFORE changing the parent! */
					std::vector<wxDialog*>::iterator find_iter
						= std::find(_tree_item_data_parent_windows.begin(),
							_tree_item_data_parent_windows.end(), data->_window->GetParent());

					/** @todo does this work everywhere? The docu says NO. */
					data->_window->Reparent(_content_panel);
					_content_panel->GetSizer()->Add(data->_window, 1, wxGROW);
					_content_panel->GetSizer()->Show(data->_window, false);

					if (find_iter != _tree_item_data_parent_windows.end())
					{
						if((*find_iter)->Destroy() == false)
						{
							mlog(MLog::error, "PreferencesDialog::build_tree_recursively")
								<< "tree_item_data_parent (a dialog) NOT destroyed!\n";
						} else
						{
//							mlog(MLog::debug, "PreferencesDialog::build_tree_recursively")
//								<< "tree_item_data_parent (a dialog) destroyed!\n";
						}
						_tree_item_data_parent_windows.erase(find_iter);
					}

				} else
				{
					mlog(MLog::warning, "PreferencesDialogLogic")
						<< "Cannot dynamic cast wxTreeItemData!\n";
				}
			}
			
			if (input_tree->ItemHasChildren(current_sibling))
			{
				wxTreeItemIdValue cookie;
				build_tree_recursively(
					input_tree,
					output_tree,
					input_tree->GetFirstChild(current_sibling, cookie),
					new_item_id
				);
			}
			
			current_sibling = input_tree->GetNextSibling(current_sibling);
		}
	}
	
	
	bool
	PreferencesDialogLogic::intern_load_dialog()
	{
		if (wxXmlResource::Get()->
			LoadDialog(this, GetParent(), wxT("ID_DIALOG_PREFERENCES")) == false)
		{
			return false;
		}
		
		_content_panel = XRCCTRL(*this, "ID_PANEL_CONTENT", wxPanel);
		
		_tree_ctrl = XRCCTRL(*this, "ID_TREECTRL", wxTreeCtrl);
		_tree_ctrl->AddRoot(wxT("MGG"));
		
		wxPanel* general_content_panel = wxXmlResource::Get()->
			LoadPanel(tree_item_data_parent(), wxT("ID_PANEL_PREFERENCES_GENERAL"));
			
		wxTreeCtrl* general_tree_item = new_tree_item();
		wxTreeItemId root_id = general_tree_item->
			AddRoot(wxT("General"), -1, -1,
				build_tree_item_data_from_window(general_content_panel)
			);
		general_tree_item->AppendItem(root_id, wxT("other things"));
		
		add_tree_item(general_tree_item);
		
		MGG_EVT_TREE_SEL_CHANGED(_tree_ctrl,
			PreferencesDialogLogic::on_tree_ctrl_selection_changed, this);
	}
	
	
	wxTreeCtrl*
	PreferencesDialogLogic::new_tree_item()
	{
		wxDialog* dialog = new wxDialog(NULL, _parent_windows_counter, wxT("tree_item_parent"));
		--_parent_windows_counter;
		_tree_item_parent_windows.push_back(dialog);
		
		return new wxTreeCtrl(dialog, -1);
	}

	
	void
	PreferencesDialogLogic::on_tree_ctrl_selection_changed(wxTreeEvent& event)
	{
		TreeItemDataWithWindow* data
			= dynamic_cast<TreeItemDataWithWindow*>(_tree_ctrl->GetItemData(event.GetOldItem()));
		if (data != NULL)
		{
			_content_panel->GetSizer()->Show(data->_window, false);
		}
		
		data = dynamic_cast<TreeItemDataWithWindow*>(_tree_ctrl->GetItemData(event.GetItem()));
		if (data != NULL)
		{
			_content_panel->GetSizer()->Show(data->_window, true);
			_content_panel->GetSizer()->Layout();
		}
	}
	
	
	void
	PreferencesDialogLogic::remove_tree_item(wxTreeCtrl* tree_item)
	{
		std::map<wxTreeCtrl*, wxTreeItemId>::iterator find_iter
			= _trees.find(tree_item);
		if (find_iter != _trees.end())
		{
			_tree_ctrl->Delete(find_iter->second);
			_trees.erase(find_iter);
		}
	}
	
	
	wxWindow*
	PreferencesDialogLogic::tree_item_data_parent()
	{
		wxDialog* dialog = new wxDialog(NULL, _parent_windows_counter, wxT("content_parent"));
		--_parent_windows_counter;
		_tree_item_data_parent_windows.push_back(dialog);
		
		return dialog;
	}
	

	/* Implementation of TreeItemDataWithPanel follows */
	
	PreferencesDialogLogic::TreeItemDataWithWindow::TreeItemDataWithWindow(wxWindow* window)
	: _window(window)
	{
	}
	
} // namespace mapgeneration_gui
