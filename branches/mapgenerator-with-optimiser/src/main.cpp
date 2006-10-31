/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/

/**/
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
 ****/
int main(int argc, char* argv[])
{
	std::cout << PACKAGE_STRING << "\n";
	std::cout << "Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz\n"
	          << "Licensed under the Academic Free License version 2.1\n\n";
	mlog(MLog::notice, "main") << "MapGenerator startet!\n";
	mlog(MLog::debug, "main") << "Instantiating ExecutionManager.\n";
	
	ExecutionManager execution_manager;
	mlog(MLog::debug, "main") << "Starting ExecutionManager.\n";
	
	if (argc > 1)
		execution_manager.set_single_trace_mode(true);
	
	execution_manager.run_without_thread();
	mlog(MLog::notice, "main") << "MapGenerator finished!\n";
}	
	/*


#include "dbconnection/filedbconnection.h"
#include "tilecache.h"
#include "tile.h"
#include "node.h"

#include <iostream>
#include <fstream>

using namespace mapgeneration;
using namespace mapgeneration_util;
int main()
{
		mlog(MLog::info, "ExecutionManager") << "Initializing DBConnection.\n";
		
		FileDBConnection* file_db_connection = new FileDBConnection();
		std::string db_directory = "./filedb";
		file_db_connection->set_parameters(db_directory);
		
		size_t tiles_table_id = file_db_connection->register_table("tiles");
		file_db_connection->connect();
		mlog(MLog::info, "ExecutionManager") << "DBConnection initialized.\n";
		
		mlog(MLog::info, "ExecutionManager") << "Starting TileCache.\n";
		TileCache* tile_cache = new TileCache(file_db_connection, tiles_table_id,
			TileCache::_FIFO, TileCache::_STANDARD_CACHE, 1000,
			100000000, 80000000);
		tile_cache->controlled_start();
		mlog(MLog::info, "ExecutionManager") << "TileCache started.\n";
		
		std::ofstream out_file("google.xml");
		out_file.precision(10);
		out_file << "<segments>\n";
		std::vector<Tile::Id> used_ids = tile_cache->get_used_ids();
		for (int i = 0; i < used_ids.size(); ++i)
		{
			if (used_ids[i] == 909724139 )
			{
				TileCache::Pointer tile = tile_cache->get(used_ids[i]);
				for (Node::LocalId j = 0; j < tile->nodes().size_including_holes(); ++j)
				{
					if (tile->exists_node(j))
					{
						const Node& node = tile->node(j);
						
						Node::D_Edges::const_iterator iter = node.successors().begin();
						Node::D_Edges::const_iterator iter_end = node.successors().end();
						for (; iter != iter_end; ++iter)
						{
							TileCache::Pointer succ_tile = tile_cache->get(
								Node::tile_id(iter->get_next_node_id()));
							const Node& succ_node = succ_tile->node(
								iter->get_next_node_id());
							
							out_file << "\t<segment lat0=\"" << node.get_latitude()
								<< "\" lon0=\"" << node.get_longitude()
								<< "\" lat1=\"" << succ_node.get_latitude()
								<< "\" lon1=\"" << succ_node.get_longitude()
								<< "\"/>\n";
							
/*							out_file << "<LineString>     <extrude>1</extrude>     <tessellate>1</tessellate>     <altitudeMode>absolute</altitudeMode>     <coordinates>";
							out_file << node.get_longitude() << "," << node.get_latitude() << ",0\n";
							out_file << succ_node.get_longitude() << "," << succ_node.get_latitude() << ",0\n";
							out_file << "</coordinates>   </LineString>";
*************							
						}
					}
				}
			}
		}
		out_file << "</segments>\n";
		out_file.flush();
		out_file.close();
		
		tile_cache->controlled_stop();
		file_db_connection->disconnect();
		
		return 0;
}*/
