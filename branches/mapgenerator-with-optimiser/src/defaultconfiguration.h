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

using mapgeneration_util::Configuration;

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
				
				v.push_back(Parameter("filteredtrace.size_factor", "double", "1.0"));
				
				v.push_back(Parameter("tilecache.min_object_capacity", "int", "20"));
				v.push_back(Parameter("tilecache.hard_max_size", "int", "120000000"));
				v.push_back(Parameter("tilecache.soft_max_size", "int", "100000000"));
				
				v.push_back(Parameter("tracefilter.longest_tunnel", "double", "50000.0"));
				v.push_back(Parameter("tracefilter.max_acceleration", "double", "15"));
				v.push_back(Parameter("tracefilter.max_distance_gap", "double", "15000.0"));
				v.push_back(Parameter("tracefilter.max_speed", "double", "70"));
				v.push_back(Parameter("tracefilter.max_time_gap", "double", "3600.0"));
				v.push_back(Parameter("tracefilter.min_trace_length", "int", "5"));
				
/* Auf Knotenanzahl optimiert */
				v.push_back(Parameter("traceprocessor.base_score_direction_difference", "double", "47.8005"));
				v.push_back(Parameter("traceprocessor.base_score_next_direction_difference", "double", "78.7322"));
				v.push_back(Parameter("traceprocessor.base_score_distant_start", "double", "10.6768"));
				v.push_back(Parameter("traceprocessor.base_score_perpendular_distance", "double", "61.2133"));
				v.push_back(Parameter("traceprocessor.base_score_r2r", "double", "37.4646"));
				v.push_back(Parameter("traceprocessor.base_score_r2vc", "double", "37.1307"));
				v.push_back(Parameter("traceprocessor.base_score_r2vf", "double", "38.8078"));
				v.push_back(Parameter("traceprocessor.base_score_vc2r", "double", "13.031"));
				v.push_back(Parameter("traceprocessor.base_score_vc2vc", "double", "10.5858"));
				v.push_back(Parameter("traceprocessor.base_score_vc2vf", "double", "97.2874"));
				v.push_back(Parameter("traceprocessor.base_score_vf2r", "double", "80.8083"));
				v.push_back(Parameter("traceprocessor.base_score_vf2vc", "double", "64.5391"));
				v.push_back(Parameter("traceprocessor.base_score_vf2vf", "double", "23.3851"));
				v.push_back(Parameter("traceprocessor.base_score_r2r_step_distance", "double", "8.18562"));
				v.push_back(Parameter("traceprocessor.base_score_r2vc_step_distance", "double", "83.3023"));
				v.push_back(Parameter("traceprocessor.base_score_r2vf_step_distance", "double", "28.2048"));
				v.push_back(Parameter("traceprocessor.base_score_vc2r_step_distance", "double", "73.8597"));
				v.push_back(Parameter("traceprocessor.base_score_vc2vc_step_distance", "double", "62.2554"));
				v.push_back(Parameter("traceprocessor.base_score_vc2vf_step_distance", "double", "101.857"));
				v.push_back(Parameter("traceprocessor.base_score_vf2r_step_distance", "double", "23.1832"));
				v.push_back(Parameter("traceprocessor.base_score_vf2vc_step_distance", "double", "29.8397"));
				v.push_back(Parameter("traceprocessor.base_score_vf2vf_step_distance", "double", "28.6824"));
				v.push_back(Parameter("traceprocessor.base_score_r2r_not_reachable", "double", "91.6023"));
				v.push_back(Parameter("traceprocessor.base_score_vc2vc_no_successor", "double", "71.3876"));
				v.push_back(Parameter("traceprocessor.base_score_vf2vf_no_successor", "double", "0.257678"));
				v.push_back(Parameter("traceprocessor.farest_start_position", "double", "30.0"));
				v.push_back(Parameter("traceprocessor.search_max_angle_difference_pi", "double", "0.5"));
				v.push_back(Parameter("traceprocessor.search_max_distance_m", "double", "25.0"));
				v.push_back(Parameter("traceprocessor.step_distance_m", "double", "16.0"));
				v.push_back(Parameter("traceprocessor.sufficient_path_length", "double", "100.0"));

