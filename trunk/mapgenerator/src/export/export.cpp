/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "export.h"

#include "../edge.h"
#include "../node.h"
#include "../tile.h"
#include "../util/mlog.h"

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <map>
#include <sstream>
#include <utility>

namespace mapgeneration_export
{
	
	Export::Export(Format format)
	: _format(format)
	{
		_db_connection = new DBConnection();
		_db_connection->init();
		_db_connection->connect("MapGeneration", "mapgeneration", "mg");
		
		_edge_cache = new EdgeCache(_db_connection, EdgeCache::_FIFO, 
			EdgeCache::_NO_WRITEBACK, 2000000, 1800000);
		_tile_cache = new TileCache(_db_connection, TileCache::_FIFO, 
			TileCache::_NO_WRITEBACK, 2000000, 1800000);
		
		mlog(MLog::debug, "Export") << "Inited!\n";
	}
	
	
	Export::~Export()
	{
		delete _edge_cache;
		delete _tile_cache;
		
		_db_connection->disconnect();
		_db_connection->destroy();
		delete _db_connection;

		mlog(MLog::debug, "Export") << "Destroyed!\n";
	}
	
	
	void
	Export::convert_to_routing_file_format()
	{
		std::stringstream node_stream;
		node_stream.setf(std::ios::showpoint);
		node_stream.setf(std::ios::fixed);
		node_stream.precision(7);
		
		std::stringstream edge_stream;
		edge_stream.setf(std::ios::showpoint);
		edge_stream.setf(std::ios::fixed);
		edge_stream.precision(7);
		
		std::stringstream detailed_edge_stream;
		detailed_edge_stream.setf(std::ios::showpoint);
		detailed_edge_stream.setf(std::ios::fixed);
		detailed_edge_stream.precision(7);
		int current_edge_id = 1;
		
		int id_counter = 1;
		std::map< std::pair<unsigned int, unsigned int> , int> id_map;

		vector<unsigned int> ids = _db_connection->get_all_used_edge_ids();
		vector<unsigned int>::iterator ids_iter = ids.begin();
		vector<unsigned int>::iterator ids_iter_end = ids.end();

		for (; ids_iter != ids_iter_end; ++ids_iter)
		{
			const EdgeCache::Pointer edge = _edge_cache->get(*ids_iter);
			if (edge != 0)
			{
				const std::pair<unsigned int, unsigned int> begin_id = edge->node_ids().front();
				const std::pair<unsigned int, unsigned int> end_id = edge->node_ids().back();
				const Node& begin_node = _tile_cache->get(begin_id.first)->nodes()[begin_id.second].second;
				const Node& end_node = _tile_cache->get(end_id.first)->nodes()[end_id.second].second;
				
				if (id_map.find(begin_id) == id_map.end())
				{
					node_stream << "N," << begin_node.get_longitude() << ","
						<< begin_node.get_latitude() << std::endl;
					
					id_map.insert(std::make_pair(begin_id, id_counter));
					++id_counter;
				}
				
				if (id_map.find(end_id) == id_map.end())
				{
					node_stream << "N," << end_node.get_longitude() << ","
						<< end_node.get_latitude() << std::endl;
					
					id_map.insert(std::make_pair(end_id, id_counter));
					++id_counter;
				}

				// 0.05 <= base_velocity <= 0.1
				// 0.072 == 50km/h
				double base_velocity = ((double)rand()) / ((double)RAND_MAX * 20.0) + 0.05;
				double length = begin_node.distance(end_node);
				double travel_time = base_velocity * length;

				edge_stream << "E,";
				edge_stream << (id_map.find(begin_id))->second;
				edge_stream << ",";
				edge_stream << (id_map.find(end_id))->second;
				edge_stream << ",";
				edge_stream << edge->get_id() + 1; 
				edge_stream << ",";
				edge_stream << length;
				edge_stream << ",";
				
				for (int j = 0 ; j < 96; ++j)
				{
					double real_travel_time = travel_time +
						(( ((double)rand()) / ((double)RAND_MAX) ) - 0.5) * travel_time;
					edge_stream << real_travel_time;
					
					if (j < 95)
						edge_stream << ",";
					else
						edge_stream << std::endl;
				}
				
				detailed_edge_stream << current_edge_id;
				std::list< std::pair<unsigned int, unsigned int> >::const_iterator edge_node_ids_iter
					= edge->node_ids().begin();
				if (edge_node_ids_iter != edge->node_ids().end())
					++edge_node_ids_iter;
				
				std::list< std::pair<unsigned int, unsigned int> >::const_iterator test_iter
					= edge_node_ids_iter;
				++test_iter;
					
				for (; test_iter != edge->node_ids().end(); ++edge_node_ids_iter, ++test_iter)
				{
					const Node& node = _tile_cache->get(edge_node_ids_iter->first)->nodes()[edge_node_ids_iter->second].second;
					
					detailed_edge_stream << "," << node.get_latitude();
					detailed_edge_stream << "," << node.get_longitude();
				}
				detailed_edge_stream << "\n";
				++current_edge_id;
				
			} //end if
		} // end for
		
		std::fstream file_stream("graph.txt", std::ios::out);
		file_stream << "# Mapgeneration-DB" << std::endl;
		time_t now = time(0);
		file_stream << "# Generated at " << ctime(&now);
		file_stream << node_stream.str();
		file_stream << edge_stream.str();

		file_stream.flush();
		file_stream.close();
		
		std::fstream another_file_stream("detailed_edge.txt", std::ios::out);
		another_file_stream << "# Mapgeneration-DB" << std::endl;
		another_file_stream << "# Generated at " << ctime(&now);
		another_file_stream << detailed_edge_stream.str();

		another_file_stream.flush();
		another_file_stream.close();
	}
	
} // namespace mapgeneration_export
