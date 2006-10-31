/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef QUADTREE_H
#define QUADTREE_H


#include <cassert>
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

#include "axisparallelrectangle.h"
#include "helperfunctions.h"
#include "quadrangle.h"
//#include "trapezoid.h"

#include "util/fixpointvector.h"
#include "util/serializer.h"

using mapgeneration_util::FixpointVector;
using mapgeneration_util::Serializer;

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
					
					
					Item(const AxisParallelRectangle<T_2dPoint>& my_span_rectangle,
						const FixpointVector<T_2dPoint>* quadtree_points);
//					Item(const AxisParallelRectangle<T_2dPoint>& my_span_rectangle,
//						const FixpointVector<const T_2dPoint*>* quadtree_points);
					
					
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
					range_query(const AxisParallelRectangle<T_2dPoint>& query_rectangle,
						const AxisParallelRectangle<T_2dPoint>& span_rectangle,
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
					
					
					AxisParallelRectangle<T_2dPoint> _my_span_rectangle;
					
					
					const FixpointVector<T_2dPoint>* _quadtree_points;
//					const FixpointVector<const T_2dPoint*>* _quadtree_points;
					
			};
			
			
		protected:
			
			struct FastAccess
			{
				T_2dPoint _point;
				Item* _item;
			};
			
		
		//---------------------------------------------------//
		//--- Main class: Quadtree --------------------------//
		//---------------------------------------------------//
		public:
			
			Quadtree();
			
			
			Quadtree(const AxisParallelRectangle<T_2dPoint>& span_rectangle);
			

			~Quadtree();
			

			inline D_IndexType
			add_point(const T_2dPoint& point);
			
			
			inline void
			deserialize(std::istream& i_stream);
			
			
			inline bool
			exists_point(D_IndexType index) const;
			
			
			inline int
			get_max_depth() const;
			
			
			inline const AxisParallelRectangle<T_2dPoint>&
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
			
			
//			inline FixpointVector<T_2dPoint>&
//			points();
			
			
			inline const FixpointVector<T_2dPoint>&
			points() const;
			
			
			inline void
			range_query(const AxisParallelRectangle<T_2dPoint>& query_rectangle,
				std::vector<D_IndexType>& query_result) const;

			
			void
			range_query(const Quadrangle<T_2dPoint>& query_quadrangle,
				std::vector<D_IndexType>& query_result) const;

			
			inline void
			remove_point(D_IndexType index);
			
			
			inline void
			serialize (std::ostream& o_stream) const;
			
			
			inline void
			set_max_depth(int max_depth);
			
			
			inline void
			set_span_rectangle(const AxisParallelRectangle<T_2dPoint>& span_rectangle);
			
			
			inline size_t
			size_of() const;
			
			
		protected:
			
//			FixpointVector<FastAccess> _entries;
			
			
			bool _inited;
			
			
			FixpointVector<Item*> _items;
			
			
			int _max_depth;
			
			
			FixpointVector<T_2dPoint> _points;
//			FixpointVector<const T_2dPoint*> _points;
			
			
			Item* _root;
			
			
			AxisParallelRectangle<T_2dPoint> _span_rectangle;
			
			
			void
			add_point(D_IndexType new_point_index, Item* starting_item,
				bool just_deserializing = false);
			
			
			inline static void
			compute_median_point(
				const AxisParallelRectangle<T_2dPoint>& in_span_rectangle,
				T_2dPoint& out_median_point);
			
			
			inline static void
			compute_new_span_rectangle(int in_quarter,
				const T_2dPoint& in_median_point,
				AxisParallelRectangle<T_2dPoint>& out_span_rectangle);
			
			
			inline static int
			compute_quarter(const T_2dPoint& in_point,
				const AxisParallelRectangle<T_2dPoint>& in_span_rectangle,
				T_2dPoint& out_median_point);
			
			
			bool
			consistency_check() const;
			
			
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
			<< quadtree._span_rectangle.get_point(0)
			<< " - "
			<< quadtree._span_rectangle.get_point(1)
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
			
			out << "| range:  " << item->_my_span_rectangle.get_point(0)
				<< ", " << item->_my_span_rectangle.get_point(1)
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
	Quadtree_Def::Item::Item(
		const AxisParallelRectangle<T_2dPoint>& my_span_rectangle,
		const FixpointVector<T_2dPoint>* quadtree_points)
