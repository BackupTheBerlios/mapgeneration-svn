/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include <iostream>

#include "executionmanager.h"
#include "util/mlog.h"

using namespace mapgeneration;
using namespace mapgeneration_util;


/**
 * @brief Main method.
 * 
 * Inits and runs the ExecutionManager.
 *  
 * @see ExecutionManager
 */
int main()
{
	mlog(MLog::notice, "main") << "MapGenerator startet!\n";
	mlog(MLog::debug, "main") << "Instantiating ExecutionManager.\n";
	ExecutionManager execution_manager;
	mlog(MLog::debug, "main") << "Starting ExecutionManager.\n";
	execution_manager.run();
	mlog(MLog::notice, "main") << "MapGenerator finished!\n";
}
