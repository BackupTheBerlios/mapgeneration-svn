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
				v.push_back(Parameter("traceserver.port", "int", "9000"));
				v.push_back(Parameter("tracefilter.max_acc", "double", "15"));
				v.push_back(Parameter("tracefilter.max_delta_time","double","120"));
				v.push_back(Parameter("tracefilter.max_outlier_count","int","10"));
				v.push_back(Parameter("tracefilter.max_distance","int","15"));
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
