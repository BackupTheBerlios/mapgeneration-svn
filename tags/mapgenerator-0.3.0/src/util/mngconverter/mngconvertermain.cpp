/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/

#include <iostream>

#include "mngconverter.h"

using mapgeneration_util::MapAndGuideConverter;

int
main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cout << "Syntax error." << std::endl;
		std::cout << "Usage: " << argv[0] << " input-file output-file" << std::endl;
		return 1;
	}
	
	std::string input_file = argv[1];
	std::string output_file = argv[2];
	
	MapAndGuideConverter mng_c;
	mng_c.set_input_file(input_file);
	mng_c.set_output_file(output_file);
	mng_c.run_converter();
	
	return 0;
}
