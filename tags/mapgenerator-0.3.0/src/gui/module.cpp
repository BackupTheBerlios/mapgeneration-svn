/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "module.h"

#include "extendedgenericvalidator.h"
#include "../util/mlog.h"
using mapgeneration_util::MLog;

namespace mapgeneration_gui
{
	
	Module::Module()
	:	_content_panel(NULL),
		_content_panel_loaded(false),
		_gave_away_content_panel(false),
		_gave_away_preferences_tree_item(false),
		_icons(),
		_icons_event_handlers(),
		_label(),
		_notebook_id(INVALID_NOTEBOOK_ID),
		_preferences_tree_item(NULL),
		_preferences_tree_item_loaded(false),
		_service_list(0),
		_toolbar_ids()
	{
		_content_panel = new wxPanel;
	}
	
	
	Module::Module(const wxString& label)
	: _content_panel(NULL),
		_content_panel_loaded(false),
		_gave_away_content_panel(false),
		_gave_away_preferences_tree_item(false),
		_icons(),
		_icons_event_handlers(),
		_label(label),
		_notebook_id(INVALID_NOTEBOOK_ID),
		_preferences_tree_item(NULL),
		_preferences_tree_item_loaded(false),
		_service_list(0),
		_toolbar_ids()
	{
		_content_panel = new wxPanel;
	}
	
	
	Module::~Module()
	{
		/** @todo which object must be deleted????? */
		
		if (_content_panel != NULL && !_gave_away_content_panel)
			delete _content_panel;
		
		if (_preferences_tree_item != NULL && !_gave_away_preferences_tree_item)
			delete _preferences_tree_item;
	}
	
	
	bool
	Module::connect_component_to_service(wxWindow* component, pubsub::GenericService* service)
	{
		if (_service_list == NULL)
		{
			mlog(MLog::notice, "Module")
				<< "No service list set. Cannot connect components.\n";
			
			return false;
		}
		
		component->SetValidator(ExtendedGenericValidator(service));
		
		return true;
	}
	
	
	wxPanel*
	Module::content_panel()
	{
		_gave_away_content_panel = true;
		return _content_panel;
	}
	
	
	size_t
	Module::get_notebook_id()
	{
		return _notebook_id;
	}
	
	
	std::vector<int>&
	Module::toolbar_ids()
	{
		return _toolbar_ids;
	}


	std::vector<wxIcon*>&
	Module::icons()
	{
		return _icons;
	}
	
	
	Module::D_IconsEventHandlersVector&
	Module::icons_event_handlers()
	{
		return _icons_event_handlers;
	}
	
	
	bool
	Module::intern_load_content_panel()
	{
		return false;
	}
	
	
	bool
	Module::intern_load_preferences_tree_item()
	{
		return false;
	}
	
	
	wxString&
	Module::label()
	{
		return _label;
	}
	
	
	bool
	Module::load_content_panel(wxWindow* parent)
	{
		if (parent != NULL)
		{
			if (!_content_panel_loaded)
			{
				_content_panel->SetParent(parent);
				bool success = intern_load_content_panel();
				if (success)
				{
					_content_panel_loaded = true;
				}
				
				return success;
				
			} else
			{
				return true;
			}
		}
		
		return false;
	}


	bool
	Module::load_preferences_tree_item(pubsub::ServiceList* service_list)
	{
		if (service_list != NULL)
		{
			if (!_preferences_tree_item_loaded)
			{
				_service_list = service_list;
				bool success = intern_load_preferences_tree_item();
				if (success)
				{
					_preferences_tree_item_loaded = true;
				}
				
				return success;
				
			} else
			{
				return true;
			}
		}
		
		return false;
	}
	
	
	bool
	Module::operator==(const Module& module)
	{
		/** @todo fill in every member variable!!! */
		
		return (
			(_content_panel == module._content_panel) &&
//			(_icons == module._icons) &&
//			(_icons_event_handlers == module._icons_event_handlers) &&
			(_label == module._label) //&&
//			(_preferences_tree_item == module._preferences_tree_item) &&
//			(_notebook_id == module._notebook_id) &&
//			(_toolbar_ids == module._toolbar_ids)
		);
	}
	
	wxTreeCtrl*
	Module::preferences_tree_item()
	{
		_gave_away_preferences_tree_item = true;
		return _preferences_tree_item;
	}
		
	
	void
	Module::set_notebook_id(size_t id)
	{
		_notebook_id = id;
	}
	
} // namespace mapgeneration_gui
