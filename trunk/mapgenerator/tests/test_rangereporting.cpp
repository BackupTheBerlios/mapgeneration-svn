/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include <cstdlib>
#include <iostream>

#include "util/rangereporting/pointnd.h"
#include "util/rangereporting/quadtree.h"

using namespace std;
using namespace rangereporting;


int main()
{
	typedef Point_nD<2, double> Point_2D;
	
	Point_2D lower_left;
	lower_left[0] = 0.0;
	lower_left[1] = 0.0;

	Point_2D upper_right;
	upper_right[0] = 1.0;
	upper_right[1] = 1.0;
	
	Rectangle<Point_2D> rectangle(lower_left, upper_right);

	Quadtree<Point_2D> quadtree(3, rectangle);
	
	srand(0);
	for (int i = 0; i < 16; ++i)
	{
		Point_2D* a = new Point_2D;
		(*a)[0] = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
		(*a)[1] = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
//		cout.setf(ios::fixed);
//		cout << *a << endl;
		quadtree.add_point(a);
	}
	
	cout << quadtree << endl;

	for (int i = 0; i < 100; ++i)
	{
		Point_2D lower_left_query;
		lower_left_query[0] = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
		lower_left_query[1] = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
	
		Point_2D upper_right_query;
		upper_right_query[0] = lower_left_query[0] + 0.2;
		upper_right_query[1] = lower_left_query[1] + 0.2;
		
		Rectangle<Point_2D> query_rectangle(lower_left_query, upper_right_query);
		//cout << query_rectangle << endl;
		
		vector< Quadtree<Point_2D>::const_iterator > query_result;
		quadtree.range_query(query_rectangle, query_result);
		
//		cout << "No of found point: " << query_result.size() << endl;
		
		for (int j = 0; j < query_result.size(); ++j)
		{
			double some_random_value = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
			double sign = (some_random_value < 0.5 ? -1.0 : 1.0);
			Point_2D* b = new Point_2D;
			(*b)[0] = (*(query_result[j]))[0] + (0.08 * sign);
			(*b)[1] = (*(query_result[j]))[1] + (0.08 * sign); 

			cout << "Move point " << *(query_result[j])
				<< " to " << *b << endl;
			quadtree.move_point(query_result[j], b);
//			cout << quadtree << endl;
		}
	}
	
	cout << quadtree << endl;
	
	
	for (int i = 0; i < 100; ++i)
	{
		Point_2D lower_left_query;
		lower_left_query[0] = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
		lower_left_query[1] = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
	
		Point_2D upper_right_query;
		upper_right_query[0] = lower_left_query[0] + 0.2;
		upper_right_query[1] = lower_left_query[1] + 0.2;
		
		Rectangle<Point_2D> query_rectangle(lower_left_query, upper_right_query);
		//cout << query_rectangle << endl;
		
		vector< Quadtree<Point_2D>::const_iterator > query_result;
		quadtree.range_query(query_rectangle, query_result);
		
//		cout << "No of found point: " << query_result.size() << endl;
		
		for (int j = 0; j < query_result.size(); ++j)
		{
//			cout << "Remove point " << *(query_result[j]) << endl;
			quadtree.remove_point(query_result[j]);
//			cout << quadtree << endl;
		}
	}
	
	
	cout << quadtree << endl;
	
	
	for (int i = 0; i < 100; ++i)
	{
		Point_2D a;
		a[0] = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
		a[1] = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
		
		Point_2D b;
		b[0] = a[0] + 0.2;
		b[1] = a[1] + 0.1;
	
		Point_2D c;
		c[0] = a[0] - 0.1;
		c[1] = a[1] - 0.1;
	
		Point_2D d;
		d[0] = a[0] + 0.1;
		d[1] = a[1] - 0.1;
		
		Trapezoid<Point_2D> query_trapezoid(a, b, c, d);
	
		vector< Quadtree<Point_2D>::const_iterator > query_result;
		quadtree.range_query(query_trapezoid, query_result);
		
		cout << query_trapezoid << endl;
		cout << "No of found point: " << query_result.size() << endl;
		for (int j = 0; j < query_result.size(); ++j)
		{
			cout << *(query_result[j]) << endl;
		}
		
		cout << endl;
	}
		
	
	return 0;
}

