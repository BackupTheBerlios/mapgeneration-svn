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

#include "tile.h"
#include "util/geocoordinate.h"

using namespace mapgeneration;
using mapgeneration_util::GeoCoordinate;

namespace mapgeneration_gui
{

	class MapGenerationDraw
	{		
		public:
		
			static void
			arrow(GPSDraw* gps_draw, const GeoCoordinate& p1,
				const GeoCoordinate& p2);
		
			static void
			edge(GPSDraw* gps_draw, const GeoCoordinate& p1,
				const GeoCoordinate& p2);

			
			static void
			tile_border(GPSDraw* gps_draw, const Tile::Id tile_id, 
				const unsigned int width_and_height=1);
								
		private:
		
			MapGenerationDraw() {};
			
	};	

} // namespace mapgeneration_gui


#endif //MAPGENERATIONDRAW_H
