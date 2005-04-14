/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include <iostream>
#include "util/rangereporting/quadtree.h"

using namespace std;
using namespace rangereporting;


class Point_2D
{
	
	public:
		
		double _x;
		double _y;
		
		bool
		operator==(const Point_2D& point) const
		{
			return ( (_x == point._x) && (_y == point._y) );
		}
		
		double
		operator[](int index) const
		{
			if (index == 0)
				return _x;
			
			if (index == 1)
				return _y;
			
			return -1;
		}

		double&
		operator[](int index)
		{
			if (index == 0)
				return _x;
			
			if (index == 1)
				return _y;
		}
};

ostream& operator<<(ostream& out, const Point_2D& point)
{
	out << "(" << point._x << "," << point._y << ")";
	return out;
}

int main()
{
	cout << endl <<"Testing rangereporting!" << endl << endl;
	
	Point_2D lower_left;
	lower_left._x = 0.0;
	lower_left._y = 0.0;

	Point_2D upper_right;
	upper_right._x = 8.0;
	upper_right._y = 8.0;

	Quadtree<Point_2D>* quadtree = new Quadtree<Point_2D>(3, lower_left, upper_right);
	
	Point_2D a;
	a._x = 1.123;
	a._y = 1;
	quadtree->add_point(&a);
	cout << *quadtree << endl;
	
	Point_2D b;
	b._x = 5;
	b._y = 5;
	quadtree->add_point(&b);
	cout << *quadtree << endl;
	
	Point_2D c;
	c._x = 3.345;
	c._y = 7.23;
	quadtree->add_point(&c);
	cout << *quadtree << endl;
	
	delete quadtree;

	return 0;
}

