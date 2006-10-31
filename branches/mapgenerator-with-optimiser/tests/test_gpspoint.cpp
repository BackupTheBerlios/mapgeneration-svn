/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include<string>
#include<iostream>
#include "gpspoint.h"
#include "util/geocoordinate.h"

using namespace mapgeneration;
using namespace mapgeneration_util;
using namespace std;

int main()
{
	
/*	GPSPoint *point = new GPSPoint();
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
	 
	return 0;*/
	
	GeoCoordinate senden(51.8511111111, 7.484166666);
	GeoCoordinate st_maergen(48.00694444, 8.09305555);
	
	GeoCoordinate lax(33.95, -118.4);
	GeoCoordinate inv_senden(51.8511111111, 7.484166666 - 180.0);
	GeoCoordinate jfk(40.633333333333, -73.7833333333333);
	GeoCoordinate somewhere_in_pacific(-40.00000101, 120.1010101022);
	GeoCoordinate near_senden(51.8507, 7.484);
	GeoCoordinate test1(50.0, 7.0);
	GeoCoordinate test2(50.001, 7.001);
	
//	senden = lax;
//	senden = somewhere_in_pacific;
	senden = test1;
//	st_maergen = jfk;
//	st_maergen = lax;
//	st_maergen = inv_senden;
//	st_maergen = near_senden;
	st_maergen = test2;
	
	cout << "Senden:      " << senden << endl;
	cout << "St. Maergen: " << st_maergen << endl;
	cout << endl;
	
	cout.setf(ios::fixed);
	cout.precision(15);
	
	cout << "Distance on a Great Circle (Senden -> St.Maergen)" << endl;
	cout << "\tRadian: " << senden.distance_on_great_circle(st_maergen, GeoCoordinate::_RADIAN) << endl;
	cout << "\tDegree: " << senden.distance_on_great_circle(st_maergen, GeoCoordinate::_DEGREE) << endl;
	cout << "\tMeters: " << senden.distance_on_great_circle(st_maergen, GeoCoordinate::_METER) << endl;
	
	cout << "Distance on a Great Circle (St.Maergen -> Senden)" << endl;
	cout << "\tRadian: " << st_maergen.distance_on_great_circle(senden, GeoCoordinate::_RADIAN) << endl;
	cout << "\tDegree: " << st_maergen.distance_on_great_circle(senden, GeoCoordinate::_DEGREE) << endl;
	cout << "\tMeters: " << st_maergen.distance_on_great_circle(senden, GeoCoordinate::_METER) << endl;
	
	cout << "Distance on a rhumb line (Senden -> St.Maergen)" << endl;
	cout << "\tRadian: " << senden.distance_on_rhumb_line(st_maergen, GeoCoordinate::_RADIAN) << endl;
	cout << "\tDegree: " << senden.distance_on_rhumb_line(st_maergen, GeoCoordinate::_DEGREE) << endl;
	cout << "\tMeters: " << senden.distance_on_rhumb_line(st_maergen, GeoCoordinate::_METER) << endl;
	
	cout << "Distance on a rhumb line (St.Maergen -> Senden)" << endl;
	cout << "\tRadian: " << st_maergen.distance_on_rhumb_line(senden, GeoCoordinate::_RADIAN) << endl;
	cout << "\tDegree: " << st_maergen.distance_on_rhumb_line(senden, GeoCoordinate::_DEGREE) << endl;
	cout << "\tMeters: " << st_maergen.distance_on_rhumb_line(senden, GeoCoordinate::_METER) << endl;
	
	cout << "Distance approximated (Senden -> St.Maergen)" << endl;
	cout << "\tRadian: " << senden.distance_approximated(st_maergen, GeoCoordinate::_RADIAN) << endl;
	cout << "\tDegree: " << senden.distance_approximated(st_maergen, GeoCoordinate::_DEGREE) << endl;
	cout << "\tMeters: " << senden.distance_approximated(st_maergen, GeoCoordinate::_METER) << endl;
	
	cout << "Distance approximated (St.Maergen -> Senden)" << endl;
	cout << "\tRadian: " << st_maergen.distance_approximated(senden, GeoCoordinate::_RADIAN) << endl;
	cout << "\tDegree: " << st_maergen.distance_approximated(senden, GeoCoordinate::_DEGREE) << endl;
	cout << "\tMeters: " << st_maergen.distance_approximated(senden, GeoCoordinate::_METER) << endl;
	
	cout << endl;
	
	cout << "Bearing on Great Circle at the start point (Senden -> St. Maergen)" << endl;
	cout << "\tRadian: " << senden.bearing_on_great_circle(st_maergen, GeoCoordinate::_RADIAN) << endl;
	cout << "\tDegree: " << senden.bearing_on_great_circle(st_maergen, GeoCoordinate::_DEGREE) << endl;
	cout << "Bearing on Great Circle at 1/4 (Senden -> St. Maergen)" << endl;
	cout << "\tRadian: " << senden.bearing_on_great_circle(st_maergen, GeoCoordinate::_RADIAN, 0.25) << endl;
	cout << "\tDegree: " << senden.bearing_on_great_circle(st_maergen, GeoCoordinate::_DEGREE, 0.25) << endl;
	cout << "Bearing on Great Circle at 1/2 (Senden -> St. Maergen)" << endl;
	cout << "\tRadian: " << senden.bearing_on_great_circle(st_maergen, GeoCoordinate::_RADIAN, 0.5) << endl;
	cout << "\tDegree: " << senden.bearing_on_great_circle(st_maergen, GeoCoordinate::_DEGREE, 0.5) << endl;
	cout << "Bearing on Great Circle at 3/4 (Senden -> St. Maergen)" << endl;
	cout << "\tRadian: " << senden.bearing_on_great_circle(st_maergen, GeoCoordinate::_RADIAN, 0.75) << endl;
	cout << "\tDegree: " << senden.bearing_on_great_circle(st_maergen, GeoCoordinate::_DEGREE, 0.75) << endl;
	
	cout << "Bearing on Great Circle at the start point (St. Maergen -> Senden)" << endl;
	cout << "\tRadian: " << st_maergen.bearing_on_great_circle(senden, GeoCoordinate::_RADIAN) << endl;
	cout << "\tDegree: " << st_maergen.bearing_on_great_circle(senden, GeoCoordinate::_DEGREE) << endl;
	cout << "Bearing on Great Circle at 1/4 (St. Maergen -> Senden)" << endl;
	cout << "\tRadian: " << st_maergen.bearing_on_great_circle(senden, GeoCoordinate::_RADIAN, 0.25) << endl;
	cout << "\tDegree: " << st_maergen.bearing_on_great_circle(senden, GeoCoordinate::_DEGREE, 0.25) << endl;
	cout << "Bearing on Great Circle at 1/2 (St. Maergen -> Senden)" << endl;
	cout << "\tRadian: " << st_maergen.bearing_on_great_circle(senden, GeoCoordinate::_RADIAN, 0.5) << endl;
	cout << "\tDegree: " << st_maergen.bearing_on_great_circle(senden, GeoCoordinate::_DEGREE, 0.5) << endl;
	cout << "Bearing on Great Circle at 3/4 (St. Maergen -> Senden)" << endl;
	cout << "\tRadian: " << st_maergen.bearing_on_great_circle(senden, GeoCoordinate::_RADIAN, 0.75) << endl;
	cout << "\tDegree: " << st_maergen.bearing_on_great_circle(senden, GeoCoordinate::_DEGREE, 0.75) << endl;
	
	cout << endl;
	
/*	cout << "Bearing on rhumb line (Senden -> St. Maergen)" << endl;
	cout << "\tRadian: " << senden.bearing_on_rhumb_line(st_maergen, GeoCoordinate::_RADIAN) << endl;
	cout << "\tDegree: " << senden.bearing_on_rhumb_line(st_maergen, GeoCoordinate::_DEGREE) << endl;
	
	cout << "Bearing on rhumb line (St. Maergen -> Senden)" << endl;
	cout << "\tRadian: " << st_maergen.bearing_on_rhumb_line(senden, GeoCoordinate::_RADIAN) << endl;
	cout << "\tDegree: " << st_maergen.bearing_on_rhumb_line(senden, GeoCoordinate::_DEGREE) << endl;
	
	cout << "Bearing approximated (Senden -> St. Maergen)" << endl;
	cout << "\tRadian: " << senden.bearing_approximated(st_maergen, GeoCoordinate::_RADIAN) << endl;
	cout << "\tDegree: " << senden.bearing_approximated(st_maergen, GeoCoordinate::_DEGREE) << endl;
	
	cout << "Bearing approximated (St. Maergen -> Senden)" << endl;
	cout << "\tRadian: " << st_maergen.bearing_approximated(senden, GeoCoordinate::_RADIAN) << endl;
	cout << "\tDegree: " << st_maergen.bearing_approximated(senden, GeoCoordinate::_DEGREE) << endl;

	GeoCoordinate x = senden.compute_geo_coordinate_on_rhumb_line(senden.bearing_on_rhumb_line(st_maergen),
		senden.distance_on_rhumb_line(st_maergen) * 0.1);
	GeoCoordinate y = senden.compute_geo_coordinate_on_great_circle(senden.bearing_on_great_circle(st_maergen),
		senden.distance_on_great_circle(st_maergen) * 0.5);
	GeoCoordinate z = senden.compute_geo_coordinate_approximated(senden.bearing_approximated(st_maergen),
		senden.distance_approximated(st_maergen) * 0.1);
	
	cout << "Senden:      " << senden << endl;
	cout << "St. Maergen: " << st_maergen << endl;
	cout << x << endl;
//	cout << y << endl;
	cout << z << endl;
	*/
	
	GeoCoordinate a(50.0, 7.5);
	GeoCoordinate b(90.0, 0.0);
	GeoCoordinate c(0.0, 90.0);
	GeoCoordinate f(50.00003, 7.00004);
//	std::cout << senden.bearing_on_great_circle(b, GeoCoordinate::_DEGREE) << std::endl;
	
	GeoCoordinate test_gc = f;
	std::cout << test_gc << std::endl << std::endl;
	
	std::cout << "senden->st_maergen" << std::endl;
	std::cout << std::boolalpha << test_gc.has_perpendicular_on_segment(senden, st_maergen) << std::endl;
	GeoCoordinate d = test_gc.nearest_geo_coordinate_to_segment_on_great_circle(senden, st_maergen);
	std::cout << d << std::endl << std::endl;

	std::cout << "st_maergen->senden" << std::endl;
	std::cout << std::boolalpha << test_gc.has_perpendicular_on_segment(st_maergen, senden) << std::endl;
	GeoCoordinate e = test_gc.nearest_geo_coordinate_to_segment_on_great_circle(st_maergen, senden);
	std::cout << e << std::endl;
	
	std::cout << std::endl << "Test the intepolate method of Direction" << std::endl;
	GeoCoordinate x (45, 0);
	GeoCoordinate y (40, 45);
	GeoCoordinate z (50, 0);
	
	Direction d1 = /*x.bearing_default(y);*/ 0.8 * PI;
	Direction d2 = /*x.bearing_default(z);*/ 1.2 * PI;
	
	std::cout << d1 << std::endl;
	std::cout << d2 << std::endl;
	
	std::cout << Direction::interpolate(d2, d1, 0.5) << std::endl;
	
	std::cout << std::endl;
	std::cout << senden.bearing_approximated(st_maergen) *r2d << "\t" << senden.distance_approximated(st_maergen) << std::endl;
	std::cout << senden.bearing_approximated2(st_maergen) *r2d << "\t" << senden.distance_approximated2(st_maergen) << std::endl;
	std::cout << senden.bearing_on_great_circle(st_maergen) *r2d << "\t" << senden.distance_on_great_circle(st_maergen) << std::endl;
	std::cout << senden.bearing_on_rhumb_line(st_maergen) *r2d << "\t" << senden.distance_on_rhumb_line(st_maergen) << std::endl;
	std::cout << st_maergen.bearing_approximated(senden) *r2d << "\t" << st_maergen.distance_approximated(senden) << std::endl;
	std::cout << st_maergen.bearing_approximated2(senden) *r2d << "\t" << st_maergen.distance_approximated2(senden) << std::endl;
	std::cout << st_maergen.bearing_on_great_circle(senden) *r2d << "\t" << st_maergen.distance_on_great_circle(senden) << std::endl;
	std::cout << st_maergen.bearing_on_rhumb_line(senden) *r2d << "\t" << st_maergen.distance_on_rhumb_line(senden) << std::endl;
	
	std::cout << senden.bearing(senden) *r2d << std::endl;
	
	return 0;
}
