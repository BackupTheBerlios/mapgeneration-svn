/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/

#include <iostream>

#include "prtconverter.h"

using mapgeneration_util::PeterRoosenTracksConverter;

int
main(int argc, char* argv[])
{
	PeterRoosenTracksConverter prtc;
	
	prtc.connect_db();
	prtc.run_converter(true);
	prtc.disconnect_db();
	
	return 0;
}
