/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include <cstdlib>
#include <iostream>
#include <list>

#include "geocoordinate.h"
#include "node.h"
#include "util/fixpointvector.h"
#include "util/rangereporting/quadtree.h"

using namespace std;
using namespace rangereporting;
using namespace mapgeneration;

class MyFixpointVector : public FixpointVector<Node>
{
	public:
	
		inline Node&
		operator[](Node::LocalId id)
		{
			return (FixpointVector<Node>::operator[](static_cast<int>(id))).second;
		}

		inline const Node&
		operator[](Node::LocalId id) const
		{
			return (FixpointVector<Node>::operator[](static_cast<int>(id))).second;
		}
};


typedef Quadtree< Node::LocalId, GeoCoordinate, MyFixpointVector > My_Quadtree;


void fast_query(const My_Quadtree& quadtree, const Trapezoid<GeoCoordinate>& query_trapezoid)
{
	vector<My_Quadtree::Id> query_result;
	quadtree.range_query(query_trapezoid, query_result);
//	cout << "Fast: " << query_result.size() << endl;
}


void slow_query(const MyFixpointVector& fv, const GeoCoordinate& query_point)
{
	std::vector<Node::LocalId> cluster_nodes;
	
	MyFixpointVector::const_iterator iter = fv.begin();
	MyFixpointVector::const_iterator iter_end = fv.end();
	for(; iter != iter_end; ++iter)
	{
		double distance = iter->second.approximated_distance(query_point);
		if (distance <= 30.0)
		{
			cluster_nodes.push_back(iter.position_number());
		}
	}
	
//	cout << "Slow: " << cluster_nodes.size() << endl;
}


int main(int argc, char* argv[])
{
	GeoCoordinate lower_left;
	lower_left[0] = 0.0;
	lower_left[1] = 0.0;

	GeoCoordinate upper_right;
	upper_right[0] = 0.01;
	upper_right[1] = 0.01;
	
	Rectangle<GeoCoordinate> rectangle;
	rectangle.set_corners(lower_left, upper_right);
	
	MyFixpointVector fv;
	
	int depth = atoi(argv[1]);
	My_Quadtree quadtree(depth, rectangle, &fv);
	
	srand(0);
	int nodes = atoi(argv[2]);
	for (int i = 0; i < nodes; ++i)
	{
		Node a;
		a[0] = 0.01 *
			(static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
		a[1] = 0.01 *
			(static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
		int id = fv.insert(a);
		quadtree.add_point(id);
	}
	
	
	for (int i = 0; i < 10000; ++i)
	{
		GeoCoordinate a;
		a[0] = 0.01 *
			(static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
		a[1] = 0.01 *
			(static_cast<double>(rand()) / static_cast<double>(RAND_MAX));
		
		double some_random_value = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
		double sign = (some_random_value < 0.5 ? -1.0 : 1.0);
		
		GeoCoordinate b;
		b[0] = a[0] + (sign * 0.0005);
		b[1] = a[1] + (sign * 0.000125);
		
		GeoCoordinate c;
		c[0] = a[0] - (sign * 0.00025);
		c[1] = a[1] + (sign * 0.00025);
	
		GeoCoordinate d;
		d[0] = a[0] + (sign * 0.00025);
		d[1] = a[1] + (sign * 0.000375);
		
		Trapezoid<GeoCoordinate> query_trapezoid(a, b, c, d);
	
		/* Now use brute force...... */
		GeoCoordinate z;
		z[0] = b[0] - (sign * c[0]);
		z[1] = b[1] - (sign * c[1]);
		
		fast_query(quadtree, query_trapezoid);
		slow_query(fv, z);
	}
	
	return 0;
}

