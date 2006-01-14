/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef QUADTREE_H
#define QUADTREE_H


#include <ostream>

#define Quadtree_Def Quadtree<T_2dPoint>
#define Quadtree_Type typename Quadtree<T_2dPoint>
#define Quadtree_Template template<typename T_2dPoint>

namespace rangereporting
{
	
	//------------------------------------------------------------------------//
	//--- Declaration section ------------------------------------------------//
	//------------------------------------------------------------------------//
	
	Quadtree_Template
	class Quadtree;
	
	
	Quadtree_Template
	std::ostream&
	operator<<(std::ostream& out, const Quadtree_Def& quadtree);
	
}

#include <cmath>
#include <iomanip>
#include <list>
#include <queue>
#include <vector>

#include "helperfunctions.h"
#include "rectangle.h"
#include "trapezoid.h"

#include "util/fixpointvector.h"
using mapgeneration_util::FixpointVector;

/** @todo Should be in the configuration file.
 * But the transferation from the ServiceList over the TraceProcessor to the
 * Tile and onwards to the Quadtree is a very long way. We should think about
 * a better solution. */
#define _MIN_DEPTH 3

namespace rangereporting
{
	
	//------------------------------------------------------------------------//
	//--- Definition section -------------------------------------------------//
	//------------------------------------------------------------------------//
	
	/**
	 * @brief Template class Quadtree provides methods to perform fast range
	 * query on a set of 2-dimensional points. And it is fully dynamical.
	 * 
	 * A quadtree is a well known data structure in computational geometry.
	 * The quadtree uses the basic principle of subdivision to narrow the
	 * search area down. Google provides more information about quadtrees
	 * and geometric data structures in general.
	 * 
	 * A quadtree does not provide logarithmical search time as
	 * a range tree does. But in extensive enquiries I have not found an
	 * appropriate data structure that
	 * <ul>
	 * <li>allows fast queries (e.g. O(log n) with a small constant)</li>
	 * <li>is dynamic, i.e. it is possible to add/delete points in fast manner</li>
	 * <li>uses minimal space; O(n log n) at most</li>
	 * </ul>
	 * ....
	 * 
	 * 
	 */
	Quadtree_Template
	class Quadtree
	{
		
		//---------------------------------------------------//
		//--- Item - Declaration ----------------------------//
		//---------------------------------------------------//
		protected:
			
			class Item;
		
		
		//---------------------------------------------------//
		//--- Typedefs and friends --------------------------//
		//---------------------------------------------------//
		protected:
		
			typedef typename std::pair<bool, Item*> D_BoolItemPair;
			
			
		public:
		
			typedef typename FixpointVector<T_2dPoint>::size_type D_IndexType;
			
			friend std::ostream& operator<< <> (std::ostream& out,
				const Quadtree_Def& quadtree);
			
			
		//---------------------------------------------------//
		//--- Item - Definition -----------------------------//
		//---------------------------------------------------//
		protected:
			
			class Item
			{
				
				friend class Quadtree_Def;
				
				friend std::ostream& operator<< <> (std::ostream& out,
					const Quadtree_Def& quadtree);
				
				
				public:
					
					typedef typename std::list<D_IndexType> D_PointIndices;
					
					
					Item(const Rectangle<T_2dPoint>& my_span_rectangle,
						const FixpointVector<T_2dPoint>* quadtree_points);
					
					
					~Item();
					
					
					bool
					contains(D_IndexType point) const;
					
					
					std::pair<bool, typename D_PointIndices::iterator>
					find(D_IndexType point_index)
					{
						typename D_PointIndices::iterator iter_end
							=_my_point_indices.end();
							
						typename D_PointIndices::iterator find_iter = std::find(
							_my_point_indices.begin(), iter_end, point_index);
						
						if (find_iter != iter_end)
							return std::make_pair(true, find_iter);
						else
							return std::make_pair(false, iter_end);
					}
					
					
					int
					find_child(const Item* child_pointer) const;
					
					
					int
					find_first_child() const;
					
					
					inline bool
					is_leaf() const;
					
					
					void
					range_query(const Rectangle<T_2dPoint>& query_rectangle,
						const Rectangle<T_2dPoint>& span_rectangle,
						std::vector<D_IndexType>& query_result) const;
					
					
					void
					report_points(std::vector<D_IndexType>& query_result) const;
					
					
				protected:
					
					// _children[0] = north east
					// _children[1] = south east
					// _children[2] = south west
					// _children[3] = north west
					std::vector<Item*> _my_children;
					
					
					int _my_no_of_children;
					
					
					Item* _my_parent;
					
					
					D_PointIndices _my_point_indices;
					
					
					Rectangle<T_2dPoint> _my_span_rectangle;
					
					
					const FixpointVector<T_2dPoint>* _quadtree_points;
					
			};
			
		
		//---------------------------------------------------//
		//--- Main class: Quadtree --------------------------//
		//---------------------------------------------------//
		public:
			
