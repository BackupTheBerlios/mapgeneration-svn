/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef QUADTREE_H
#define QUADTREE_H

#include <ostream>

#define Quadtree_Def Quadtree<T_Id, T_Point, T_Id_Point_Mapper>
#define Quadtree_Type typename Quadtree<T_Id, T_Point, T_Id_Point_Mapper>

namespace rangereporting
{
	
	//-------------------------------------------------------------------------//
	//--- Declaration section -------------------------------------------------//
	//-------------------------------------------------------------------------//
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	class Quadtree;
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	std::ostream&
	operator<<(std::ostream& out, const Quadtree_Def& quadtree);
	
}

#include <iomanip>
#include <list>
#include <queue>
#include <vector>

#include "helperfunctions.h"
#include "rectangle.h"
#include "trapezoid.h"

//#include "util/mlog.h"
//using mapgeneration_util::MLog;

namespace rangereporting
{
	
	//-------------------------------------------------------------------------//
	//--- Definition section --------------------------------------------------//
	//-------------------------------------------------------------------------//
	
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
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	class Quadtree
	{
		
		friend std::ostream& operator<< <> (std::ostream& out,
			const Quadtree_Def& quadtree);
		
		//---------------------------------------------------//
		//--- Id --------------------------------------------//
		//---------------------------------------------------//
		protected:
			
			class Item;
			
		public:
			
			class Id
			{
				
				friend class Quadtree_Def;
				
				public:
					
					inline Id(const T_Id& id, Item* item);
					
					
					inline const T_Id&
					operator*() const;
					
					
				protected:
					
					T_Id _id;
					
					
					Item* _item;
					
			};
			
			
		//---------------------------------------------------//
		//--- Item ------------------------------------------//
		//---------------------------------------------------//
		protected:
			
			class Item
			{
				
				friend class Quadtree_Def;
				
				friend std::ostream& operator<< <> (std::ostream& out,
					const Quadtree_Def& quadtree);
				
				
				public:
					
					typedef std::list<T_Id> D_Points;
					
					inline Item(const Quadtree_Def& quadtree,
						const Rectangle<T_Point>& my_span_rectangle);
					
					
					~Item();
					
					
					inline bool
					contains(const T_Id& point_id) const;
					
					
					inline std::pair<bool, typename D_Points::iterator>
					find(const T_Id& point_id);
					
					
					inline int
					find_child(const Item* const child_pointer) const;
					
					
					inline int
					find_first_child() const;
					
					
					inline bool
					is_leaf() const;
					
					
					void
					range_query(const Rectangle<T_Point>& query_rectangle,
						const Rectangle<T_Point>& span_rectangle,
						std::vector<Quadtree_Type::Id>& query_result) const;
					
					
					void
					report_points(std::vector<Quadtree_Type::Id>& query_result) const;
					
					
				protected:
					
					// _children[0] = north east
					// _children[1] = south east
					// _children[2] = south west
					// _children[3] = north west
					std::vector<Item*> _children;
					
					
					Rectangle<T_Point> _my_span_rectangle;
					
					
					int _no_of_children;
					
					
					Item* _parent;
					
					
					D_Points _point_ids;
					
					
					const Quadtree_Def& _quadtree;
					
			};
			
		
		//---------------------------------------------------//
		//--- Main class: Quadtree --------------------------//
		//---------------------------------------------------//
		public:
			
			inline
			Quadtree();
			
			
			inline
			Quadtree(int max_depth,
				const Rectangle<T_Point>& span_rectangle,
				const T_Id_Point_Mapper* points);
			

			inline
			~Quadtree();
			

			inline bool
			add_point(const T_Id& id);
			
			
			inline int
			get_max_depth() const;
			
			
			inline const T_Id_Point_Mapper*
			get_points() const;
			
			
			inline const Rectangle<T_Point>&
			get_span_rectangle() const;
			
			
			inline void
			init_ready();
			
			
			inline bool
			is_inited() const;
			
			
			bool
			move_point(Quadtree_Type::Id& from_point, const T_Point& to_point);
			
			
			inline void
			range_query(const Rectangle<T_Point>& query_rectangle,
				std::vector<Quadtree_Type::Id>& query_result) const;

			
			void
			range_query(const Trapezoid<T_Point>& query_trapezoid,
				std::vector<Quadtree_Type::Id> & query_result) const;

			
			inline bool
			remove_point(Quadtree_Type::Id& point);
			
			
			inline void
			set_max_depth(int max_depth);
			
			
			inline void
			set_points(const T_Id_Point_Mapper* points);
			
			
			inline void
			set_span_rectangle(const Rectangle<T_Point>& span_rectangle);
			
			
		protected:
			
			bool _inited;
			
			
			int _max_depth;
			
			
			const T_Id_Point_Mapper* _points;
			
			
			Item* _root;
			
			
			Rectangle<T_Point> _span_rectangle;
			
			
			bool
			add_point(const T_Id& id, Item* starting_item);
			
			
			inline static void
			compute_median_point(const Rectangle<T_Point>& in_span_rectangle,
				T_Point& out_median_point);
			
			
			inline static void
			compute_new_span_rectangle(int in_quarter,
				const T_Point& in_median_point,
				Rectangle<T_Point>& out_span_rectangle);
			
			
			inline static int
			compute_quarter(const T_Point& in_point,
				const Rectangle<T_Point>& in_span_rectangle,
				T_Point& out_median_point);
			
			
			bool
			remove_point(Quadtree_Type::Id& point,
				bool repair_now);
			
			
			void
			repair_tree(Quadtree_Type::Item* starting_item,
				bool starting_item_contains_one_point);
			
	};
	
	
	
