/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef TRACEPROCESSOR_H
#define TRACEPROCESSOR_H

#include <set>
#include <cc++/thread.h>
//#include <ext/hash_set>
#include "filteredtrace.h"
#include "node.h"
//#include "tile.h"
//#include "tilemanager.h"
//#include "util/constants.h"
//#include "util/mlog.h"
//#include "util/pubsub/servicesystem.h"
#include "util/rangereporting/quadtree.h"

/* Forward declarations... */
namespace pubsub
{
	
	class ServiceList;
	
} // namespace pubsub

namespace mapgeneration
{
	
	class TileCache;
	class TileManager;
	
} // namespace mapgeneration
/* Forward declarations done! */

#include "tilecache.h"

namespace mapgeneration
{

	/**
	 * @brief TraceProcessor process a new trace and include it into the
	 * existing map.
	 */
	class TraceProcessor : public ost::Thread
	{
		
		public:
			
			class PathEntry;
			
			
			
		public:

			typedef std::multimap<double, PathEntry*> D_Path;
			typedef std::multimap<double, PathEntry> D_PathHardCopy;

			// whatever you do: _INVALID_PATH_SCORE must be smaller than 
			// _WORST_SCORE
			static const double _BEST_SCORE = 1000000000.0;
			static const double _WORST_SCORE = 0.0;
			static const double _INVALID_PATH_SCORE = - 1.0;
			static const Node::Id _VIRTUAL_NODE_ID_OFFSET = 0xFFFFFFFF00000001ULL;
		
/*	const double TraceProcessor::_BEST_SCORE = 10000000.0;
	const double TraceProcessor::_WORST_SCORE = -10000000.0;
	const double TraceProcessor::_INVALID_PATH_SCORE = _WORST_SCORE - 1.0;
	const Node::Id TraceProcessor::_VIRTUAL_NODE_ID_OFFSET = 0xFFFFFFFF00000001ULL;
*/		
		
		public:
			
			static const int _CONFIGURATION_VALUES = 30;
			
			enum ConfigurationValue
			{
				_STEP_DISTANCE_M = 0,
				
				_SEARCH_MAX_DISTANCE_M,
				_SEARCH_MAX_ANGLE_DIFFERENCE_PI,
				
				_SUFFICIENT_PATH_LENGTH,
				_FAREST_START_POSITION,
				
				_BASE_SCORE_DISTANT_START,
				_BASE_SCORE_DIRECTION_DIFFERENCE,
				_BASE_SCORE_NEXT_DIRECTION_DIFFERENCE,
				_BASE_SCORE_PERPENDULAR_DISTANCE,
				_BASE_SCORE_R2R,
				_BASE_SCORE_R2VC,
				_BASE_SCORE_R2VF,
				_BASE_SCORE_VC2R,
				_BASE_SCORE_VC2VC,
				_BASE_SCORE_VC2VF,
				_BASE_SCORE_VF2R,
				_BASE_SCORE_VF2VC,
				_BASE_SCORE_VF2VF,
				_BASE_SCORE_R2R_STEP_DISTANCE,
				_BASE_SCORE_R2VC_STEP_DISTANCE,
				_BASE_SCORE_R2VF_STEP_DISTANCE,
				_BASE_SCORE_VC2R_STEP_DISTANCE,
				_BASE_SCORE_VC2VC_STEP_DISTANCE,
				_BASE_SCORE_VC2VF_STEP_DISTANCE,
				_BASE_SCORE_VF2R_STEP_DISTANCE,
				_BASE_SCORE_VF2VC_STEP_DISTANCE,
				_BASE_SCORE_VF2VF_STEP_DISTANCE,
				
				_BASE_SCORE_R2R_NOT_REACHABLE,
				_BASE_SCORE_VC2VC_NO_SUCCESSOR,
				_BASE_SCORE_VF2VF_NO_SUCCESSOR
			};

			
			class Configuration
			{
				public:
					
					Configuration(pubsub::ServiceList* service_list);
					
					
					double
					get(ConfigurationValue cv) const;
					
					template <typename T_ReturnType>
					void
					get(ConfigurationValue cv, T_ReturnType& output) const;
					
					const std::vector<std::string>&
					identifier_strings() const;
					
					
//				private:
					
					// at the moment a double array is sufficient.
					std::vector<double> _values;
					std::vector<std::string> _identifier_strings;
					
			};
			
			
			struct CrossingItem
			{
				int _predecessors;
				int _successors;
				
				bool _has_new_predecessors;
				bool _has_new_successors;
				
//				PathEntry* _previous_entry;
				PathEntry* _entry;
			};
			
			
			class Hash_uint64_t
			{
				public:
					
