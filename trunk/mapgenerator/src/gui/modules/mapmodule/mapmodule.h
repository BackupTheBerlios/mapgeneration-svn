/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef MAPMODULE_H
#define MAPMODULE_H

#include "dbconnection.h"
#include "tilecache.h"
#include "util/mlog.h"
#include "gui/module.h"
#include "mappanellogic.h"


using namespace mapgeneration;
using namespace mapgeneration_util;


namespace mapgeneration_gui
{
	
	class MapModule : public Module
	{
		
		public:
			
			MapModule();
			
			~MapModule();
		
		protected:
			
			bool
			intern_load_content_panel();
			
			
			bool
			intern_load_preferences_tree_item();
			
			
		private:
						
			MapPanelLogic* _map_panel_logic;
		
	};
	
} // namespace mapgeneration_util

#endif //MAPMODULE_H
