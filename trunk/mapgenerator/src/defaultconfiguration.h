/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef DEFAULTCONFIGURATION_H
#define DEFAULTCONFIGURATION_H

#define CONFIGURATION_PATH "mapgenerator.conf"

#include "util/configuration.h"


using namespace mapgeneration_util;


namespace mapgeneration
{

	class DefaultConfiguration {
		
		public:
		
			typedef Configuration::Parameter Parameter;
		
			DefaultConfiguration()
			{
				v.push_back(Parameter("accepted_licence", "string", "no"));
				
				v.push_back(Parameter("db.type", "string", "file"));
				v.push_back(Parameter("db.file.directory", "string", "filedb"));
				v.push_back(Parameter("db.odbc.dns", "string", "MapGeneration"));
				v.push_back(Parameter("db.odbc.user", "string", "mapgeneration"));
				v.push_back(Parameter("db.odbc.password", "string", "mg"));
				
				v.push_back(Parameter("traceserver.port", "int", "9000"));
				
				v.push_back(Parameter("traceprocessor.search_step_size_m", "double", "10"));
				v.push_back(Parameter("traceprocessor.search_radius_m", "double", "24"));
				v.push_back(Parameter("traceprocessor.search_max_angle_difference_pi", "double", "0.25"));
				v.push_back(Parameter("traceprocessor.threshold_tile_border", "double", "30"));
				
				v.push_back(Parameter("filteredtrace.size_factor", "double", "1.0"));
				
				v.push_back(Parameter("tracefilter.longest_tunnel", "double", "50000.0"));
				v.push_back(Parameter("tracefilter.max_acceleration", "double", "15"));
				v.push_back(Parameter("tracefilter.max_distance_gap", "double", "15000.0"));
				v.push_back(Parameter("tracefilter.max_speed", "double", "70"));
				v.push_back(Parameter("tracefilter.max_time_gap", "double", "3600.0"));
				v.push_back(Parameter("tracefilter.min_trace_length", "int", "5"));
			};

		
			std::vector< Configuration::Parameter >
			get()
			{
				return v;
			}
			
			
		private:
			
			std::vector< Configuration::Parameter > v;
		
	};
	

} // namespace mapgeneration

#endif //DEFAULTCONFIGURATION_H
