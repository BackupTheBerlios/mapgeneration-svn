/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include<string>
#include<iostream>
#include "gpspoint.h"

using namespace mapgeneration;
using namespace std;

int main()
{
	
	GPSPoint *point = new GPSPoint();
	string gpgga_nmea = "$GPGGA,191410,4735.5634,N,00739.3538,E,1,04,4.4,351.5,M,48.0,M,,*45";
	string gprmc_nmea = "$GPRMC,191410,A,4735.5634,N,00739.3538,E,0.0,0.0,181102,0.4,E,A*19";

	point->parse_nmea_string(gpgga_nmea, gprmc_nmea);
	cout << point->get_time()<<"\t \t" <<point->get_latitude()<<"\t \t"<<point->get_longitude()<<"\t \t"<<point->get_altitude() << endl;
	
	GPSPoint p1(89, 180);
	GPSPoint p2(89, 0);
	cout << p1.distance(p2) << endl;
	
	GPSPoint senden(51.8511111111, 7.484166666);
	GPSPoint sankt_maergen(48.00694444, 8.09305555);
	cout << "Von Senden nach Sankt M???rgen sind es ca. " << senden.distance(sankt_maergen) << " m." << endl;
	
	GPSPoint flensburg(51.8511111111, 7.484166666);
	GPSPoint oberstdorf(48.00694444, 8.09305555);
	cout << "Von Flensburg nach Oberstdorf sind es ca. " << flensburg.distance(oberstdorf) << " m." << endl;
	 
	return 0;
}
