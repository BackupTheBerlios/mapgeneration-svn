/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include <iostream>

#include "traceserver.h"
#include "util/mlog.h"

using namespace mapgeneration_util;


int main() {
	mlog(MLog::info, "test_traceserver") << "test_traceserver startet.\n";

	new mapgeneration::TraceServer(0);
	
	return 0;
}

