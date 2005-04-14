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

#include "util/mlog.h"
using mapgeneration_util::MLog;

namespace rangereporting
{
	
	//-------------------------------------------------------------------------//
	//--- Definition section --------------------------------------------------//
	//-------------------------------------------------------------------------//
	
	template<typename T_Point_2D>
	class Quadtree;
	
	//---------------------------------------------------//
	//--- Output operator -------------------------------//
	//---------------------------------------------------//
	template<typename T_Point_2D>
	std::ostream& operator<<(std::ostream& out,
		const Quadtree<T_Point_2D>& quadtree);


	//---------------------------------------------------//
	//--- Quadtree & inner classes ----------------------//
	//---------------------------------------------------//
	template<typename T_Point_2D>
	class Quadtree
	{
		
		friend std::ostream& operator<< <> (std::ostream& out,
			const Quadtree<T_Point_2D>& quadtree);

		
		//---------------------------------------------------//
		//--- Item ------------------------------------------//
		//---------------------------------------------------//
		protected:
			
			class Item
			{
				
				public:
					
					typedef std::list<const T_Point_2D*> D_Points;
				
					Item* _parent;
	
					// _children[0] = north east
					// _children[1] = south east
					// _children[2] = south west
					// _children[3] = north west
					std::vector<Item*> _children;
					
					int _no_of_children;
	
					D_Points _points;
					
					Item();
					
					~Item();
			};
		
		
		//---------------------------------------------------//
		//--- Iterators -------------------------------------//
		//---------------------------------------------------//
		public:
			
			class const_iterator
			{
				
				public:
					
					const_iterator(const T_Point_2D* point, const Item* item);
					
					const T_Point_2D& operator*() const;
					const T_Point_2D* operator->() const;
					bool operator==(const const_iterator& iter) const;
					bool operator!=(const const_iterator& iter) const;
					
					
				protected:
					
					T_Point_2D* _point;
					Item* _item;
					
					const Item*
					item() const;
			};
			
			typedef const_iterator iterator;
			

		//---------------------------------------------------//
		//--- Trapezoid -------------------------------------//
		//---------------------------------------------------//
		public:
			
			class Trapezoid
			{
				
				protected:
					
					T_Point_2D _lower_left_corner;
					
					
					// have to be given clockwise or use verify_point_order!
					std::vector<T_Point_2D> _points;
					
					
					T_Point_2D _upper_right_corner;


					Trapezoid(const T_Point_2D& point_1, const T_Point_2D& point_2,
						const T_Point_2D& point_3, const T_Point_2D& point_4,
						bool verify_point_order = false);
					
					
					void
					verify_point_order();
			};


		//---------------------------------------------------//
		//--- Main class: Quadtree --------------------------//
		//---------------------------------------------------//
		public:
			
			Quadtree(int max_depth, const T_Point_2D& lower_left_corner,
				const T_Point_2D& upper_right_corner);
			

			~Quadtree();
			

			bool
			add_point(const T_Point_2D* point);
			
			
			static Quadtree<T_Point_2D>::Trapezoid
			build_trapezoid(const T_Point_2D& point_1, const T_Point_2D& point_2,
				const T_Point_2D& point_3, const T_Point_2D& point_4);
			
			
			int
			get_max_depth() const;
			
			
			void
			range_query(const Trapezoid& trapezoid,
				std::vector<const_iterator>& result) const;

			
			bool
			remove_point(const const_iterator& point_iter);
		
		protected:
			
			T_Point_2D _lower_left_corner;
			T_Point_2D _upper_right_corner;

			Item _root;
			
			int _max_depth;
			
			
			bool
			add_points(const T_Point_2D* point_1, const T_Point_2D* point_2,
				Item* current_item, int current_depth,
				T_Point_2D& current_lower_left_corner, T_Point_2D& upper_right_corner);
			
			
			void
			compute_new_bounds(int quarter, const T_Point_2D& median_point,
				T_Point_2D& lower_left_corner, T_Point_2D& upper_right_corner) const;
			
			
			int
			compute_quarter(const T_Point_2D& point,
				const T_Point_2D& lower_left_corner,
				const T_Point_2D& upper_right_corner, T_Point_2D& median_point) const;
			
			
			static int
			orientation(const T_Point_2D& segment_first_point,
				const T_Point_2D& segment_second_point, const T_Point_2D& test_point);
			
			
			void
			range_query(const T_Point_2D& lower_left_point,
				const T_Point_2D& upper_right_point,
				std::vector<const_iterator>& result) const;
			
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
		