					size_t
					operator()(uint64_t value) const
					{
						return value;
					}
			};
			
			
			struct PathDeletion
			{
				Node::Id _start_id;
				Node::Id _destination_id;
				Node::Id _first_successor;
			};
			
			
			class PathEntry
			{
				public:
					
					// first encountered virtual nodes are nodes that were
					// created directly from the trace in every beginning of
					// the great loop.
					enum State
					{
						_REAL,
						_VIRTUAL_FOUND,
						_VIRTUAL_CREATED
					};
				
					PathEntry*	_connection;
					PathEntry*	_backward_connection;
					Direction	_direction;
					bool		_do_not_use_connection;
					int			_equal_state_successors_count;
					bool		_interpolated_nodes_in_between;
					bool		_is_beginning;
					bool		_is_destination;
					bool		_is_extra_entry;
					bool		_is_interpolated;
					Node::Id 	_node_id;
					double		_position_on_trace; // only needed for virtual entries!!!
					double		_scan_position;
					double		_score;
					mutable int			_serializer_id;
					mutable int			_serializer_connection_id;
					State		_state;
					Node::Id	_virtual_node_id;
					
					
					PathEntry();
					
	//				PathEntry(const PathEntry& path_entry);
					
					void
					deserialize(std::istream& i_stream);
					
					State
					get_state() const;
					
					bool
					has_state(State state) const;
					
					static bool
					is_virtual_node_id(Node::Id node_id);
					
					bool
					is_virtual_predecessor_of(const PathEntry& path_entry) const;
					
					bool
					is_virtual_successor_of(const PathEntry& path_entry) const;
					
					bool
					operator==(const PathEntry& entry) const;
					
					void
					serialize(std::ostream& o_stream) const;
					
			};
			
			
			class Protocol
			{
				public:
					
					bool _auto_delete;
					FilteredTrace _trace;
					
					mutable std::set<Tile::Id> _used_tile_ids;
					mutable std::vector<Tile*> _used_tiles;
					mutable TileCache* _tile_cache;
					#warning Bad hacks!
					
					D_Path _path;
					
					Protocol(bool auto_delete = true);
					
					~Protocol();
					
					void
					delete_all_path_entries();
					
					void
					deserialize(std::istream& i_stream);
					
					void
					serialize (std::ostream& o_stream) const;
					
			};
			
			
			class VirtualNode : public GPSPoint
			{
				public:
					
					double _position_on_trace;
					
					
					VirtualNode();
						
					VirtualNode(const GPSPoint& point);
					
					
					VirtualNode&
					operator=(const GeoCoordinate& point);
					
					VirtualNode&
					operator=(const GPSPoint& point);
					
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

		
		private:
			
			static const double _MIN_STEP_DISTANCE_M;
			static const double _MAX_CURVATURE;
			static const double _STEP_DISTANCE_FACTOR;
			static const int _MAX_DEPTH;
			static const int _MAX_INVALID_ENTRIES_COUNTER;
			static const double _MAX_INVALID_ENTRIES_DISTANCE;
			
//			typedef __gnu_cxx::hash_set<Node::Id, TraceProcessor::Hash_uint64_t> D_NodeIds;
			typedef std::set<Node::Id> D_NodeIds;
//			typedef std::multimap<double, Node::Id> T_EncounteredNodesPositions;
			typedef rangereporting::Quadtree<VirtualNode> D_VirtualEntries;
			typedef D_VirtualEntries::D_IndexType D_IndexType;
			
			
			TraceProcessor::Configuration _conf;
			
			
			/**
			 * @brief the FilteredTrace the TraceProcessor is processing
			 */
			FilteredTrace _filtered_trace;
			
			
			D_NodeIds _found_real_node_ids;
			
			
			/**
			 * @brief the ID of this traceprocessor
			 */
			unsigned int _id;
			
			
			D_Path _path;
			
			
			Protocol _protocol;
			
			
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
			
			
			D_VirtualEntries _virtual_entries;
			
			
			
//			void
//			append_to_protocol(T_Path& path);
			
			
			bool
			apply_module();
			
			
			