//		const FixpointVector<const T_2dPoint*>* quadtree_points)
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
		assert(child_pointer != 0);
		
		for(int i = 0; i < 4; ++i)
		{
			if (_my_children[i] == child_pointer)
				return i;
		}
		
		assert(false);
		return -1;
	}
	
	
	Quadtree_Template
	inline int
	Quadtree_Def::Item::find_first_child() const
	{
		assert(_my_no_of_children > 0);
		
		for(int i = 0; i < 4; ++i)
		{
			if (_my_children[i] != 0)
				return i;
		}
		
		assert(false);
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
		const AxisParallelRectangle<T_2dPoint>& query_rectangle,
		const AxisParallelRectangle<T_2dPoint>& span_rectangle,
		std::vector<Quadtree_Type::D_IndexType>& query_result) const
	{
		if (query_rectangle.contains(span_rectangle))
		{
			report_points(query_result);
			
		} else if (query_rectangle.intersects(span_rectangle))
		{
			if (is_leaf())
			{
				assert(!_my_point_indices.empty());
				
				typename D_PointIndices::const_iterator iter
					= _my_point_indices.begin();
				for (; iter != _my_point_indices.end(); ++iter)
				{
					const T_2dPoint& test_point
						= ((*_quadtree_points)[*iter]).second;
//					const T_2dPoint* test_point
//						= ((*_quadtree_points)[*iter]).second;
					
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
						AxisParallelRectangle<T_2dPoint> new_span_rectangle
							= span_rectangle;
						compute_new_span_rectangle(i, median_point,
							new_span_rectangle);
						
						_my_children[i]->range_query(query_rectangle,
							new_span_rectangle, query_result);
					}
				}
			}
		} // end if (query_rectangle....)
	}
	
	
	Quadtree_Template
	void
	Quadtree_Def::Item::report_points(
		std::vector<Quadtree_Type::D_IndexType>& query_result) const
	{
		if (is_leaf())
		{
			assert(!_my_point_indices.empty());
			
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
//	: _entries(), _inited(false), _max_depth(_MIN_DEPTH), _points(), _root(0),
//		_span_rectangle()
	{
	}
	
	
	Quadtree_Template
	Quadtree_Def::Quadtree(
		const AxisParallelRectangle<T_2dPoint>& span_rectangle)
	: _inited(true), _items(), _max_depth(_MIN_DEPTH), _points(), _root(0),
		_span_rectangle(span_rectangle)
//	: _entries(), _inited(true), _max_depth(_MIN_DEPTH), _points(), _root(0),
//		_span_rectangle(span_rectangle)
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
		
		assert(point_index == item_index);
		
		if (point_index != item_index)
		{
			std::cout << "Quadtree::add_point: Will throw an exception now!"
				<< std::endl;
			throw "Quadtree::add_point: indices NOT equal! Problem!!!";
		}
		
		add_point(point_index, _root);
		
		assert(_points[point_index].first);
		assert(_points[point_index].second == point);
		assert(_items[point_index].first);
		assert(_items[point_index].second != 0);
		assert(_items[point_index].second->contains(point_index));
		
		assert(consistency_check());
		
		return point_index;
	}
	
	
	Quadtree_Template
	void
	Quadtree_Def::deserialize(std::istream& i_stream)
	{
		// deserialize _points
		_points.clear();
		Serializer::deserialize(i_stream, _points);
		
		// Uuh, ugly hack. We need ONE data structure for points AND items.
		// Think about the "+ 1". *g*
		_items.clear();
		for (int i = 0; i < _points.size_including_holes() + 1; ++i)
		{
			Item* item = 0;
			_items.insert(item);
		}
		
		assert(_points.size_including_holes() == _items.size_including_holes() - 1);
		
		#warning NOT NICE. If you change FixpointVector's implementation you ruin this hack!
		for (int i = 0; i < _points.free_positions().size(); ++i)
			_items.erase(_points.free_positions()[i]);
		
		_items.erase(_items.size_including_holes() - 1);
		
/*		#ifndef NDEBUG
			assert(_points.size() == _items.size());
			assert(_points.size_including_holes() == _items.size_including_holes());
			
			for (int i = 0; i < _items.size_including_holes(); ++i)
				assert(_items[i].first == _points[i].first);
		#endif*/
		
		
		// calculate _max_depth
		_max_depth = static_cast<int>(ceil(log10(_points.size()) / log10(4)));
		if (_max_depth < _MIN_DEPTH)
			_max_depth = _MIN_DEPTH;
		
		// calculate _span_rectangle
		T_2dPoint lower_left_corner;
		T_2dPoint upper_right_corner;
		Serializer::deserialize(i_stream, lower_left_corner);
		Serializer::deserialize(i_stream, upper_right_corner);
		_span_rectangle.set_point(0, lower_left_corner);
		_span_rectangle.set_point(1, upper_right_corner);
		_span_rectangle.validate_corners();
		
		init_ready();
		
		// okay, now build up the quadtree:
		typename FixpointVector<T_2dPoint>::iterator iter
			= _points.begin();
		typename FixpointVector<T_2dPoint>::iterator iter_end
			= _points.end();
		D_IndexType index = 0;
		for (D_IndexType i = 0; i < _points.size_including_holes(); ++i)
		{
			// Do not use a iterator here. It skips empty entries.
			
			if (_points[i].first)
				add_point(i, _root, true);
		}
		
		assert(consistency_check());
	}
	
	
	Quadtree_Template
	inline bool
	Quadtree_Def::exists_point(Quadtree_Type::D_IndexType index) const
	{
		assert(index >= 0);
		
		return ( (index < _points.size_including_holes()) && (_points[index].first) );
	}
	
	
	Quadtree_Template
	inline int
	Quadtree_Def::get_max_depth() const
	{
		return _max_depth;
	}
	
	
	Quadtree_Template
	inline const AxisParallelRectangle<T_2dPoint>&
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
		assert(from_point_index >= 0);
		assert(from_point_index < _points.size_including_holes());
		assert(_points[from_point_index].first);
		
		if ( !_span_rectangle.contains(to_point) )
		{
			// this quadtree does not cover to_point. YOU are responsible to
			// delete to_point from this quadtree and to insert it to the right
			// one. So really test the return value!
			return false;
		}
		
		assert(_items.size() == _points.size());
		assert(_items.size_including_holes() == _points.size_including_holes());
		assert(_items[from_point_index].first);
		assert(_items[from_point_index].second->contains(from_point_index));
		
		// save item of from_point_index
		Item* from_point_item = item(from_point_index);
		assert(from_point_item == _items[from_point_index].second);
		
		// find from_point_index in item
		std::pair<bool, typename Item::D_PointIndices::iterator> find_result
			= from_point_item->find(from_point_index);
		
		// from_point_item WAS responsible for from_point_index
		// (and it had to be, otherwise something went wrong!!!)
		assert(find_result.first);
		
		if (find_result.first)
		{
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
				assert(!from_point_item->contains(from_point_index));
				
				// insert to to_point_index = from_point_index into the quadtree
				add_point(from_point_index, _root);
				
				assert(_items[from_point_index].first);
				assert(_items[from_point_index].second != from_point_item);
				assert(_items[from_point_index].second->contains(from_point_index));
				
				// repair tree if necessary
				if (from_point_item->_my_point_indices.empty())
					repair_tree(from_point_item, false);
				else if (from_point_item->_my_point_indices.size() == 1)
					repair_tree(from_point_item, true);
			}
			
			assert(consistency_check());
			
			return true;
		} else
		{
			std::string error_msg;
			error_msg.append("Quadtree::move_point: from_point_index does ");
			error_msg.append("not correspond to from_point_item. Something ");
			error_msg.append("was wrong. Consult the programmer.");
			std::cout << error_msg << std::endl;
			
			throw(error_msg);
		}
	}
	
	
	Quadtree_Template
	inline T_2dPoint&
	Quadtree_Def::point(Quadtree_Type::D_IndexType index)
	{
		assert(index >= 0);
		assert(index < _points.size_including_holes());
		assert(_points[index].first);
		assert(_items[index].first);
//		assert(_items[index].second != 0);
//		assert(_items[index].second->contains(index));
		
		return _points[index].second;
	}
	
	
	Quadtree_Template
	inline const T_2dPoint&
	Quadtree_Def::point(Quadtree_Type::D_IndexType index) const
	{
		assert(index >= 0);
		assert(index < _points.size_including_holes());
		assert(_points[index].first);
		assert(_items[index].first);
//		assert(_items[index].second != 0);
//		assert(_items[index].second->contains(index));
		
		return _points[index].second;
	}
	
	