		std::queue< typename Quadtree<T_Point_2D>::Item* > queue;
		typename Quadtree<T_Point_2D>::Item* root_item
			= const_cast< typename Quadtree<T_Point_2D>::Item* >(&quadtree._root);
		queue.push(root_item);
		
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
			out << "| NE = " << std::setw(10) << item->_children[0] << " |  =====" <<std::endl;
			out << "| SE = " << std::setw(10) << item->_children[1] << " |  |" << state_3 << "|" << state_0 << "|" <<std::endl;
			out << "|                "        <<                       " |  |---|" << std::endl;
			out << "| SW = " << std::setw(10) << item->_children[2] << " |  |" << state_2 << "|" << state_1 << "|" << std::endl;
			out << "| NW = " << std::setw(10) << item->_children[3] << " |  =====" << std::endl;
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
			out << "----------------------------------" << std::endl;
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
	Quadtree<T_Point_2D>::Item::Item()
	: _children(4), _no_of_children(0), _parent(0), _points()
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
	
	
	//---------------------------------------------------//
	//--- Iterators -------------------------------------//
	//---------------------------------------------------//
	template<typename T_Point_2D>
	Quadtree<T_Point_2D>::const_iterator::const_iterator(const T_Point_2D* point,
		const Item* item)
	: _point(point), _item(item)
	{}


	template<typename T_Point_2D>
	const T_Point_2D&
	Quadtree<T_Point_2D>::const_iterator::operator*() const
	{
		return *_point;
	}


	template<typename T_Point_2D>
	const T_Point_2D*
	Quadtree<T_Point_2D>::const_iterator::operator->() const
	{
		return _point;
	}
	
	
	template<typename T_Point_2D>
	bool
	Quadtree<T_Point_2D>::const_iterator::operator==(const const_iterator& iter) const
	{
		return ( _point == *iter );
	}
	
	
	template<typename T_Point_2D>
	bool
	Quadtree<T_Point_2D>::const_iterator::operator!=(const const_iterator& iter) const
	{
		return ( !operator==(iter) );
	}
	
	
	template<typename T_Point_2D>
	const typename Quadtree<T_Point_2D>::Item*
	Quadtree<T_Point_2D>::const_iterator::item() const
	{
		return _item;
	}
	
	
	//---------------------------------------------------//
	//--- Trapezoid -------------------------------------//
	//---------------------------------------------------//
	template<typename T_Point_2D>
	Quadtree<T_Point_2D>::Trapezoid::Trapezoid(const T_Point_2D& point_1,
		const T_Point_2D& point_2, const T_Point_2D& point_3,
		const T_Point_2D& point_4, bool verify_point_order)
	: _points()
	{
		_points.push_back(point_1);
		_points.push_back(point_2);
		_points.push_back(point_3);
		_points.push_back(point_4);
		
		_lower_left_corner = _points[0];
		_upper_right_corner = _points[0];
		for (int i = 1; i < 4; ++i)
		{
			if (_points[i][0] < _lower_left_corner[0])
				_lower_left_corner[0] = _points[i][0];

			if (_points[i][1] < _lower_left_corner[1])
				_lower_left_corner[1] = _points[i][1];

			if (_points[i][0] > _upper_right_corner[0])
				_upper_right_corner[0] = _points[i][0];

			if (_points[i][1] > _upper_right_corner[1])
				_upper_right_corner[1] = _points[i][1];
		}
		

		if (verify_point_order)
			verify_point_order();
	}


	template<typename T_Point_2D>
	void
	Quadtree<T_Point_2D>::Trapezoid::verify_point_order()
	{
		for (int i = 0; i < 4; ++i)
		{
			int orientation
				= orientation(_points[i % 4], _points[(i+1) % 4], _points[(i+2) % 4]);
			
			if (orientation < 0)
			{
				T_Point_2D temp = _points[(i+1) % 4];
				_points[(i+1) % 4] = _points[(i+2) % 4];
				_points[(i+2) % 4] = temp;
			}
		}
	}
	
	
	//---------------------------------------------------//
	//--- Main class: Quadtree --------------------------//
	//---------------------------------------------------//
	template<typename T_Point_2D>
	Quadtree<T_Point_2D>::Quadtree(int max_depth,
		const T_Point_2D& lower_left_corner, const T_Point_2D& upper_right_corner)
	: _lower_left_corner(lower_left_corner),
		_upper_right_corner(upper_right_corner),
		_max_depth(max_depth),
		_root()
	{
	}