			Quadtree();
			
			
			Quadtree(const Rectangle<T_2dPoint>& span_rectangle);
			

			~Quadtree();
			

			inline D_IndexType
			add_point(const T_2dPoint& point);
			
			
			inline void
			deserialize(std::istream& i_stream);
			
			
			inline bool
			exists_point(D_IndexType index) const;
			
			
			inline int
			get_max_depth() const;
			
			
			inline const Rectangle<T_2dPoint>&
			get_span_rectangle() const;
			
			
			inline void
			init_ready();
			
			
			inline bool
			is_inited() const;
			
			
			bool
			move_point(D_IndexType from_id, const T_2dPoint& to_point);
			
			
			inline T_2dPoint&
			point(D_IndexType index);
			
			
			inline const T_2dPoint&
			point(D_IndexType index) const;
			
			
			inline const FixpointVector<T_2dPoint>&
			points() const;
			
			
			inline void
			range_query(const Rectangle<T_2dPoint>& query_rectangle,
				std::vector<D_IndexType>& query_result) const;

			
			void
			range_query(const Trapezoid<T_2dPoint>& query_trapezoid,
				std::vector<D_IndexType>& query_result) const;

			
			inline void
			remove_point(D_IndexType index);
			
			
			inline void
			serialize (std::ostream& o_stream) const;
			
			
			inline void
			set_max_depth(int max_depth);
			
			
			inline void
			set_span_rectangle(const Rectangle<T_2dPoint>& span_rectangle);
			
			
			inline size_t
			size_of() const;
			
			
		protected:
			
			bool _inited;
			
			
			FixpointVector<Item*> _items;
			
			
			int _max_depth;
			
			
			FixpointVector<T_2dPoint> _points;
			
			
			Item* _root;
			
			
			Rectangle<T_2dPoint> _span_rectangle;
			
			
			void
			add_point(D_IndexType new_point_index, Item* starting_item);
			
			
			inline static void
			compute_median_point(const Rectangle<T_2dPoint>& in_span_rectangle,
				T_2dPoint& out_median_point);
			
			
			inline static void
			compute_new_span_rectangle(int in_quarter,
				const T_2dPoint& in_median_point,
				Rectangle<T_2dPoint>& out_span_rectangle);
			
			
			inline static int
			compute_quarter(const T_2dPoint& in_point,
				const Rectangle<T_2dPoint>& in_span_rectangle,
				T_2dPoint& out_median_point);
			
			
			inline Item*&
			item(D_IndexType index);
			
			
			inline const Item*&
			item(D_IndexType index) const;
			
			
			void
			remove_point(D_IndexType index, bool repair_now);
			
			
			void
			repair_tree(Item* starting_item,
				bool starting_item_contains_one_point);
			
	};
	
} //namespace rangereporting


namespace rangereporting
{
	
	//------------------------------------------------------------------------//
	//--- Implementation section ---------------------------------------------//
	//------------------------------------------------------------------------//
	
	//---------------------------------------------------//
	//--- Output operator -------------------------------//
	//---------------------------------------------------//
	Quadtree_Template
	std::ostream&
	operator<<(std::ostream& out, const Quadtree<T_2dPoint>& quadtree)
	{
		out << "Quadtree:" << std::endl;
		out << "=========" << std::endl;
		out << "\trange: "
			<< quadtree._span_rectangle.lower_left_corner()
			<< " - "
			<< quadtree._span_rectangle.upper_right_corner()
			<< std:: endl;
		out << "\troot: " << quadtree._root << std::endl;
		out << std::endl;
		
		// Exit on _root being 0
		if (quadtree._root == 0)
			return out;
		
		std::queue<Quadtree_Type::Item*> queue;
		queue.push(quadtree._root);
		
		char state_0;
		char state_1;
		char state_2;
		char state_3;

		Quadtree_Type::Item* item;
		Quadtree_Type::Item::D_PointIndices::const_iterator iter;
		Quadtree_Type::Item::D_PointIndices::const_iterator iter_end;

		while ( !queue.empty() )
		{
			item = queue.front();
			queue.pop();
			
			if (item->_my_children[0] == 0)
				state_0 = ' ';
			else
				state_0 = 'c';
				
			if (item->_my_children[1] == 0)
				state_1 = ' ';
			else
				state_1 = 'c';
				
			if (item->_my_children[2] == 0)
				state_2 = ' ';
			else
				state_2 = 'c';
				
			if (item->_my_children[3] == 0)
				state_3 = ' ';
			else
				state_3 = 'c';
				
			std::ios_base::fmtflags original_flags(out.flags());
		
			out << "--- Quadtree item (" << item << ") ---" << std::endl;
			
			out << "| range:  " << item->_my_span_rectangle.lower_left_corner()
				<< ", " << item->_my_span_rectangle.upper_right_corner()
				<< std::endl;
			out << "| parent: " << item->_my_parent << std::endl;
			
			out << "|-------------------------------|" << std::endl;
			out << "| NE = " << std::setw(10) << item->_my_children[0] << " |    =====" <<std::endl;
			out << "| SE = " << std::setw(10) << item->_my_children[1] << " |    |" << state_3 << "|" << state_0 << "|" <<std::endl;
			out << "|                "        <<                          " |    |---|" << std::endl;
			out << "| SW = " << std::setw(10) << item->_my_children[2] << " |    |" << state_2 << "|" << state_1 << "|" << std::endl;
			out << "| NW = " << std::setw(10) << item->_my_children[3] << " |    =====" << std::endl;
			out << "|                "        <<                          " |     " << std::endl;
			out << "| Points: {";
			
			out.flags(original_flags);
			
			iter = item->_my_point_indices.begin();
			iter_end = item->_my_point_indices.end();
			for (; iter != iter_end; ++iter)
			{
				out << quadtree.point(*iter);
			}
							
			out << "}" << std::endl;;
			out << "---------------------------------" << std::endl;
			out << std::endl;
			
			for (int i = 0; i < 4; ++i)
			{
				if (item->_my_children[i] != 0)
					queue.push(item->_my_children[i]);
			}
		}
		
		return out;
	}


