/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "modulemanager.h"

#include "module.h"
#include "modules/testmodule.h"
#include "modules/mapmodule/mapmodule.h"
#include "modules/tracelogmodule/tracelogmodule.h"

namespace mapgeneration_gui
{
	
	ModuleManager::ModuleManager()
	: _modules()
	{
		/** @todo make it dynamic */
		_modules.push_back(new TestModule("TestModule"));
		_modules.push_back(new MapModule());
		_modules.push_back(new TraceLogModule());
	}
	
	
	ModuleManager::~ModuleManager()
	{
		std::vector<Module*>::iterator iter = _modules.begin();
		std::vector<Module*>::iterator iter_end = _modules.end();
		for(; iter != iter_end; ++iter)
		{
			delete *iter;
		}
	}
	
	
	Module*
	ModuleManager::module(const wxString& label)
	{
		std::vector<Module*>::iterator iter = _modules.begin();
		std::vector<Module*>::iterator iter_end = _modules.end();
		for(; iter != iter_end; ++iter)
		{
			if ((*iter)->label() == label)
			{
				return *iter;
			}
		}
		
		return NULL;		
	}
	
	
	Module*
	ModuleManager::operator[](int index)
	{
		return _modules[index];
	}
		
} // namespace mapgeneration_gui
