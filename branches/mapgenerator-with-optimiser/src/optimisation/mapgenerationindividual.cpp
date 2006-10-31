/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/

#include "mapgenerationindividual.h"

//#include "util/rangereporting/quadrangle.h"



#include <dirent.h>
#include <fstream>
//#include <sys/stat.h>

#include "executionmanager.h"
//#include "filteredtrace.h"
#include "tile.h"
#include "tilecache.h"
#include "tilemanager.h"
#include "traceprocessor.h"
#include "util/mlog.h"
#include "util/pubsub/servicesystem.h"

using namespace mapgeneration;
using mapgeneration_util::MLog;
//using ost::Thread;


namespace mapgeneration_opt
{
	
	MapGenerationIndividual::MapGenerationIndividual()
	: AsymmetricAdaptiveIndividual<MAP_GENERATION_INDIVIDUAL_DIMENSION>(),
		_age(0), _a(1.0), _b(1.0), _test_pool(),
		_total_number_of_crossings(0), _total_number_of_nodes(0),
		_total_surface_area(HUGE_VAL)
	{
	}
	
	
	MapGenerationIndividual::MapGenerationIndividual(
		const std::vector<FilteredTrace>& test_pool)
	: AsymmetricAdaptiveIndividual<MAP_GENERATION_INDIVIDUAL_DIMENSION>(),
		_age(0), _a(1.0), _b(1.0), _test_pool(test_pool),
		_total_number_of_crossings(0), _total_number_of_nodes(0),
		_total_surface_area(HUGE_VAL)
	{
	}
	
	
	void
	MapGenerationIndividual::deserialize(std::istream& i_stream)
	{
		AsymmetricAdaptiveIndividual<MAP_GENERATION_INDIVIDUAL_DIMENSION>::
			deserialize(i_stream);
		Serializer::deserialize(i_stream, _age);
	}
	
	
	unsigned int
	MapGenerationIndividual::get_age() const
	{
		return _age;
	}
	
	
	const std::vector<FilteredTrace>&
	MapGenerationIndividual::get_test_pool() const
	{
		return _test_pool;
	}
	
	
	double
	MapGenerationIndividual::impl_calculate_fitness() const
	{
		mlog.set_display_level(MLog::alert);
		
		std::cout << "object_components:" << std::endl;
		for (int i = 0; i < MAP_GENERATION_INDIVIDUAL_DIMENSION; ++i)
		{
			std::cout << std::setw(10) << this->_object_components[i] << ", ";
			
			if (i % 13 == 12)
				std::cout << std::endl;
		}
		std::cout << std::endl;
		
		std::cout << "PROCESSES EXECUTIONMANAGER..." << std::endl;
		ExecutionManager* execution_manager = new ExecutionManager;
		execution_manager->set_start_trace_filter(false);
		execution_manager->set_start_trace_server(false);
		execution_manager->controlled_start();
		
		pubsub::ServiceList* service_list = execution_manager->service_list();
		while (service_list == 0)
		{
			ost::Thread::yield();
			service_list = execution_manager->service_list();
		}
		
		TileManager* tile_manager = execution_manager->tile_manager();
		while(tile_manager == 0)
		{
			ost::Thread::yield();
			tile_manager = execution_manager->tile_manager();
		}
		tile_manager->set_optimisation_mode(true);
		
		// set configuration values...
		TraceProcessor::Configuration conf(service_list);
		std::vector<std::string>::const_iterator conf_iter
			= conf.identifier_strings().begin();
		std::vector<std::string>::const_iterator conf_iter_end
			= conf.identifier_strings().end();
		
		int i = 0;
		while (conf_iter != conf_iter_end)
		{
			// HACK for TraceProcessor::Configuration (todo: take a constant!)
			if (i >= 5)
			{
				pubsub::Service<double>* service
					= service_list->find_service<double>(*conf_iter);
				service->receive(this->_object_components[i-5]);
			}
			
			++conf_iter;
			++i;
		}
		assert(i == TraceProcessor::_CONFIGURATION_VALUES);
		// done.
		
		// send traces...
		std::vector<FilteredTrace>::iterator trace_iter = _test_pool.begin();
		std::vector<FilteredTrace>::iterator trace_iter_end = _test_pool.end();
		for (; trace_iter != trace_iter_end; ++trace_iter)
		{
			tile_manager->new_trace(*trace_iter);
		}
		// done.
		
		// wait for traces to be processed...
		while ( (tile_manager->get_number_of_processed_traces() < _test_pool.size())
			&& (tile_manager->running()) )
		{
			ost::Thread::yield();
		}
		// done.
		
		std::cout << "DONE." << std::endl;
		
		bool found_invalid_score = false;
		if (tile_manager->get_number_of_processed_traces() < _test_pool.size())
			found_invalid_score = true;
		
		if (tile_manager->get_invalid_path_score())
			found_invalid_score = true;
		
		// calculate fitness...
		double total_surface_area = 0.0;
		
		std::string directory_name;
		directory_name = "./traceprocessor_logs";
		directory_name += "/";
		
		DIR *dir;
		struct dirent *direntp;
		dir = opendir (directory_name.c_str());
		if (dir == NULL)
			throw "Invalid directory!";
		readdir(dir); readdir(dir);
		
		while ( (!found_invalid_score) && ((direntp = readdir(dir)) != NULL) )
		{
			// build filename
			std::string file_name = directory_name;
			file_name += "/";
			file_name += direntp->d_name;
			
			// deserialize protocol
			TraceProcessor::Protocol protocol;
			std::ifstream protocol_file(file_name.c_str());
			Serializer::deserialize(protocol_file, protocol);
			
			std::map<Tile::Id, Tile*> used_tiles;
			for (int i = 0; i < protocol._used_tiles.size(); ++i)
			{
				used_tiles.insert(std::make_pair(
					protocol._used_tiles[i]->get_id(), protocol._used_tiles[i]));
			}
			
			TraceProcessor::PathEntry* entry = 0;
			if (!found_invalid_score)
			{
				TraceProcessor::D_Path::iterator iter = protocol._path.begin();
				TraceProcessor::D_Path::iterator iter_end = protocol._path.end();
				
				// search the beginning
				while( (iter != iter_end) && (!iter->second->_is_beginning) )
				{
					++iter;
				}
				
				if (iter == iter_end)
				{
					// Protocol too short. That happens if
					// 1st: no valid path (part) was found (that's bad)
					// 2nd: trace too short (choose longer traces!!!)
					found_invalid_score = true;
					
				} else
				{
					assert(iter->second->_is_beginning);
					entry = iter->second;
					
					if (entry->_score == TraceProcessor::_INVALID_PATH_SCORE)
					{
						found_invalid_score = true;
					} else
					{
						// Increment entry.
						do
							entry = entry->_connection;
						while (entry->_is_interpolated);
					}
				}
			}
			
			// Beginning entry found.
			// Begin loop for calculating the score.
			while ( (!found_invalid_score) && (entry != 0) )
			{
				assert(!entry->_is_interpolated);
				assert(entry->_score != TraceProcessor::_INVALID_PATH_SCORE);
				
				// Get the previous not interpolated entry.
				TraceProcessor::PathEntry* previous_entry
					= entry->_backward_connection;
				while (previous_entry->_is_interpolated)
				{
					previous_entry = previous_entry->_backward_connection;
					
					assert(previous_entry != 0);
				}
				
				assert(!previous_entry->_is_interpolated);
				
				// The nodes and point.
				assert(used_tiles.find(Node::tile_id(previous_entry->_node_id))
					!= used_tiles.end());
				assert(used_tiles.find(Node::tile_id(entry->_node_id))
					!= used_tiles.end());
				
				const GeoCoordinate geo_1 = used_tiles
					.find(Node::tile_id(previous_entry->_node_id))->second
					->node(previous_entry->_node_id);
				
				const GeoCoordinate geo_2 = protocol._trace.gps_point_at(
					previous_entry->_scan_position);
				
				const GeoCoordinate geo_3 = used_tiles
					.find(Node::tile_id(entry->_node_id))->second
					->node(entry->_node_id);
				
				const GeoCoordinate geo_4 = protocol._trace.gps_point_at(
					entry->_scan_position);
				
				rangereporting::Quadrangle<GeoCoordinate> quadrangle;
				quadrangle.set_point(0, geo_1);
				quadrangle.set_point(1, geo_2);
				quadrangle.set_point(2, geo_3);
				quadrangle.set_point(3, geo_4);
				total_surface_area += quadrangle.area_size();
				
//				if (quadrangle.area_size() > 1000.0)
//				{
//					std::cout << "Quadrangle.area_size = "
//						<< quadrangle.area_size() << std::endl;
//					std::cout << geo_1 << std::endl << geo_2 << std::endl
//						<< geo_3 << std::endl << geo_4 << std::endl;
//				}
				
				// Increment entry.
				do
					entry = entry->_connection;
				while ( (entry != 0) && (entry->_is_interpolated) );
			} // end while ( (!found_invalid_score) && (entry != 0) )
		} // end while ( (!found_invalid_score) && ((direntp = readdir(dir)) != NULL) )
		
		closedir(dir);
		
		// Calculate the number of nodes and crossings
		unsigned int total_number_of_crossings = 0;
		unsigned int total_number_of_nodes = 0;
		if (!found_invalid_score)
		{
			std::vector<Tile::Id> used_tiles
				= tile_manager->get_tile_cache()->get_used_ids();
			
			std::vector<Tile::Id>::const_iterator iter = used_tiles.begin();
			std::vector<Tile::Id>::const_iterator iter_end = used_tiles.end();
			
			for (; iter != iter_end; ++iter)
			{
				TileCache::Pointer tile = tile_manager->get_tile_cache()
					->get(*iter);
				
				unsigned int number_of_nodes = tile->nodes().size();
				total_number_of_nodes += number_of_nodes;
				
				unsigned int number_of_crossings = 0;
				Tile::D_Nodes::const_iterator nodes_iter
					= tile->nodes().begin();
				Tile::D_Nodes::const_iterator nodes_iter_end
					= tile->nodes().end();
				for (; nodes_iter != nodes_iter_end; ++nodes_iter)
				{
					if (nodes_iter->first)
					{
						const Node& node = nodes_iter->second;
						if ( (node.predecessors().size() > 1)
							|| (node.successors().size() > 1) )
						{
							++number_of_crossings;
						}
					}
				}
				total_number_of_crossings += number_of_crossings;
			}
		}
		
		// shut down execution manager
		execution_manager->set_delete_database(true);
		execution_manager->set_delete_tracelogs(true);
		execution_manager->controlled_stop();
		delete execution_manager;
		
		std::cout << "ExecutionManager stopped." << std::endl;
		
		// return the fitness value
		double fitness = HUGE_VAL;
		if (!found_invalid_score)
		{
			_total_number_of_crossings = total_number_of_crossings;
			_total_number_of_nodes = total_number_of_nodes;
			_total_surface_area = total_surface_area;
			
//			fitness = _a * total_surface_area * total_surface_area
//				+ _b * static_cast<double>(total_number_of_nodes);
			
//			fitness = 0.001 * total_surface_area 
//				+ 1.0 * static_cast<double>(total_number_of_nodes);
			
			if (total_number_of_nodes > 1)
			{
				fitness
					= 10.0 * total_number_of_nodes;
			}
//			else if (total_number_of_nodes > 588)
//				fitness = 10.0 * total_number_of_nodes - 0.01 * total_surface_area;
//			else
//				fitness = 0.1 * total_surface_area;
		}
		
		std::cout.setf(std::ios::fixed);
		std::cout << "Fitness: " << -fitness
			<< "(TSA=" << total_surface_area
			<< "; TNN=" << total_number_of_nodes
			<< "; TNC=" << total_number_of_crossings
			<< "), Age = "
			<< get_age() << std::endl;
		std::cout.unsetf(std::ios::fixed);
		std::cout << std::endl;
		
		return -fitness;
	}
	
	
/*	double
	MapGenerationIndividual::impl_calculate_fitness() const
	{
		mlog.set_display_level(MLog::alert);
		
		std::cout << "object_components:" << std::endl;
		for (int i = 0; i < MAP_GENERATION_INDIVIDUAL_DIMENSION; ++i)
		{
			std::cout << std::setw(10) << this->_object_components[i] << ", ";
			
			if (i % 13 == 12)
				std::cout << std::endl;
		}
		std::cout << std::endl;
		
		std::cout << "PROCESSES EXECUTIONMANAGER..." << std::endl;
		ExecutionManager* execution_manager = new ExecutionManager;
		execution_manager->set_start_trace_filter(false);
		execution_manager->set_start_trace_server(false);
		execution_manager->controlled_start();
		
		pubsub::ServiceList* service_list = execution_manager->service_list();
		while (service_list == 0)
		{
//			ost::Thread::sleep(10);
			ost::Thread::yield();
			service_list = execution_manager->service_list();
		}
		
		TileManager* tile_manager = execution_manager->tile_manager();
		while(tile_manager == 0)
		{
//			ost::Thread::sleep(10);
			ost::Thread::yield();
			tile_manager = execution_manager->tile_manager();
		}
		tile_manager->set_optimisation_mode(true);
		
		// set configuration values...
		TraceProcessor::Configuration conf(service_list);
		std::vector<std::string>::const_iterator conf_iter
			= conf.identifier_strings().begin();
		std::vector<std::string>::const_iterator conf_iter_end
			= conf.identifier_strings().end();
		
		int i = 0;
		while (conf_iter != conf_iter_end)
		{
			// HACK for TraceProcessor::Configuration (todo: take a constant!)
			if (i >= 5)
			{
				pubsub::Service<double>* service
					= service_list->find_service<double>(*conf_iter);
				service->receive(this->_object_components[i-5]);
			}
			
			++conf_iter;
			++i;
		}
		assert(i == TraceProcessor::_CONFIGURATION_VALUES);
		// done.
		
		// send traces...
		std::vector<FilteredTrace>::iterator trace_iter = _test_pool.begin();
		std::vector<FilteredTrace>::iterator trace_iter_end = _test_pool.end();
		for (; trace_iter != trace_iter_end; ++trace_iter)
		{
			tile_manager->new_trace(*trace_iter);
		}
		// done.
		
		// wait for traces to be processed...
		while ( (tile_manager->get_number_of_processed_traces() < _test_pool.size())
			&& (tile_manager->running()) )
		{
//			std::cout << tile_manager->get_number_of_processed_traces()
//				<< "(" << tile_manager->running() << "), ";
			
//			std::cout.flush();
			
//			ost::Thread::sleep(100);
			ost::Thread::yield();
		}
//		std::cout << std::endl;
		// done.
		
		bool found_invalid_score = false;
		if (tile_manager->get_number_of_processed_traces() < _test_pool.size())
			found_invalid_score = true;
		
		if (tile_manager->get_invalid_path_score())
			found_invalid_score = true;
		
		std::cout << "DONE." << std::endl;
		
		// calculate fitness...
		double total_surface_area = 0.0;
		
		std::string directory_name;
		directory_name = "./traceprocessor_logs";
		directory_name += "/";
		
		DIR *dir;
		struct dirent *direntp;
		dir = opendir (directory_name.c_str());
		if (dir == NULL)
			throw "Invalid directory!";
		readdir(dir); readdir(dir);
		
		while ( (!found_invalid_score) && ((direntp = readdir(dir)) != NULL) )
		{
			// build filename
			std::string file_name = directory_name;
			file_name += "/";
			file_name += direntp->d_name;
			
//			std::cout << "\tWORKING ON " << file_name << std::endl;
			
			// deserialize protocol
			TraceProcessor::Protocol protocol;
			std::ifstream protocol_file(file_name.c_str());
			Serializer::deserialize(protocol_file, protocol);
			
			std::map<Tile::Id, Tile*> used_tiles;
			for (int i = 0; i < protocol._used_tiles.size(); ++i)
			{
				used_tiles.insert(std::make_pair(
					protocol._used_tiles[i]->get_id(), protocol._used_tiles[i]));
			}
			
//			std::cout << protocol << std::endl;
			
			TraceProcessor::D_Path::iterator protocol_iter
				= protocol._path.begin();
			TraceProcessor::D_Path::iterator protocol_iter_end
				= protocol._path.end();
			
			// search the beginning
			while( (protocol_iter != protocol_iter_end)
				&& (!protocol_iter->second->_is_beginning) )
			{
				++protocol_iter;
			}
			
			// Some useful variables
			TraceProcessor::PathEntry* previous_entry = 0;
			TraceProcessor::PathEntry* entry = 0;
			
			// Protocol too short. That happens if
			// 1st: no valid path (part) was found (that's bad)
			// 2nd: trace too short (choose longer traces!!!)
			if (protocol_iter == protocol_iter_end)
			{
				found_invalid_score = true;
			} else
			{
				// Inits the entries. Skip interpolated entries.
				// This is unprecise but a useful approximation!
				previous_entry = protocol_iter->second;
				entry = previous_entry->_connection;
				while( (!found_invalid_score)
					&& (entry != 0) && (entry->_is_interpolated) )
				{
					entry = entry->_connection;
				}
			}
			
			// First entries found. Begin loop for calculating the score.
			while( (!found_invalid_score)
				&& (previous_entry != 0) && (entry != 0)
				/*&& (protocol_iter->second->_connection != 0)******** )
			{
				
				if ((previous_entry->_score == TraceProcessor::_INVALID_PATH_SCORE)
					|| (entry->_score == TraceProcessor::_INVALID_PATH_SCORE))
				{
					found_invalid_score = true;
//					std::cout << protocol._path << std::endl;
					
/*				} else if (false) // (previous_entry->_was_deleted || entry->_was_deleted)
				#warning change that!
				{
					bool exit_loop = false;
					while (!exit_loop)
					{
						if (previous_entry == 0 || entry == 0)
						{
							exit_loop = true;
							
						} else if (false) //(previous_entry->_was_deleted || entry->_was_deleted)
						#warning dito
						{
							previous_entry = entry;
							entry = entry->_connection;
							
							#warning Perhaps we should give a penalty here.
						} else
						{
							exit_loop = true;
						}
					}***********
				}
				
				// if an interpolated entry occurs, skip it.
				while( (previous_entry != 0) && (previous_entry->_is_interpolated) )
					previous_entry = previous_entry->_connection;
				
				if (previous_entry != 0)
				{
					entry = previous_entry->_connection;
					while( (entry != 0) && (entry->_is_interpolated) )
						entry = entry->_connection;
				}
				
				if ( (!found_invalid_score) && (previous_entry != 0)
					&& (entry != 0) )
				{
					// intentionally copied!
					assert(used_tiles.find(Node::tile_id(previous_entry->_node_id))
						!= used_tiles.end());
					assert(used_tiles.find(Node::tile_id(entry->_node_id))
						!= used_tiles.end());
					
					GeoCoordinate geo_1 = used_tiles
						.find(Node::tile_id(previous_entry->_node_id))->second
						->node(previous_entry->_node_id);
					
//					tile_manager->get_tile_cache()
//						->get(Node::tile_id(previous_entry->_node_id))
//						->node(previous_entry->_node_id);
					
					GeoCoordinate geo_2 = protocol._trace.gps_point_at(
						previous_entry->_scan_position);
					
					// intentionally copied!
					GeoCoordinate geo_3 = used_tiles
						.find(Node::tile_id(entry->_node_id))->second
						->node(entry->_node_id);
					
//					GeoCoordinate geo_3 = tile_manager->get_tile_cache()
//						->get(Node::tile_id(entry->_node_id))
//						->node(entry->_node_id);
					
					GeoCoordinate geo_4 = protocol._trace.gps_point_at(
						entry->_scan_position);
					
					rangereporting::Quadrangle<GeoCoordinate> quadrangle;
					quadrangle.set_point(0, geo_1);
					quadrangle.set_point(1, geo_2);
					quadrangle.set_point(2, geo_3);
					quadrangle.set_point(3, geo_4);
					total_surface_area += quadrangle.area_size();
					
					if (quadrangle.area_size() > 1000.0)
					{
						std::cout << "Quadrangle.area_size = "
							<< quadrangle.area_size() << std::endl;
						std::cout << geo_1 << std::endl << geo_2 << std::endl
							<< geo_3 << std::endl << geo_4 << std::endl;
					}
					
					// move iters forward
					previous_entry = entry;
					entry = entry->_connection;
					
					// if an interpolated entry occurs, skip it.
//					while( (entry != 0) && (entry->_scan_position == -2.0) )
//						entry = entry->_connection;
				}
			} // end while
		} // end while
		
		closedir(dir);
		// done. (calculate fitness)
		
		// calculate the number of nodes
		// only needed if we do not found an invalid score
		unsigned int total_number_of_nodes = 0;
		if (!found_invalid_score)
		{
			std::vector<Tile::Id> used_tiles
				= tile_manager->get_tile_cache()->get_used_ids();
			
//			std::cout << "Used tiles: " << used_tiles.size() << std::endl;
			std::vector<Tile::Id>::const_iterator used_tiles_iter
				= used_tiles.begin();
			std::vector<Tile::Id>::const_iterator used_tiles_iter_end
				= used_tiles.end();
			
//			int counter= 0;
			for (; used_tiles_iter != used_tiles_iter_end; ++used_tiles_iter)
			{
				TileCache::Pointer tile = tile_manager->get_tile_cache()
					->get(*used_tiles_iter);
				
				unsigned int number_of_nodes = tile->nodes().size();
				total_number_of_nodes += number_of_nodes;
				
//				++counter;
//				if (counter % 500 == 0)
//					std::cout << counter << " ";
				
/*				FixpointVector<Node>::const_iterator nodes_iter
					= tile->nodes().begin();
				FixpointVector<Node>::const_iterator nodes_iter_end
					= tile->nodes().end();
				int crossings = 0;
				for (; nodes_iter != nodes_iter_end; ++nodes_iter)
				{
					if ( (nodes_iter->second.predecessors().size() > 1)
						|| (nodes_iter->second.successors().size() > 1) )
					{
						++crossings;
					}
				}
				
				if (*used_tiles_iter == 930171207)
				{
					if (crossings > 2)
					{
//						total_number_of_nodes += 500;
//						std::cout << "930171207 BÖSE!!!" << std::endl;
					}
				} else if ( (*used_tiles_iter >= 897469031) && *used_tiles_iter <= 897600108)
				{
					if (crossings > 1)
					{
//						total_number_of_nodes += 500;
						std::cout << "897469031-897600108 BÖSE!!!\t"
							<< *used_tiles_iter << std::endl;
					}
				} else if ( (*used_tiles_iter >= 930040128) && *used_tiles_iter <= 930302282)
				{
					if (crossings > 1)
					{
//						total_number_of_nodes += 500;
//						std::cout << "930040128-930302282 BÖSE!!!\t" 
//							<< *used_tiles_iter<< std::endl;
					}
				}**********
			}
			
//			std::cout << std::endl;
		}
		// done.
		
		// shut down execution manager
/*		if (_id > -1)
		{
			DIR* opt_dir = opendir("./opt/");
			if (dir == NULL)
			{
//				std::cout << "Create ./opt/ dir." << std::endl;
				mkdir("./opt/", 0700);
			} else
			{
//				std::cout << "./opt/ dir exists." << std::endl;
			}
			closedir(opt_dir);
			
			int file_id = _id + (_adder * _file_counter);
			std::stringstream ss;
			ss << "./opt/" << file_id << "/";
			ss.flush();
			std::string opt_directory_name = ss.str();
			
			std::cout << file_id << " = " << _id << " + " << _adder << " * " << _file_counter << std::endl;
			
			++_file_counter;
			
			DIR *dir;
			dir = opendir (opt_directory_name.c_str());
			if (dir != NULL)
			{
				closedir(dir);
				
				std::string command_string = "rm -rf " + opt_directory_name;
				int result = system(command_string.c_str());
				if (result != 0)
					std::cout << "Remove does not work.";
			} else
			{
				closedir(dir);
			}
			
			mkdir(opt_directory_name.c_str(), 0700);
			
			std::string command_string = "mv ./filedb " + opt_directory_name;
			int result = system(command_string.c_str());
			if (result != 0)
				std::cout << "Move does work. (filedb)" <<std::endl;
			
			command_string.clear();
			command_string = "mv ./traceprocessor_logs " + opt_directory_name;
			result = system(command_string.c_str());
			if (result != 0)
				std::cout << "Move does work. (traceprocessor_logs)" << std::endl;
		}
**********		
		execution_manager->set_delete_database(true);
		execution_manager->set_delete_tracelogs(true);
		execution_manager->controlled_stop();
		delete execution_manager;
		
		std::cout << "ExecutionManager stopped." << std::endl;
		
		// return the fitness value
		double fitness = HUGE_VAL;
		if (!found_invalid_score)
		{
			_total_number_of_nodes = total_number_of_nodes;
			_total_surface_area = total_surface_area;
			
//			fitness = _a * total_surface_area * total_surface_area
//				+ _b * static_cast<double>(total_number_of_nodes);
			
//			fitness = 0.001 * total_surface_area 
//				+ 1.0 * static_cast<double>(total_number_of_nodes);
			
			if (total_number_of_nodes > 1995)
				fitness = 10.0 * total_number_of_nodes + 0.0001 * total_surface_area;
//			else if (total_number_of_nodes > 588)
//				fitness = 10.0 * total_number_of_nodes - 0.01 * total_surface_area;
			else
				fitness = 0.1 * total_surface_area;
		}
		
		std::cout.setf(std::ios::fixed);
		std::cout << "Fitness: " << -fitness
			<< "(TSA=" << total_surface_area
			<< "; TNN=" << total_number_of_nodes << "), Age = "
			<< get_age() << std::endl;
		std::cout.unsetf(std::ios::fixed);
		std::cout << std::endl;
		
		return -fitness;
	}*/
	
	
	void
	MapGenerationIndividual::impl_mutate()
	{
		AsymmetricAdaptiveIndividual<MAP_GENERATION_INDIVIDUAL_DIMENSION>
			::impl_mutate();
		
		for (int i = 0; i < MAP_GENERATION_INDIVIDUAL_DIMENSION; ++i)
		{
			if (this->_object_components[i] < 0.0)
				this->_object_components[i] = 0.0;
		}
	}
	
	
	void
	MapGenerationIndividual::increment_age()
	{
		++_age;
	}
	
	
	MapGenerationIndividual&
	MapGenerationIndividual::operator=(const Individual& individual)
	{
		AsymmetricAdaptiveIndividual<MAP_GENERATION_INDIVIDUAL_DIMENSION>
			::operator=(individual);
		
		try
		{
			const MapGenerationIndividual& mgi_individual
				= dynamic_cast< const MapGenerationIndividual& >(individual);
			
			_age = mgi_individual._age;
			_a = mgi_individual._a;
			_b = mgi_individual._b;
			_test_pool = mgi_individual._test_pool;
			
		} catch (...)
		{
			std::cout << "MapGenerationIndividual::operator= failed!"
				<< std::endl;
		}
		
		return *this;
	}
	
	
	void
	MapGenerationIndividual::reset_age()
	{
		_age = 0;
	}
	
	
	void
	MapGenerationIndividual::serialize(std::ostream& o_stream) const
	{
		AsymmetricAdaptiveIndividual<MAP_GENERATION_INDIVIDUAL_DIMENSION>::
			serialize(o_stream);
		Serializer::serialize(o_stream, _age);
	}
	
	
	void
	MapGenerationIndividual::set_test_pool(
		const std::vector<FilteredTrace>& test_pool)
	{
		_test_pool = test_pool;
	}
	
} //namespace mapgeneration_opt