	//---------------------------------------------------//
	//--- Item ------------------------------------------//
	//---------------------------------------------------//
	Quadtree_Template
	Quadtree_Def::Item::Item(const Rectangle<T_2dPoint>& my_span_rectangle,
		const FixpointVector<T_2dPoint>* quadtree_points)
	: _my_children(4), _my_span_rectangle(my_span_rectangle),
		_my_no_of_children(0), _my_parent(0), _my_point_indices(),
		_quadtree_points(quadtree_points)
	{
		_my_children[0] = 0;
		_my_children[1] = 0;
		_my_children[2] = 0;
		_my_children[3] = 0;
	}
	
	
	Quadtree_Template
	Quadtree_Def::Item::~Item()
	{
		for (int i = 0; i < 4 ;++i)
		{
			if (_my_children[i] != 0)
			{
				delete _my_children[i];
				_my_children[i] = 0;
			}
		}
	}
	
	
	Quadtree_Template
	bool
	Quadtree_Def::Item::contains(Quadtree_Type::D_IndexType point_index) const
	{
		typename D_PointIndices::const_iterator iter_end
			=_my_point_indices.end();
			
		typename D_PointIndices::const_iterator find_iter
			= std::find(_my_point_indices.begin(), iter_end, point_index);
		
		if (find_iter != iter_end)
			return true;
		else
			return false;
	}
	
	
	Quadtree_Template
	int
	Quadtree_Def::Item::find_child(const Item* child_pointer) const
	{
		for(int i = 0; i < 4; ++i)
		{
			if (_my_children[i] == child_pointer)
				return i;
		}
		
		return -1;
	}
	
	
	Quadtree_Template
	inline int
	Quadtree_Def::Item::find_first_child() const
	{
		for(int i = 0; i < 4; ++i)
		{
			if (_my_children[i] != 0)
				return i;
		}
		
		return -1;
	}
	
	
	Quadtree_Template
	inline bool
	Quadtree_Def::Item::is_leaf() const
	{
		return _my_no_of_children == 0;
	}
	
	/** @todo denk mal uebers span_rectangle nach!!! */
	/** @todo denk mal uebers span_rectangle nach!!! */
	/** @todo denk mal uebers span_rectangle nach!!! */
	/** @todo denk mal uebers span_rectangle nach!!! */
	/** @todo denk mal uebers span_rectangle nach!!! */
	/** @todo denk mal uebers span_rectangle nach!!! */
	/** @todo denk mal uebers span_rectangle nach!!! */
	/** @todo denk mal uebers span_rectangle nach!!! */
	/** @todo denk mal uebers span_rectangle nach!!! */
	/** @todo denk mal uebers span_rectangle nach!!! */
	/** @todo denk mal uebers span_rectangle nach!!! */
	/** @todo denk mal uebers span_rectangle nach!!! */
	/** @todo denk mal uebers span_rectangle nach!!! */
	/** @todo denk mal uebers span_rectangle nach!!! */
	/** @todo denk mal uebers span_rectangle nach!!! */
	/** @todo denk mal uebers span_rectangle nach!!! */
	/** @todo denk mal uebers span_rectangle nach!!! */
	/** @todo denk mal uebers span_rectangle nach!!! */
	/** @todo denk mal uebers span_rectangle nach!!! */
	/** @todo denk mal uebers span_rectangle nach!!! */
	Quadtree_Template
	void
	Quadtree_Def::Item::range_query(
		const Rectangle<T_2dPoint>& query_rectangle,
		const Rectangle<T_2dPoint>& span_rectangle,
		std::vector<Quadtree_Type::D_IndexType>& query_result) const
	{
		if (query_rectangle.contains(span_rectangle))
		{
			report_points(query_result);
			
		} else if (query_rectangle.intersects(span_rectangle))
		{
			if (is_leaf())
			{
				typename D_PointIndices::const_iterator iter
					= _my_point_indices.begin();
				for (; iter != _my_point_indices.end(); ++iter)
				{
					const T_2dPoint& test_point
						= ((*_quadtree_points)[*iter]).second;
					
					if (query_rectangle.contains(test_point))
						query_result.push_back(*iter);
				}
			} else
			{
				T_2dPoint median_point;
				compute_median_point(span_rectangle, median_point);
				
				for (int i = 0; i < 4 ;++i)
				{
					if (_my_children[i] != 0)
					{
						Rectangle<T_2dPoint> new_span_rectangle
							= span_rectangle;
						compute_new_span_rectangle(i, median_point,
							new_span_rectangle);
						
						_my_children[i]->range_query(query_rectangle,
							new_span_rectangle, query_result);
					}
				}
			}
		}
	}
	
	
	Quadtree_Template
	void
	Quadtree_Def::Item::report_points(
		std::vector<Quadtree_Type::D_IndexType>& query_result) const
	{
		if (is_leaf())
		{
			typename D_PointIndices::const_iterator iter
				= _my_point_indices.begin();
			for (; iter != _my_point_indices.end(); ++iter)
			{
				query_result.push_back(*iter);
			}
			
		} else
		{
			for (int i = 0; i < 4; ++i)
			{
				if (_my_children[i] != 0)
				{
					_my_children[i]->report_points(query_result);
				}
			}
		}
	}
	
	
	
