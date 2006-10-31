/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "gpspoint.h"
#include "util/mlog.h"
#include "util/serializer.h"

//using namespace std;
using namespace mapgeneration;
using namespace mapgeneration_util;


void show_vector(std::vector<GPSPoint> vec)
{
	std::vector<GPSPoint>::iterator iter = vec.begin();
	std::vector<GPSPoint>::iterator iter_end = vec.end();
	for (; iter != iter_end; ++iter)
	{
		std::cout << "(" << (*iter).get_longitude() << ", " << (*iter).get_latitude() 
			<< ", " << (*iter).get_altitude() << ", " << (*iter).get_time() 
			<< ")" << " ";
	}
	std::cout << std::endl << "size: " << vec.size() 
		<< " capacity: " << vec.capacity() << std::endl;
}


int main()
{
	mlog(MLog::info, "test_serializer") << "Starting!\n";
	
	mlog(MLog::debug, "test_serializer") << "Serializing and deserializing...\n";
	
	double before_double = 5.12345;
	std::string serialized_double = Serializer::serialize(before_double);
	double after_double = Serializer::deserialize<double>(serialized_double);
	std::string result = (before_double == after_double ? "Ok" : "Error");
	mlog(MLog::debug, "test_serializer") << "double " << serialized_double.size() << " Bytes "
		<< before_double << ": " << result << "\n";
	
	float before_float = 6.12345;
	std::string serialized_float = Serializer::serialize(before_float);
	float after_float = Serializer::deserialize<float>(serialized_float);
	result = (before_float == after_float ? "Ok" : "Error");
	mlog(MLog::debug, "test_serializer") << "float " << serialized_float.size() << " Bytes "
		<< before_float << ": " << result << "\n";
	
	int before_int = 1234567;
	std::string serialized_int = Serializer::serialize(before_int);
	int after_int = Serializer::deserialize<int>(serialized_int);
	result = (before_int == after_int ? "Ok" : "Error");
	mlog(MLog::debug, "test_serializer") << "int " << serialized_int.size() << " Bytes "
		<< before_int << ": " << result << "\n";
	
/*	long before_long = 1234567890;
	std::string serialized_long = Serializer::serialize(before_long);
	long after_long = Serializer::deserialize<long>(serialized_long);
	result = (before_long == after_long ? "Ok" : "Error");
	mlog(MLog::debug, "test_serializer") << "long " << serialized_long.size() << " Bytes "
		<< before_long << ": " << result << "\n";*/
		
	bool before_bool = false;
	std::string serialized_bool = Serializer::serialize(before_bool);
	bool after_bool = Serializer::deserialize<bool>(serialized_bool);
	result = (before_bool == after_bool ? "Ok" : "Error");
	mlog(MLog::debug, "test_serializer") << "bool " << serialized_bool.size() << " Bytes "
		<< before_bool << ": " << result << "\n";
		
	std::string before_string = "abcdeffedcba";
	std::string serialized_string = Serializer::serialize(before_string);
	std::string after_string = Serializer::deserialize<std::string>(serialized_string);
	result = (before_string == after_string ? "Ok" : "Error");
	mlog(MLog::debug, "test_serializer") << "string " << serialized_string.size() << " Bytes "
		<< before_string << ": " << result << "\n";
	
	typedef std::pair<int, std::string> test_pair;
	test_pair before_pair;
	before_pair.first = 1234567890;
	before_pair.second = "Eintrag 1234567890";
	std::string serialized_pair = Serializer::serialize(before_pair);
	test_pair after_pair = Serializer::deserialize<test_pair>(serialized_pair);
	result = (before_pair.first == after_pair.first && 
		before_pair.second == after_pair.second ? "Ok" : "Error");
	mlog(MLog::debug, "test_serializer") << "pair " << serialized_pair.size() << " Bytes (" 
		<< before_pair.first << ", " << before_pair.second << ") : " << result << "\n";
	

	GPSPoint gps1, gps2;
	gps1.set_longitude(1.56);
	gps1.set_latitude(212.12);
	gps1.set_altitude(10.01);
	gps2.set_longitude(2.56);
	gps2.set_latitude(412.12);
	gps2.set_altitude(20.01);

	std::vector<GPSPoint> vec;
	for (int i=0; i<2; ++i)
	{
		vec.push_back(gps1);
		vec.push_back(gps2);
	}
	show_vector(vec);

	std::string vec_string = Serializer::serialize(vec);
	std::cout << "Ok: " << vec_string.size() << " Bytes" << std::endl;
	
	std::vector<GPSPoint> new_vec = 
		Serializer::deserialize< std::vector<GPSPoint> >(vec_string);
	show_vector(new_vec);
	
  mlog(MLog::info, "test_serializer") << "Finished.";
  
	return 0;
}
