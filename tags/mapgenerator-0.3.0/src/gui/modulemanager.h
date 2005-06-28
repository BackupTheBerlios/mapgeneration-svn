/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef MODULEMANAGER_H
#define MODULEMANAGER_H

#include <vector>
#include <wx/string.h>

namespace mapgeneration_gui
{
	
	class Module; /* DO NOT INCLUDE module.h HERE!!! */
	
	
	class ModuleManager
	{
		
		public:
			
			ModuleManager();
			
			
			~ModuleManager();
			
			
			Module*
			module(const wxString& identifier);
			
			
			Module*
			operator[](int index);
			
			
		private:
			
			std::vector<Module*> _modules;
			
	};
	
} // namespace mapgeneration_util

#endif // MODULEMANAGER_H