	//---------------------------------------------------//
	//--- Main class: Quadtree --------------------------//
	//---------------------------------------------------//
	Quadtree_Template
	Quadtree_Def::Quadtree()
	: _inited(false), _items(), _max_depth(_MIN_DEPTH), _points(), _root(0),
		_span_rectangle()
	{
	}
	
	
	Quadtree_Template
	Quadtree_Def::Quadtree(const Rectangle<T_2dPoint>& span_rectangle)
	: _inited(true), _items(), _max_depth(_MIN_DEPTH), _points(), _root(0),
		_span_rectangle(span_rectangle)
	{
	}


	Quadtree_Template
	Quadtree_Def::~Quadtree()
	{
		delete _root;
	}
	
	/** @todo item und point gleichzeitig einfügen!!!!!!!!!! */
	/** @todo item und point gleichzeitig einfügen!!!!!!!!!! */
	/** @todo item und point gleichzeitig einfügen!!!!!!!!!! */
	/** @todo item und point gleichzeitig einfügen!!!!!!!!!! */
	/** @todo item und point gleichzeitig einfügen!!!!!!!!!! */
	/** @todo item und point gleichzeitig einfügen!!!!!!!!!! */
	/** @todo item und point gleichzeitig einfügen!!!!!!!!!! */
	/** @todo item und point gleichzeitig einfügen!!!!!!!!!! */
	/** @todo item und point gleichzeitig einfügen!!!!!!!!!! */
	/** @todo item und point gleichzeitig einfügen!!!!!!!!!! */
	/** @todo item und point gleichzeitig einfügen!!!!!!!!!! */
	/** @todo item und point gleichzeitig einfügen!!!!!!!!!! */
	/** @todo item und point gleichzeitig einfügen!!!!!!!!!! */
	/** @todo item und point gleichzeitig einfügen!!!!!!!!!! */
	/** @todo item und point gleichzeitig einfügen!!!!!!!!!! */
	Quadtree_Template
	inline Quadtree_Type::D_IndexType
	Quadtree_Def::add_point(const T_2dPoint& point)
	{
		D_IndexType point_index = _points.insert(point);
		D_IndexType item_index = _items.insert(0);
		
		if (point_index != item_index)
			throw "Quadtree::add_point: indices NOT equal! Problem!!!";
		
		add_point(point_index, _root);
		
		return point_index;
	}
	
	
	Quadtree_Template
	void
	Quadtree_Def::deserialize(std::istream& i_stream)
	{
		// deserialize _points
		Serializer::deserialize(i_stream, _points);
		
		// calculate _max_depth
		_max_depth = static_cast<int>(ceil(log10(_points.size()) / log10(4)));
		if (_max_depth < _MIN_DEPTH)
			_max_depth = _MIN_DEPTH;
		
		// calculate _span_rectangle
		T_2dPoint lower_left_corner;
		T_2dPoint upper_right_corner;
		Serializer::deserialize(i_stream, lower_left_corner);
		Serializer::deserialize(i_stream, upper_right_corner);
		_span_rectangle.set_corners(lower_left_corner, upper_right_corner);
		
		init_ready();
		
		// okay, now build up the quadtree:
		typename FixpointVector<T_2dPoint>::iterator iter
			= _points.begin();
		typename FixpointVector<T_2dPoint>::iterator iter_end
			= _points.end();
		D_IndexType index = 0;
		while (iter != iter_end)
		{
			_items.insert(0);
			
			if ((*iter).first)
				add_point(index, _root);
			
			++iter;
			++index;
		}
		
	}
	
	
	Quadtree_Template
	inline bool
	Quadtree_Def::exists_point(Quadtree_Type::D_IndexType index) const
	{
		return _points[index].first;
	}
	
	
	Quadtree_Template
	inline int
	Quadtree_Def::get_max_depth() const
	{
		return _max_depth;
	}
	
	
	Quadtree_Template
	inline const Rectangle<T_2dPoint>&
	Quadtree_Def::get_span_rectangle() const
	{
		return _span_rectangle;
	}
	
	
	Quadtree_Template
	inline void
	Quadtree_Def::init_ready()
	{
		_inited = true;
	}
	
	
	Quadtree_Template
	inline bool
	Quadtree_Def::is_inited() const
	{
		return _inited;
	}
	
	
	Quadtree_Template
	bool
	Quadtree_Def::move_point(Quadtree_Type::D_IndexType from_point_index,
		const T_2dPoint& to_point)
	{
		if ( !_span_rectangle.contains(to_point) )
		{
			// this quadtree does not cover to_point. YOU are responsible to
			// delete to_point from this quadtree and to insert it to the right
			// one. So really test the return value!
			return false;
		}
		
		// save item of from_point_index
		Item* from_point_item = item(from_point_index);
		
		// find from_point_index in item
		std::pair<bool, typename Item::D_PointIndices::iterator> find_result
			= from_point_item->find(from_point_index);
		
		if (find_result.first)
		{
			// from_point_item WAS responsible for from_point_index
			// (and it had to be, otherwise something went wrong!!!)
			
			// update _points: now from_point is to_point
			// BUT: Only update the coodinates. Otherwise we will destroy
			// other (important) information from from_point!!!
			T_2dPoint& from_point = point(from_point_index);
			from_point[0] = to_point[0];
			from_point[1] = to_point[1];
			// done.
			
			// now check if from_point_item is NOT responsible for to_point
			if ( !from_point_item->_my_span_rectangle.contains(to_point) )
			{
				// then remove from_point_index from from_point_item
				from_point_item->_my_point_indices.erase(find_result.second);
				
				// insert to to_point_index = from_point_index into the quadtree
				add_point(from_point_index, _root);
				
				// repair tree if necessary
				if (from_point_item->_my_point_indices.empty())
					repair_tree(from_point_item, false);
				else if (from_point_item->_my_point_indices.size() == 1)
					repair_tree(from_point_item, true);
			}
			
			return true;
		} else
		{
			std::string error_msg;
			error_msg.append("Quadtree::move_point: from_point_index does ");
			error_msg.append("not correspond to from_point_item. Something ");
			error_msg.append("was wrong. Consult the programmer.");
			throw(error_msg);
		}
	}
	
	
	Quadtree_Template
	inline T_2dPoint&
	Quadtree_Def::point(Quadtree_Type::D_IndexType index)
	{
		return _points[index].second;
	}
	
	
	Quadtree_Template
	inline const T_2dPoint&
	Quadtree_Def::point(Quadtree_Type::D_IndexType index) const
	{
		return _points[index].second;
	}
	
	
	Quadtree_Template
	inline const FixpointVector<T_2dPoint>&
	Quadtree_Def::points() const
	{
		return _points;
	}
	
	
	Quadtree_Template
	inline void
	Quadtree_Def::range_query(const Rectangle<T_2dPoint>& query_rectangle,
		std::vector<Quadtree_Type::D_IndexType>& query_result) const
	{
		if (_root != 0)
		{
			_root->range_query(query_rectangle, _span_rectangle, query_result);
		}
	}
	
	
	Quadtree_Template
	void
	Quadtree_Def::range_query(const Trapezoid<T_2dPoint>& trapezoid,
		std::vector<Quadtree_Type::D_IndexType>& query_result) const
	{
		range_query(trapezoid.bounding_rectangle(), query_result);
		
		typename std::vector<D_IndexType>::iterator iter
			= query_result.begin();
		while (iter != query_result.end())
		{
			bool erased_point = false;
			
			for (int i = 0; i < 4; ++i)
			{
				int relative_pos =
					relative_position(trapezoid[i % 4], trapezoid[(i+1) % 4],
						point(*iter));
				if (relative_pos < 0)
				{
					iter = query_result.erase(iter);
					i = 4; // break the loop!
					erased_point = true;
				}
			}
			
			if ( !erased_point )
				++iter;
		}
	}
	
	
	Quadtree_Template
	inline void
	Quadtree_Def::remove_point(Quadtree_Type::D_IndexType point_index)
	{
		remove_point(point_index, true);
	}
	
	
	Quadtree_Template
	void
	Quadtree_Def::serialize(std::ostream& o_stream) const
	{
		Serializer::serialize(o_stream, _points);
		
		const T_2dPoint& lower_left_corner
			= _span_rectangle.lower_left_corner();
		const T_2dPoint& upper_right_corner
			=  _span_rectangle.upper_right_corner();
		Serializer::serialize(o_stream, lower_left_corner);
		Serializer::serialize(o_stream, upper_right_corner);
	}
	
	
	Quadtree_Template
	inline void
	Quadtree_Def::set_max_depth(int max_depth)
	{
		if ( !is_inited() )
			_max_depth = max_depth;
	}
	
	
	Quadtree_Template
	inline void
	Quadtree_Def::set_span_rectangle(const Rectangle<T_2dPoint>& span_rectangle)
	{
		if ( !is_inited() )
			_span_rectangle = span_rectangle;
	}
	
	
	Quadtree_Template
	inline size_t
	Quadtree_Def::size_of() const
	{
		size_t the_size = sizeof(Quadtree<T_2dPoint>);
		
		the_size += _items.size_of();
		the_size += _points.size_of();
		
		return the_size;
	}
	
	
		//---------------------------------------------------//
		//--- Main class: protected methods -----------------//
		//---------------------------------------------------//
	
