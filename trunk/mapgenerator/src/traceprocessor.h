/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef TRACEPROCESSOR_H
#define TRACEPROCESSOR_H

namespace mapgeneration
{
	class TraceProcessor;
}

#include <deque>
#include <cc++/thread.h>
#include "filteredtrace.h"
#include "node.h"
#include "tile.h"
#include "tilemanager.h"
//#include "tracelogwriter.h"
#include "util/constants.h"
#include "util/fixedsizequeue.h"
#include "util/mlog.h"
#include "util/pubsub/servicesystem.h"


namespace mapgeneration
{

	/**
	 * @brief TraceProcessor process a new trace and include it into the
	 * existing map.
	 */
	class TraceProcessor : public ost::Thread
	{
		
		
		private:
		
			class PathConnection;
		
		
			class PathEntry
			{
				public:
				
					Node::Id _node_id;
					Node _node_copy;
					double _position;
					int _virtual_node_id;
					
					bool _is_destination;
					double _points;
					int _time_stamp;
					PathEntry* _connection;
					
//					D_RangeReporting::Id _range_id;
					Node::Id _range_id;
					
					
					PathEntry&
					operator=(const PathEntry& p)
					{
						_node_id = p._node_id;
						_node_copy = p._node_copy;
						_position = p._position;
						_virtual_node_id = p._virtual_node_id;
						_points = p._points;
						_time_stamp = p._time_stamp;
						_connection = p._connection;
						_range_id = p._range_id;
						
						return *this;
					}
					
					
					bool
					operator==(PathEntry path_entry)
					{
						if (_node_id == path_entry._node_id)
							return true;
						else
							return false;
					}
					
					
					PathEntry()
					: _position(0), _node_id(0), _connection(0), _time_stamp(0),
						_node_copy(), _is_destination(false),
						_virtual_node_id(0), _range_id(0)
					{
					}


					PathEntry(const double position, const Node::Id node_id)
					: _position(position), _node_id(node_id), _connection(0), _time_stamp(0),
						_node_copy(), _is_destination(false), 
						_virtual_node_id(0), _range_id(0)
					{
					}
					
					
					PathEntry(const PathEntry& p)
					: _node_id(p._node_id),
						_node_copy(p._node_copy),
						_position(p._position),
						_virtual_node_id(p._virtual_node_id),
						_points(p._points),
						_time_stamp(p._time_stamp),
						_connection(p._connection),
						_is_destination(p._is_destination),					
						_range_id(p._range_id)
					{
					}
					
			};

		
		
		public:	

			/**
			 * @brief Constructor that transfers the necessary inforamtion to run
			 * the TraceProcessor.
			 */
			TraceProcessor (unsigned int id, TileManager* tile_manager, 
				pubsub::ServiceList* service_list, 
				FilteredTrace& filtered_trace);
			
			
			/**
			 * @brief The run method. Mainly the cluster algorithm.
			 */
			void
			run();

		
			/**
			 * @brief a constant for the angle difference
			 * 
			 * @todo This value has to move to a central place (configuration file).
			 */
//			static const double _ANGLE_DIFFERENCE = PI / 4.0; // was 30??
			
			
			/**
			 * @brief a constant for the maximum angle difference between two
			 * following nodes
			 * 
			 * @todo This value has to move to a central place (configuration file).
			 */
//			static const double _MAX_ANGLE_DIFFERENCE_BETWEEN_FOLLOWING_NODES = PI * 0.9;
			
			
			/**
			 * @brief a constant for the search radius (in meters)
			 * 
			 * @todo This value has to move to a central place (configuration file).
			 */
//			static const double _SEARCH_RADIUS = 24.0; // was 16m
		
		
		private:
		
		
			/*
			 * We start with an extra section for all the parameters:
			 * *****************************************************
			 */

			double _search_step_size_meters;
			
			
			double _search_radius_m;
			
			
			double _search_max_angle_difference_pi;
			
			

			/*
			 * Now the ordinary members:
			 * *************************
			 */
			
			/**
			 * @brief the FilteredTrace the TraceProcessor is processing
			 */
			FilteredTrace _filtered_trace;
			
			
			/**
			 * @brief the ID
			 */
			unsigned int _id;
			
			
			/**
			 * @brief The list of already processed nodes.
			 */
			std::list< std::pair <Node::Id, double> > _processed_nodes;
			
			
			/**
			 * @brief The service list.
			 */
			pubsub::ServiceList* _service_list;
			
			
			/**
			 * @brief Pointer to the TileCache
			 */
			TileCache* _tile_cache;
			
			
			/**
			 * @brief Pointer to the TileManager
			 */
			TileManager* _tile_manager;
			
			
			/**
			 * @brief A counter to set unique time stamps in build_connections.
			 */
			int _time;
			
			
			/**
			 * @brief The TraceProcessorLogger for this TraceProcessor
			 */
//			TraceLogWriter* _trace_log;
			
			
			/**
			 * @brief Recursive function to calculate the best path.
			 * 
			 * @param path A reference to the main algorithms path.
			 * @param path_iter The iterator of the path entry to start from.
			 * @param only_connected True if only connections should be checked,
			 * false otherwise.
			 * 
			 * @return The points of the best path from the given path_iter to
			 * the last entry of the path.
			 */
			double
			build_connections(std::list<PathEntry>& path,
				std::list<PathEntry>::iterator path_iter, 
				double previous_direction);
				
				
			void
			build_finished_segment(std::list<PathEntry>& path,
				std::list<PathEntry>& finished_segment,	PathEntry* start_entry);


