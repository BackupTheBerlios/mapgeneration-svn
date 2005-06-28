/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "mainframelogic.h"
#endif

#include "mainframelogic.h"

#include <iostream>
#include <wx/menu.h>
#include <wx/toolbar.h>
#include <wx/xrc/xmlres.h>

#include "eventmacros.h"
#include "module.h"
#include "../util/configuration.h"
#include "../util/mlog.h"
using mapgeneration_util::MLog;

namespace mapgeneration_gui
{
	
	IMPLEMENT_DYNAMIC_CLASS(MainFrameLogic, wxFrame)
	
	BEGIN_EVENT_TABLE(MainFrameLogic, wxFrame)
		EVT_MENU(XRCID("ID_MENUITEM_OPENCONNECTION"),	MainFrameLogic::on_openconnection_click)
		EVT_MENU(XRCID("ID_TOOL_OPENCONNECTION"),	MainFrameLogic::on_openconnection_click)
		
		EVT_MENU(XRCID("ID_MENUITEM_PREFERENCES"), MainFrameLogic::on_preferences_click)
		EVT_MENU(XRCID("ID_TOOL_PREFERENCES"), MainFrameLogic::on_preferences_click)
		
		EVT_MENU(XRCID("ID_MENUITEM_EXIT"), MainFrameLogic::on_exit_click)
//		EVT_MENU(XRCID("ID_TOOL_EXIT"), MainFrameLogic::on_exit_click)
	END_EVENT_TABLE();

	
	MainFrameLogic::MainFrameLogic()
	: _dialog_open_connection_logic(),
		_dialog_preferences_logic(),
		_last_connections(4),
		_module_manager(),
		_registered_modules(),
		_next_id(MIN_ID),
		_notebook(NULL),
		_service_list()
	{
	}
	
	
	MainFrameLogic::~MainFrameLogic()
	{
		std::vector<Module*>::iterator iter = _registered_modules.begin();
		std::vector<Module*>::iterator iter_end = _registered_modules.end();
		for (; iter != iter_end; ++iter)
		{
			unregister_module(*iter);
		}
	}
	
	
	void
	MainFrameLogic::increment_next_id()
	{
		if (_next_id == MAX_ID)
		{
			/** @todo Problem (but minor!). Think about it. */
			mlog(MLog::warning, "MainFrameLogic") << "_next_id == MAX_ID\n";
		} else
		{
			++_next_id;
		}
	}
	
	
	bool
	MainFrameLogic::load_frame()
	{
		wxXmlResource::Get()->LoadFrame(this, GetParent(), wxT("ID_MAINFRAME"));
		
		_dialog_open_connection_logic.load_dialog(this);
		_dialog_preferences_logic.load_dialog(this);
		
		_notebook = XRCCTRL(*this, "ID_NOTEBOOK", wxNotebook);
		
		_notebook->Connect(
			_notebook->GetId(),
			wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,
			(wxObjectEventFunction) (wxEventFunction) (wxNotebookEventFunction)
				(&MainFrameLogic::on_notebook_page_changed),
			NULL,
			this
		);
		
		/** @todo read configuration and register modules */
		mapgeneration_util::Configuration conf(_CONFIGURATION_FILE, &_service_list);
		if (!conf.read_configuration())
		{
			mlog(MLog::warning, "MainFrameLogic") << "Cannot read configration!\n";
			throw("Cannot read configuration!");
		}
		
		std::string licence_accepted;
		if (!_service_list.get_service_value("accepted_licence", 
			licence_accepted) ||
			(licence_accepted != "yes"))
		{
			std::cout << "\nYou did not yet accept the licence.\n"
			          << "This program is licenced under the Academic Free License version 2.1.\n"
			          << "The complete licence text is available in the LICENCE file and at\n"
			          << "http://www.opensource.org/licenses/afl-2.1.php\n"
			          << "\n"
			          << "If you understand and accept the licence terms enter 'y' to continue, \n"
			          << "otherwise press enter to abort program execution.\n"
			          << "If you are the only user you may also change the accepted_licence\n"
			          << "value in the mapgenerator.conf file to 'yes' to accept the licence.\n\n";
			char c = getchar();
			if (c != 'y' && c != 'Y')
			{
				mlog(MLog::info, "MainFrameLogic") << 
					"Licence not accepted.\n";
					throw("Licence not accepted!");
			}
		}

		register_module(_module_manager[0]);
		register_module(_module_manager[1]);
		register_module(_module_manager[2]);	
//		unregister_module(_module_manager[0]);
//		register_module(_module_manager[0]);

		return true;
	}
	
/*	ModuleManager&
	MainFrameLogic::module_manager()
	{
		return _module_manager;
	}*/
	
	
	void
	MainFrameLogic::on_exit_click(wxCommandEvent& event)
	{
		Close();
	}
	
	
	void
	MainFrameLogic::on_generated_menuitem_lastconnections_click(wxCommandEvent& event)
	{
		wxObject* obj = event.GetEventObject();
		wxMenu* item = dynamic_cast<wxMenu*>(obj);
		if (item != 0)
		{
			const wxString& item_label = item->FindItem(event.GetId())->GetLabel();
			std::cout << item_label << std::endl;
			_last_connections.push(item_label);
			reorganize_lastconnections_submenu();
			
			/** @todo init a connection to "item_label" */
		}
	}
	
