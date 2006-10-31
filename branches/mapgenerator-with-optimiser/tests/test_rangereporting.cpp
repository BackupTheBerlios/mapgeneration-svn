/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include <cstdlib>
#include <exception>
#include <iostream>
#include <list>

#include "util/geocoordinate.h"
#include "util/rangereporting/quadtree.h"
//#include "util/rangereporting/helperfunctions.h"
#include "util/rangereporting/quadrangle.h"
#include "util/rangereporting/triangle.h"

using namespace std;
using namespace rangereporting;
using namespace mapgeneration_util;

class MyPoint
{
	public:
		
		double _a[2];
		
		MyPoint()
		{
			_a[0] = 0.0;
			_a[1] = 0.0;
		}
		
		MyPoint(double a, double b)
		{
			_a[0] = a;
			_a[1] = b;
		}
		
		double
		distance(const MyPoint& p) const
		{
			return sqrt( pow(_a[0] - p._a[0], 2.0) + pow (_a[1] - p._a[1], 2.0) );
		}
		
		static MyPoint
		interpolate(const MyPoint& p1, const MyPoint& p2, double weight_on_first)
		{
			double x = weight_on_first * p1._a[0] + (1.0 - weight_on_first) * p2._a[0];
			double y = weight_on_first * p1._a[1] + (1.0 - weight_on_first) * p2._a[1];
			MyPoint result(x, y);
			
			return result;
		}
		
		double
		operator[](int index) const
		{
			return _a[index];
		}
		
};

int main(int argc, char* argv[])
{
/*	MyPoint a(0.0, 0.0);
	MyPoint b(1.0, 0.0);
	MyPoint c(1.0, 1.0);
//	MyPoint d(0.1, 0.0999999999999999);
//	MyPoint d(0.1, 0.1);
	MyPoint d(0.1, 0.1000000000000001);
	
	Triangle<MyPoint> t1;
	t1.set_point(0, a);
	t1.set_point(1, b);
	t1.set_point(2, c);
	
	std::cout << std::endl;
	std::cout << "Triangle" << std::endl;
	std::cout << std::boolalpha << t1.is_left_system() << std::endl;
	std::cout << t1.area_size() << std::endl;
	std::cout << t1.contains(d) << std::endl;
	
	MyPoint e(0.0, 1.0);
	Quadrangle<MyPoint> q;
	q.set_point(0, a);
	q.set_point(1, c);
	q.set_point(2, b);
	q.set_point(3, e);
	
	std::cout << std::endl;
	std::cout << "Quadrangle" << std::endl;
	std::cout << q.area_size() << std::endl;
	std::cout << q.contains(d) << std::endl;
*/	
	GeoCoordinate lower_left;
	lower_left[0] = 0.0;
	lower_left[1] = 0.0;

	GeoCoordinate upper_right;
	upper_right[0] = 1.0;
	upper_right[1] = 1.0;
	
	
	AxisParallelRectangle<GeoCoordinate> rectangle;
	rectangle.set_point(0, lower_left);
	rectangle.set_point(1, upper_right);
	rectangle.validate_corners();
	
	std::cout << "Set span rectangle." << std::endl;
	
	int nodes = 10;
	try
	{
		if (argc > 1)
			nodes = atoi(argv[1]);
	} catch (std::exception e)
	{};
	
	std::cout << "Read #numbers: " << nodes << std::endl;
	
	Quadtree<GeoCoordinate> quadtree;
	quadtree.set_span_rectangle(rectangle);
	quadtree.set_max_depth(3);
	srand(0);
	
	std::cout << "Inited Quadtree" << std::endl;
	
	
	for (int i = 0; i < nodes; ++i)
	{
		GeoCoordinate a;
		a[0] = (static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
		a[1] = (static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
		quadtree.add_point(a);
	}
	
	for (int i = 0; i < nodes; i += 1)
		quadtree.move_point(i, GeoCoordinate(0.1, 0.1));
	
	quadtree.move_point(0, GeoCoordinate(0.5, 0.5));
	
	std::cout << quadtree << std::endl;
	
	std::vector< Quadtree<GeoCoordinate>::D_IndexType > results;
	AxisParallelRectangle<GeoCoordinate> qr;
	qr.set_point(0, GeoCoordinate(0.4, 0.4));
	qr.set_point(1, GeoCoordinate(0.6, 0.6));
	qr.validate_corners();
	
	quadtree.range_query(qr, results);
	
	std::cout << "Query result: size=" << results.size() << std::endl;
	std::vector< Quadtree<GeoCoordinate>::D_IndexType >::const_iterator iter = results.begin();
	std::vector< Quadtree<GeoCoordinate>::D_IndexType >::const_iterator iter_end = results.end();
	for (; iter != iter_end; ++iter)
	{
		std::cout << *iter << ", ";
	}
	std::cout << std::endl;
/*	for (int i = 0; i < nodes; ++i)
	{
		GeoCoordinate a;
		a[0] = (static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
		a[1] = (static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
		quadtree.add_point(a);
	}
	std::cout << "Added nodes" << std::endl;
	
	for (int i = 100; i < 500; i += 7)
		quadtree.remove_point(i);
	
	
	for (int i = 0; i < 10000; ++i)
	{
		GeoCoordinate a;
		a[0] = (static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
		a[1] = (static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
		
		double some_random_value
			= static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
		double sign = (some_random_value < 0.5 ? -1.0 : 1.0);
		
		GeoCoordinate b;
		b[0] = a[0] + (sign * 0.05);
		b[1] = a[1] + (sign * 0.0125);
		
		GeoCoordinate c;
		c[0] = a[0] - (sign * 0.025);
		c[1] = a[1] + (sign * 0.025);
	
		GeoCoordinate d;
		d[0] = a[0] + (sign * 0.025);
		d[1] = a[1] + (sign * 0.0375);
		
		Trapezoid<GeoCoordinate> query_trapezoid(a, b, c, d);
		
		std::vector< Quadtree<GeoCoordinate>::D_IndexType > result;
		quadtree.range_query(query_trapezoid, result);
		
//		std::cout << result.size() << "  ";
	}
	std::cout << "Made 10000 queries." << std::endl;
	
	for (int i = 0 ; i < 1000; ++i)
	{
		GeoCoordinate a;
		a[0] = (static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
		a[1] = (static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
		
		double some_random_value
			= static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
		double sign = (some_random_value < 0.5 ? -1.0 : 1.0);
		
		GeoCoordinate b;
		b[0] = a[0] + (sign * 0.01);
		b[1] = a[1] + (sign * 0.01);
		
		Rectangle<GeoCoordinate> query_rectangle;
		query_rectangle.set_corners(a, b);
		
		std::vector< Quadtree<GeoCoordinate>::D_IndexType > result;
		quadtree.range_query(query_rectangle, result);
		
		if (result.size() > 0)
		{
			GeoCoordinate c;
			some_random_value
				= static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
			if (some_random_value < 0.5 )
			{
				c[0] = a[0] + 0.001;
				c[1] = a[1] + 0.001;
			} else
			{
				c[0] = a[0] + 0.1;
				c[1] = a[1] + 0.1;
			}
				
			bool move_result = quadtree.move_point(result.front(), c);
			if (!move_result)
				std::cout << "Out of range: " << c << std::endl;
		}
	}
	
	std::cout << std::endl;
*/	
	return 0;
}