	//-------------------------------------------------------------------------//
	//--- Implementation section ----------------------------------------------//
	//-------------------------------------------------------------------------//
	
	//---------------------------------------------------//
	//--- Output operator -------------------------------//
	//---------------------------------------------------//
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	std::ostream&
	operator<<(std::ostream& out,
		const Quadtree<T_Id, T_Point, T_Id_Point_Mapper>& quadtree)
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
		
		/* Exit on _root being 0 */
		if (quadtree._root == 0)
			return out;
		/* Exit on _root being 0 */
		
		std::queue<Quadtree_Type::Item*> queue;
		queue.push(quadtree._root);
		
		char state_0;
		char state_1;
		char state_2;
		char state_3;

		Quadtree_Type::Item* item;
		Quadtree_Type::Item::D_Points::const_iterator iter;
		Quadtree_Type::Item::D_Points::const_iterator iter_end;

		while ( !queue.empty() )
		{
			item = queue.front();
			queue.pop();
			
			if (item->_children[0] == 0)
				state_0 = ' ';
			else
				state_0 = 'c';
				
			if (item->_children[1] == 0)
				state_1 = ' ';
			else
				state_1 = 'c';
				
			if (item->_children[2] == 0)
				state_2 = ' ';
			else
				state_2 = 'c';
				
			if (item->_children[3] == 0)
				state_3 = ' ';
			else
				state_3 = 'c';
				
			std::ios_base::fmtflags original_flags(out.flags());
		
			out << "--- Quadtree item (" << item << ") ---" << std::endl;
			
			out << "| range:  " << item->_my_span_rectangle.lower_left_corner()
				<< ", " << item->_my_span_rectangle.upper_right_corner()
				<< std::endl;
			out << "| parent: " << item->_parent << std::endl;
			
			out << "|-------------------------------|" << std::endl;
			out << "| NE = " << std::setw(10) << item->_children[0] << " |    =====" <<std::endl;
			out << "| SE = " << std::setw(10) << item->_children[1] << " |    |" << state_3 << "|" << state_0 << "|" <<std::endl;
			out << "|                "        <<                       " |    |---|" << std::endl;
			out << "| SW = " << std::setw(10) << item->_children[2] << " |    |" << state_2 << "|" << state_1 << "|" << std::endl;
			out << "| NW = " << std::setw(10) << item->_children[3] << " |    =====" << std::endl;
			out << "|                "        <<                       " |     " << std::endl;
			out << "| Points: {";
			
			out.flags(original_flags);
			
			iter = item->_point_ids.begin();
			iter_end = item->_point_ids.end();
			for (; iter != iter_end; ++iter)
			{
				out << (*quadtree._points)[*iter];
			}
							
			out << "}" << std::endl;;
			out << "---------------------------------" << std::endl;
			out << std::endl;
			
			for (int i = 0; i < 4; ++i)
			{
				if (item->_children[i] != 0)
					queue.push(item->_children[i]);
			}
		}
		
