/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "mapgenerationdraw.h"

#include "geocoordinate.h"


namespace mapgeneration_gui
{
		
	void
	MapGenerationDraw::edge(GPSDraw* gps_draw, 
		const std::vector< Node >& edge_nodes, const double detail)
	{
		int jump_size = (int)(1.0/detail);
				
		std::vector< Node >::const_iterator 
			nodes_iter = edge_nodes.begin();
		std::vector< Node >::const_iterator 
			nodes_iter_end = edge_nodes.end();
		
		std::vector< Node >::const_iterator
			previous_node = nodes_iter;
		
		if (nodes_iter != nodes_iter_end)
		{
			++nodes_iter;
		}
		
		bool last_run = false;
		while (nodes_iter != nodes_iter_end)
		{
			gps_draw->line(previous_node->get_latitude(), previous_node->get_longitude(), 
				nodes_iter->get_latitude(), nodes_iter->get_longitude());
			
			previous_node = nodes_iter;
			
			for (int i=0; i<=jump_size && nodes_iter != nodes_iter_end; ++i, ++nodes_iter);
			if (nodes_iter == nodes_iter_end && !last_run)
			{
				--nodes_iter;
				last_run = true;
			}
		}
	}


	void
	MapGenerationDraw::tile_border(GPSDraw* gps_draw, const unsigned int tile_id, 
	const unsigned int width_and_height)
	{
		int northing, easting;
		GeoCoordinate::split_tile_id(tile_id, northing, easting);
		
		double latitude1=((double)northing) / 100.0 - 90.0;
		double longitude1=((double) easting) / 100.0 - 180.0;
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
