/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "mapgenerationdraw.h"


namespace mapgeneration_gui
{
	
	void
	MapGenerationDraw::arrow(GPSDraw* gps_draw, const GeoCoordinate& p1,
		const GeoCoordinate& p2)
	{
		gps_draw->arrow(p1.get_latitude(), p1.get_longitude(), 
			p2.get_latitude(), p2.get_longitude(), 5.0);
	}
	
		
	void
	MapGenerationDraw::edge(GPSDraw* gps_draw, const GeoCoordinate& p1,
		const GeoCoordinate& p2)
	{
		gps_draw->line(p1.get_latitude(), p1.get_longitude(), 
			p2.get_latitude(), p2.get_longitude());
	}


	void
	MapGenerationDraw::tile_border(GPSDraw* gps_draw, const Tile::Id tile_id, 
	const unsigned int width_and_height)
	{
		Tile::Id northing, easting;
		Tile::split_tile_id(tile_id, northing, easting);
		
		double latitude1=((double)northing) / 100.0 - 90.0;
		double longitude1=((double)easting) / 100.0 - 180.0;
		double latitude2=(((double)northing) + (double)width_and_height) / 100.0 - 90.0;
		double longitude2=((double)easting) / 100.0 - 180.0;
		double latitude3=(((double)northing) + (double)width_and_height) / 100.0 - 90.0;
		double longitude3=(((double)easting) + (double)width_and_height) / 100.0 - 180.0;
		double latitude4=((double)northing) / 100.0 - 90.0;
		double longitude4=(((double)easting) + (double)width_and_height) / 100.0 - 180.0;

		gps_draw->line(latitude1, longitude1, latitude2, longitude2);
		gps_draw->line(latitude2, longitude2, latitude3, longitude3);
		gps_draw->line(latitude3, longitude3, latitude4, longitude4);
		gps_draw->line(latitude4, longitude4, latitude1, longitude1);
	}

}
