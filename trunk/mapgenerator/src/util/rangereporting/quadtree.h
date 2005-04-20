/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef QUADTREE_H
#define QUADTREE_H

#include <iomanip>
#include <list>
#include <ostream>
#include <queue>
#include <stack>
#include <vector>

#include "rangereportingsystem.h"
#include "rectangle.h"
#include "trapezoid.h"
#include "util/mlog.h"

#include "helperfunctions.h"

using mapgeneration_util::MLog;

namespace rangereporting
{
	
	//-------------------------------------------------------------------------//
	//--- Definition section --------------------------------------------------//
	//-------------------------------------------------------------------------//
	
	template<typename T_Point_2D>
	class Quadtree
	{
		friend std::ostream& operator<< <> (std::ostream& out,
			const Quadtree<T_Point_2D>& quadtree);

		
		//---------------------------------------------------//
		//--- Iterators -------------------------------------//
		//---------------------------------------------------//
		protected:
			
			class Item;
			
			
		public:
			
			class const_iterator
			{
				
				friend class Quadtree<T_Point_2D>;
				
				public:
					
					/** @todo make protected or so... */
					inline const_iterator(const T_Point_2D* point, Item* item);
					
					
					inline const T_Point_2D&
					operator*() const;
					
					
					inline const T_Point_2D*
					operator->() const;
					
					
					inline bool
					operator==(const const_iterator& iter) const;
					
					
					inline bool
					operator!=(const const_iterator& iter) const;
					
					
				protected:
					
					const T_Point_2D* _point;
					
					
					Item* _item;
					
					
					Item*
					item();
			};
			
			
			typedef const_iterator iterator;
			

		//---------------------------------------------------//
		//--- Item ------------------------------------------//
		//---------------------------------------------------//
		protected:
			
			class Item
			{
				
				public:
					
					typedef std::list<const T_Point_2D*> D_Points;
					
					
					// _children[0] = north east
					// _children[1] = south east
					// _children[2] = south west
					// _children[3] = north west
					std::vector<Item*> _children;
					
					
					Rectangle<T_Point_2D> _my_span_rectangle;
					
					
					int _no_of_children;
					
					
					Item* _parent;
					
					
					D_Points _points;
					
					
					inline Item();
					
					
					inline Item(const Rectangle<T_Point_2D>& my_span_rectangle);
					
					
					~Item();
					
					
					inline bool
					is_leaf() const;
					
					
					void
					range_query(const Rectangle<T_Point_2D>& query_rectangle,
						const Rectangle<T_Point_2D>& span_rectangle,
						std::vector< Quadtree<T_Point_2D>::const_iterator >& query_result)
						const;
					
					
					void
					report_points(
						std::vector< Quadtree<T_Point_2D>::const_iterator >& query_result)
						const;
					
			};
		
		
		//---------------------------------------------------//
		//--- Main class: Quadtree --------------------------//
		//---------------------------------------------------//
		public:
			
			inline Quadtree(int max_depth,
				const Rectangle<T_Point_2D>& span_rectangle);
			

			~Quadtree();
			

			inline bool
			add_point(const T_Point_2D* point);
			
			
			inline int
			get_max_depth() const;
			
			
			bool
			move_point(Quadtree<T_Point_2D>::const_iterator& from_point_iter,
				const T_Point_2D* to_point);
			
			
			inline void
			range_query(const Rectangle<T_Point_2D>& query_rectangle,
				std::vector< Quadtree<T_Point_2D>::const_iterator >& query_result) const;

			
			void
			range_query(const Trapezoid<T_Point_2D>& query_trapezoid,
				std::vector<  Quadtree<T_Point_2D>::const_iterator> & query_result) const;

			
			inline bool
			remove_point(Quadtree<T_Point_2D>::const_iterator& point_iter);
		
		protected:
			
			int _max_depth;
			
			
			Item* _root;
			
			
			Rectangle<T_Point_2D> _span_rectangle;
			
			
			bool
			add_point(const T_Point_2D* point, Item* starting_item);
			
			
			inline static void
			compute_median_point(const Rectangle<T_Point_2D>& in_span_rectangle,
				T_Point_2D& out_median_point);
			
			
			inline static void
			compute_new_span_rectangle(int in_quarter,
				const T_Point_2D& in_median_point,
				Rectangle<T_Point_2D>& out_span_rectangle);
			
			
			inline static int
			compute_quarter(const T_Point_2D& in_point,
				const Rectangle<T_Point_2D>& in_span_rectangle,
				T_Point_2D& out_median_point);
			
			
			bool
			remove_point(Quadtree<T_Point_2D>::const_iterator& point_iter,
				bool repair_now);
			
			
			void
			repair_tree(Quadtree<T_Point_2D>::Item* starting_item,
				bool starting_item_contains_one_point);
			
	};
	

