/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "traceprocessor.h"

namespace mapgeneration
{
	
	TraceProcessor::PathEntry::PathEntry()
	: _connection(0), _backward_connection(0),
		_direction(), _do_not_use_connection(false),
		_equal_state_successors_count(1),
		_interpolated_nodes_in_between(false),
		_is_beginning(false), _is_destination(false), _is_extra_entry(false),
		_is_interpolated(false),
		_node_id(0), _position_on_trace(-1.0), _scan_position(-1.0),
		_score(_INVALID_PATH_SCORE),
		_serializer_id(-1), _serializer_connection_id(-1),
		_state(), _virtual_node_id(_VIRTUAL_NODE_ID_OFFSET - 1)
	{
	}
	
	
/*	TraceProcessor::PathEntry::PathEntry(const PathEntry& entry)
	: _connection(0), _direction(entry._direction),
		_is_beginning(entry._is_beginning),
		_is_destination(entry._is_destination), _is_visited(entry._is_visited),
		_node_id(entry._node_id), _position_on_trace(entry._position_on_trace),
		_scan_position(entry._scan_position), _score(entry._score),
		_state(entry._state), _virtual_node_id(entry._virtual_node_id)
	{
	}
*/	
	
	void
	TraceProcessor::PathEntry::deserialize(std::istream& i_stream)
	{
		Serializer::deserialize(i_stream, _direction);
		Serializer::deserialize(i_stream, _do_not_use_connection);
		Serializer::deserialize(i_stream, _equal_state_successors_count);
		Serializer::deserialize(i_stream, _interpolated_nodes_in_between);
		Serializer::deserialize(i_stream, _is_beginning);
		Serializer::deserialize(i_stream, _is_destination);
		Serializer::deserialize(i_stream, _is_extra_entry);
		Serializer::deserialize(i_stream, _is_interpolated);
		Serializer::deserialize(i_stream, _node_id);
		Serializer::deserialize(i_stream, _position_on_trace);
		Serializer::deserialize(i_stream, _scan_position);
		Serializer::deserialize(i_stream, _score);
		Serializer::deserialize(i_stream, _serializer_id);
		Serializer::deserialize(i_stream, _serializer_connection_id);
		Serializer::deserialize(i_stream, _virtual_node_id);
		
		int state_as_integer;
		Serializer::deserialize(i_stream, state_as_integer);
		if (state_as_integer == 1)
			_state = PathEntry::_REAL;
		else if (state_as_integer == 2)
			_state = PathEntry::_VIRTUAL_CREATED;
		else if (state_as_integer == 3)
			_state = PathEntry::_VIRTUAL_FOUND;
		else
			throw "Invalid state!";
	}
	
	
	TraceProcessor::PathEntry::State
	TraceProcessor::PathEntry::get_state() const
	{
		if (has_state(_REAL))
			return _REAL;
		
		if (has_state(_VIRTUAL_CREATED))
			return _VIRTUAL_CREATED;
		
		if (has_state(_VIRTUAL_FOUND))
			return _VIRTUAL_FOUND;
	}
	
	
	bool
	TraceProcessor::PathEntry::has_state(State state) const
	{
		return (state == _state);
	}
	
	
	bool
	TraceProcessor::PathEntry::is_virtual_node_id(Node::Id node_id)
	{
		return (node_id >= _VIRTUAL_NODE_ID_OFFSET);
	}
	
	
	bool
	TraceProcessor::PathEntry::is_virtual_predecessor_of(
		const PathEntry& path_entry) const
	{
		return (_virtual_node_id + 1 == path_entry._virtual_node_id);
	}
	
	
	bool
	TraceProcessor::PathEntry::is_virtual_successor_of(
		const PathEntry& path_entry) const
	{
		return path_entry.is_virtual_predecessor_of(*this);
	}
	
	
	bool
	TraceProcessor::PathEntry::operator==(const PathEntry& path_entry) const
	{
		// if you change something here, take a look at
		// TraceProcessor::scan_module. And see the remark there!
		return (_direction == path_entry._direction)
			&& (_is_beginning == path_entry._is_beginning)
			&& (_is_destination == path_entry._is_destination)
//			&& (_is_visited == path_entry._is_visited)
			&& (_node_id == path_entry._node_id)
			&& (_position_on_trace == path_entry._position_on_trace)
			&& (_scan_position == path_entry._scan_position)
			&& (_score == path_entry._score)
			&& (_serializer_id == path_entry._serializer_id)
			&& (_serializer_connection_id == path_entry._serializer_connection_id)
			&& (_virtual_node_id == path_entry._virtual_node_id);
	}
	
	
	void
	TraceProcessor::PathEntry::serialize(std::ostream& o_stream) const
	{
		Serializer::serialize(o_stream, _direction);
		Serializer::serialize(o_stream, _do_not_use_connection);
		Serializer::serialize(o_stream, _equal_state_successors_count);
		Serializer::serialize(o_stream, _interpolated_nodes_in_between);
		Serializer::serialize(o_stream, _is_beginning);
		Serializer::serialize(o_stream, _is_destination);
		Serializer::serialize(o_stream, _is_extra_entry);
		Serializer::serialize(o_stream, _is_interpolated);
		Serializer::serialize(o_stream, _node_id);
		Serializer::serialize(o_stream, _position_on_trace);
		Serializer::serialize(o_stream, _scan_position);
		Serializer::serialize(o_stream, _score);
		Serializer::serialize(o_stream, _serializer_id);
		Serializer::serialize(o_stream, _serializer_connection_id);
		Serializer::serialize(o_stream, _virtual_node_id);
		
		int state_as_integer;
		if (_state == PathEntry::_REAL)
			state_as_integer = 1;
		else if (_state == PathEntry::_VIRTUAL_CREATED)
			state_as_integer = 2;
		else if (_state == PathEntry::_VIRTUAL_FOUND)
			state_as_integer = 3;
		else
			throw "Invalid state!";
		Serializer::serialize(o_stream, state_as_integer);
	}
	
	
	std::ostream&
	operator<< (std::ostream& out, const TraceProcessor::PathEntry& entry)
	{
		out << "Entry: "
			<< "ad=" << &entry << ", "
			<< "co=" << std::setw(9) << entry._connection << ", "
			<< "sp=" << std::setw(5) << entry._scan_position << ", "
			<< "pt=" << std::setw(5) << entry._position_on_trace << ", "
			<< "ni=" << std::setw(22) << entry._node_id << ", "
			<< "vi=" << std::setw(22) << entry._virtual_node_id << ", "
			<< "st=";
		
		if (entry.has_state(TraceProcessor::PathEntry::_VIRTUAL_CREATED))
			out << "VC";
		else if (entry.has_state(TraceProcessor::PathEntry::_VIRTUAL_FOUND))
			out << "VF";
		else
			out << "R!";
		
		out << ", sc=";
		
		if (entry._score >= 0.0)
			out << "  ";
		
		out.setf(std::ios::fixed);
		out << entry._score;
		out.unsetf(std::ios::fixed);
		
		return out;
	}
	
	
	std::ostream&
	operator<< (std::ostream& out, const TraceProcessor::D_Path& path)
	{
		out << "Printing Path" << std::endl;
		out << "=============" << std::endl;
		
		TraceProcessor::D_Path::const_iterator iter = path.begin();
		TraceProcessor::D_Path::const_iterator iter_end = path.end();
		for (; iter != iter_end; ++iter)
		{
			if (iter->second->_is_beginning)
			{
				out << "S";
				if (iter->second->_is_destination)
					out << "/D ";
				else
					out << "   ";
			} else
			{
				if (iter->second->_is_destination)
					out << "  D ";
				else
					out << "    ";
			}
			
			out << *(iter->second) << std::endl;
		}
		
		out << "============================================================" << std::endl;
		
		return out;
	}
	
} // namespace mapgeneration