	template<typename T_Point_2D>
	Quadtree<T_Point_2D>::~Quadtree()
	{
		for (int i = 0; i < 4 ;++i)
		{
			if (_root._children[i] != 0)
			{
				delete _root._children[i];
				_root._children[i] = 0;
			}
		}
	}
	
	
	template<typename T_Point_2D>
	bool
	Quadtree<T_Point_2D>::add_point(const T_Point_2D* point)
	{
//		mlog(MLog::debug, "Quadtree") << "Add point:" << *point << "\n";
		
		Item* current_item = &_root;
		T_Point_2D current_lower_left_corner = _lower_left_corner;
		T_Point_2D current_upper_right_corner = _upper_right_corner;
		int current_depth = 0;
		
		bool should_continue = true;
		while (should_continue)
		{
			if (current_depth < _max_depth)
			{
//				mlog(MLog::debug, "Quadtree") << "\tcurrent_depth < _max_depth\n";
				// not at last level...
				if (current_item->_no_of_children == 0)
				{
//					mlog(MLog::debug, "Quadtree") << "\tcurrent_item->_no_of_children == 0\n";
					// no children: possibly we are right here
					if (current_item->_points.empty())
					{
//						mlog(MLog::debug, "Quadtree") << "\tcurrent_item->_points.empty()\n";
						// we are right!
						current_item->_points.push_back(point);
						should_continue = false;
						
					} else // explicit: if ( !current_item->_points.empty() )
					{
//						mlog(MLog::debug, "Quadtree") << "\t!current_item->_points.empty()\n";
						// well, here is already a point: split...
						const T_Point_2D* old_point = current_item->_points.front();
						
						if (*old_point == *point)
						{
							return false;
						} else
						{
							current_item->_points.clear();
						
							return add_points(old_point, point, current_item, current_depth,
								current_lower_left_corner, current_upper_right_corner);
						}
						
					} // end: if (current_item->_points.empty())
					
				} else // explicit: if (current_item->_no_of_children != 0)
				{
//					mlog(MLog::debug, "Quadtree") << "\tcurrent_item->_no_of_children != 0\n";
					// we have children: compute the quarter where to insert the point...
					T_Point_2D median_point;
					int point_quarter = compute_quarter(*point,
						current_lower_left_corner, current_upper_right_corner,
						median_point);
					
					if (current_item->_children[point_quarter] == 0)
					{
//						mlog(MLog::debug, "Quadtree") << "\tcurrent_item->_children[" << point_quarter << "] == 0\n";
						// this child is 0: insert point and be happy!
						current_item->_children[point_quarter] = new Item;
						current_item->_children[point_quarter]->_parent = current_item;
						++(current_item->_no_of_children);
						
						current_item->_children[point_quarter]->_points.push_back(point);
						should_continue = false;
						
					} else // explicit: if (current_item->_children[point_quarter] != 0)
					{
//						mlog(MLog::debug, "Quadtree") << "\tcurrent_item->_children[" << point_quarter << "] != 0\n";
						// child already exists: descend the tree...
						current_item = current_item->_children[point_quarter];
						compute_new_bounds(point_quarter, median_point,
							current_lower_left_corner, current_upper_right_corner);
					} // end: if (current_item->_children[point_quarter] == 0)
				} // end: if (current_item->_no_of_children == 0)
				
			} else // explicit: if (current_depth >= _max_depth)
			{
//				mlog(MLog::debug, "Quadtree") << "\tcurrent_depth >= _max_depth\n";
				current_item->_points.push_back(point);
				should_continue = false;
			} // end: if (current_depth < _max_depth)
			
			++current_depth;
		} // end: while(should_continue)
		
		return true;		
	}
	
	
	template<typename T_Point_2D>
	typename Quadtree<T_Point_2D>::Trapezoid
	Quadtree<T_Point_2D>::build_trapezoid(const T_Point_2D& point_1,
		const T_Point_2D& point_2, const T_Point_2D& point_3,
		const T_Point_2D& point_4)
	{
		return Trapeziod(point_1, point_2, point_3, point_4, true);
	}
	
	
	template<typename T_Point_2D>
	int
	Quadtree<T_Point_2D>::get_max_depth() const
	{
		return _max_depth;
	}
	
	
	template<typename T_Point_2D>
	void
	Quadtree<T_Point_2D>::range_query(const Trapezoid& trapezoid,
		std::vector<const_iterator>& result) const
	{
		range_query(trapezoid._lower_left_corner, trapezoid._upper_right_corner,
			result);
		
		typename std::vector<const_iterator>::iterator iter = result.begin();
		while (iter != result.end())
		{
			bool erased_point = false;
			
			for (int i = 0; i < 4; ++i)
			{
				int orientation	= orientation(trapeziod._points[i % 4],
					trapeziod._points[(i+1) % 4], *iter);
				if (orientation < 0)
				{
					iter = result.erase(iter);
					i = 4; // break for loop!
					erased_point = true;
				}
			}
			
			if ( !erased_point )
				++iter;
		}
	}
	
	
	template<typename T_Point_2D>
	bool
	Quadtree<T_Point_2D>::remove_point(const const_iterator& point_iter)
	{
		// find point in given point_iter.item() and remove it...
		Item* point_item = point_iter.item();
		
		typename Item::D_Points::iterator iter = point_item->_points.begin();
		typename Item::D_Points::iterator iter_end = point_item->_points.end();
		for (; iter != iter_end; ++iter)
		{
			if (*iter == *point)
				break;
		}
		
		if (point_iter == iter_end)
			return false;
		
		point_item->_points.erase(iter);
		// done!
		
		// walk up tree and "repair it"...
		if (point_item->_points.empty())
		{
			Item* previous_item = point_item;
			Item* current_item = point_item->_parent;
			
			while ( (previous_item->_no_of_children == 0) && (current_item != 0) )
			{
				int i = 0;
				for (; i < 4; ++i)
				{
					if (current_item->_children[i] == previous_item)
						break;
				}
				
				delete previous_item;
				current_item->_children[i] = 0;
				--(current_item->_no_of_children);
				
				previous_item = current_item;
				current_item = current_item->_parent;
			}
		}
		
	}
	
	
	template<typename T_Point_2D>
	bool
	Quadtree<T_Point_2D>::add_points(const T_Point_2D* point_1,
		const T_Point_2D* point_2, Item* current_item, int current_depth,
		T_Point_2D& current_lower_left_corner, T_Point_2D& current_upper_right_corner)
	{
//		mlog(MLog::debug, "Quadtree") << "Add points: " << *point_1 << ", " << *point_2 << "\n";
		
		if (current_depth < _max_depth)
		{
//			mlog(MLog::debug, "Quadtree") << "\tcurrent_depth < _max_depth\n";
			T_Point_2D median_point;
			int point_1_quarter = compute_quarter(*point_1, current_lower_left_corner,
				current_upper_right_corner, median_point);
			int point_2_quarter = compute_quarter(*point_2, current_lower_left_corner,
				current_upper_right_corner, median_point);
			
			if (point_1_quarter == point_2_quarter)
			{
//				mlog(MLog::debug, "Quadtree") << "\tpoint_1_quarter == point_2_quarter\n";
				current_item->_children[point_1_quarter] = new Item;
				current_item->_children[point_1_quarter]->_parent = current_item;
				++(current_item->_no_of_children);
				
				compute_new_bounds(point_1_quarter, median_point,
					current_lower_left_corner, current_upper_right_corner);
				
				return add_points(point_1, point_2,
					current_item->_children[point_1_quarter], current_depth + 1,
					current_lower_left_corner, current_upper_right_corner);
					
			} else // explicit: if (point_1_quarter != point_2_quarter)
			{
//				mlog(MLog::debug, "Quadtree") << "\tpoint_1_quarter != point_2_quarter\n";
				current_item->_children[point_1_quarter] = new Item;
				current_item->_children[point_1_quarter]->_parent = current_item;
				++(current_item->_no_of_children);

				current_item->_children[point_1_quarter]->_points.push_back(point_1);
				
				current_item->_children[point_2_quarter] = new Item;
				current_item->_children[point_2_quarter]->_parent = current_item;
				++(current_item->_no_of_children);
				
				current_item->_children[point_2_quarter]->_points.push_back(point_2);
				
			} // end: if (point_1_quarter == point_2_quarter)
			
		} else // explicit: if (current_depth >= _max_depth)
		{
//			mlog(MLog::debug, "Quadtree") << "\tcurrent_depth >= _max_depth\n";
			current_item->_points.push_back(point_1);
			current_item->_points.push_back(point_2);
		}
		
		return true;
	}
	
	
	template<typename T_Point_2D>
	void
	Quadtree<T_Point_2D>::compute_new_bounds(int quarter,
		const T_Point_2D& median_point, T_Point_2D& lower_left_corner,
		T_Point_2D& upper_right_corner) const
	{
		switch (quarter)
		{
			case 0:
				lower_left_corner[0] = median_point[0];
				lower_left_corner[1] = median_point[1];
				break;
			
			case 1:
				lower_left_corner[0] = median_point[0];
				upper_right_corner[1] = median_point[1];
				break;
			
			case 2:
				upper_right_corner[0] = median_point[0];
				upper_right_corner[1] = median_point[1];
				break;
			
			case 3:
				upper_right_corner[0] = median_point[0];
				lower_left_corner[1] = median_point[1];
				break;
		}
	}
	
	
	template<typename T_Point_2D>
	int
	Quadtree<T_Point_2D>::compute_quarter(const T_Point_2D& point,
		const T_Point_2D& lower_left_corner,
		const T_Point_2D& upper_right_corner,
		T_Point_2D& median_point) const
	{
		median_point[0] = (upper_right_corner[0] + lower_left_corner[0]) / 2.0;
		median_point[1] = (upper_right_corner[1] + lower_left_corner[1]) / 2.0;
		
		int next_item;
		if (point[0] >= median_point[0])
		{
			if (point[1] >= median_point[1])
				return 0;
			else
				return 1;

		} else
		{
			if (point[1] >= median_point[1])
				return 3;
			else
				return 2;
		}
	}
	
	
	template<typename T_Point_2D>
	int
	Quadtree<T_Point_2D>::orientation(const T_Point_2D& segment_first_point,
		const T_Point_2D& segment_second_point, const T_Point_2D& test_point)
	{
		/** @todo put it to the configuration file! Or somewhere else. */
		double EPSILON = 0.0001;
		
		double determinant
			= segment_first_point[0] * segment_second_point[1]
				+ segment_first_point[1] * test_point[0]
				+ segment_second_point[0] * test_point[1];
				- segment_second_point[1] * test_point[0]
				- segment_first_point[0] * test_point[1]
				- segment_first_point[1] * segment_second_point[0];
		
		if (abs(determinant) < EPSILON)
			return 0;
		
		if (determinant < 0)
			return -1;
		else
			return 0;
	}
	
	
	template<typename T_Point_2D>
	void
	Quadtree<T_Point_2D>::range_query(const T_Point_2D& lower_left_point,
		const T_Point_2D& upper_right_point,
		std::vector<const_iterator>& result) const
	{
/*		Item* current_item = &_root;
		T_Point_2D current_lower_left_corner = _lower_left_corner;
		T_Point_2D current_upper_right_corner = _upper_right_corner;
		int current_depth = 0;
		
		bool should_continue = true;
		while (should_continue)
		{
			if (current_depth < _max_depth)
			{
				if ( current_item->_no_of_children == 0)
				{
				}
				
				
				
			T_Point_2D median_point;
			int lower_left_point_quarter = compute_quarter(lower_left_point,
				current_lower_left_corner, current_upper_right_corner, median_point);
			int upper_right_point_quarter = compute_quarter(upper_right_point,
				current_lower_left_corner, current_upper_right_corner, median_point);
			
			if (lower_left_point_quarter == upper_right_point_quarter)
		}*/
	}
	
	
/*	template<typename T_Point_2D>
	void
	Quadtree<T_Point_2D>::range_query(const T_Point_2D* point_1,
		const T_Point_2D* point_2, Item* current_item, int current_depth,
		T_Point_2D& current_lower_left_corner, T_Point_2D& upper_right_corner,
		std::vector<const_iterator>& result) const
	{
		
	}*/
}

#endif //QUADTREE_H