	void
	MainFrameLogic::on_notebook_page_changed(wxNotebookEvent& event)
	{
		/* Find module for old page and remove tools from toolbar... */
		std::vector<Module*>::iterator find_old_iter = _registered_modules.begin();
		std::vector<Module*>::iterator find_old_iter_end = _registered_modules.end();
		for (; find_old_iter != find_old_iter_end; ++find_old_iter)
		{
			if ((*find_old_iter)->get_notebook_id() == event.GetOldSelection())
				break;
		}
		
		if (find_old_iter != find_old_iter_end)
		{
			if ((*find_old_iter)->icons().size() > 0)
			{
				if ((*find_old_iter)->icons().size() != (*find_old_iter)->icons_event_handlers().size())
				{
					mlog(MLog::warning, "MainFrameLogic")
						<< "Module has different numbers of icons and corresponding event "
						<< "handlers. You really must not change these attributes after "
						<< "registering a module!\n";
				}
				
				if ((*find_old_iter)->toolbar_ids().size() != (*find_old_iter)->icons_event_handlers().size())
				{
					mlog(MLog::error, "MainFrameLogic")
						<< "Module has different numbers of icons event handlers and "
						<< "saved toolbar ids. You really must not change these attributes "
						<< "after registering a module! Consequence: I cannot unregister "
						<< "the event handler. This may cause a fatal error in wx!\n";
				}

				std::vector<int>::iterator ids_iter = (*find_old_iter)->toolbar_ids().begin();
				std::vector<int>::iterator ids_iter_end = (*find_old_iter)->toolbar_ids().end();
				
				Module::D_IconsEventHandlersVector::iterator
					icons_event_handlers_iter = (*find_old_iter)->icons_event_handlers().begin();
				Module::D_IconsEventHandlersVector::iterator
					icons_event_handlers_iter_end = (*find_old_iter)->icons_event_handlers().end();
					
				for (; ids_iter != ids_iter_end; ++ids_iter)
				{
					GetToolBar()->RemoveTool(*ids_iter);
					
					if (icons_event_handlers_iter != icons_event_handlers_iter_end)
					{
						Disconnect(*ids_iter, wxEVT_COMMAND_TOOL_CLICKED,
							(wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction) // only casts!
							(icons_event_handlers_iter->first));
						
						++icons_event_handlers_iter;
					}
				}
				
				(*find_old_iter)->toolbar_ids().clear();
			}
		}
		
		/* Find module for new page and add tools... */
		std::vector<Module*>::iterator find_new_iter = _registered_modules.begin();
		std::vector<Module*>::iterator find_new_iter_end = _registered_modules.end();
		for (; find_new_iter != find_new_iter_end; ++find_new_iter)
		{
			if ((*find_new_iter)->get_notebook_id() == event.GetSelection())
				break;
		}
		
		if (find_new_iter != find_new_iter_end)
		{
			if ((*find_new_iter)->icons().size() > 0)
			{
				if ((*find_new_iter)->icons().size() != (*find_new_iter)->icons_event_handlers().size())
				{
					mlog(MLog::warning, "MainFrameLogic")
						<< "Module has different numbers of icons and corresponding event "
						<< "handlers. Module's icons will not be added to toolbar!\n";
				} else
				{
					std::vector<wxIcon*>::iterator icons_iter = (*find_new_iter)->icons().begin();
					std::vector<wxIcon*>::iterator icons_iter_end = (*find_new_iter)->icons().end();
					
					Module::D_IconsEventHandlersVector::iterator
						icons_event_handlers_iter = (*find_new_iter)->icons_event_handlers().begin();
					
					for (; icons_iter != icons_iter_end; ++icons_iter, ++icons_event_handlers_iter)
					{
						(*find_new_iter)->toolbar_ids().push_back(_next_id);
						increment_next_id();
						
						GetToolBar()->AddTool((*find_new_iter)->toolbar_ids().back(), **icons_iter);
						GetToolBar()->Connect(
							(*find_new_iter)->toolbar_ids().back(),
							wxEVT_COMMAND_TOOL_CLICKED,
							(wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction) // only casts!
							(icons_event_handlers_iter->first),
							NULL,
							icons_event_handlers_iter->second
						);
					}
				}
			}
		}
	}


