/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef QUADRANGLE_H
#define QUADRANGLE_H

//#include <ostream>
#include <cassert>

namespace rangereporting
{
	
	//-------------------------------------------------------------------------//
	//--- Declaration section -------------------------------------------------//
	//-------------------------------------------------------------------------//
	
	template<typename T_Point_2D>
	class Quadrangle;
	
	
	template<typename T_Point_2D>
	std::ostream&
	operator<<(std::ostream& out, const Quadrangle<T_Point_2D>& segment);
	
}

#include "axisparallelrectangle.h"
#include "helperfunctions.h"
#include "polygon.h"
#include "segment.h"
#include "triangle.h"
#include "util/constants.h"

namespace rangereporting
{
	
	//-------------------------------------------------------------------------//
	//--- Definition section --------------------------------------------------//
	//-------------------------------------------------------------------------//
	
	template<typename T_Point_2D>
	class Quadrangle : public Polygon<T_Point_2D>
	{
		
		public:
			
			Quadrangle();
			
			
			Quadrangle(const std::vector<T_Point_2D>& points);
			
			
			virtual double
			area_size() const;
			
			
			inline const AxisParallelRectangle<T_Point_2D>&
			bounding_box() const;
			
			
			inline void
			build_equidistant_quadrangle(const Segment<T_Point_2D>& segment,
				double distance);
			
			
			void
			build_quadrangle(const Segment<T_Point_2D>& segment,
				double perpendicular_distance, double head_distance,
				double tail_distance);
			
			
			void
			build_quadrangle(const T_Point_2D& point,
				double perpendicular_distance);
			
			
			virtual bool
			contains(const T_Point_2D& point) const;
			
			
		protected:
			
			virtual inline void
			after_set_point(int index, const T_Point_2D& point);
			
			
			virtual inline void
			after_set_points(const std::vector<T_Point_2D>& points);
			
			
		private:
			
			enum Status
			{
				_convex_left_system,
				_convex_right_system,
				_convex_no_system,
				_concave,
				_crossed_first_edge, // with third edge
				_crossed_second_edge // with forth edge
			};
			
			
			mutable double _cached_area_size;
			mutable bool _cached_area_size_is_valid;
			
			mutable AxisParallelRectangle<T_Point_2D> _cached_bounding_box;
			mutable bool _cached_bounding_box_is_valid;
			
			mutable Status _cached_status;
			mutable bool _cached_status_is_valid;
			
			
			void
			calculate_bounding_box() const;
			
			
			void
			calculate_status() const;
			
	};
	
	
	//-------------------------------------------------------------------------//
	//--- Implementation section ----------------------------------------------//
	//-------------------------------------------------------------------------//
	
