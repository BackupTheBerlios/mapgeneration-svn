/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/

#include "config.h"

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
	std::cout << PACKAGE_STRING << "\n";
	std::cout << "Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz\n"
	          << "Licensed under the Academic Free License version 2.1\n\n";
	mlog(MLog::notice, "main") << "MapGenerator startet!\n";
	mlog(MLog::debug, "main") << "Instantiating ExecutionManager.\n";
	ExecutionManager execution_manager;
	mlog(MLog::debug, "main") << "Starting ExecutionManager.\n";
	execution_manager.run();
	mlog(MLog::notice, "main") << "MapGenerator finished!\n";
}