			void
			ai_module();
			
			
			/**
			 * @brief Recursive function to calculate the best path.
			 * 
			 * @param path A reference to the main algorithms path.
			 * @param start_entry The iterator of the path entry to start from.
			 * @param previous_direction The previous direction.
			 * 
			 * @return The points of the best path from the given start_iter to
			 * the last entry (the destination) of the path.
			 */
			double
			build_best_connections_recursively(D_Path::iterator start_iter,
				int similar_nodes_in_row, PathEntry::State similar_nodes_state/*,
				D_NodeIds& this_recursion_virtual_found_node_ids*/);
			
			
			void
			build_crossings_mapper(
				std::multimap<Node::Id, CrossingItem>& crossings_mapper,
				const std::map<Node::Id, CrossingItem>& excluded_crossings,
				bool do_not_verify = false) const;
			
			
			/**
			 * @brief Calculates all clusterable nodes and adds them to the
			 * given vector.
			 * 
			 * @param geo_coordinate The position to start from.
			 * @param result The vector to store the information into.
			 */
			void
			calculate_cluster_nodes(const GeoCoordinate& start_gc,
				const GeoCoordinate& end_gc, std::list<Node::Id>& result) const;
			
			
			double
			calculate_connection_score(PathEntry* start_entry,
				PathEntry* end_entry);
			
			
			void
			connect_nodes(Node::Id from_node_id, Node::Id to_node_id);
			
			
			bool
			consistency_check(TileCache::Pointer pointer) const;
			
			
			// return the first successor, i.e. the first interpolated node.
			Node::Id
			create_extra_nodes_on_connection(const Node::Id from_node_id,
				const Node::Id to_node_id, double step_distance_m);
			
			
			void
			create_extra_nodes_on_connection(PathEntry* const from_entry,
				PathEntry* const to_entry, double step_distance_m);
			
			
			void
			create_extra_nodes_on_connections(
				const std::multimap<Node::Id, CrossingItem>& crossings_mapper);
			
			
//			void
//			create_extra_nodes_at_connections();
			
			
			void
			delete_ways(const std::multimap<int, PathDeletion>& ways_for_deletion,
				std::set<Node::Id>& deleted_node_ids, 
				bool delete_all = false);
			
			
			void
			disconnect_nodes(Node::Id from_node_id, Node::Id to_node_id);
			
			
			bool
			exists_multiply_ways_in_map(Node::Id start_id,
				Node::Id destination_id, int max_depth) const;
			
			
			bool
			exists_way_in_map(const Node::Id start_id,
				const Node::Id destination_id, Node::Id recursion_start_id,
				int max_depth) const;
			
			
			bool
			exists_way_in_path(const PathEntry* entry, Node::Id destination_id,
				Node::Id first_successor, int max_depth, int* steps = 0) const;
			
			
			bool
			find_and_set_most_fitting_position_on_trace_for(
				PathEntry* path_entry);
			
			
			/**
			 * @brief Removes all nodes from the processed nodes list with
			 * positions smaller than position_m.
			 * 
			 * @param position_m Node position that is not erased.
			 */
//			void
//			cut_encountered_nodes(T_EncounteredNodeIds& eni,
//				T_EncounteredNodesPositions& enp, double position_m) const;
			
			
//			void
//			handle_connection(
//				std::multimap<Node::Id, PathEntry*>& node_ids_entries_mapper,
//				PathEntry* const start_entry, PathEntry* const end_entry);
			
			
			void
			handle_double_ways(
				const std::multimap<Node::Id, CrossingItem>& crossings_mapper);
			
			
			void
			handle_loops(
				const std::multimap<Node::Id, CrossingItem>& crossings_mapper);
			
			
			void
			handle_parallel_autobahn_ways(
				const std::multimap<Node::Id, CrossingItem>& crossings_mapper);
			
			
//			void
//			handle_double_connections();
			
			
			void
			insert_entry_into_path(PathEntry* entry);
			
			
			void
			insert_extra_entries_into_path();
			
			
//			double
//			length_m_of(const D_Path& path) const;
			
			
			void
			merge(std::multimap<Node::Id, PathEntry*>& node_ids_entries_mapper,
				PathEntry* entry, PathEntry* corresponding_virtual_entry = 0);
			
			
			double
			minimal_direction_difference_between(Node::Id from_node_id,
				Node::Id over_node_id, Node::Id to_node_id) const;
			
			
//			bool
//			path_contains_entry(const PathEntry* entry) const;
			
			
			bool
			path_contains_id_in_interval(Node::Id, double interval_start,
				double interval_end) const;
			
			
			bool
			path_is_valid(PathEntry* start_entry,
				bool should_repair = false);
			
			
//			Node::Id
//			register_new_node(const Node& node);
			Node::Id
			register_new_node(const GeoCoordinate& gc);
			
			
			void
			scan_module();
			
			
			bool
			search_and_set_beginning();
			
			
			bool
			search_and_set_ending();
			
			
			bool
			search_and_use_alterative_path_entries(PathEntry* valid_start_entry,
				PathEntry* valid_end_entry);
			
			
			bool
			search_parallel_ways(Node::Id node_id, Node::Id& destination_id,
				Node::Id& first_successor) const;
			
			
			double
			search_perpendicular(const GeoCoordinate& test_gc,
				double start_position_m, double end_position_m,
				double min_position_diff) const;
			
			
			std::pair<bool, PathEntry*>
			sequence_found(D_Path::iterator iter, PathEntry* first_entry,
				PathEntry* last_entry, PathEntry::State state);
			
			
			void
			serialize_protocol();
			
			
//			void
//			smooth_connections();
			
			
			void
			smooth_connections(
				const std::multimap<Node::Id, CrossingItem>& crossings_mapper);
			
			
			bool
			verify_node_bearings(Node::Id node_id) const;
			
			
			
// -------------------------------------------------------------------------- //
// OLD METHODS OLD METHODS OLD METHODS OLD METHODS OLD METHODS OLD METHODS OL //
// -------------------------------------------------------------------------- //
			
