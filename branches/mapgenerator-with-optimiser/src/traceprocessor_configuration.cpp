/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "traceprocessor.h"

#include <fstream>

#include "optimisation/mapgenerationindividual.h"
#include "optimisation/randomnumberfactory.h"
#include "util/pubsub/servicesystem.h"

using mapgeneration_opt::MapGenerationIndividual;
using mapgeneration_opt::RandomNumberFactory;

namespace mapgeneration
{
	
	TraceProcessor::Configuration::Configuration(
		pubsub::ServiceList* service_list)
	{
		// init identifier strings
		_identifier_strings.resize(_CONFIGURATION_VALUES);
		
		_identifier_strings[_STEP_DISTANCE_M]
			= "traceprocessor.step_distance_m";
		_identifier_strings[_SEARCH_MAX_DISTANCE_M]
			= "traceprocessor.search_max_distance_m";
		_identifier_strings[_SEARCH_MAX_ANGLE_DIFFERENCE_PI]
			= "traceprocessor.search_max_angle_difference_pi";
		_identifier_strings[_FAREST_START_POSITION]
			= "traceprocessor.farest_start_position";
		_identifier_strings[_SUFFICIENT_PATH_LENGTH]
			= "traceprocessor.sufficient_path_length";
		
		_identifier_strings[_BASE_SCORE_DIRECTION_DIFFERENCE]
			= "traceprocessor.base_score_direction_difference";
		_identifier_strings[_BASE_SCORE_NEXT_DIRECTION_DIFFERENCE]
			= "traceprocessor.base_score_next_direction_difference";
		_identifier_strings[_BASE_SCORE_DISTANT_START]
			= "traceprocessor.base_score_distant_start";
		_identifier_strings[_BASE_SCORE_PERPENDULAR_DISTANCE]
			= "traceprocessor.base_score_perpendular_distance";
		
		_identifier_strings[_BASE_SCORE_R2R]
			= "traceprocessor.base_score_r2r";
		_identifier_strings[_BASE_SCORE_R2VC]
			= "traceprocessor.base_score_r2vc";
		_identifier_strings[_BASE_SCORE_R2VF]
			= "traceprocessor.base_score_r2vf";
		_identifier_strings[_BASE_SCORE_VC2R]
			= "traceprocessor.base_score_vc2r";
		_identifier_strings[_BASE_SCORE_VC2VC]
			= "traceprocessor.base_score_vc2vc";
		_identifier_strings[_BASE_SCORE_VC2VF]
			= "traceprocessor.base_score_vc2vf";
		_identifier_strings[_BASE_SCORE_VF2R]
			= "traceprocessor.base_score_vf2r";
		_identifier_strings[_BASE_SCORE_VF2VC]
			= "traceprocessor.base_score_vf2vc";
		_identifier_strings[_BASE_SCORE_VF2VF]
			= "traceprocessor.base_score_vf2vf";
		
		_identifier_strings[_BASE_SCORE_R2R_STEP_DISTANCE]
			= "traceprocessor.base_score_r2r_step_distance";
		_identifier_strings[_BASE_SCORE_R2VC_STEP_DISTANCE]
			= "traceprocessor.base_score_r2vc_step_distance";
		_identifier_strings[_BASE_SCORE_R2VF_STEP_DISTANCE]
			= "traceprocessor.base_score_r2vf_step_distance";
		_identifier_strings[_BASE_SCORE_VC2R_STEP_DISTANCE]
			= "traceprocessor.base_score_vc2r_step_distance";
		_identifier_strings[_BASE_SCORE_VC2VC_STEP_DISTANCE]
			= "traceprocessor.base_score_vc2vc_step_distance";
		_identifier_strings[_BASE_SCORE_VC2VF_STEP_DISTANCE]
			= "traceprocessor.base_score_vc2vf_step_distance";
		_identifier_strings[_BASE_SCORE_VF2R_STEP_DISTANCE]
			= "traceprocessor.base_score_vf2r_step_distance";
		_identifier_strings[_BASE_SCORE_VF2VC_STEP_DISTANCE]
			= "traceprocessor.base_score_vf2vc_step_distance";
		_identifier_strings[_BASE_SCORE_VF2VF_STEP_DISTANCE]
			= "traceprocessor.base_score_vf2vf_step_distance";
		
		_identifier_strings[_BASE_SCORE_R2R_NOT_REACHABLE]
			= "traceprocessor.base_score_r2r_not_reachable";
		_identifier_strings[_BASE_SCORE_VC2VC_NO_SUCCESSOR]
			= "traceprocessor.base_score_vc2vc_no_successor";
		_identifier_strings[_BASE_SCORE_VF2VF_NO_SUCCESSOR]
			= "traceprocessor.base_score_vf2vf_no_successor";
		// done.
		
		// read values from the service list:
		_values.resize(_CONFIGURATION_VALUES);
		for (int i = 0; i < _CONFIGURATION_VALUES; ++i)
			service_list->get_service_value(_identifier_strings[i], _values[i]);
		// done.
		
		std::ifstream opt_stream("./optimisation_results.bin");
		if (opt_stream)
		{
			std::cout << "Loading values from \"./optimisation_results.bin\"..."
				<< std::endl;;
			RandomNumberFactory::load(opt_stream);
			
			int int_trash;
			double double_trash;
			Serializer::deserialize(opt_stream, int_trash);
			Serializer::deserialize(opt_stream, double_trash);
			Serializer::deserialize(opt_stream, int_trash);
			
			std::multimap< double, MapGenerationIndividual/*, std::greater<double>*/ >
				fitness_individual_map;
			Serializer::deserialize(opt_stream, fitness_individual_map);
			
//			std::multimap< double, MapGenerationIndividual>::const_iterator iter = fitness_individual_map.begin();
//			for (; iter != fitness_individual_map.end(); ++iter)
//			{
//				std::cout << iter->first << ": " << iter->second << std::endl;
//			}
//			
//			double abc = fitness_individual_map.rbegin()->first;
//			const MapGenerationIndividual& bbbb = fitness_individual_map.rbegin()->second;
//			std::cout << abc << ": " << bbbb << std::endl;
//			
//			std::cout << "blablablablablabla" << std::endl;
//			std::cout.flush();
			
			MapGenerationIndividual& mgi = fitness_individual_map.rbegin()->second;
			const std::vector<double>& opt_values = mgi.object_components();
			for (int i = 0; i < opt_values.size(); ++i)
			{
				_values[i + 5] = opt_values[i];
//				std::cout << opt_values[i] << ", ";
			}	
			
			std::cout << "done." << std::endl;
		}
	}
	
	
	double
	TraceProcessor::Configuration::get(ConfigurationValue cv) const
	{
		return _values[cv];
	}
	
	
	template <typename T_ReturnType>
	void
	TraceProcessor::Configuration::get(ConfigurationValue cv,
		T_ReturnType& output) const
	{
		output = _values[cv];
	}
	
	
	const std::vector<std::string>&
	TraceProcessor::Configuration::identifier_strings() const
	{
		return _identifier_strings;
	}
	
} // namespace mapgeneration
