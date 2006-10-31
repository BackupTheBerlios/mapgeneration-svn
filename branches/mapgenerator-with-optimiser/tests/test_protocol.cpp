/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


/**
 * to test the tracefilter
 * todo:
 * - change the lines (document the line) _tile_manager->new_trace(...); with the couts above 
 */

#include <iostream>
#include <fstream>
#include "traceprocessor.h"

using namespace mapgeneration;

int main(int argc, char* argv[])
{
/*	TraceProcessor::Protocol protocol;
	TraceProcessor::PathEntry* entry;
	
	entry = new TraceProcessor::PathEntry;
	entry->_connection = 0;
	entry->_direction = 1.5;
	entry->_is_beginning = true;
	entry->_is_destination = false;
	entry->_is_visited = true;
	entry->_node_id = 100;
	entry->_position_on_trace = 2.5;
	entry->_scan_position = 25.0;
	entry->_score = 1000;
	entry->_state = TraceProcessor::PathEntry::_REAL;
	entry->_virtual_node_id = 1000000;
	
	protocol._path.insert(std::make_pair(entry->_scan_position, entry));
	
	std::ofstream out_file_stream("./test_serialized_protocol");
	Serializer::serialize(out_file_stream, protocol);
	out_file_stream.flush();
	out_file_stream.close();
*/	
	std::ifstream in_file_stream;
	if (argc > 1)
		in_file_stream.open(argv[1]);
	else
		in_file_stream.open("./test_serialized_protocol");
		
	TraceProcessor::Protocol deserialized_protocol;
	Serializer::deserialize(in_file_stream, deserialized_protocol);
	in_file_stream.close();
	
	TraceProcessor::PathEntry* deserialized_entry
		= deserialized_protocol._path.begin()->second;
	
	std::cout << deserialized_protocol._path << std::endl;
	
/*	std::cout << "Entry vs. deserialized Entry" << std::endl;
	std::cout << entry->_connection << " - " << deserialized_entry->_connection << std::endl;
	std::cout << entry->_direction << " - " << deserialized_entry->_direction << std::endl;
	std::cout << entry->_is_beginning << " - " << deserialized_entry->_is_beginning << std::endl;
	std::cout << entry->_is_destination << " - " << deserialized_entry->_is_destination << std::endl;
	std::cout << entry->_is_visited << " - " << deserialized_entry->_is_visited << std::endl;
	std::cout << entry->_node_id << " - " << deserialized_entry->_node_id << std::endl;
	std::cout << entry->_position_on_trace << " - " << deserialized_entry->_position_on_trace << std::endl;
	std::cout << entry->_scan_position << " - " << deserialized_entry->_scan_position << std::endl;
	std::cout << entry->_score << " - " << deserialized_entry->_score << std::endl;
	std::cout << entry->_state << " - " << deserialized_entry->_state << std::endl;
	std::cout << entry->_virtual_node_id << " - " << deserialized_entry->_virtual_node_id << std::endl;
//	std::cout << entry->_ << " - " << deserialized_entry->_ << std::endl;
//	std::cout << entry->_ << " - " << deserialized_entry->_ << std::endl;
//	std::cout << entry->_ << " - " << deserialized_entry->_ << std::endl;
	*/
	return 0;
}
