/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


/* How to write a module. Cont. */

/* IMPORTANT: READ THE DOCUMENTATION COMPLETELY!!! */

#include "testmodule.h"

#include "../eventmacros.h"
#include "../../util/mlog.h"
using mapgeneration_util::MLog;

#include <wx/button.h>

#include <iostream>

namespace mapgeneration_gui
{
	
	/* Needed for event handling. See below. */
	class AnotherEventHandler : public wxEvtHandler
	{
		public:
			AnotherEventHandler() {}
			void on_button_right_click(wxCommandEvent& event);
	};
	
	
	/* The default constructor. Initialize your "things" that are NOT dependent
	 * on _content_panel or _preferences_tree_item.
	 * 
	 * To avoid duplicated code in your constructors use the
	 * "initialize_module"-Method. */
	TestModule::TestModule()
	{
		initialize_module();
	}
	
	
	/* The I-want-some-comfort constructor. See remarks in testmodule.h. */
	TestModule::TestModule(const wxString& label)
	: Module::Module(label)
	{
		initialize_module();
	}
	
	
	void
	TestModule::initialize_module()
	{
		wxIcon* icon = new wxIcon(wxT("images/cut.xpm"), wxBITMAP_TYPE_XPM);
		std::pair<void (wxEvtHandler::*)(wxCommandEvent&), wxEvtHandler*> the_pair;
		the_pair.first = (void (wxEvtHandler::*)(wxCommandEvent&))(&TestModule::on_tool_click);
		the_pair.second = this;
		
		_icons.push_back(icon);
		_icons_event_handlers.push_back(the_pair);

		wxIcon* icon_2 = new wxIcon(wxT("images/cut.xpm"), wxBITMAP_TYPE_XPM);
		std::pair<void (wxEvtHandler::*)(wxCommandEvent&), wxEvtHandler*> the_pair_2;
		the_pair_2.first = (void (wxEvtHandler::*)(wxCommandEvent&))(&TestModule::on_tool_2_click);
		the_pair_2.second = this;
		
		_icons.push_back(icon_2);
		_icons_event_handlers.push_back(the_pair_2);
	}
	
	
	/* This method overwrites the corresponding virtual method of Module and is
	 * called by Module::load_content_panel(wxWindow* parent).
	 * The method is responsable for completing the _content_panel. It should
	 * call LoadPanel from wxXmlResource with apropriate parameters (The caller
	 * garantuees that a parent is set for _content_panel!).
	 * If you want to initialize some other members here is the right place.
	 * Especially for components of the panel. Use XRCCTRL here. */
	bool
	TestModule::intern_load_content_panel()
	{
		/* Loads a panel from the pool of XRC-files (loaded somewhere else) with
		 * the specified identifier string into _content_panel.
		 * Test here for success before proceeding. */
		if(wxXmlResource::Get()->LoadPanel(
			_content_panel, _content_panel->GetParent(), wxT("ID_TESTMODULE")) == false)
		{
			return false;
		}
		
		/* Get a component. You may use that construction to initialize members. */
		wxButton* enable_button = XRCCTRL(*_content_panel, "ID_BUTTON_ENABLE", wxButton);
		
		/* One example should be enough. So following calls are obsolent as of
		 * introduction of the new MGG_XRC_EVT_* macros. See below for details.
		 * 
		wxButton* left_button = XRCCTRL(*_content_panel, "ID_BUTTON_LEFT", wxButton);
		wxButton* right_button = XRCCTRL(*_content_panel, "ID_BUTTON_RIGHT", wxButton);
		 */
		
		/* Register event proccessing methods:
		 * 	MGG_EVT_BUTTON(enable_button, "ID_BUTTON_ENABLE",
		 * 		TestModule::on_button_enable_click, this);
		 * 	MGG_EVT_BUTTON(left_button, "ID_BUTTON_LEFT",
		 * 		TestModule::on_button_left_click, this);
		 * 	MGG_EVT_BUTTON(right_button, "ID_BUTTON_RIGHT",
		 * 		AnotherEventHandler::on_button_right_click, aeh);
		 * 
		 * Syntax:
		 * 1. event_source: (enable_button) [pointer]
		 * 			the component which generates events
		 * 2. event_source_xrc_id_string: ("ID_BUTTON_ENABLE") [const wxString&]
		 * 			the xrc identifier string for the component
		 * 3. function: (TestModule::on_button_enable_click) [class function pointer]
		 * 			the method (not neccessarily in the same class class) which
		 * 			handles the event (parameter: a event of apropriate type,
		 * 			void return value)
		 * 4. function_owner: (this) [pointer]
		 * 			the object which owns the function (do not set it wrong!)
		 * 
		 * Remember last version?
		 * I connected the event to a parent of the event source. Why? I don't know.
		 * It looked so simple but it is unlogical! And it does not work properly.
		 * I think it might work when the component you connect the event to, does
		 * not generate the same event nor the components in between do.
		 * 
		 * So MSG_EVT_BUTTON(wxPanel, wxButtonID, ...) works because a panel cannot
		 * generate a wxEVT_COMMAND_BUTTON_CLICKED event. While
		 * MSD_EVT_PAINT(wxPanel, wxScrolledWindowID, ...) collidates because both
		 * components generate a wxEVT_PAINT event.
		 * That is my conclusion.
		 * 
		 * Another more technical question I thought about: What the hell is the
		 * identifier needed by Connect(...)? If the identifier gives you the
		 * unique component (as it should?!) then the call
		 * 	"panel->Connect(panel_id, wxEVT_PAINT, ...)"
		 * contains redudant information. But that does not seem to be the function
		 * of the identifier because the call
		 * 	"panel->Connect(any_child_window_id, wxEVT_PAINT, ...)"
		 * does not work properly (as stated above).
		 * => 2 possibilities:
		 * 1. the identifier has nothing to do with the component we get an
		 * 	identifer from (with XRCID(...)). That's NOT true.
		 * 2. Connect(...) has a little strange behaviour. And I think that is true.
		 * 
		 * Well, long text short message:
		 * WHEN HANDLING EVENTS ONLY CONNECT TO THE EVENT SOURCE!!!
		 * 
		 * 
		 * New thought: I talked some much about redudance.
		 * Normally we do not have an own member variable for buttons,
		 * checkboxes, ... In fact we do not have any member variables for "simple"
		 * components at all (and simple components dominates the GUI). Why should
		 * we call
		 * 	"wxWindow a_window = XRCCTRL(...)"
		 * and then
		 * 	"MGG_EVT_*(a_window, ...)"
		 * and forget about the "a_window"-pointer immediately?
		 * 
		 * Conclusion: New Syntax for MGG_EVT_BUTTON (and probably for all macros).
		 * 1. parent: (_content_panel) [pointer]
		 * 			any parent of the component which generates the events
		 * 			(incl. the component itself!). Preferably _content_panel!
		 * 2. event_source_xrc_id_string: ("ID_BUTTON_ENABLE") [const wxString&]
		 * 			the xrc identifier string for the component
		 * 3. function: (TestModule::on_button_enable_click) [class function pointer]
		 * 			the method (not neccessarily in the same class class) which
		 * 			handles the event (parameter: a event of apropriate type,
		 * 			void return value)
		 * 4. function_owner: (this) [pointer]
		 * 			the object which owns the function (do not set it wrong!)
		 * 
		 * MGG_EVT_BUTTON(_content_panel, "ID_BUTTON_ENABLE",
		 * 	TestModule::on_button_enable_click, this);
		 * MGG_EVT_BUTTON(_content_panel, "ID_BUTTON_LEFT",
		 * 	TestModule::on_button_left_click, this);
		 * 
		 * 
		 * OKAY, next NEW SYNTAX: This time only little changes are made. I thought
		 * we might need a macro for components we load from XRC resources and
		 * another one for "normal" components (just to have some comfort, so
		 * we do not need to use "Connect" at all).
		 * 
		 * Syntax for XRC resources:
		 * 	Look above! The macro name changed from MGG_EVT_* to MGG_XRC_EVT_*.
		 * 	And that's it!
		 * 
		 * Syntax for "normal" components:
		 * 	1. event_source: [pointer]
		 * 			the component which generate the event
		 * 	2. function: [class function pointer]
		 * 			the method (not neccessarily in the same class class) which
		 * 			handles the event (parameter: a event of apropriate type,
		 * 			void return value)
		 * 	3. function_owner: [pointer]
		 * 			the object which owns the function (do not set it wrong!) */
		 
		/* "normal component": */
		MGG_EVT_BUTTON(enable_button, TestModule::on_button_enable_click, this);
		
		/* xrc component: */
		MGG_XRC_EVT_BUTTON(_content_panel, "ID_BUTTON_LEFT",
			TestModule::on_button_left_click, this);
		
		/* and an event handler from another class: */
		AnotherEventHandler* aeh = new AnotherEventHandler;
		MGG_XRC_EVT_BUTTON(_content_panel, "ID_BUTTON_RIGHT",
			AnotherEventHandler::on_button_right_click, aeh);
		
		return true;
	}
	
	
	/* This method overwrites the corresponding virtual method of Module.
	 * The method is responsable for completing the _preferences_tree_item (The
	 * caller garantuees that a service list (via _service_list) is available!). */
	bool
	TestModule::intern_load_preferences_tree_item()
	{
		/* In the beginning there was chaos...
		 * 
		 * So we have to search for the light switch:
		 * First we get a new tree item. */
		_preferences_tree_item = PreferencesDialogLogic::new_tree_item();
		
		/* And now we will complete it!
		 * We load a panel which contains the information we want to display
		 * when the user clicks on the tree entry. (I will re-use the
		 * PREFERENCES_GENERALE panel because I'm too lazy. 
		 * 
		 * IMPORTANT: You have to specify a parent for your panel.
		 * Take PreferencesDialogLogic::tree_item_data_parent()! */
		wxPanel* test_module_panel = wxXmlResource::Get()->
			LoadPanel(PreferencesDialogLogic::tree_item_data_parent(), wxT("ID_PANEL_PREFERENCES_GENERAL"));
		
		/* Build the root node. Make a wxTreeItemData-compatible object from your
		 * panel, button, checkbox, ... (whatever, but it MUST inherite from
		 * wxWindow!) by calling PreferencesDialogLogic::build_tree_item_da...
		 * Quite long. Should think about some macros! */
		wxTreeItemId root_id = _preferences_tree_item->
			AddRoot(wxT("TestModule"), -1, -1,
				PreferencesDialogLogic::build_tree_item_data_from_window(test_module_panel)
			);
		
		/* Now build up your tree. You should always link a panel to a tree item
		 * (so do not copy my examples!).
		 * Further details how to build a wxTreeCtrl: Look into the docu. */
		wxTreeItemId another_id = _preferences_tree_item->AppendItem(root_id, wxT("other things"));
		_preferences_tree_item->AppendItem(another_id, wxT("foo"));
		_preferences_tree_item->AppendItem(root_id, wxT("whatever"));
		
		/* And one correct example: */
		wxPanel* panel_2 = wxXmlResource::Get()->
			LoadPanel(PreferencesDialogLogic::tree_item_data_parent(), wxT("ID_PANEL_PREFERENCES_GENERAL"));
		_preferences_tree_item->AppendItem(another_id, wxT("bar"), -1, -1,
			PreferencesDialogLogic::build_tree_item_data_from_window(panel_2));
		
		return true;
	}
	
	
	/* The event handling methods. Do I need to say more. Don't think so. */
	void TestModule::on_button_enable_click(wxCommandEvent& event)
	{
		XRCCTRL(*_content_panel, "ID_BUTTON_LEFT", wxButton)->Enable();
		XRCCTRL(*_content_panel, "ID_BUTTON_RIGHT", wxButton)->Enable();
	}


	void TestModule::on_button_left_click(wxCommandEvent& event)
	{
		XRCCTRL(*_content_panel, "ID_BUTTON_LEFT", wxButton)->Enable(false);
	}
	
	
	void TestModule::on_tool_click(wxCommandEvent& event)
	{
		std::cout << "You clicked the tool." << std::endl;
	}


	void TestModule::on_tool_2_click(wxCommandEvent& event)
	{
		std::cout << "You clicked the tool_2." << std::endl;
	}


	void AnotherEventHandler::on_button_right_click(wxCommandEvent& event)
	{
		std::cout << "Yeah! AnotherEventHandler::on_button_right_click works!" << std::endl;
	}
			
} // namespace mapgeneration_gui
