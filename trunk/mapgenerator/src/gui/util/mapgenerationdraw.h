/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef MAPGENERATIONDRAW_H
#define MAPGENERATIONDRAW_H

#include "gpsdraw.h"

#include <vector>

#include "node.h"


using namespace mapgeneration;


namespace mapgeneration_gui
{

	class MapGenerationDraw
	{		
		public:
		
			static void
			edge(GPSDraw* gps_draw, const std::vector< Node >& edge_nodes,
				const double detail = 1.0);

			
			static void
			tile_border(GPSDraw* gps_draw, const unsigned int tile_id, 
				const unsigned int width_and_height=1);
								
		private:
		
			MapGenerationDraw() {};
			
	};	

} // namespace mapgeneration_gui


#endif //MAPGENERATIONDRAW_H
