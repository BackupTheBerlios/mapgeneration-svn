/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include<iostream>
#include<list>
#include<string>
#include<vector>
#include "gpspoint.h"
#include "filteredtrace.h"

using namespace mapgeneration;
using namespace std;



	int main()
	{



string asciiNMEA ="$GPRMC,191410,A,4735.5634,N,00739.3538,E,0.0,0.0,181102,0.4,E,A*19\n$GPRMB,A,9.99,L,,Exit,4726.8323,N,00820.4822,E,29.212,107.2,,V,A*69\n$GPGGA,191410,4735.5634,N,00739.3538,E,1,04,4.4,351.5,M,48.0,M,,*45\n$GPGSA,A,3,,,,15,17,18,23,,,,,,4.7,4.4,1.5*3F\n$GPGSV,2,1,08,02,59,282,00,03,42,287,00,06,16,094,00,15,80,090,48*79\n$GPGLL,4735.5634,N,00739.3538,E,191410,A,A*4A\n$GPBOD,221.9,T,221.5,M,Exit,*6B\n$GPVTG,0.0,T,359.6,M,0.0,N,0.0,K*47\n$PGRME,24.7,M,23.5,M,34.1,M*1D\n$PGRMZ,1012,f*36\n$PGRMM,WGS 84*06\n$HCHDG,170.4,,,0.4,E*03\n$GPRTE,1,1,c,*37\n$GPRMC,191410,A,4736.5634,N,00739.3538,E,0.0,0.0,181102,0.4,E,A*19\n$GPRMB,A,9.99,L,,Exit,4726.8323,N,00820.4822,E,29.212,107.2,,V,A*69\n$GPGGA,191410,4785.5634,N,00739.3538,E,1,04,4.4,351.5,M,48.0,M,,*45\n$GPGSA,A,3,,,,15,17,18,23,,,,,,4.7,4.4,1.5*3F\n$GPGSV,2,1,08,02,59,282,00,03,42,287,00,06,16,094,00,15,80,090,48*79\n$GPGLL,4735.5634,N,00739.3538,E,191410,A,A*4A\n$GPBOD,221.9,T,221.5,M,Exit,*6B\n$GPVTG,0.0,T,359.6,M,0.0,N,0.0,K*47\n$PGRME,24.7,M,23.5,M,34.1,M*1D\n$PGRMZ,1012,f*36\n$PGRMM,WGS 84*06\n$HCHDG,170.4,,,0.4,E*03\n$GPRTE,1,1,c,*37\n$GPRMC,191410,A,4735.5634,N,00739.3538,E,0.0,0.0,181102,0.4,E,A*19\n$GPRMB,A,9.99,L,,Exit,4726.8323,N,00820.4822,E,29.212,107.2,,V,A*69\n$GPGGA,191410,4735.5634,N,00739.3538,E,1,04,4.4,351.5,M,48.0,M,,*45\n$GPGSA,A,3,,,,15,17,18,23,,,,,,4.7,4.4,1.5*3F\n$GPGSV,2,1,08,02,59,282,00,03,42,287,00,06,16,094,00,15,80,090,48*79\n$GPGLL,4735.5634,N,00739.3538,E,191410,A,A*4A\n$GPBOD,221.9,T,221.5,M,Exit,*6B\n$GPVTG,0.0,T,359.6,M,0.0,N,0.0,K*47\n$PGRME,24.7,M,23.5,M,34.1,M*1D\n$PGRMZ,1012,f*36\n$PGRMM,WGS 84*06\n$HCHDG,170.4,,,0.4,E*03\n$GPRTE,1,1,c,*37";

	FilteredTrace *f_trace = new FilteredTrace();
	
	list<GPSPoint>::iterator gpspoint_iter;
	
	f_trace->parse_nmea_string(asciiNMEA);
	
//	delete f_trace;
	

//	f_trace = new FilteredTrace();	
	GPSPoint point;
	point.set_latitude(1);
	point.set_altitude(0);
	point.set_longitude(0);
	point.set_time(0);
	f_trace->push_back(point);
	point.set_latitude(2);
	f_trace->push_back(point);
	point.set_latitude(3);
	f_trace->push_back(point);
	point.set_latitude(4);
	f_trace->push_back(point);
	point.set_latitude(5);
	f_trace->push_back(point);
	
	list<GPSPoint>::iterator iter = f_trace->begin();
	++iter;
	list<GPSPoint>::iterator iter2 = f_trace->end();
	(--iter2);
	
//	f_trace->erase(iter, iter2);
	cout.setf(std::ios::fixed);
	
	cout<<"f_trace.size(): "<<f_trace->size()<<endl;  
    
	cout<<"\n---------------------------------------------------------------------------------"<<endl;
    cout<<"|TIME (DDMMYYhhmmss)|\t"<<"LATITUDE\t|\t"<<"LONGITUDE\t|\t"<<"ALTITUDE |\t"<<endl;
    
	cout<<"-----------------------------------------------------------------------------------"<<endl;
    int i=1;
    for(gpspoint_iter =f_trace->begin();gpspoint_iter!=f_trace->end();gpspoint_iter++,i++)
    {
    GPSPoint g1 = *gpspoint_iter;
    cout<<i<<". "<<g1.get_time()<<"\t\t"<<g1.get_latitude()<<"   \t\t"<<g1.get_longitude()<<"   \t\t"
    <<g1.get_altitude()<<endl;
  
	}
	cout<<"last time: "<<f_trace->get_last_time()<<endl << endl;
      
      
    return 0;
}

	