			/**
			 * @brief Calculates all clusterable nodes and adds them to the
			 * given vector.
			 * 
			 * @param geo_coordinate The position to start from.
			 * @param result_vector The vector to store the information into.
			 */
			void
			calculate_cluster_nodes(GPSPoint gps_point,
				std::list<Node::Id>& result_vector);
			

			/**
			 * @brief Connects the nodes with the given ids.
			 * 
			 * @param first_node_id The id of the first node.
			 * @param second_node_id You guess it...
			 */
			void
			connect_nodes(Node::Id first_node_id, Node::Id second_node_id);
			
			
			/**
			 * @brief Checks of the nodes with the given ids are connected.
			 * 
			 * @return True if the nodes with are 
			 * connected, false otherwise.
			 */
			bool
			connection_from_to(Node::Id node_id_1, Node::Id node_id_2);
			
			
			void
			create_connection(
				double& completed_position_m, Node::Id& previous_node_id, 
				double start_position, Node::Id destination_id);			


			void
			create_disconnection(
				double& completed_position_m, Node::Id& previous_node_id, 
				Node::Id start_id, double destination_position);		


			/**
			 * @brief Creates a new Node.
			 * 
			 * @param gps_point the GPSPoint the Node is created from.
			 * 
			 * @return Id of the new created node.
			 */
			Node::Id
			create_new_node(GPSPoint& gps_point);
			
			
			/**
			 * @brief Creates new nodes from the given completed_position_m to 
			 * the end_position_m.
			 * 
			 * @param completed_position_m A reference to the main algorithms'
			 * completed_position_m.
			 * 
			 * @param end_position_m The position at which to stop creating
			 * nodes.
			 * 
			 * @param use_end_position If true the last node is created at
			 * end_position_m, if false the last node is created one step before
			 * end_position_m.
			 * 
			 * @param previous_node_id A reference to the main algorithms'
			 * previous_node_id.
			 */
			void
			create_nodes(double& completed_position_m, double end_position_m,
				bool use_end_position, Node::Id& previous_node_id);


			/**
			 * @brief Removes all nodes from the processed nodes list with positions
			 * smaller than min_position_m
			 * 
			 * @param min_position_m Minimal node position not to erase.
			 */
			void
			cut_processed_nodes(double position_m);			
			
			
			/**
			 * @brief Inserts the node_id and position into the processed nodes 
			 * list.
			 * 
			 * @param node_id The id to insert.
			 * @param position_m The position to insert.
			 */
			void
			insert_into_processed_nodes(Node::Id node_id, double position_m);
			
			
			/**
			 * @brief Calculates the point of the trace in meters that 
			 * optimaly corresponds to the path_entry.
			 * 
			 * @return The optimal position in meters.
			 */
			double
			optimal_node_position(PathEntry path_entry);
				
	
			/**
			 * @brief Returns true if the given node id is found in the processed nodes
			 * list.
			 * 
			 * @param node_id Node::Id to search for.
			 * @return True if the node id is found in the processed nodes.
			 */
			bool
			search_in_processed_nodes(Node::Id node_id);
			
			
			/**
			 * @brief Calculated the best path using the nodes in the path
			 * variable.
			 * 
			 * simplify_path starts one or more instances of the recursive
			 * build_connections method to calculate the best possible
			 * path starting at the previous_node_id and ending at the last
			 * entry of the path.
			 * 
			 * @param previous_node_id Id of the node at which the path has to
			 * start. May be zero = invalid.
			 * @param path A reference to the main algorithms' path.
			 * @param finished_segments A reference to the list that should
			 * contain the calculated segments.
			 */
			void
			simplify_path(Node::Id previous_node_id, 
				std::list<PathEntry>& path,
				std::list<PathEntry>& finished_segments);
			
			
			/**
			 * @brief Returns a TileCache::Pointer to the tile the node
			 * with the given id is on.
			 * 
			 * @return TileCache::Pointer to the tile the node is on.
			 */
			TileCache::Pointer
			tile(Node::Id node_id);
			
			
			
			/**
			 * @brief Uses the finished_segments list calculated by
			 * simplify_path and build_segments to merge the current trace
			 * to the map.
			 * 
			 * @param finished_segments A reference to the segment list that 
			 * has to be processed.
			 * @param complete_position_m A reference to the main
			 * algorithms' complete_position_m.
			 * @param A reference to the previous_node_id of the main algorithm.
			 */
			void
			use_segment(std::list<PathEntry>& finished_segment,
				double& complete_position_m, Node::Id& previous_node_id);
	};
	
	
} // namespace mapgeneration

#endif //TRACEPROCESSOR_H