/* Auf abstand optimiert
				v.push_back(Parameter("traceprocessor.base_score_direction_difference", "double", "3.4946"));
				v.push_back(Parameter("traceprocessor.base_score_next_direction_difference", "double", "152.132"));
				v.push_back(Parameter("traceprocessor.base_score_distant_start", "double", "38.2522"));
				v.push_back(Parameter("traceprocessor.base_score_perpendular_distance", "double", "0.246713"));
				v.push_back(Parameter("traceprocessor.base_score_r2r", "double", "6.82904"));
				v.push_back(Parameter("traceprocessor.base_score_r2vc", "double", "86.9602"));
				v.push_back(Parameter("traceprocessor.base_score_r2vf", "double", "43.3384"));
				v.push_back(Parameter("traceprocessor.base_score_vc2r", "double", "52.7902"));
				v.push_back(Parameter("traceprocessor.base_score_vc2vc", "double", "34.7856"));
				v.push_back(Parameter("traceprocessor.base_score_vc2vf", "double", "156.215"));
				v.push_back(Parameter("traceprocessor.base_score_vf2r", "double", "149.36"));
				v.push_back(Parameter("traceprocessor.base_score_vf2vc", "double", "57.8344"));
				v.push_back(Parameter("traceprocessor.base_score_vf2vf", "double", "64.2395"));
				v.push_back(Parameter("traceprocessor.base_score_r2r_step_distance", "double", "3.6542"));
				v.push_back(Parameter("traceprocessor.base_score_r2vc_step_distance", "double", "0"));
				v.push_back(Parameter("traceprocessor.base_score_r2vf_step_distance", "double", "151.129"));
				v.push_back(Parameter("traceprocessor.base_score_vc2r_step_distance", "double", "4.65077"));
				v.push_back(Parameter("traceprocessor.base_score_vc2vc_step_distance", "double", "5.2773"));
				v.push_back(Parameter("traceprocessor.base_score_vc2vf_step_distance", "double", "78.8088"));
				v.push_back(Parameter("traceprocessor.base_score_vf2r_step_distance", "double", "143.791"));
				v.push_back(Parameter("traceprocessor.base_score_vf2vc_step_distance", "double", "173.819"));
				v.push_back(Parameter("traceprocessor.base_score_vf2vf_step_distance", "double", "156.473"));
				v.push_back(Parameter("traceprocessor.base_score_r2r_not_reachable", "double", "98.1473"));
				v.push_back(Parameter("traceprocessor.base_score_vc2vc_no_successor", "double", "19.5837"));
				v.push_back(Parameter("traceprocessor.base_score_vf2vf_no_successor", "double", "46.4389"));
				v.push_back(Parameter("traceprocessor.farest_start_position", "double", "50.0"));
				v.push_back(Parameter("traceprocessor.search_max_angle_difference_pi", "double", "0.4"));
				v.push_back(Parameter("traceprocessor.search_max_distance_m", "double", "18.0"));
				v.push_back(Parameter("traceprocessor.step_distance_m", "double", "8.0"));
				v.push_back(Parameter("traceprocessor.sufficient_path_length", "double", "100.0"));
*/
/*				
				v.push_back(Parameter("traceprocessor.base_score_direction_difference", "double", "150.0"));
				v.push_back(Parameter("traceprocessor.base_score_next_direction_difference", "double", "100.0"));
				v.push_back(Parameter("traceprocessor.base_score_distant_start", "double", "1.0"));
				v.push_back(Parameter("traceprocessor.base_score_perpendular_distance", "double", "3.5"));
				v.push_back(Parameter("traceprocessor.base_score_r2r", "double", "0.0"));
				v.push_back(Parameter("traceprocessor.base_score_r2vc", "double", "20.0"));
				v.push_back(Parameter("traceprocessor.base_score_r2vf", "double", "5000.0"));
				v.push_back(Parameter("traceprocessor.base_score_vc2r", "double", "20.0"));
				v.push_back(Parameter("traceprocessor.base_score_vc2vc", "double", "40.0"));
				v.push_back(Parameter("traceprocessor.base_score_vc2vf", "double", "5000.0"));
				v.push_back(Parameter("traceprocessor.base_score_vf2r", "double", "5000.0"));
				v.push_back(Parameter("traceprocessor.base_score_vf2vc", "double", "5000.0"));
				v.push_back(Parameter("traceprocessor.base_score_vf2vf", "double", "5000.0"));
				v.push_back(Parameter("traceprocessor.base_score_r2r_step_distance", "double", "0.5"));
				v.push_back(Parameter("traceprocessor.base_score_r2vc_step_distance", "double", "5.0"));
				v.push_back(Parameter("traceprocessor.base_score_r2vf_step_distance", "double", "2000.0"));
				v.push_back(Parameter("traceprocessor.base_score_vc2r_step_distance", "double", "5.0"));
				v.push_back(Parameter("traceprocessor.base_score_vc2vc_step_distance", "double", "7.5"));
				v.push_back(Parameter("traceprocessor.base_score_vc2vf_step_distance", "double", "2000.0"));
				v.push_back(Parameter("traceprocessor.base_score_vf2r_step_distance", "double", "2000.0"));
				v.push_back(Parameter("traceprocessor.base_score_vf2vc_step_distance", "double", "2000.0"));
				v.push_back(Parameter("traceprocessor.base_score_vf2vf_step_distance", "double", "2000.0"));
				v.push_back(Parameter("traceprocessor.base_score_r2r_not_reachable", "double", "20.0"));
				v.push_back(Parameter("traceprocessor.base_score_vc2vc_no_successor", "double", "5000.0"));
				v.push_back(Parameter("traceprocessor.base_score_vf2vf_no_successor", "double", "5000.0"));
				v.push_back(Parameter("traceprocessor.farest_start_position", "double", "50.0"));
				v.push_back(Parameter("traceprocessor.search_max_angle_difference_pi", "double", "0.4"));
				v.push_back(Parameter("traceprocessor.search_max_distance_m", "double", "18.0"));
				v.push_back(Parameter("traceprocessor.step_distance_m", "double", "8.0"));
				v.push_back(Parameter("traceprocessor.sufficient_path_length", "double", "100.0"));
*/				
				v.push_back(Parameter("traceserver.port", "int", "9000"));
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