	template<typename T_Point_2D>
	std::ostream&
	operator<<(std::ostream& out, const Quadrangle<T_Point_2D>& quadtangle)
	{
		out << "Quadrangle: The points: " << std::endl;;
		for (int i = 0; i < quadtangle.points().size(); ++i)
			out << "\t" << quadtangle.get_point(i) << std::endl;
		
		return out;
	}
	
	
	template<typename T_Point_2D>
	Quadrangle<T_Point_2D>::Quadrangle()
	: Polygon<T_Point_2D>(4),
		_cached_area_size(0.0), _cached_area_size_is_valid(true),
		_cached_bounding_box(), _cached_bounding_box_is_valid(true),
		_cached_status(_convex_no_system), _cached_status_is_valid(true)
	{
	}
	
	
	template<typename T_Point_2D>
	Quadrangle<T_Point_2D>::Quadrangle(const std::vector<T_Point_2D>& points)
	: Polygon<T_Point_2D>(points),
		_cached_area_size(0.0), _cached_area_size_is_valid(false),
		_cached_bounding_box(), _cached_bounding_box_is_valid(false),
		_cached_status(), _cached_status_is_valid(false)
	{
	}
	
	
	template<typename T_Point_2D>
	inline void
	Quadrangle<T_Point_2D>::after_set_point(int index, const T_Point_2D& point)
	{
		_cached_area_size_is_valid = false;
		_cached_bounding_box_is_valid = false;
		_cached_status_is_valid = false;
	}
	
	
	template<typename T_Point_2D>
	inline void
	Quadrangle<T_Point_2D>::after_set_points(
		const std::vector<T_Point_2D>& points)
	{
		_cached_area_size_is_valid = false;
		_cached_bounding_box_is_valid = false;
		_cached_status_is_valid = false;
	}
	

// precise but uncomplete version :-)
/*	
	template<typename T_Point_2D>
	double
	Quadrangle<T_Point_2D>::area_size() const
	{
		if (!_cached_area_size_is_valid)
		{
			calculate_status();
			
			// Okay, now we can calculate the area size.
			if (_cached_status == _convex_no_system)
			{
				_cached_area_size = 0.0;
			} else if ( (_cached_status == _crossed_first_edge)
				&& (_cached_status == _crossed_second_edge) )
			{
				// _cached_status shows us which edges cross.
				// So first we set an offset.
				int offset = (_cached_status == _crossed_first_edge ? 0 : 1);
				
				// The situation can be described as the following:
				// (o = offset)
				//
				//                        2+o
				//                      x
				//                 x ´  |
				//  0+o    z      ´     |
				//   x---------+--------x
				//   |      ´       y     1+o
				//   |   ´w
				//   x
				// 3+o
				//
				// Give the edges some names:
				// (everything plus offset!)
				// a = 3 -> 0
				// b = 0 -> 1
				// c = 1 -> 2
				// d = 2 -> 3
				//
				// Some equations that holds:
				// b = y + z
				// d = w + x
				//
				// To calculate the area size we need the area sizes of the two
				// triangles |awz| and |cxy|. We need w, x, y, z.
				
				double a = this->get_point((3+offset) % 4).distance(
					this->get_point(offset));
				double b = this->get_point(offset).distance(
					this->get_point(1+offset));
				double c = this->get_point(1+offset).distance(
					this->get_point(2+offset));
				double d = this->get_point(2+offset).distance(
					this->get_point((3+offset) % 4));
				
				double z = (a*b) / (a+c);
				double y = b-z;
				
				double w = (a*d) / (a+c);
				double x = d-w;
				
				_cached_area_size = herons_area_size(a, w, z)
					+ herons_area_size(c, x, y);
				
				_cached_area_size_is_valid = true;
				
			} else
			{
				double distance_0_to_1
					= this->get_point(0).distance(this->get_point(1));
				double distance_1_to_2
					= this->get_point(1).distance(this->get_point(2));
				double distance_2_to_3
					= this->get_point(2).distance(this->get_point(3));
				double distance_3_to_0
					= this->get_point(3).distance(this->get_point(0));
				
				double distance_diagonale
					= this->get_point(2).distance(this->get_point(0));
				
				_cached_area_size = herons_area_size(distance_0_to_1,
					distance_1_to_2, distance_diagonale) + herons_area_size(
					distance_2_to_3, distance_3_to_0, distance_diagonale);
				
				_cached_area_size_is_valid = true;
			}
		}
		
		return _cached_area_size;
	}
*/
	
	
	// only an approximation!!!
	template<typename T_Point_2D>
	double
	Quadrangle<T_Point_2D>::area_size() const
	{
		if (!_cached_area_size_is_valid)
		{
			double distance_0_to_1
				= this->get_point(0).distance(this->get_point(1));
			double distance_1_to_2
				= this->get_point(1).distance(this->get_point(2));
			double distance_2_to_3
				= this->get_point(2).distance(this->get_point(3));
			double distance_3_to_0
				= this->get_point(3).distance(this->get_point(0));
			
			double distance_diagonale
				= this->get_point(2).distance(this->get_point(0));
			
			_cached_area_size = herons_area_size(distance_0_to_1,
				distance_1_to_2, distance_diagonale) + herons_area_size(
				distance_2_to_3, distance_3_to_0, distance_diagonale);
			
			_cached_area_size_is_valid = true;
		}
		
		return _cached_area_size;
	}
	
	
	template<typename T_Point_2D>
	const AxisParallelRectangle<T_Point_2D>&
	Quadrangle<T_Point_2D>::bounding_box() const
	{
		calculate_bounding_box();
		
		return _cached_bounding_box;
	}
	
	
	template<typename T_Point_2D>
	void
	Quadrangle<T_Point_2D>::build_equidistant_quadrangle(
		const Segment<T_Point_2D>& segment, double distance)
	{
		build_quadrangle(segment, distance, distance, distance);
	}
	
	
	template<typename T_Point_2D>
	void
	Quadrangle<T_Point_2D>::build_quadrangle(const Segment<T_Point_2D>& segment,
		double perpendicular_distance, double head_distance,
		double tail_distance)
	{
		assert(perpendicular_distance > 0.0);
		assert(tail_distance >= 0.0);
		assert(head_distance >= 0.0);
		
		// compute point-to-point bearing
		double p2p_bearing
			= segment.get_point(0).bearing(segment.get_point(1)); // in radian
		
		// compute the four corners (a, b, ...) of the quadrangle:
		//
		//   tail                 head
		//
		// a......p           p..........b       /\
		// .      |           |          .       |  perpendicular
		// .      |           |          .       |
		// <------t---------->h----------h0      \/
		//
		// t = point(0), h = point(1)
		//
		// dist(t,a) = sqrt(tail² + perp²)
		// arc(tp,ta) = PI/2 + arccos(tp/ta)
		//
		// dist(h,b) = sqrt(head² + perp²)
		// arc(hh0,hb) = arccos(hh0, hb)
		
		// calculate the corresponding values
		double dist_t2a = sqrt(tail_distance * tail_distance
			+ perpendicular_distance * perpendicular_distance);
		double relative_tail_bearing
			= PI/2.0 + acos(perpendicular_distance / dist_t2a);
		
		double dist_h2b = sqrt(head_distance * head_distance
			+ perpendicular_distance * perpendicular_distance);
		double relative_head_bearing = acos(head_distance / dist_h2b);
		
		double current_bearing;
		T_Point_2D point;
		
		// set the two back points
		current_bearing = p2p_bearing - relative_tail_bearing;
		point = segment.get_point(0).compute_geo_coordinate(current_bearing,
			dist_t2a);
		this->set_point(0, point);
		
		current_bearing = p2p_bearing + relative_tail_bearing;
		point = segment.get_point(0).compute_geo_coordinate(current_bearing,
			dist_t2a);
		this->set_point(1, point);
		
		// set the two front points
		current_bearing = p2p_bearing + relative_head_bearing;
		point = segment.get_point(1).compute_geo_coordinate(current_bearing,
			dist_h2b);
		this->set_point(2, point);
		
		current_bearing = p2p_bearing - relative_head_bearing;
		point = segment.get_point(1).compute_geo_coordinate(current_bearing,
			dist_h2b);
		this->set_point(3, point);
	}
	
	
	template<typename T_Point_2D>
	void
	Quadrangle<T_Point_2D>::build_quadrangle(const T_Point_2D& point,
		double perpendicular_distance)
	{
		assert(perpendicular_distance > 0.0);
		
		double distance2edge = sqrt(2.0) * perpendicular_distance;
		double current_bearing = PI/4.0;
		T_Point_2D new_point;
		
		for (int i = 0; i < 4; ++i)
		{
			new_point = point.compute_geo_coordinate(
				current_bearing, distance2edge);
			this->set_point(i, new_point);
			
			current_bearing += PI/2.0;
		}
	}
	
	
	template<typename T_Point_2D>
	void
	Quadrangle<T_Point_2D>::calculate_bounding_box() const
	{
		if (!_cached_bounding_box_is_valid)
		{
			double min_x = this->get_point(0)[0];
			double min_y = this->get_point(0)[1];
			
			double max_x = min_x;
			double max_y = min_y;
			
			for (int i = 1; i < 4; ++i)
			{
				if (this->get_point(i)[0] < min_x)
					min_x = this->get_point(i)[0];
				else if (this->get_point(i)[0] > max_x)
					max_x = this->get_point(i)[0];
				
				if (this->get_point(i)[1] < min_y)
					min_y = this->get_point(i)[1];
				else if (this->get_point(i)[1] > max_y)
					max_y = this->get_point(i)[1];
			}
			
			T_Point_2D lower_left_corner;
			lower_left_corner[0] = min_x;
			lower_left_corner[1] = min_y;
			
			T_Point_2D upper_right_corner;
			upper_right_corner[0] = max_x;
			upper_right_corner[1] = max_y;
			
			_cached_bounding_box.set_point(0, lower_left_corner);
			_cached_bounding_box.set_point(1, upper_right_corner);
			_cached_bounding_box.validate_corners();
			
			_cached_bounding_box_is_valid = true;
		}
	}
	

// precise but uncomplete version :-)
/** @todo complete it! */
	template<typename T_Point_2D>
	void
	Quadrangle<T_Point_2D>::calculate_status() const
	{
		if (!_cached_status_is_valid)
		{
			// First take a look, if we have a regular quadrangle or
			// a crossed one.
			Triangle<T_Point_2D> test_triangle;
			test_triangle.set_point(0, this->get_point(0));
			test_triangle.set_point(1, this->get_point(1));
			test_triangle.set_point(2, this->get_point(2));
			bool test_triangle_system = test_triangle.is_left_system();
		
			int i = 1;
			bool found_crossed_quadrangle = false;
			while ( (!found_crossed_quadrangle) && (i < 4) ) 
			{
				// optimisation possibilities:
				// i < 3 should be enough. think about it in more detail.
				
				test_triangle.set_point( (i-1)%3, this->get_point((i+2)%4) );
				
				found_crossed_quadrangle
					= (test_triangle_system != test_triangle.is_left_system());
				
				++i;
			}
			
			if (found_crossed_quadrangle)
			{
				if (i % 2 == 1)
					_cached_status = _crossed_first_edge;
				else
					_cached_status = _crossed_second_edge;
			} else
			{
				if (test_triangle_system) // == true -> left sytem (see above)
					_cached_status = _convex_left_system;
				else
					_cached_status = _convex_right_system;
			}
			
			_cached_status_is_valid = true;
		}
	}

	
	
// precise but uncomplete version :-)
/** @todo complete it! */
	template<typename T_Point_2D>
	bool
	Quadrangle<T_Point_2D>::contains(const T_Point_2D& point) const
	{
		calculate_status();
		calculate_bounding_box();
		
		if ( !_cached_bounding_box.contains(point) )
			return false;
		
		if ( (_cached_status == _crossed_first_edge)
			|| (_cached_status == _crossed_second_edge) )
		{
			// _cached_status_crossing_flag shows us which edges cross.
			// So first we set an offset.
			int offset = (_cached_status == _crossed_first_edge ? 0 : 1);
			
			// The situation can be described as the following:
			// (o = offset)
			//
			//                        2+o
			//                      x
			//                 x ´  |
			//  0+o    z      ´     |
			//   x---------+--------x
			//   |      ´       y     1+o
			//   |   ´w
			//   x
			// 3+o
			//
			// Give the edges some names:
			// (everything plus offset!)
			// a = 3 -> 0
			// b = 0 -> 1
			// c = 1 -> 2
			// d = 2 -> 3
			//
			// Some equations that holds:
			// b = y + z
			// d = w + x
			//
			// We need the crossing point which we can interpolate from
			// the segment 0->1 and the weight z/b. It holds z/b = a/(a+c).
			// So need only 'a' and 'c'.
			
			double a = this->get_point((3+offset) % 4).distance(
				this->get_point(offset));
//			double b = this->get_point(offset).distance(
//				this->get_point(1+offset));
			double c = this->get_point(1+offset).distance(
				this->get_point(2+offset));
//			double d = this->get_point(2+offset).distance(
//				this->get_point((3+offset) % 4));
			
			T_Point_2D crossing_point = T_Point_2D::interpolate(
				this->get_point(0+offset), this->get_point(1+offset),
				a / (a+c));
			
			// Okay, now we can decide if the test point is in |awz| or |cxy|.
			Triangle<T_Point_2D> the_one;
			the_one.set_point(0, this->get_point(0+offset));
			the_one.set_point(1, crossing_point);
			the_one.set_point(2, this->get_point((3+offset) % 4));
			
			Triangle<T_Point_2D> the_other;
			the_other.set_point(0, this->get_point(1+offset));
			the_other.set_point(1, crossing_point);
			the_other.set_point(2, this->get_point(2+offset));
			
			return (the_one.contains(point) || the_other.contains(point));
			
		} else
		{
			// _cached_status tells us which system the quadrangle has.
			// So we test against it.
			if (_cached_status == _convex_left_system)
			{
				for (int i = 0; i < 4; ++i)
				{
					double relative_pos = relative_position(
						this->get_point(i % 4), this->get_point((i+1) % 4),
						point);
					if (relative_pos < 0.0) // < 0 means: a right system!
					{
						return false;
					}
				}
			
			} else
			{
				for (int i = 0; i < 4; ++i)
				{
					double relative_pos = relative_position(
						this->get_point(i % 4), this->get_point((i+1) % 4),
						point);
					if (relative_pos > 0.0) // ONLY DIFFERENCE
					{
						return false;
					}
				}
			}
			
			return true;
		}
	}

	
} //namespace rangereporting

#endif //QUADRANGLE_H
