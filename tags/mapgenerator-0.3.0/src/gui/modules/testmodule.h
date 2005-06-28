/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/

 
/* Read SVN log! */

/* How to write a module:
 * Choose a name and make .h and .cpp files.
 * #include "../module.h" and inheritate from Module. */
 
#ifndef TESTMODULE_H
#define TESTMODULE_H

#include "../module.h"

namespace mapgeneration_gui
{
	
	class TestModule : public Module, public wxEvtHandler
	{
		
		public:
			
			/* Constructors:
			 * We need only the default constructor.
			 * Second one is just for comfort. It sets the label attribute
			 * but you can do it with label() method. */
			TestModule();
			
			
			TestModule(const wxString& label);
			
			
		protected:
			
			/* A method to avoid duplicated code. See testmodule.cpp. */
			void
			initialize_module();
			
			
			/* Define these two methods.
			 * First one have to load the content panel (see testmodule.cpp).
			 * Second one have to load the tree items included into the
			 * preferences tree. If you want to supply a preferences tree item
			 * call the method from your constructor. */
			bool
			intern_load_content_panel();
			
			
			bool
			intern_load_preferences_tree_item();
			
			
			/* Define some methods for event handling. These methods do not need to be
			 * members of the module class. So you can write a class for event
			 * handling explicitly. Access rights does not matter within the same
			 * class. Outside the class they have to be public. Otherwise the event
			 * system will not find them.
			 * Look into testmodule.cpp to see how to register event methods. */
			void
			on_button_enable_click(wxCommandEvent& event);
			
			
			void
			on_button_left_click(wxCommandEvent& event);
			
			
			void
			on_button_right_click(wxCommandEvent& event);
			
			
			void
			on_tool_click(wxCommandEvent& event);

			
			void
			on_tool_2_click(wxCommandEvent& event);
			
	};
	
} // namespace mapgeneration_gui

#endif // TESTMODULE_H
