/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


/**
 * to test the tracefilter
 * todo:
 * - change the lines (document the line) _tile_manager->new_trace(...); with the couts above 
 */

#include<iostream>
#include<list>
#include<string>
#include<vector>
#include<fstream>
#include "gpspoint.h"
#include "filteredtrace.h"
#include "tracefilter.h"
#include "util/pubsub/servicelist.h"
#include "util/configuration.h"

using namespace mapgeneration;
using namespace std;
using namespace pubsub;
using namespace  mapgeneration_util;



int main()
{
	/*
	bool stop = false;
	bool delete_db = false;
	FilteredTrace *f_trace = new FilteredTrace();
	GPSPoint *g =new GPSPoint();
	list<GPSPoint> gpspoint_list;
	list<GPSPoint>::iterator gpspoint_iter;
	fstream file("orgTrace.txt",ios::in);
	string buffer;
	string zeile ;
	string zeile2;
	while(getline(file,buffer))
	{
		string anfang =buffer.substr(0,6);
		if(anfang.compare("$GPRMC")==0)
			zeile=buffer;
        if(anfang.compare("$GPGGA")==0)
        {
			zeile2=buffer;
            
			g->parse_nmea_string(zeile2,zeile);
			f_trace->push_back(*g);
		}
	}
	int i=1;
	for(gpspoint_iter =f_trace->begin();gpspoint_iter!=--f_trace->end();gpspoint_iter++,i++)
	{
		GPSPoint g1 = *gpspoint_iter;
		GPSPoint g2 =*(++gpspoint_iter);
		--gpspoint_iter;
		double time_diff = g2.get_time() - g1.get_time();
		cout<<(i+2)<<". "<<time_diff<<"\t\t"<<g1.get_latitude()<<"   \t\t"<<g1.get_longitude()<<"   \t\t"
		<<g1.get_altitude()<<endl;
	}

	ServiceList* service_list = new ServiceList();


	TraceFilter *trace_filter=new TraceFilter(service_list);
	cout<<"Tracefilter ersellt"<<endl;
	
	
	
	trace_filter->controlled_start();
	cout<<"trace_filter gestartet"<<endl;
	
	trace_filter->add_trace_in_queue(*f_trace);
	cout<<"filteredTrace in Queue added"<<endl;
	
	// trace_filter->show_outlier_and_time_offset_free_trace_queue();
	//std::cout<<"#########################################Finish"<<std::endl;
	/*
	trace_filter->controlled_stop();
	delete trace_filter;
	
	*/
	return 0;
}