		return out;
	}
	


	//---------------------------------------------------//
	//--- Id --------------------------------------------//
	//---------------------------------------------------//
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	inline
	Quadtree_Def::Id::Id(const T_Id& id, Quadtree_Type::Item* item)
	: _id(id), _item(item)
	{
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	inline const T_Id&
	Quadtree_Def::Id::operator*() const
	{
		return _id;
	}
	
	
	
	//---------------------------------------------------//
	//--- Item ------------------------------------------//
	//---------------------------------------------------//
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	inline
	Quadtree_Def::Item::Item(const Quadtree_Def& quadtree,
		const Rectangle<T_Point>& my_span_rectangle)
	: _children(4), _my_span_rectangle(my_span_rectangle), _no_of_children(0),
		_parent(0), _point_ids(), _quadtree(quadtree)
	{
		_children[0] = 0;
		_children[1] = 0;
		_children[2] = 0;
		_children[3] = 0;
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	Quadtree_Def::Item::~Item()
	{
		for (int i = 0; i < 4 ;++i)
		{
			if (_children[i] != 0)
			{
				delete _children[i];
				_children[i] = 0;
			}
		}
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	inline bool
	Quadtree_Def::Item::contains(const T_Id& point_id) const
	{
		Quadtree_Type::Item::D_Points::const_iterator end_iter = _point_ids.end();
		Quadtree_Type::Item::D_Points::const_iterator find_iter
			=  std::find(_point_ids.begin(), end_iter, point_id);
		
		if (find_iter != end_iter)
			return true;
		else
			return false;
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	inline std::pair<bool, Quadtree_Type::Item::D_Points::iterator>
	Quadtree_Def::Item::find(const T_Id& point_id)
	{
		Quadtree_Type::Item::D_Points::iterator end_iter = _point_ids.end();
		Quadtree_Type::Item::D_Points::iterator find_iter
			=  std::find(_point_ids.begin(), end_iter, point_id);
		
		if (find_iter != end_iter)
			return std::make_pair(true, find_iter);
		else
			return std::make_pair(false, end_iter);
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	inline int
	Quadtree_Def::Item::find_child(const Item* const child_pointer) const
	{
		for(int i = 0; i < 4; ++i)
		{
			if (_children[i] == child_pointer)
				return i;
		}
		
		return -1;
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	inline int
	Quadtree_Def::Item::find_first_child() const
	{
		for(int i = 0; i < 4; ++i)
		{
			if (_children[i] != 0)
				return i;
		}
		
		return -1;
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	inline bool
	Quadtree_Def::Item::is_leaf() const
	{
		return _no_of_children == 0;
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	void
	Quadtree_Def::Item::range_query(
		const Rectangle<T_Point>& query_rectangle,
		const Rectangle<T_Point>& span_rectangle,
		std::vector<Quadtree_Type::Id>& query_result) const
	{
		if (query_rectangle.contains(span_rectangle))
		{
			report_points(query_result);
			
		} else if (query_rectangle.intersects(span_rectangle))
		{
			if (is_leaf())
			{
				Quadtree_Type::Item::D_Points::const_iterator iter
					= _point_ids.begin();
				Quadtree_Type::Item::D_Points::const_iterator iter_end
					= _point_ids.end();
				for (; iter != iter_end; ++iter)
				{
					if ( query_rectangle.contains((*_quadtree._points)[*iter]) )
					{
						/** @todo Does this work. It does. But is it save?! */
						Quadtree_Type::Id result_id(*iter, const_cast<Item*>(this));
						query_result.push_back(result_id);
					}
				}
			} else
			{
				T_Point median_point;
				compute_median_point(span_rectangle, median_point);
				
				for (int i = 0; i < 4 ;++i)
				{
					if (_children[i] != 0)
					{
						Rectangle<T_Point> new_span_rectangle
							= span_rectangle;
						compute_new_span_rectangle(i, median_point,
							new_span_rectangle);
						
						_children[i]->range_query(query_rectangle,
							new_span_rectangle, query_result);
					}
				}
			}
		}
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	void
	Quadtree_Def::Item::report_points(
		std::vector<Quadtree_Type::Id>& query_result) const
	{
		if (is_leaf())
		{
			Quadtree_Type::Item::D_Points::const_iterator iter
				= _point_ids.begin();
			Quadtree_Type::Item::D_Points::const_iterator iter_end
				= _point_ids.end();
			for (; iter != iter_end; ++iter)
			{
				/** @todo Does this work. It does. But is it save?! */
				Quadtree_Type::Id result_id(*iter, const_cast<Item*>(this));
				query_result.push_back(result_id);
			}
			
		} else
		{
			for (int i = 0; i < 4; ++i)
			{
				if (_children[i] != 0)
				{
					_children[i]->report_points(query_result);
				}
			}
		}
	}
	
	
	
	//---------------------------------------------------//
	//--- Main class: Quadtree --------------------------//
	//---------------------------------------------------//
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	inline
	Quadtree_Def::Quadtree()
	: _inited(false), _max_depth(0), _points(0), _root(0),
		_span_rectangle()
	{
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	inline
	Quadtree_Def::Quadtree(int max_depth,
		const Rectangle<T_Point>& span_rectangle,
		const T_Id_Point_Mapper* points)
	: _inited(true), _max_depth(max_depth), _points(points), _root(0),
		_span_rectangle(span_rectangle)
	{
	}


	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	inline
	Quadtree_Def::~Quadtree()
	{
		delete _root;
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	inline bool
	Quadtree_Def::add_point(const T_Id& id)
	{
		return add_point(id, _root);
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	inline int
	Quadtree_Def::get_max_depth() const
	{
		return _max_depth;
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	inline const T_Id_Point_Mapper*
	Quadtree_Def::get_points() const
	{
		return _points;
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	inline const Rectangle<T_Point>&
	Quadtree_Def::get_span_rectangle() const
	{
		return _span_rectangle;
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	inline void
	Quadtree_Def::init_ready()
	{
		_inited = true;
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	inline bool
	Quadtree_Def::is_inited() const
	{
		return _inited;
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	bool
	Quadtree_Def::move_point(Quadtree_Type::Id& from_point_id,
		const T_Point& to_point)
	{
		if ( !_span_rectangle.contains(to_point) )
			return false;
		
		/* save item and point of from_point_iter */
		Quadtree_Type::Item* from_point_item = from_point_id._item;
		
		std::pair<bool, Quadtree_Type::Item::D_Points::iterator> find_result =
			from_point_item->find(*from_point_id);
		if (find_result.first)
		{
			if (from_point_item->_my_span_rectangle.contains(to_point))
			{
				return true;
			} else
			{
				from_point_item->_point_ids.erase(find_result.second);
				
				/* the Id has not changed, but its point's location!
				 * to stress it, I gave the Id a new name */
				T_Id to_point_id = *from_point_id;
				bool return_value = add_point(to_point_id);
				
				if (from_point_item->_point_ids.empty())
					repair_tree(from_point_item, false);
				else if (from_point_item->_point_ids.size() == 1)
					repair_tree(from_point_item, true);
				
				return return_value;
			}
		}
		
		return false;
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	inline void
	Quadtree_Def::range_query(const Rectangle<T_Point>& query_rectangle,
		std::vector<Quadtree_Type::Id>& query_result) const
	{
		if (_root != 0)
		{
			_root->range_query(query_rectangle, _span_rectangle, query_result);
		}
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	void
	Quadtree_Def::range_query(const Trapezoid<T_Point>& trapezoid,
		std::vector<Quadtree_Type::Id>& query_result) const
	{
		range_query(trapezoid.bounding_rectangle(), query_result);
		
		typename std::vector<Quadtree_Type::Id>::iterator iter
			= query_result.begin();
		while (iter != query_result.end())
		{
			bool erased_point = false;
			
			for (int i = 0; i < 4; ++i)
			{
				const T_Point& iter_point = (*_points)[**iter];
				
				int relative_pos =
					relative_position(trapezoid[i % 4], trapezoid[(i+1) % 4],
						iter_point);
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
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	inline bool
	Quadtree_Def::remove_point(Quadtree_Type::Id& point_id)
	{
		return remove_point(point_id, true);
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	inline void
	Quadtree_Def::set_max_depth(int max_depth)
	{
		if ( !is_inited() )
			_max_depth = max_depth;
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	inline void
	Quadtree_Def::set_points(const T_Id_Point_Mapper* points)
	{
		if ( !is_inited() )
			_points = points;
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	inline void
	Quadtree_Def::set_span_rectangle(const Rectangle<T_Point>& span_rectangle)
	{
		if ( !is_inited() )
			_span_rectangle = span_rectangle;
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	bool
	Quadtree_Def::add_point(const T_Id& point_id,
		Quadtree_Type::Item* starting_item)
	{
		if (starting_item == 0)
		{
			/* We are definitely at _root. Otherwise the stupid programmer
			 * has made a mistake!!! */
			_root = new Quadtree_Type::Item(*this, _span_rectangle);
			_root->_point_ids.push_back(point_id);
			return true;
		}
		
		Quadtree_Type::Item* current_item = starting_item;
		Rectangle<T_Point> current_span_rectangle
			= starting_item->_my_span_rectangle;
		
		/* calculate current depth... */
		int current_depth = 0;
		Quadtree_Type::Item* test_item = starting_item;
		while (test_item->_parent != 0)
		{
			++current_depth;
			test_item = test_item->_parent;
		}
		/* done */
		
		/* insert point to quad tree */
		while (current_depth < _max_depth)
		{
			if (current_item->is_leaf())
			{
				Quadtree_Type::Item::D_Points::iterator
					old_point_id_iter = current_item->_point_ids.begin();
				T_Id old_point_id = *old_point_id_iter;
				
				if (old_point_id == point_id)
				{
					return false;
					
				} else // explicit: if (*old_point != *point)
				{
					current_item->_point_ids.erase(old_point_id_iter);

					while (current_depth < _max_depth)
					{
						T_Point median_point;
						int point_quarter = compute_quarter(
							(*_points)[point_id],
							current_span_rectangle, median_point);
						int old_point_quarter = compute_quarter(
							(*_points)[old_point_id], current_span_rectangle,
							median_point);
						
						if (point_quarter == old_point_quarter)
						{
							compute_new_span_rectangle(point_quarter,
								median_point, current_span_rectangle);
							current_item->_children[point_quarter]
								= new Quadtree_Type::Item(*this, current_span_rectangle);
							current_item->_children[point_quarter]->_parent
								= current_item;
							++(current_item->_no_of_children);
							
							current_item = current_item->_children[point_quarter];
							++current_depth;
							
						} else // explicit: if (point_quarter != old_point_quarter)
						{
							Rectangle<T_Point> saved_current_span_rectangle
								= current_span_rectangle;
							
							compute_new_span_rectangle(point_quarter,
								median_point, current_span_rectangle);
							current_item->_children[point_quarter]
								= new Quadtree_Type::Item(*this, current_span_rectangle);
							current_item->_children[point_quarter]->_parent
								= current_item;
							++(current_item->_no_of_children);
			
							current_item->_children[point_quarter]
								->_point_ids.push_back(point_id);
							
							compute_new_span_rectangle(old_point_quarter,
								median_point, saved_current_span_rectangle);
							current_item->_children[old_point_quarter]
								= new Quadtree_Type::Item(*this, saved_current_span_rectangle);
							current_item->_children[old_point_quarter]->_parent
								= current_item;
							++(current_item->_no_of_children);
							
							current_item->_children[old_point_quarter]
								->_point_ids.push_back(old_point_id);
							
							return true;
							
						} // end: if (point_quarter == old_point_quarter)
					} // end: while (current_depth >= _max_depth)
			
					current_item->_point_ids.push_back(point_id);
					current_item->_point_ids.push_back(old_point_id);
			
					return true;
					
				} // end: if (*old_point == *point)
				
			} else // explicit: if ( !current_item->is_leaf() )
			{
				T_Point median_point;
				int point_quarter = compute_quarter( (*_points)[point_id],
					current_span_rectangle, median_point );
				
				compute_new_span_rectangle(point_quarter, median_point,
					current_span_rectangle);
				
				if (current_item->_children[point_quarter] == 0)
				{
					// this child is 0: insert point and be happy!
					current_item->_children[point_quarter]
						= new Quadtree_Type::Item(*this, current_span_rectangle);
					current_item->_children[point_quarter]->_parent
						= current_item;
					++(current_item->_no_of_children);
					
					current_item->_children[point_quarter]
						->_point_ids.push_back(point_id);
					return true;
					
				} else // explicit: if (current_item->_children[point_quarter] != 0)
				{
					// child already exists: descend the tree...
					current_item = current_item->_children[point_quarter];
					++current_depth;
				} // end: if (current_item->_children[point_quarter] == 0)
			}// end: if (current_item->is_leaf())
		} // end: while (current_depth < _max_depth)
		
		if (current_item->contains(point_id))
			return false;
		
		current_item->_point_ids.push_back(point_id);
		return true;
		
		/** @todo fast data structure for D_Points? */
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	inline void
	Quadtree_Def::compute_median_point(
		const Rectangle<T_Point>& in_span_rectangle, T_Point& out_median_point)
	{
		const T_Point& llc = in_span_rectangle.lower_left_corner();
		const T_Point& urc = in_span_rectangle.upper_right_corner();
		
		out_median_point[0] = (urc[0] + llc[0]) / 2.0;
		out_median_point[1] = (urc[1] + llc[1]) / 2.0;
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	inline void
	Quadtree_Def::compute_new_span_rectangle(int in_quarter,
		const T_Point& in_median_point,
		Rectangle<T_Point>& out_span_rectangle)
	{
		T_Point& llc = out_span_rectangle.lower_left_corner();
		T_Point& urc = out_span_rectangle.upper_right_corner();
		
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
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	inline int
	Quadtree_Def::compute_quarter(const T_Point& in_point,
		const Rectangle<T_Point>& in_span_rectangle,
		T_Point& out_median_point)
	{
		compute_median_point(in_span_rectangle, out_median_point);
		
		int next_item;
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
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
	bool
	Quadtree_Def::remove_point(Quadtree_Type::Id& point_id, bool repair_now)
	{
		/* find point in given point_id.item() and remove it... */
		Quadtree_Type::Item* current_item = point_id._item;
		
		std::pair<bool, Quadtree_Type::Item::D_Points::iterator> find_result
			= current_item->find(*point_id);
		if ( !find_result.first )
			return false;
		
		current_item->_point_ids.erase(find_result.second);
		/* done! */
		
		/* if current_item is "empty", repair the tree if desired... */
		if (repair_now)
		{
			if (current_item->_point_ids.empty())
				repair_tree(current_item, false);
			else if (current_item->_point_ids.size() == 1)
				repair_tree(current_item, true);
		}
		/* done */
		
		return true;
	}
	
	
	template<typename T_Id, typename T_Point, typename T_Id_Point_Mapper>
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
		Quadtree_Type::Item* parent_item = starting_item->_parent;
		int child_id = parent_item->find_child(starting_item);
		
		if ( !starting_item_contains_one_point )
		{
			delete (parent_item->_children[child_id]);
			parent_item->_children[child_id] = 0;
			--(parent_item->_no_of_children);
		}
		/* done */
		
		/* Find siblings of starting_item.
		 * If only one sibling is found, it is part of a chain which can
		 * be shorted now. Then we have to find the other end of the chain.
		 * 
		 * But step-by-step... */
		if (parent_item->_no_of_children == 1)
		{
			/* find the sibling... */
			child_id = parent_item->find_first_child();
			/* done */
			
			/* test if that sibling is a leaf... */
			if (parent_item->_children[child_id]->is_leaf())
			{
				/* it is a leaf: we can short the chain... */
				
				/* save pointer */
				Quadtree_Type::Item* end_of_chain_item
					= parent_item->_children[child_id];
				
				/* Break the tree. That is necessary, when applying the
				 * "delete" operator (cf. below) */
				parent_item->_children[child_id] = 0;
				
				/* Now search the start of the chain... */
				Quadtree_Type::Item* start_of_chain_item = parent_item;
				Quadtree_Type::Item* previous_start_of_chain_item
					= end_of_chain_item;
				
				bool exited_on_children = false;
				while (start_of_chain_item->_parent != 0)
				{
					previous_start_of_chain_item = start_of_chain_item;
					start_of_chain_item = start_of_chain_item->_parent;
					
					if (start_of_chain_item->_no_of_children > 1)
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
					end_of_chain_item->_parent = 0;
					
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
					delete start_of_chain_item->_children[child_id];
					
					/* link end_of_chain_item to start_of_chain_item->_children[j]... */
					start_of_chain_item->_children[child_id] = end_of_chain_item;
					end_of_chain_item->_parent = start_of_chain_item;
					
					/* set correct end_of_chain_item->_my_span_rectangle */
					end_of_chain_item->_my_span_rectangle
						= start_of_chain_item->_my_span_rectangle;
					
					T_Point median_point;
					compute_median_point(start_of_chain_item->_my_span_rectangle,
						median_point);
					compute_new_span_rectangle(child_id, median_point,
						end_of_chain_item->_my_span_rectangle);
					/* done */
				}
				
				/* if there are more than one point in end_of_chain_item, split
				 * that item... */
				/** @todo DO NOT USE std::list::size()!!! */
				if (end_of_chain_item->_point_ids.size() > 1)
				{
					Quadtree_Type::Item::D_Points::iterator
						begin_splice_iter = end_of_chain_item->_point_ids.begin();
					++begin_splice_iter;
					
					Quadtree_Type::Item::D_Points points;
					points.splice(points.begin(), end_of_chain_item->_point_ids,
						begin_splice_iter, end_of_chain_item->_point_ids.end());
					
					Quadtree_Type::Item::D_Points::iterator
						iter = points.begin();
					Quadtree_Type::Item::D_Points::iterator
						iter_end = points.end();
					
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
	
}

#endif //QUADTREE_H
