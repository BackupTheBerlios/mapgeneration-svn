/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include <iostream>
#include <sstream>
#include <string>

#include "gpspoint.h"
#include "util/fixpointvector.h"

using namespace std;
using namespace mapgeneration;
using namespace mapgeneration_util;


void show_fixpointvector(FixpointVector<GPSPoint> fpv)
{
	FixpointVector<GPSPoint>::iterator iter = fpv.begin();
	FixpointVector<GPSPoint>::iterator iter_end = fpv.end();
	for (; iter != iter_end; iter.next_pair())
	{
		cout << "(" << iter.position_number() << ")" << (*iter).first << " ";
	}
	cout << endl << "size: " << fpv.size() << " including holes: "
		<< fpv.size_including_holes() << " capacity: " << fpv.capacity() << endl;
}


int main()
{
	cout << endl << "Creating a FixpointVector: " << endl;
	FixpointVector<GPSPoint> fpv;
	show_fixpointvector(fpv);
	
	cout << endl << "Adding one GPSPoint: " << endl;
	fpv.insert(GPSPoint());
	show_fixpointvector(fpv);
	
	cout << endl << "Adding another 9 GPSPoints: " << endl;
	for (int i=0; i<9; i++)
	{
		fpv.insert(GPSPoint());
	}
	show_fixpointvector(fpv);
	
	cout << endl << "Serializing and deserializing the FixpointVector: ";
	string serialized_fpv = Serializer::serialize(fpv);
	cout << serialized_fpv.size() << " Bytes" << endl;
	FixpointVector<GPSPoint> fpv_2 = Serializer::deserialize< FixpointVector<GPSPoint> >(serialized_fpv);	
	show_fixpointvector(fpv_2);
	
	cout << endl << "Deleting GPSPoints 2, 3, 7, 9:" << endl;
	FixpointVector<GPSPoint>::iterator iter = fpv_2.begin();
	FixpointVector<GPSPoint>::iterator iter_end = fpv_2.end();
	int i = 0;
	for (; iter != fpv_2.end(); )
	{
		if (i==2 || i==3 || i==7 || i==9)
			fpv_2.erase(iter);
		else 
			iter.next_pair();
		++i;
	}	
	show_fixpointvector(fpv_2);
	
	cout << endl << "Serializing and deserializing the FixpointVector: ";
	serialized_fpv = Serializer::serialize(fpv_2);
	cout << serialized_fpv.size() << " Bytes" << endl;
	fpv = Serializer::deserialize< FixpointVector<GPSPoint> >(serialized_fpv);	
	show_fixpointvector(fpv);
	
	cout << endl << "Adding 2 new GPSPoints: " << endl;
	for (int i=0; i<2; i++)
	{
		fpv.insert(GPSPoint());
	}
	show_fixpointvector(fpv);
	
	cout << endl << "Serializing and deserializing the FixpointVector: ";
	serialized_fpv = Serializer::serialize(fpv);
	cout << serialized_fpv.size() << " Bytes" << endl;
	fpv_2 = Serializer::deserialize< FixpointVector<GPSPoint> >(serialized_fpv);	
	show_fixpointvector(fpv_2);
	
	cout << endl << "Adding another 3 new GPSPoints: " << endl;
	for (int i=0; i<3; i++)
	{
		fpv_2.insert(GPSPoint());
	}
	show_fixpointvector(fpv_2);
		
	cout << endl;
	
	cout << endl << "Serializing and deserializing the FixpointVector: ";
	serialized_fpv = Serializer::serialize(fpv_2);
	cout << serialized_fpv.size() << " Bytes" << endl;
	fpv = Serializer::deserialize< FixpointVector<GPSPoint> >(serialized_fpv);	
	show_fixpointvector(fpv);
	
	return 0;
}
