/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "export.h"

using namespace mapgeneration_export;

/**
 * @brief Main method of the exporter. Very small :-)
 */
int main(int argc, int** argv)
{
	Export exporter(Export::_ROUTING_FILE);
	exporter.convert();
	return 0;
}
