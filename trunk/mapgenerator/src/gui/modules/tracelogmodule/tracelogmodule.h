/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef TRACELOGMODULE_H
#define TRACELOGMODULE_H

#include "dbconnection.h"
#include "edgecache.h"
#include "tilecache.h"
#include "util/mlog.h"
#include "gui/module.h"
#include "tracelogpanellogic.h"


using namespace mapgeneration;
using namespace mapgeneration_util;


namespace mapgeneration_gui
{
	
	class TraceLogModule : public Module
	{
		
		public:
			
			TraceLogModule();
			
			~TraceLogModule();
		
		protected:
			
			bool
			intern_load_content_panel();
			
			
			bool
			intern_load_preferences_tree_item();

			
		private:
						
			TraceLogPanelLogic* _trace_log_panel_logic;
			
	};
	
} // namespace mapgeneration_util

#endif //TRACELOGMODULE_H