			/**
			 * @brief Build finished segment.
			 *
			 * The following steps are performed:
			 * <ul>
			 * <li>erase finished_segment</li>
			 * <li>start at start_entry</li>
			 * <li>and walk on the path entering the entries into
			 * finished_segment<li>
			 * <li>the result is given back in finished_segment.</li>
			 * </ul>
			 *
			 * @param path A reference to the main algorithm's path.
			 * @param finished_segment A reference to the finished segments.
			 *	A list of PathEntrys.
			 * @param start_entry The start entry.
			 */
/*			void
			build_finished_segment(std::list<PathEntry>& path,
				std::list<PathEntry>& finished_segment,	PathEntry* start_entry);*/


			/**
			 * @brief Connects the nodes with the given ids.
			 * 
			 * @param first_node_id The id of the first node.
			 * @param second_node_id You guess it...
			 */
/*			void
			connect_nodes(Node::Id first_node_id, Node::Id second_node_id);*/
			
			
			/**
			 * @brief Checks of the nodes with the given ids are connected.
			 * 
			 * @return True if the nodes with are connected, false otherwise.
			 */
/*			bool
			connection_from_to(Node::Id node_id_1, Node::Id node_id_2);*/
			
			
			/**
			 * @brief Creates a straight connection from start_position to
			 * destination_position.
			 *
			 * In contrast to create_nodes which follows the FilteredTrace.
			 *
			 * @see create_nodes
			 *
			 * @param completed_position_m The main algorithm's
			 * completed_position_m. It is needed inside for the call of
			 * insert_into_processed_nodes.
			 * @param previous_node_id The previous Node::Id. Used to make the
			 * first connection.
			 * @param start_position The start position in meter on the
			 * FilteredTrace. Used to calculate the first Node::Id which is
			 * created.
			 * @param destination_id The destination Node::Id.
			 */
/*			void
			create_connection(
				double& completed_position_m, Node::Id& previous_node_id, 
				double start_position, Node::Id destination_id);*/
			
			
			/**
			 * @brief Does the same as create_connection.
			 */
/*			void
			create_disconnection(
				double& completed_position_m, Node::Id& previous_node_id, 
				Node::Id start_id, double destination_position);*/


			/**
			 * @brief Creates new nodes from the given completed_position_m to 
			 * the end_position_m.
			 *
			 * In contrast to create_connection this method follows the
			 * shape of the FilteredTrace.
			 *
			 * @see create_connection
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
/*			void
			create_nodes(double& completed_position_m, double end_position_m,
				bool use_end_position, Node::Id& previous_node_id);*/


			/**
			 * @brief Calculates the point of the trace in meters that 
			 * optimaly corresponds to the path_entry. By approximating the
			 * perpendicular (german: Lot).
			 * 
			 * @return The optimal position in meters.
			 */
/*			double
			optimal_node_position(PathEntry path_entry);*/
				
	
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
/*			void
			simplify_path(Node::Id previous_node_id, 
				std::list<PathEntry>& path,
				std::list<PathEntry>& finished_segments);*/
			
			
			/**
			 * @brief Returns a TileCache::Pointer to the tile the node
			 * with the given id is on.
			 * 
			 * @return TileCache::Pointer to the tile the node is on.
			 */
/*			TileCache::Pointer
			tile(Node::Id node_id);*/
			
			
			
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
/*			void
			use_segment(std::list<PathEntry>& finished_segment,
				double& complete_position_m, Node::Id& previous_node_id);*/
	};
	
	
	
	std::ostream&
	operator<< (std::ostream& out, const TraceProcessor::D_Path& path);
	
	
	std::ostream&
	operator<< (std::ostream& out, const TraceProcessor::PathEntry& entry);
	
	
	std::ostream&
	operator<< (std::ostream& out, const TraceProcessor::Protocol& protocol);
	
	
} // namespace mapgeneration

#endif //TRACEPROCESSOR_H