	Quadtree_Template
	void
	Quadtree_Def::add_point(Quadtree_Type::D_IndexType new_point_index,
		Quadtree_Type::Item* starting_item)
	{
		if (starting_item == 0)
		{
			// We are definitely at _root. Otherwise the programmer
			// has made a mistake!!!
			_root = new Item(_span_rectangle, &_points);
			_root->_my_point_indices.push_back(new_point_index);
			item(new_point_index) = _root;
			
			return;
		}
		
		Item* current_item = starting_item;
		Rectangle<T_2dPoint> current_span_rectangle
			= starting_item->_my_span_rectangle;
		
		// calculate current depth...
		int current_depth = 0;
		Quadtree_Type::Item* test_item = starting_item;
		while (test_item->_my_parent != 0)
		{
			++current_depth;
			test_item = test_item->_my_parent;
		}
		// done
		
		// insert point to quad tree
		while (current_depth < _max_depth)
		{
			if (current_item->is_leaf())
			{
				typename Item::D_PointIndices::iterator existent_point_index_iter
					= current_item->_my_point_indices.begin();
				D_IndexType existent_point_index = *existent_point_index_iter;
				
				if (new_point_index == existent_point_index)
				{
					// this case would never ever happen.
					// as a precaution an exception is thrown
					std::string error_msg;
					error_msg.append("Quadtree::add_point: it occured an error ");
					error_msg.append("that should never ever occur. ");
					error_msg.append("Ask your programmer or examine the ");
					error_msg.append("code. Thank you :-)");
					throw error_msg;
					
					return;
					
				} else // explicit: if (new_point_index != existent_point_index)
				{
					current_item->_my_point_indices.erase(existent_point_index_iter);

					while (current_depth < _max_depth)
					{
						T_2dPoint median_point;
						int new_point_quarter = compute_quarter(
							point(new_point_index), current_span_rectangle,
							median_point);
						int existent_point_quarter = compute_quarter(
							point(existent_point_index), current_span_rectangle,
							median_point);
						
						if (new_point_quarter == existent_point_quarter)
						{
							compute_new_span_rectangle(new_point_quarter,
								median_point, current_span_rectangle);
							current_item->_my_children[new_point_quarter]
								= new Item(current_span_rectangle, &_points);
							current_item->_my_children[new_point_quarter]->_my_parent
								= current_item;
							
							++(current_item->_my_no_of_children);
							current_item = current_item->_my_children[new_point_quarter];
							++current_depth;
							
						} else // explicit: if (new_point_quarter != existent_point_quarter)
						{
							// quarters are different: save point indices:
							
							// the existent point
							Rectangle<T_2dPoint> saved_current_span_rectangle
								= current_span_rectangle;
							compute_new_span_rectangle(existent_point_quarter,
								median_point, saved_current_span_rectangle);
							current_item->_my_children[existent_point_quarter]
								= new Item(saved_current_span_rectangle, &_points);
							current_item->_my_children[existent_point_quarter]->_my_parent
								= current_item;
							++(current_item->_my_no_of_children);
							current_item->_my_children[existent_point_quarter]
								->_my_point_indices.push_back(existent_point_index);
							
							// finally update the item pointer in _points!
							item(existent_point_index)
								= current_item->_my_children[existent_point_quarter];
							
							
							// the new point
							compute_new_span_rectangle(new_point_quarter,
								median_point, current_span_rectangle);
							current_item->_my_children[new_point_quarter]
								= new Item(current_span_rectangle, &_points);
							current_item->_my_children[new_point_quarter]->_my_parent
								= current_item;
							++(current_item->_my_no_of_children);
							current_item->_my_children[new_point_quarter]
								->_my_point_indices.push_back(new_point_index);
							
							// finally insert the item pointer in _points!
							item(new_point_index)
								= current_item->_my_children[new_point_quarter];
							
							// and exit this method
							return;
							
						} // end: if (new_point_quarter == existent_point_quarter)
					} // end: while (current_depth < _max_depth)
			
					current_item->_my_point_indices.push_back(new_point_index);
					item(new_point_index) = current_item;
					
					current_item->_my_point_indices.push_back(existent_point_index);
					item(existent_point_index) = current_item;
			
					return;
					
				} // end: if (new_point_id == existent_point_id)
				
			} else // explicit: if ( !current_item->is_leaf() )
			{
				T_2dPoint median_point;
				int new_point_quarter = compute_quarter(point(new_point_index),
					current_span_rectangle, median_point);
				
				compute_new_span_rectangle(new_point_quarter, median_point,
					current_span_rectangle);
				
				if (current_item->_my_children[new_point_quarter] == 0)
				{
					// this child is 0: insert point and be happy!
					current_item->_my_children[new_point_quarter]
						= new Item(current_span_rectangle, &_points);
					current_item->_my_children[new_point_quarter]->_my_parent
						= current_item;
					++(current_item->_my_no_of_children);
					
					current_item->_my_children[new_point_quarter]
						->_my_point_indices.push_back(new_point_index);
					
					item(new_point_index)
						= current_item->_my_children[new_point_quarter];
					
					return;
					
				} else // explicit: if (current_item->_children[new_point_quarter] != 0)
				{
					// child already exists: descend the tree...
					current_item = current_item->_my_children[new_point_quarter];
					++current_depth;
				} // end: if (current_item->_children[new_point_quarter] == 0)
			}// end: if (current_item->is_leaf())
		} // end: while (current_depth < _max_depth)
		
		if (current_item->contains(new_point_index))
		{
			// this case would never ever happen.
			// as a precaution an exception is thrown
			std::string error_msg;
			error_msg.append("Quadtree::add_point: it occured an error ");
			error_msg.append("that should never ever occur. ");
			error_msg.append("Ask your programmer or examine the ");
			error_msg.append("code. Thank you :-)");
			throw error_msg;
			
			return;
		}
		
		current_item->_my_point_indices.push_back(new_point_index);
		item(new_point_index) = current_item;
		
		return;
	}
	
	
	Quadtree_Template
	inline void
	Quadtree_Def::compute_median_point(
		const Rectangle<T_2dPoint>& in_span_rectangle,
		T_2dPoint& out_median_point)
	{
		const T_2dPoint& llc = in_span_rectangle.lower_left_corner();
		const T_2dPoint& urc = in_span_rectangle.upper_right_corner();
		
		out_median_point[0] = (urc[0] + llc[0]) / 2.0;
		out_median_point[1] = (urc[1] + llc[1]) / 2.0;
	}
	
	
	Quadtree_Template
	inline void
	Quadtree_Def::compute_new_span_rectangle(int in_quarter,
		const T_2dPoint& in_median_point,
		Rectangle<T_2dPoint>& out_span_rectangle)
	{
		T_2dPoint& llc = out_span_rectangle.lower_left_corner();
		T_2dPoint& urc = out_span_rectangle.upper_right_corner();
		
		switch (in_quarter)
		{
			case 0:
				llc[0] = in_median_point[0];
				llc[1] = in_median_point[1];
				break;
			
			case 1:
				llc[0] = in_median_point[0];
				urc[1] = in_median_point[1];
				break;
			
			case 2:
				urc[0] = in_median_point[0];
				urc[1] = in_median_point[1];
				break;
			
			case 3:
				urc[0] = in_median_point[0];
				llc[1] = in_median_point[1];
				break;
		}
	}
	
	
	Quadtree_Template
	inline int
	Quadtree_Def::compute_quarter(const T_2dPoint& in_point,
		const Rectangle<T_2dPoint>& in_span_rectangle,
		T_2dPoint& out_median_point)
	{
		compute_median_point(in_span_rectangle, out_median_point);
		
		if (in_point[0] >= out_median_point[0])
		{
			if (in_point[1] >= out_median_point[1])
				return 0;
			else
				return 1;

		} else
		{
			if (in_point[1] >= out_median_point[1])
				return 3;
			else
				return 2;
		}
	}
	
	
	Quadtree_Template
	inline Quadtree_Type::Item*&
	Quadtree_Def::item(Quadtree_Type::D_IndexType index)
	{
		return _items[index].second;
	}
	
	
	Quadtree_Template
	inline const Quadtree_Type::Item*&
	Quadtree_Def::item(Quadtree_Type::D_IndexType index) const
	{
		return _items[index].second;
	}
	
	
	Quadtree_Template
	void
	Quadtree_Def::remove_point(Quadtree_Type::D_IndexType point_index,
		bool repair_now)
	{
		/* find the point to the given point_id and remove it... */
		Quadtree_Type::Item* current_item = item(point_index);
		current_item->_my_point_indices.erase(
			current_item->find(point_index).second);
		_points.erase(point_index);
		/* done! */
		
		/* if current_item is "empty", repair the tree if desired... */
		if (repair_now)
		{
			if (current_item->_my_point_indices.empty())
				repair_tree(current_item, false);
			else if (current_item->_my_point_indices.size() == 1)
				repair_tree(current_item, true);
		}
		/* done */
	}
	
	
	Quadtree_Template
	void
	Quadtree_Def::repair_tree(Quadtree_Type::Item* starting_item,
		bool starting_item_contains_one_point)
	{
		/* ALWAYS remember:
		 * if starting_item->_point_ids.size() > 1, we will NEVER
		 * enter this method!!! */
		 
		/* We know:
		 * 	- starting_item contains no points,
		 * 	  if and only if starting_item_contains_one point == false
		 * 	- starting_item contains ONE point,
		 * 	  if and only if starting_item_contains_one_point == true */
		
		if (starting_item == _root)
		{
			if ( !starting_item_contains_one_point )
			{
				delete _root;
				_root = 0;
			}
			
			return;
		}
		
		/* Delete starting item... */
		Quadtree_Type::Item* parent_item = starting_item->_my_parent;
		int child_id = parent_item->find_child(starting_item);
		
		if ( !starting_item_contains_one_point )
		{
			delete (parent_item->_my_children[child_id]);
			parent_item->_my_children[child_id] = 0;
			--(parent_item->_my_no_of_children);
		}
		/* done */
		
		/* Find siblings of starting_item.
		 * If only one sibling is found, it is part of a chain which can
		 * be shorted now. Then we have to find the other end of the chain.
		 * 
		 * But step-by-step... */
		if (parent_item->_my_no_of_children == 1)
		{
			/* find the sibling... */
			child_id = parent_item->find_first_child();
			/* done */
			
			/* test if that sibling is a leaf... */
			if (parent_item->_my_children[child_id]->is_leaf())
			{
				/* it is a leaf: we can short the chain... */
				
				/* save pointer */
				Quadtree_Type::Item* end_of_chain_item
					= parent_item->_my_children[child_id];
				
				/* Break the tree. That is necessary, when applying the
				 * "delete" operator (cf. below) */
				parent_item->_my_children[child_id] = 0;
				
				/* Now search the start of the chain... */
				Quadtree_Type::Item* start_of_chain_item = parent_item;
				Quadtree_Type::Item* previous_start_of_chain_item
					= end_of_chain_item;
				
				bool exited_on_children = false;
				while (start_of_chain_item->_my_parent != 0)
				{
					previous_start_of_chain_item = start_of_chain_item;
					start_of_chain_item = start_of_chain_item->_my_parent;
					
					if (start_of_chain_item->_my_no_of_children > 1)
					{
						exited_on_children = true;
						break;
					}
				}
				
				/* Did we exit because of meeting _root->_parent? */
				if ( !exited_on_children )
				{
					/* We did! */
					end_of_chain_item->_my_span_rectangle
						= _root->_my_span_rectangle;
					end_of_chain_item->_my_parent = 0;
					
					delete _root;
					
					_root = end_of_chain_item;
					
				} else
				{
					/* No, we did't.
					 * We found an "ordinary" start */
					
					/* search for the pointer to start_of_chain_item's child... */
					child_id = start_of_chain_item->find_child(
						previous_start_of_chain_item);
					/* we found the pointer */
					
					/* delete that item (that will automatically delete
					 * every descendant; end_of_chain_item exclusively) */
					delete start_of_chain_item->_my_children[child_id];
					
					/* link end_of_chain_item to start_of_chain_item->_children[j]... */
					start_of_chain_item->_my_children[child_id] = end_of_chain_item;
					end_of_chain_item->_my_parent = start_of_chain_item;
					
					/* set correct end_of_chain_item->_my_span_rectangle */
					end_of_chain_item->_my_span_rectangle
						= start_of_chain_item->_my_span_rectangle;
					
					T_2dPoint median_point;
					compute_median_point(start_of_chain_item->_my_span_rectangle,
						median_point);
					compute_new_span_rectangle(child_id, median_point,
						end_of_chain_item->_my_span_rectangle);
					/* done */
				}
				
				/* if there are more than one point in end_of_chain_item, split
				 * that item... */
				/** @todo DO NOT USE std::list::size()!!! */
				if (end_of_chain_item->_my_point_indices.size() > 1)
				{
					typename Item::D_PointIndices::iterator begin_splice_iter
						= end_of_chain_item->_my_point_indices.begin();
					++begin_splice_iter;
					
					typename Item::D_PointIndices points;
					points.splice(points.begin(),
						end_of_chain_item->_my_point_indices, begin_splice_iter,
						end_of_chain_item->_my_point_indices.end());
					
					typename Item::D_PointIndices::iterator iter = points.begin();
					typename Item::D_PointIndices::iterator iter_end = points.end();
					
					for (; iter != iter_end; ++iter)
					{
						add_point(*iter, end_of_chain_item);
					}
				}
				/* done */
				
				/* repair complete! */
				
			} // end: if (parent_item->_children[j]->_is_leaf())
		} // end: if (parent_item->_no_of_children == 1)
	}
	
} //namespace rangereporting

#endif //QUADTREE_H