	//-------------------------------------------------------------------------//
	//--- Implementation section ----------------------------------------------//
	//-------------------------------------------------------------------------//
	
	//---------------------------------------------------//
	//--- Output operator -------------------------------//
	//---------------------------------------------------//
	template<typename T_Point_2D>
	std::ostream& operator<<(std::ostream& out,
		const Quadtree<T_Point_2D>& quadtree)
	{
		out << "Quadtree:" << std::endl;
		out << "=========" << std::endl;
		out << "\trange: "
			<< quadtree._span_rectangle._lower_left_corner
			<< " - "
			<< quadtree._span_rectangle._upper_right_corner
			<< std:: endl;
		out << "\troot: " << quadtree._root << std::endl;
		out << std::endl;
		
		/* Exit on _root being 0 */
		if (quadtree._root == 0)
			return out;
		/* Exit on _root being 0 */
		
		std::queue< typename Quadtree<T_Point_2D>::Item* > queue;
		queue.push(quadtree._root);
		
		char state_0;
		char state_1;
		char state_2;
		char state_3;

		typename Quadtree<T_Point_2D>::Item* item;
		typename Quadtree<T_Point_2D>::Item::D_Points::const_iterator iter;
		typename Quadtree<T_Point_2D>::Item::D_Points::const_iterator iter_end;

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
			
			out << "| range: " << item->_my_span_rectangle._lower_left_corner
				<< ", " << item->_my_span_rectangle._upper_right_corner
				<< std::endl;
			out << "| parent: " << item->_parent << std::endl;			
			
			out << "|-------------------------------|" << std::endl;
			out << "| NE = " << std::setw(10) << item->_children[0] << " |    =====" <<std::endl;
			out << "| SE = " << std::setw(10) << item->_children[1] << " |    |" << state_3 << "|" << state_0 << "|" <<std::endl;
			out << "|                "        <<                       " |    |---|" << std::endl;
			out << "| SW = " << std::setw(10) << item->_children[2] << " |    |" << state_2 << "|" << state_1 << "|" << std::endl;
			out << "| NW = " << std::setw(10) << item->_children[3] << " |    =====" << std::endl;
			out << "|                "        <<                       " |     " << std::endl;
			out << "| Points: [";
			
			out.flags(original_flags);
			
			iter = item->_points.begin();
			iter_end = item->_points.end();
			for (; iter != iter_end; ++iter)
			{
				out << **iter;
			}
							
			out << "]" << std::endl;;
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
	//--- Item ------------------------------------------//
	//---------------------------------------------------//
	template<typename T_Point_2D>
	inline
	Quadtree<T_Point_2D>::Item::Item()
	: _children(4), _my_span_rectangle(), _no_of_children(0),
		_parent(0), _points()
	{
		_children[0] = 0;
		_children[1] = 0;
		_children[2] = 0;
		_children[3] = 0;
	}
	
	
	template<typename T_Point_2D>
	inline
	Quadtree<T_Point_2D>::Item::Item(
		const Rectangle<T_Point_2D>& my_span_rectangle)
	: _children(4), _my_span_rectangle(my_span_rectangle), _no_of_children(0),
		_parent(0), _points()
	{
		_children[0] = 0;
		_children[1] = 0;
		_children[2] = 0;
		_children[3] = 0;
	}
	
	
	template<typename T_Point_2D>
	Quadtree<T_Point_2D>::Item::~Item()
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
	
	
	template<typename T_Point_2D>
	inline bool
	Quadtree<T_Point_2D>::Item::is_leaf() const
	{
		return _no_of_children == 0;
	}
	
	
	template<typename T_Point_2D>
	void
	Quadtree<T_Point_2D>::Item::range_query(
		const Rectangle<T_Point_2D>& query_rectangle,
		const Rectangle<T_Point_2D>& span_rectangle,
		std::vector< Quadtree<T_Point_2D>::const_iterator >& query_result) const
	{
		/** @todo think about an iterative approach in Quadtree! */
		
		if (query_rectangle.contains(span_rectangle))
		{
			report_points(query_result);
			
		} else if (query_rectangle.intersects(span_rectangle))
		{
			if (is_leaf())
			{
				typename Quadtree<T_Point_2D>::Item::D_Points::const_iterator
					iter = _points.begin();
				typename Quadtree<T_Point_2D>::Item::D_Points::const_iterator
					iter_end = _points.end();
				for (; iter != iter_end; ++iter)
				{
					if (query_rectangle.contains(**iter))
					{
						/** @todo Does this work. It does. But is it save?! */
						Quadtree<T_Point_2D>::const_iterator
							result_point_iterator(*iter, const_cast<Item*>(this));
						query_result.push_back(result_point_iterator);
					}
				}
			} else
			{
				T_Point_2D median_point;
				compute_median_point(span_rectangle, median_point);
				
				for (int i = 0; i < 4 ;++i)
				{
					if (_children[i] != 0)
					{
						Rectangle<T_Point_2D> new_span_rectangle
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
	
	
	template<typename T_Point_2D>
	void
	Quadtree<T_Point_2D>::Item::report_points(
		std::vector< Quadtree<T_Point_2D>::const_iterator >& query_result) const
	{
		if (is_leaf())
		{
			typename Quadtree<T_Point_2D>::Item::D_Points::const_iterator
				iter = _points.begin();
			typename Quadtree<T_Point_2D>::Item::D_Points::const_iterator
				iter_end = _points.end();
			for (; iter != iter_end; ++iter)
			{
				/** @todo Does this work. It does. But is it save?! */
				Quadtree<T_Point_2D>::const_iterator
					result_point_iterator(*iter, const_cast<Item*>(this));
				query_result.push_back(result_point_iterator);
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
	//--- Iterators -------------------------------------//
	//---------------------------------------------------//
	template<typename T_Point_2D>
	inline
	Quadtree<T_Point_2D>::const_iterator::const_iterator(const T_Point_2D* point,
		Quadtree<T_Point_2D>::Item* item)
	: _point(point), _item(item)
	{}


	template<typename T_Point_2D>
	inline const T_Point_2D&
	Quadtree<T_Point_2D>::const_iterator::operator*() const
	{
		return *_point;
	}


	template<typename T_Point_2D>
	inline const T_Point_2D*
	Quadtree<T_Point_2D>::const_iterator::operator->() const
	{
		return _point;
	}
	
	
	template<typename T_Point_2D>
	inline bool
	Quadtree<T_Point_2D>::const_iterator::operator==(
		const Quadtree<T_Point_2D>::const_iterator& iter) const
	{
		return ( _point == *iter );
	}
	
	
	template<typename T_Point_2D>
	inline bool
	Quadtree<T_Point_2D>::const_iterator::operator!=(
		const Quadtree<T_Point_2D>::const_iterator& iter) const
	{
		return ( !operator==(iter) );
	}
	
	
	template<typename T_Point_2D>
	inline typename Quadtree<T_Point_2D>::Item*
	Quadtree<T_Point_2D>::const_iterator::item()
	{
		return _item;
	}
	
	
	//---------------------------------------------------//
	//--- Main class: Quadtree --------------------------//
	//---------------------------------------------------//
	template<typename T_Point_2D>
	inline
	Quadtree<T_Point_2D>::Quadtree(int max_depth,
		const Rectangle<T_Point_2D>& span_rectangle)
	: _max_depth(max_depth),
		_root(0),
		_span_rectangle(span_rectangle)
	{
	}


	template<typename T_Point_2D>
	Quadtree<T_Point_2D>::~Quadtree()
	{
		delete _root;
	}
	
	
	template<typename T_Point_2D>
	inline bool
	Quadtree<T_Point_2D>::add_point(const T_Point_2D* point)
	{
		return add_point(point, _root);
	}
	
	
	template<typename T_Point_2D>
	inline int
	Quadtree<T_Point_2D>::get_max_depth() const
	{
		return _max_depth;
	}
	
	
	template<typename T_Point_2D>
	bool
	Quadtree<T_Point_2D>::move_point(
		Quadtree<T_Point_2D>::const_iterator& from_point_iter,
		const T_Point_2D* to_point)
	{
		if ( !_span_rectangle.contains(*to_point) )
			return false;
		
		/* save item and point of from_point_iter */
		Quadtree<T_Point_2D>::Item* from_point_item = from_point_iter.item();
		//const T_Point_2D from_point = *from_point_iter;
		
		if ( !remove_point(from_point_iter, false) )
		{
			return false;
		} else
		{
			if ( from_point_item->_my_span_rectangle.contains(*to_point) )
			{
				from_point_item->_points.push_back(to_point);
				return true;
			} else
			{
				bool return_value = add_point(to_point);

				if (from_point_item->_points.empty())
					repair_tree(from_point_item, false);
				else if (from_point_item->_points.size() == 1)
					repair_tree(from_point_item, true);
				
				return return_value;
			}
		}
	}
	
	
	template<typename T_Point_2D>
	inline void
	Quadtree<T_Point_2D>::range_query(
		const Rectangle<T_Point_2D>& query_rectangle,
		std::vector< Quadtree<T_Point_2D>::const_iterator >& query_result) const
	{
		if (_root != 0)
		{
			_root->range_query(query_rectangle, _span_rectangle, query_result);
		}
	}
	
	
	template<typename T_Point_2D>
	void
	Quadtree<T_Point_2D>::range_query(const Trapezoid<T_Point_2D>& trapezoid,
		std::vector< Quadtree<T_Point_2D>::const_iterator >& query_result) const
	{
		range_query(trapezoid._bounding_rectangle, query_result);
		
		typename std::vector< Quadtree<T_Point_2D>::const_iterator >::iterator
			iter = query_result.begin();
		
		while (iter != query_result.end())
		{
			bool erased_point = false;
			
			for (int i = 0; i < 4; ++i)
			{
				int relative_pos =
					relative_position(trapezoid._points[i % 4],
						trapezoid._points[(i+1) % 4], **iter);
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
	
	
	template<typename T_Point_2D>
	inline bool
	Quadtree<T_Point_2D>::remove_point(
		Quadtree<T_Point_2D>::const_iterator& point_iter)
	{
		return remove_point(point_iter, true);
	}
	
	
	template<typename T_Point_2D>
	bool
	Quadtree<T_Point_2D>::add_point(const T_Point_2D* point,
		Quadtree<T_Point_2D>::Item* starting_item)
	{
		if (starting_item == 0)
		{
			/* We are definitely at _root. Otherwise the stupid programmer
			 * has made a mistake!!! */
			_root = new Item(_span_rectangle);
			_root->_points.push_back(point);
			return true;
		}
		
		Quadtree<T_Point_2D>::Item* current_item = starting_item;
		Rectangle<T_Point_2D> current_span_rectangle
			= starting_item->_my_span_rectangle;
		
		/* calculate current depth... */
		int current_depth = 0;
		Quadtree<T_Point_2D>::Item* test_item = starting_item;
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
				typename Quadtree<T_Point_2D>::Item::D_Points::iterator
					old_point_iter = current_item->_points.begin();
				const T_Point_2D* old_point = *old_point_iter;
				
				if (*old_point == *point)
				{
					return false;
					
				} else // explicit: if (*old_point != *point)
				{
					current_item->_points.erase(old_point_iter);

					while (current_depth < _max_depth)
					{
			//			mlog(MLog::debug, "Quadtree") << "\tcurrent_depth < _max_depth\n";
						T_Point_2D median_point;
						int point_quarter = compute_quarter(*point,
							current_span_rectangle, median_point);
						int old_point_quarter = compute_quarter(*old_point,
							current_span_rectangle, median_point);
						
						if (point_quarter == old_point_quarter)
						{
			//				mlog(MLog::debug, "Quadtree") << "\tpoint_1_quarter == point_2_quarter\n";
							compute_new_span_rectangle(point_quarter,
								median_point, current_span_rectangle);
							current_item->_children[point_quarter]
								= new Item(current_span_rectangle);
							current_item->_children[point_quarter]->_parent
								= current_item;
							++(current_item->_no_of_children);
							
							current_item = current_item->_children[point_quarter];
							++current_depth;
							
						} else // explicit: if (point_quarter != old_point_quarter)
						{
			//				mlog(MLog::debug, "Quadtree") << "\tpoint_1_quarter != point_2_quarter\n";
							Rectangle<T_Point_2D> saved_current_span_rectangle
								= current_span_rectangle;
							
							compute_new_span_rectangle(point_quarter,
								median_point, current_span_rectangle);
							current_item->_children[point_quarter]
								= new Item(current_span_rectangle);
							current_item->_children[point_quarter]->_parent
								= current_item;
							++(current_item->_no_of_children);
			
							current_item->_children[point_quarter]
								->_points.push_back(point);
							
							compute_new_span_rectangle(old_point_quarter,
								median_point, saved_current_span_rectangle);
							current_item->_children[old_point_quarter]
								= new Item(saved_current_span_rectangle);
							current_item->_children[old_point_quarter]->_parent
								= current_item;
							++(current_item->_no_of_children);
							
							current_item->_children[old_point_quarter]
								->_points.push_back(old_point);
							
							return true;
							
						} // end: if (point_quarter == old_point_quarter)
					} // end: while (current_depth >= _max_depth)
			
			//		mlog(MLog::debug, "Quadtree") << "\tcurrent_depth >= _max_depth\n";
					current_item->_points.push_back(point);
					current_item->_points.push_back(old_point);
			
					return true;
					
				} // end: if (*old_point == *point)
				
			} else // explicit: if ( !current_item->is_leaf() )
			{
				T_Point_2D median_point;
				int point_quarter = compute_quarter(*point,
					current_span_rectangle, median_point);
				
				compute_new_span_rectangle(point_quarter, median_point,
					current_span_rectangle);
				
				if (current_item->_children[point_quarter] == 0)
				{
//					mlog(MLog::debug, "Quadtree") << "\tcurrent_item->_children[" << point_quarter << "] == 0\n";
					// this child is 0: insert point and be happy!
					current_item->_children[point_quarter]
						= new Item(current_span_rectangle);
					current_item->_children[point_quarter]->_parent
						= current_item;
					++(current_item->_no_of_children);
					
					current_item->_children[point_quarter]->_points.push_back(point);
					return true;
					
				} else // explicit: if (current_item->_children[point_quarter] != 0)
				{
//					mlog(MLog::debug, "Quadtree") << "\tcurrent_item->_children[" << point_quarter << "] != 0\n";
					// child already exists: descend the tree...
					current_item = current_item->_children[point_quarter];
					++current_depth;
				} // end: if (current_item->_children[point_quarter] == 0)
			}// end: if (current_item->is_leaf())
		} // end: while (current_depth < _max_depth)
		
		typename Quadtree<T_Point_2D>::Item::D_Points::const_iterator
			iter = current_item->_points.begin();
		typename Quadtree<T_Point_2D>::Item::D_Points::const_iterator
			iter_end = current_item->_points.end();
		for (; iter != iter_end; ++iter)
		{
			if (**iter == *point)
				return false;
		}
		
		current_item->_points.push_back(point);
		return true;
	}
	
	
	template<typename T_Point_2D>
	inline void
	Quadtree<T_Point_2D>::compute_median_point(
		const Rectangle<T_Point_2D>& in_span_rectangle,
		T_Point_2D& out_median_point)
	{
		out_median_point[0] = (in_span_rectangle._upper_right_corner[0]
			+ in_span_rectangle._lower_left_corner[0]) / 2.0;
		out_median_point[1] = (in_span_rectangle._upper_right_corner[1]
			+ in_span_rectangle._lower_left_corner[1]) / 2.0;
	}
	
	
	template<typename T_Point_2D>
	inline void
	Quadtree<T_Point_2D>::compute_new_span_rectangle(int in_quarter,
		const T_Point_2D& in_median_point,
		Rectangle<T_Point_2D>& out_span_rectangle)
	{
		switch (in_quarter)
		{
			case 0:
				out_span_rectangle._lower_left_corner[0] = in_median_point[0];
				out_span_rectangle._lower_left_corner[1] = in_median_point[1];
				break;
			
			case 1:
				out_span_rectangle._lower_left_corner[0] = in_median_point[0];
				out_span_rectangle._upper_right_corner[1] = in_median_point[1];
				break;
			
			case 2:
				out_span_rectangle._upper_right_corner[0] = in_median_point[0];
				out_span_rectangle._upper_right_corner[1] = in_median_point[1];
				break;
			
			case 3:
				out_span_rectangle._upper_right_corner[0] = in_median_point[0];
				out_span_rectangle._lower_left_corner[1] = in_median_point[1];
				break;
		}
	}
	
	
	template<typename T_Point_2D>
	inline int
	Quadtree<T_Point_2D>::compute_quarter(const T_Point_2D& in_point,
		const Rectangle<T_Point_2D>& in_span_rectangle,
		T_Point_2D& out_median_point)
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
	
	
	template<typename T_Point_2D>
	bool
	Quadtree<T_Point_2D>::remove_point(
		Quadtree<T_Point_2D>::const_iterator& point_iter, bool repair_now)
	{
		/* find point in given point_iter.item() and remove it... */
		Item* current_item = point_iter.item();
		
		typename Quadtree<T_Point_2D>::Item::D_Points::iterator
			iter = current_item->_points.begin();
		typename Quadtree<T_Point_2D>::Item::D_Points::iterator
			iter_end = current_item->_points.end();
		for (; iter != iter_end; ++iter)
		{
			if (**iter == *point_iter)
				break;
		}
		
		if (iter == iter_end)
			return false;
		
		current_item->_points.erase(iter);
		/* done! */
		
		/* if current_item is "empty", repair the tree if desired... */
		if (repair_now)
		{
			if (current_item->_points.empty())
				repair_tree(current_item, false);
			else if (current_item->_points.size() == 1)
				repair_tree(current_item, true);
		}
		
		return true;
	}
	
	
	template<typename T_Point_2D>
	void
	Quadtree<T_Point_2D>::repair_tree(Quadtree<T_Point_2D>::Item* starting_item,
		bool starting_item_contains_one_point)
	{
		/* ALWAYS remember:
		 * if starting_item->_points.size() > 1, we will NEVER
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
		Quadtree<T_Point_2D>::Item* parent_item = starting_item->_parent;
		
		int i;
		for (i = 0; i < 4; ++i)
		{
			if (parent_item->_children[i] == starting_item)
				break;
		}
		
		if ( !starting_item_contains_one_point )
		{
			delete (parent_item->_children[i]);
			parent_item->_children[i] = 0;
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
			int j;
			for (j = 0; j < 4; ++j)
			{
				if (parent_item->_children[j] != 0)
					break;
			}
			/* done */
			
			/* test if that sibling is a leaf... */
			if (parent_item->_children[j]->is_leaf())
			{
				/* it is a leaf: we can short the chain... */
				
				/* save pointer */
				Quadtree<T_Point_2D>::Item* end_of_chain_item
					= parent_item->_children[j];
				
				/* Break the tree. That is necessary, when applying the
				 * "delete" operator (cf. below) */
				parent_item->_children[j] = 0;
				
				/* Now search the start of the chain... */
				Quadtree<T_Point_2D>::Item* start_of_chain_item = parent_item;
				Quadtree<T_Point_2D>::Item* previous_start_of_chain_item
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
					int k;
					for (k = 0; k < 4 ;++k)
					{
						if (start_of_chain_item->_children[k] == previous_start_of_chain_item)
							break;
					}
					/* we found the pointer */
					
					/* delete that item (that will automatically delete
					 * every descendant; end_of_chain_item exclusively) */
					delete start_of_chain_item->_children[k];
					
					/* link end_of_chain_item to start_of_chain_item->_children[j]... */
					start_of_chain_item->_children[k] = end_of_chain_item;
					end_of_chain_item->_parent = start_of_chain_item;
					
					/* set correct end_of_chain_item->_my_span_rectangle */
					end_of_chain_item->_my_span_rectangle
						= start_of_chain_item->_my_span_rectangle;
					
					T_Point_2D median_point;
					compute_median_point(start_of_chain_item->_my_span_rectangle,
						median_point);
					compute_new_span_rectangle(k, median_point,
						end_of_chain_item->_my_span_rectangle);
					/* done */
				}
				
				/* if there are more than one point in end_of_chain_item, split
				 * that item... */
				/** @todo DO NOT USE std::list::size()!!! */
				if (end_of_chain_item->_points.size() > 1)
				{
					typename Quadtree<T_Point_2D>::Item::D_Points::iterator
						begin_splice_iter = end_of_chain_item->_points.begin();
					++begin_splice_iter;
					
					typename Quadtree<T_Point_2D>::Item::D_Points points;
					points.splice(points.begin(), end_of_chain_item->_points,
						begin_splice_iter, end_of_chain_item->_points.end());
					
					typename Quadtree<T_Point_2D>::Item::D_Points::iterator
						iter = points.begin();
					typename Quadtree<T_Point_2D>::Item::D_Points::iterator
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