	void
	MainFrameLogic::on_openconnection_click(wxCommandEvent& event)
	{
		int return_code = _dialog_open_connection_logic.show_dialog();
		if (return_code == wxID_OK)
		{
			_last_connections.push(
				_dialog_open_connection_logic.current_host() + wxT(":") +
				_dialog_open_connection_logic.current_port()
			);
			reorganize_lastconnections_submenu();
			
			/** @todo init a connection */
			std::cout << "OpenConnectionDialog = ok" << std::endl;
		}
	}
	

	void
	MainFrameLogic::on_preferences_click(wxCommandEvent& event)
	{
		int return_code = _dialog_preferences_logic.show_dialog();
		if (return_code == wxID_OK)
		{
			/** @todo apply probably changed preferences */
			std::cout << "PreferencesDialog = ok" << std::endl;
		}
	}


	bool
	MainFrameLogic::register_module(Module* module)
	{
		if (module == 0)
			return false;
		
		bool found_module = false;
		std::vector<Module*>::iterator iter = _registered_modules.begin();
		std::vector<Module*>::iterator iter_end = _registered_modules.end();
		for (; iter != iter_end; ++iter)
		{
			if (*iter != 0 && **iter == *module)
			{
				found_module = true;
				break;
			}
		}
		
		if (found_module)
		{
//			mlog(MLog::debug, "register_module") << "found module\n";
			return false;
		} else
		{
//			mlog(MLog::debug, "register_module") << "did not find module\n";

			_registered_modules.push_back(module);
			
			module->load_preferences_tree_item(&_service_list);
			module->load_content_panel(_notebook);			
			
			_dialog_preferences_logic.add_tree_item(module->preferences_tree_item());
			_notebook->AddPage(module->content_panel(), module->label());
			
			module->set_notebook_id(_notebook->GetPageCount() - 1);
		}

		return true;
	}
	
	
	void
	MainFrameLogic::reorganize_lastconnections_submenu()
	{
		wxMenuBar* menu_bar = GetMenuBar();
		int id = menu_bar->FindMenuItem(wxT("File"), wxT("Last connections"));
		wxMenu* submenu_last_connections = menu_bar->FindItem(id)->GetSubMenu();
		
		while (submenu_last_connections->GetMenuItemCount() > 0)
		{
			wxMenuItem* item = submenu_last_connections->FindItemByPosition(0);
			Disconnect(
				item->GetId(),
				wxEVT_COMMAND_MENU_SELECTED,
				(wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction) // only casts!
				(&MainFrameLogic::on_generated_menuitem_lastconnections_click)
			);
			submenu_last_connections->Destroy(item);
		}
		
		int next_id = wxID_LOWEST - 1;
		mapgeneration_util::FixedSizeQueue<wxString>::const_iterator
			iter = _last_connections.begin();
		mapgeneration_util::FixedSizeQueue<wxString>::const_iterator
			iter_end = _last_connections.end();
		for (; iter != iter_end; ++iter, --next_id)
		{
			submenu_last_connections->Append(next_id, *iter, wxT("Open connection to ") + *iter);
			Connect(
				next_id,
				wxEVT_COMMAND_MENU_SELECTED,
				(wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction) // only casts!
				(&MainFrameLogic::on_generated_menuitem_lastconnections_click)
			);
		}
	}
	
	
/*	pubsub::ServiceList&
	MainFrameLogic::service_list()
	{
		return _service_list;
	}*/
	
	
	bool
	MainFrameLogic::show_frame()
	{
		return Show(true);
	}
	