/*	Quadtree_Template
	inline FixpointVector<T_2dPoint>&
	Quadtree_Def::points()
	{
		return _points;
	}*/
	
	
	Quadtree_Template
	inline const FixpointVector<T_2dPoint>&
	Quadtree_Def::points() const
	{
		return _points;
	}
	
	
	Quadtree_Template
	inline void
	Quadtree_Def::range_query(
		const AxisParallelRectangle<T_2dPoint>& query_rectangle,
		std::vector<Quadtree_Type::D_IndexType>& query_result) const
	{
		if (_root != 0)
		{
			_root->range_query(query_rectangle, _span_rectangle, query_result);
		}
	}
	
	
	Quadtree_Template
	void
	Quadtree_Def::range_query(const Quadrangle<T_2dPoint>& quadrangle,
		std::vector<Quadtree_Type::D_IndexType>& query_result) const
	{
		range_query(quadrangle.bounding_box(), query_result);
		
		typename std::vector<D_IndexType>::iterator iter
			= query_result.begin();
		while (iter != query_result.end())
		{
			if (!quadrangle.contains(point(*iter)))
				iter = query_result.erase(iter);
			else
				++iter;
		}
	}
	
	
	Quadtree_Template
	inline void
	Quadtree_Def::remove_point(Quadtree_Type::D_IndexType point_index)
	{
		remove_point(point_index, true);
		
		assert(consistency_check());
	}
	
	
	Quadtree_Template
	void
	Quadtree_Def::serialize(std::ostream& o_stream) const
	{
		assert(consistency_check());
			
		Serializer::serialize(o_stream, _points);
		
		const T_2dPoint& lower_left_corner = _span_rectangle.get_point(0);
		const T_2dPoint& upper_right_corner = _span_rectangle.get_point(1);
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
	Quadtree_Def::set_span_rectangle(
		const AxisParallelRectangle<T_2dPoint>& span_rectangle)
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
		Quadtree_Type::Item* starting_item, bool just_deserializing)
	{
		#warning If program crashes, data may become corrupted. Write a rescue/recovery tool.
		
		if (starting_item == 0)
		{
			// We are definitely at _root. Otherwise the programmer
			// has made a mistake!!!
			_root = new Item(_span_rectangle, &_points);
			_root->_my_point_indices.push_back(new_point_index);
			item(new_point_index) = _root;
			
			assert(_points.size() == _items.size());
			assert(_points.size_including_holes() == _items.size_including_holes());
			assert(_points[new_point_index].first);
			assert(_items[new_point_index].first);
			assert(_items[new_point_index].second != 0);
			assert(_items[new_point_index].second->contains(new_point_index));
			
			return;
		}
		
		Item* current_item = starting_item;
		AxisParallelRectangle<T_2dPoint> current_span_rectangle
			= starting_item->_my_span_rectangle;
		
		// calculate current depth...
		int current_depth = 0;
		Quadtree_Type::Item* test_item = starting_item;
		while (test_item->_my_parent != 0)
		{
			++current_depth;
			test_item = test_item->_my_parent;
		}
		
//		assert(current_depth >= _MIN_DEPTH);
		assert(current_depth <= _max_depth);
		// done
		
		// insert point to quad tree
		while (current_depth < _max_depth)
		{
			if (current_item->is_leaf())
			{
				assert(current_item->_my_point_indices.size() == 1);
				
				typename Item::D_PointIndices::iterator existent_point_index_iter
					= current_item->_my_point_indices.begin();
				D_IndexType existent_point_index = *existent_point_index_iter;
				
				// Otherwise _points.insert(...) failed.
				assert(new_point_index != existent_point_index);
				
				if (new_point_index == existent_point_index)
				{
					// this case would never ever happen.
					// as a precaution an exception is thrown
					std::string error_msg;
					error_msg.append("Quadtree::add_point: it occured an error ");
					error_msg.append("that should never ever occur. ");
					error_msg.append("Ask your programmer or examine the ");
					error_msg.append("code. Thank you :-)");
					std::cout << error_msg << std::endl;
					
					throw error_msg;
					
					return;
					
				} else // explicit: if (new_point_index != existent_point_index)
				{
					current_item->_my_point_indices.erase(existent_point_index_iter);
					
					assert(current_item->_my_point_indices.empty());

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
							AxisParallelRectangle<T_2dPoint> saved_current_span_rectangle
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
							
							assert(_points[existent_point_index].first);
							assert(_items[existent_point_index].first);
							assert(_items[existent_point_index].second
								== current_item->_my_children[existent_point_quarter]);
							assert(_items[existent_point_index].second->contains(existent_point_index));
							
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
							
							assert(_points[new_point_index].first);
							assert(_items[new_point_index].first);
							assert(_items[new_point_index].second
								== current_item->_my_children[new_point_quarter]);
							assert(_items[new_point_index].second->contains(new_point_index));
							
							// and exit this method
							return;
							
						} // end: if (new_point_quarter == existent_point_quarter)
					} // end: while (current_depth < _max_depth)
			
					current_item->_my_point_indices.push_back(new_point_index);
					item(new_point_index) = current_item;
					
					assert(_points[new_point_index].first);
					assert(_items[new_point_index].first);
					assert(_items[new_point_index].second == current_item);
					assert(_items[new_point_index].second->contains(new_point_index));
					
					current_item->_my_point_indices.push_back(existent_point_index);
					item(existent_point_index) = current_item;
					
					assert(_points[existent_point_index].first);
					assert(_items[existent_point_index].first);
					assert(_items[existent_point_index].second == current_item);
					assert(_items[existent_point_index].second->contains(existent_point_index));
					
					assert(_items[existent_point_index].second == _items[new_point_index].second);
			
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
					
					assert(_points[new_point_index].first);
					assert(_items[new_point_index].first);
					assert(_items[new_point_index].second == current_item->_my_children[new_point_quarter]);
					assert(_items[new_point_index].second->contains(new_point_index));
					
					return;
					
				} else // explicit: if (current_item->_children[new_point_quarter] != 0)
				{
					// child already exists: descend the tree...
					current_item = current_item->_my_children[new_point_quarter];
					++current_depth;
				} // end: if (current_item->_children[new_point_quarter] == 0)
			}// end: if (current_item->is_leaf())
		} // end: while (current_depth < _max_depth)
		
		assert(!current_item->contains(new_point_index));
		
		if (current_item->contains(new_point_index))
		{
			// this case would never ever happen.
			// as a precaution an exception is thrown
			std::string error_msg;
			error_msg.append("Quadtree::add_point: it occured an error ");
			error_msg.append("that should never ever occur. ");
			error_msg.append("Ask your programmer or examine the ");
			error_msg.append("code. Thank you :-)");
			std::cout << error_msg << std::endl;
			
			throw error_msg;
			
			return;
		}
		
		current_item->_my_point_indices.push_back(new_point_index);
		item(new_point_index) = current_item;
		
		assert(_points[new_point_index].first);
		assert(_items[new_point_index].first);
		assert(_items[new_point_index].second == current_item);
		assert(_items[new_point_index].second->contains(new_point_index));
		
		return;
	}
	
	
	Quadtree_Template
	inline void
	Quadtree_Def::compute_median_point(
		const AxisParallelRectangle<T_2dPoint>& in_span_rectangle,
		T_2dPoint& out_median_point)
	{
		const T_2dPoint& llc = in_span_rectangle.get_point(0);
		const T_2dPoint& urc = in_span_rectangle.get_point(1);
		
		out_median_point[0] = (urc[0] + llc[0]) / 2.0;
		out_median_point[1] = (urc[1] + llc[1]) / 2.0;
	}
	
	
	Quadtree_Template
	inline void
	Quadtree_Def::compute_new_span_rectangle(int in_quarter,
		const T_2dPoint& in_median_point,
		AxisParallelRectangle<T_2dPoint>& out_span_rectangle)
	{
		T_2dPoint llc = out_span_rectangle.get_point(0);
		T_2dPoint urc = out_span_rectangle.get_point(1);
		
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
		
		out_span_rectangle.set_point(0, llc);
		out_span_rectangle.set_point(1, urc);
		out_span_rectangle.validate_corners();
	}
	
	
	Quadtree_Template
	inline int
	Quadtree_Def::compute_quarter(const T_2dPoint& in_point,
		const AxisParallelRectangle<T_2dPoint>& in_span_rectangle,
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
	bool
	Quadtree_Def::consistency_check() const
	{
		#ifndef NDEBUG
			assert(_points.size() == _items.size());
			assert(_points.size_including_holes() == _items.size_including_holes());
			
/*			for (int i = 0; i < _items.size_including_holes(); ++i)
			{
				assert(_items[i].first == _points[i].first);
				
				if (_items[i].first)
				{
					assert(_items[i].second != 0);
					assert(_items[i].second->contains(i));
				}
			}*/
		#endif
		
		return true;
	}
	
	
	Quadtree_Template
	inline Quadtree_Type::Item*&
	Quadtree_Def::item(Quadtree_Type::D_IndexType index)
	{
		assert(index >= 0);
		assert(index < _items.size_including_holes());
		assert(_items[index].first);
		
		return _items[index].second;
	}
	
	
	Quadtree_Template
	inline const Quadtree_Type::Item*&
	Quadtree_Def::item(Quadtree_Type::D_IndexType index) const
	{
		assert(index >= 0);
		assert(index < _items.size_including_holes());
		assert(_items[index].first);
		
		return _items[index].second;
	}
	
	
	Quadtree_Template
	void
	Quadtree_Def::remove_point(Quadtree_Type::D_IndexType point_index,
		bool repair_now)
	{
		assert(point_index >= 0);
		assert(point_index < _points.size_including_holes());
		
		assert(_items.size() == _points.size());
		assert(_items.size_including_holes() == _points.size_including_holes());
		assert(_items[point_index].first == _points[point_index].first);
		
		if (_points[point_index].first == false)
			return;
		
		assert(_items[point_index].second != 0);
		assert(_items[point_index].second->contains(point_index));
		
		/* find the point to the given point_id and remove it... */
		Quadtree_Type::Item* current_item = item(point_index);
		assert(current_item == _items[point_index].second);
		
		current_item->_my_point_indices.erase(
			current_item->find(point_index).second);
		_points.erase(point_index);
		_items.erase(point_index);
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
		assert(starting_item != 0);
		
		/* ALWAYS remember:
		 * if starting_item->_point_ids.size() > 1, we will NEVER
		 * enter this method!!! */
		assert(starting_item->_my_point_indices.size() <= 1);
		 
		/* We know:
		 * 	- starting_item contains no points,
		 * 	  if and only if starting_item_contains_one point == false
		 * 	- starting_item contains ONE point,
		 * 	  if and only if starting_item_contains_one_point == true */
		if (starting_item_contains_one_point)
			assert(starting_item->_my_point_indices.size() == 1);
		else
			assert(starting_item->_my_point_indices.size() == 0);
		
		if (starting_item == _root)
		{
			if ( !starting_item_contains_one_point )
			{
				assert(_root->_my_point_indices.empty());
				
				delete _root;
				_root = 0;
				
				assert(_points.size() == 0);
				assert(_items.size() == 0);
				
			} else
			{
				assert(_root->_my_point_indices.size() == 1);
				assert(_points.size() == 1);
				assert(_items.size() == 1);
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