	bool
	MainFrameLogic::unregister_module(Module* module)
	{
		if (module == 0)
			return false;
		
		bool found_module = false;
		std::vector<Module*>::iterator search_iter = _registered_modules.begin();
		std::vector<Module*>::iterator search_iter_end = _registered_modules.end();
		for (; search_iter != search_iter_end; ++search_iter)
		{
			if (*search_iter != 0 && **search_iter == *module)
			{
				found_module = true;
				break;
			}
		}
		
		if (found_module)
		{
			_registered_modules.erase(search_iter);
			
			/** @todo problems: module._content_panel & _preferences_tree_item
			 * may not be deleted. Test that! (Key words: Module::~Module &
			 * _gave_away_...) */
			_dialog_preferences_logic.remove_tree_item(module->preferences_tree_item());
			_notebook->RemovePage(module->get_notebook_id());
			
			std::vector<Module*>::iterator module_iter = _registered_modules.begin();
			std::vector<Module*>::iterator module_iter_end = _registered_modules.end();
			for (; module_iter != module_iter_end; ++module_iter)
			{
				if ((*module_iter)->get_notebook_id() > module->get_notebook_id())
				{
					(*module_iter)->set_notebook_id((*module_iter)->get_notebook_id() - 1);
				}
			}

/*			if (module->icons().size() > 0)
			{
				if (module->icons().size() != module->icons_event_handlers().size())
				{
					mlog(MLog::warning, "MainFrameLogic")
						<< "Module has different numbers of icons and corresponding event "
						<< "handlers. You really must not change these attributes after "
						<< "registering a module!\n";
				}
				
				if (module->toolbar_ids().size() != module->icons_event_handlers().size())
				{
					mlog(MLog::error, "MainFrameLogic")
						<< "Module has different numbers of icons event handlers and "
						<< "saved toolbar ids. You really must not change these attributes "
						<< "after registering a module! Consequence: I cannot unregister "
						<< "the event handler. This may cause a fatal error in wx!\n";
					
					return false;
					
				} else
				{
					std::vector<int>::iterator ids_iter = module->toolbar_ids().begin();
					std::vector<int>::iterator ids_iter_end = module->toolbar_ids().end();
					
					Module::D_IconsEventHandlersVector::iterator
						icons_event_handlers_iter = module->icons_event_handlers().begin();
						
					for (; ids_iter != ids_iter_end; ++ids_iter, ++icons_event_handlers_iter)
					{
						GetToolBar()->RemoveTool(*ids_iter);
						
						Disconnect(*ids_iter, wxEVT_COMMAND_TOOL_CLICKED,
							(wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction) // only casts!
							(icons_event_handlers_iter->first));
					}
					
					module->toolbar_ids().clear();
				}
			}*/
			
			return true;
		}
		
		return false;
	}
		
} // namespace mapgeneration_gui
