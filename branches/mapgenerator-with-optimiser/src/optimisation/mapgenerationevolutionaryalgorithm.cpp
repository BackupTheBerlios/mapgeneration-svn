/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "mapgenerationevolutionaryalgorithm.h"

#include <dirent.h>
#include <fstream>
#include <functional>

#include "filteredtrace.h"

namespace mapgeneration_opt
{
	
	MapGenerationEvolutionaryAlgorithm::MapGenerationEvolutionaryAlgorithm(
		int parents_count, int descendants_count)
	: AsymmetricAdaptiveEvolutionaryAlgorithm<MAP_GENERATION_INDIVIDUAL_DIMENSION>(parents_count, descendants_count),
		_best_fitness(-HUGE_VAL), _best_fitness_counter(0), _counter(0),
		_previous_optimisation_results_used(false),
		_optimisation_results_ifstream()
	{
//		RandomNumberFactory::_simulation_mode = true;
		
		_optimisation_results_ifstream.open("./optimisation_results");
		if (_optimisation_results_ifstream)
		{
			bool loaded = RandomNumberFactory::load(
				_optimisation_results_ifstream);
			if (!loaded)
			{
				std::cout << "RandomNumberFactory cannot be loaded!"
					<< std::endl;
				throw "Oops!";
			}
			
			Serializer::deserialize(_optimisation_results_ifstream, _counter);
			Serializer::deserialize(_optimisation_results_ifstream, _best_fitness);
			Serializer::deserialize(_optimisation_results_ifstream,
				_best_fitness_counter);
			_previous_optimisation_results_used = true;
		}
		
		// deserialize traces...
		std::string directory_name = "optimisation_traces/";
		
		DIR *dir;
		struct dirent *direntp;
		dir = opendir (directory_name.c_str());
		if (dir != NULL)
		{
			readdir(dir); readdir(dir);
			while ((direntp = readdir(dir)) != NULL)
			{
				std::string file_name = directory_name;
				file_name += "/";
				file_name += direntp->d_name;
				
				FilteredTrace filtered_trace;
				
				std::ifstream in_file_stream(file_name.c_str());
				Serializer::deserialize(in_file_stream, filtered_trace);
				in_file_stream.close();
				
				_test_pool.push_back(filtered_trace);
			}
			closedir (dir);
		}
		// done.
		
	}
	
	
	MapGenerationEvolutionaryAlgorithm::~MapGenerationEvolutionaryAlgorithm()
	{
	}
	
	
	void
	MapGenerationEvolutionaryAlgorithm::apply_selector(
		const std::vector<Individual*>& current_parents,
		std::vector<bool>& selected_parents,
		const std::vector<Individual*>& current_descendants,
		std::vector<bool>& selected_descendants)
	{
		EvolutionaryAlgorithmFactory::deterministic_plus_selector(
			current_parents, selected_parents,
			current_descendants, selected_descendants);
		
		assert(current_parents.size() == selected_parents.size());
		
		for (int i = 0; i < current_parents.size(); ++i)
		{
			MapGenerationIndividual* mgi_parent
				= dynamic_cast<MapGenerationIndividual*>(current_parents[i]);
			if (mgi_parent == 0)
			{
				std::cout << "MapGenerationEvolutionaryAlgorithm::apply_selector: first cast failed." << std::endl;
				throw "Oops!";
			}
			
			if (selected_parents[i])
			{
				mgi_parent->increment_age();
				if (mgi_parent->get_age() > _MAX_AGE)
				{
					std::vector<Individual*> ind_pool(1);
					AsymmetricAdaptiveEvolutionaryAlgorithm<MAP_GENERATION_INDIVIDUAL_DIMENSION>
						::init_individuals(ind_pool);
//					this->init_individuals(ind_pool);
					
					MapGenerationIndividual* mgi_new_ind
						= dynamic_cast<MapGenerationIndividual*>(ind_pool[0]);
					if (mgi_new_ind == 0)
					{
						std::cout << "MapGenerationEvolutionaryAlgorithm::apply_selector: second cast failed." << std::endl;
						throw "Oops!";
					}
					*mgi_parent = *mgi_new_ind;
				}
			} else
			{
				mgi_parent->reset_age();
			}
		}
		
		for (int i = 0; i < current_descendants.size(); ++i)
		{
			MapGenerationIndividual* mgi_descendant
				= dynamic_cast<MapGenerationIndividual*>(current_descendants[i]);
			if (mgi_descendant == 0)
			{
				std::cout << "MapGenerationEvolutionaryAlgorithm::apply_selector: third cast failed." << std::endl;
				throw "Oops!";
			}
			
			if (selected_descendants[i])
			{
				mgi_descendant->increment_age();
			} else
			{
				mgi_descendant->reset_age();
			}
		}
			
	}
	
	
	bool
	MapGenerationEvolutionaryAlgorithm::cancel_condition(
		const std::vector<Individual*>* individuals)
	{
/*		std::cout << std::endl
			<< "MapGenerationEvolutionaryAlgorithm::cancel_condition: "
			<< "counter = " << _counter
			<< ", best_fitness = " << _best_fitness
			<< ", best_fitness_counter = " << _best_fitness_counter << std::endl;
*/		
		double best_fitness = (*individuals)[0]->get_fitness();
		for (int i = 1; i < individuals->size(); ++i)
		{
			double fitness = (*individuals)[i]->get_fitness();
			
			if (fitness > best_fitness)
				best_fitness = fitness;
		}
		
		if (best_fitness == _best_fitness)
		{
			++_best_fitness_counter;
//		} else if (best_fitness < _best_fitness)
//		{
//			std::cout << "EXIT NOW!!!!!" << std::endl;
//			std::cout << best_fitness << ", _" << _best_fitness << std::endl;
//			throw "Oops!";
		} else
		{
			_best_fitness = best_fitness;
			_best_fitness_counter = 0;
		}
		
		if (_best_fitness_counter > 50)
			return true;
		
		// write fitnesses to file:
		std::stringstream ss;
		ss << "./optimisation_results_" << _counter;
		ss.flush();
		std::string result_stream_string(ss.str());
		std::ofstream result_stream(ss.str().c_str());
		
		ss << ".bin";
		ss.flush();
		std::string result_bin_stream_string(ss.str());
		std::ofstream result_bin_stream(ss.str().c_str());
		
		std::cout << "Saving the RandomNumberFactory...";
		RandomNumberFactory::save(result_bin_stream);
		std::cout << "done." << std::endl;
		
		std::cout << "Saving some doubles...";
		Serializer::serialize(result_bin_stream, _counter);
		Serializer::serialize(result_bin_stream, _best_fitness);
		Serializer::serialize(result_bin_stream, _best_fitness_counter);
		std::cout << "done." << std::endl;
		
		std::cout << "Saving the individuals...";
		std::multimap< double, MapGenerationIndividual/*, std::greater<double>*/ >
			fitness_individual_map;
		
		for (int i = 0; i < individuals->size(); ++i)
		{
			MapGenerationIndividual* mgi
				= dynamic_cast<MapGenerationIndividual*>( (*individuals)[i] );
			
			if (mgi == 0)
			{
				std::cout << "Cast failed!" << std::endl;
				throw "Oops!";
			}
			
			fitness_individual_map.insert(
				std::make_pair(mgi->get_fitness(), *mgi) );
			
//			std::cout << "INSERTED: " << mgi->get_fitness() << ": " << *mgi << std::endl;
		}
		
//		std::multimap< double, MapGenerationIndividual>::iterator iter = fitness_individual_map.begin();
//		for (; iter != fitness_individual_map.end(); ++iter)
//		{
//			std::cout << iter->first << ": " << iter->second << std::endl;
//		}
		
		
		// Serialize
		Serializer::serialize(result_bin_stream, fitness_individual_map);
		
		// Text output
		std::map<double, MapGenerationIndividual>::reverse_iterator map_iter
			= fitness_individual_map.rbegin();
		std::map<double, MapGenerationIndividual>::reverse_iterator map_iter_end
			= fitness_individual_map.rend();
		for (; map_iter != map_iter_end; ++map_iter)
		{
			result_stream << "Fitness = " << map_iter->second.get_fitness()
				<< ", Age = " << map_iter->second.get_age()
				<< std::endl << map_iter->second << std::endl << std::endl;
			
		}
		
		std::cout << "done." << std::endl;
		
		result_bin_stream.close();
		unlink("./optimisation_results");
		symlink(result_bin_stream_string.c_str(), "./optimisation_results");
		result_stream.close();
		
		++_counter;
		
		return (_counter > _MAX_ITERATION);
	}
	
	
	MapGenerationIndividual*
	MapGenerationEvolutionaryAlgorithm::create_individual()
	{
		MapGenerationIndividual* mi = new MapGenerationIndividual(_test_pool);
/*		mi->set_id(_individual_counter, _parents_count + _descendants_count + 10);
		
		std::cout << "set_id: " << _individual_counter << ", "
			<< _parents_count << ", " << _descendants_count << std::endl;
		++_individual_counter;
*/		
		return mi;
	}
	
	
	double
	MapGenerationEvolutionaryAlgorithm::get_random_number_for_object_component(
		int dimension)
	{
		return RandomNumberFactory::Ur(0.0, 100.0);
//		return RandomNumberFactory::Ur(-100.0, 100.0);
	}
	
	
	void
	MapGenerationEvolutionaryAlgorithm::init_individuals(
		std::vector<Individual*>& individual_pool)
	{
		AsymmetricAdaptiveEvolutionaryAlgorithm<MAP_GENERATION_INDIVIDUAL_DIMENSION>
			::init_individuals(individual_pool);
		
		if (_previous_optimisation_results_used)
		{
			std::multimap<double, MapGenerationIndividual> fitness_individual_map;
			Serializer::deserialize(_optimisation_results_ifstream,
				fitness_individual_map);
			
			std::map<double, MapGenerationIndividual>::iterator
				map_iter = fitness_individual_map.begin();
			std::map<double, MapGenerationIndividual>::iterator
				map_iter_end = fitness_individual_map.end();
			
			int i = 0; 
			while ( (i < fitness_individual_map.size()) && (i < individual_pool.size()) )
			{
				// Set TestPool!
				map_iter->second.set_test_pool(_test_pool);
				
				// Assign individual to pool
				*(individual_pool[i]) = map_iter->second;
				
				// Increment everything
				++i;
				++map_iter;
			}
			
		} else
		{
			std::ifstream starting_individuals("./starting_individuals");
			if (starting_individuals)
			{
				int count = 0;
				while (starting_individuals.eof() == false)
				{
					std::vector< std::vector<double> > vec;
					Serializer::deserialize(starting_individuals, vec);
				
					while ( (count < vec.size()) && (count < _parents_count) )
					{
						if (vec[count].size() >= MAP_GENERATION_INDIVIDUAL_DIMENSION)
							individual_pool[count]->init(vec[count]);
						
						++count;
					}
				}
			}
			
/*			std::vector<double> object_components;
			object_components.resize(MAP_GENERATION_INDIVIDUAL_DIMENSION);
			
			double individuals[10][25] = {
				{3.28022,    151.098,    34.9845,          0,          0,    22.0671,    60.1153,    39.6512,    29.1861,    30.1204,    148.397,    67.0079,    164.858,    2.45265,    9.19968,    159.525,    18.5306,     5.7505,    160.121,    167.475,    169.366,    64.4422,    99.4128,    160.329,    45.3154},
				{3.96896,    150.131,    26.1947,    2.19925,    14.8819,    22.8314,    60.6984,    49.4849,    29.3875,    24.1881,    54.2064,    91.3822,    63.6602,    2.93774,    9.84125,    160.481,    4.78616,    2.43644,    73.7553,    165.914,    166.275,    64.2846,    102.625,    15.0619,     43.234},
				{0.992823,    75.2857,    34.4227,    5.48386,          0,     25.328,    157.095,    18.8861,    29.5466,     95.549,    152.691,    67.0762,    57.1105,    3.43805,    5.80646,    152.513,    6.98879,    9.76734,    163.048,    162.141,    73.3189,    158.228,    96.7708,    15.2872,    41.0776},
				{7.22482,    42.2788,    18.7699,    6.32424,          0,    25.3086,    70.8544,    48.4973,    29.0689,    95.8177,     160.24,    160.465,     59.689,   0.819208,    10.1897,    157.029,    6.93193,    2.15125,    75.7963,    71.2481,    60.6413,    166.831,    104.165,    15.7893,    36.1181},
				{97.0704,    36.5003,    99.7842,    5.01459,          0,    18.7744,    160.098,    44.3202,    42.9525,    23.4893,    153.732,     48.238,    57.8718,          0,   0.745498,    160.318,    3.68146,    12.1972,    156.592,    12.3223,    165.066,    155.754,    91.3953,    15.7809,    39.5929},
				{4.74501,    148.845,    29.5599,    4.85858,          0,    21.2754,    160.418,    19.1771,    48.7558,    24.2198,     153.93,    77.8226,    64.6569,   0.645786,    6.43761,    161.572,    14.2846,    1.83523,    71.9539,    166.222,    87.7962,    150.135,    47.9147,    20.5676,     40.933},
				{1.42085,     70.695,    105.116,    9.72403,    1.33904,    25.8266,    158.271,    19.7027,    38.1128,    21.4559,    24.5928,    78.1334,    53.4167,     1.7262,    11.1171,    160.781,    5.54835,    10.2021,    159.721,    160.062,    164.886,    63.6056,    99.2954,    15.0035,    42.6462},
				{5.1585,    148.567,     38.432,    3.79475,     0.4221,    17.6254,    58.9495,    41.7807,    47.3195,    32.2543,    161.164,    75.9041,    161.406,          0,    6.69195,    161.631,    78.8498,    1.76203,    161.256,    160.675,    165.725,    22.5496,    25.9422,    11.3343,    159.408},
				{1,        150,        100,        3.5,        0.1,         20,        160,         20,         40,        160,        160,        160,        160,        0.5,          5,        160,          5,        7.5,        160,        160,        160,        160,         20,        160,        160},
				{0,    151.054,    35.3092,    26.8779,          0,    19.9826,    71.0152,    50.8481,     29.439,    21.6979,    153.508,    57.9603,    63.6669,          0,    5.83828,    161.381,    12.0972,    3.04232,     74.026,    161.889,    163.157,    153.241,    87.1905,    164.496,    44.7923}
			};
			
				{3.78442 ,    151.103,    36.5508,          0,    2.42322,    25.3623,    159.512,    46.2938,    28.8026,    159.751,    52.3883,    76.7739,    164.625,    1.12142,    3.94996,    157.293,    3.60927,    2.78219,     75.643,    70.7855,    168.999,    23.5583,    91.8948,    15.2616,    43.7146},
				{3.829   ,    148.326,    27.4574,          0,          0,    14.9621,    154.393,    49.1804,    28.8308,    33.7347,    151.235,    59.1163,    58.7586,          0,    7.05763,     158.04,          0,    7.15423,    80.9016,    162.316,     60.965,    157.921,    103.607,    18.3116,     38.282},
				{5.60067 ,    72.0604,    35.2126,          0,          0,    15.8919,    61.0016,    45.9203,    25.0506,     32.023,    149.478,    68.5151,    58.3311,   0.481737,    8.59429,    159.399,    6.29811,     11.417,    71.8023,    167.822,    169.964,     163.13,    92.7133,    17.7771,    42.9503},
				{0       ,    74.0678,    27.7332,          0,          0,    16.1857,     159.25,    49.2942,    33.3621,    156.664,    149.353,    65.0041,    53.6885,   0.624044,    5.15504,    157.104,    2.71759,    10.8901,    77.6498,    170.332,    163.542,    160.224,    99.7063,    26.0021,    45.3164},
				{0       ,    150.864,    34.4923,          0,    3.92484,    84.1934,    62.5734,    39.6292,    28.5023,    156.689,    160.087,    63.7373,    62.5688,    2.52794,    5.13106,    164.242,    2.17975,    9.16325,    74.8271,    161.436,    169.377,    158.266,    93.8031,    16.3061,    44.9047},
				{0.138163,    151.132,    43.8397,          0,          0,    88.3566,    154.227,    40.8925,    27.1548,    33.1022,    155.606,    65.6064,    52.4519,   0.551083,    7.32867,    152.871,    2.80665,    9.52796,    71.7046,    168.923,    61.9523,    164.617,     90.982,    17.0587,    37.8602},
				{0       ,    151.735,    23.4458,          0,   0.137317,    25.2194,    63.5117,    52.1171,    29.0376,    34.0043,    148.591,    77.0196,    164.099,    2.39592,    6.11072,    157.937,    1.51241,    2.46654,    156.257,    69.6712,    171.509,    166.427,    88.6715,    19.0256,    38.8242},
				{4.13402 ,    150.288,     43.832,          0,   0.804623,    18.8802,    61.2515,    48.4875,    32.7227,    23.3147,    150.199,    65.3153,     54.426,          0,    12.2548,    156.745,    11.3204,    8.83987,     74.797,    161.719,    168.381,     156.74,    94.6595,    17.4779,    43.7394},
				{4.66876 ,    71.9529,    42.4431,          0,          0,    21.2142,    58.1389,    47.1403,    25.3899,    162.771,    164.122,    64.1314,    62.1287,    4.40761,    7.49341,     154.25,    10.9421,    8.62279,    162.626,    161.342,    168.073,    156.456,    104.228,    157.775,    43.7392}, 
				{2.55409 ,    151.615,    25.4205,          0,    2.01097,    15.9085,    160.348,    51.9328,    27.6014,    33.6836,    155.191,    62.2523,    169.128,    3.37167,    10.4515,    152.522,     13.117,    9.42166,    159.908,    167.136,    60.3025,      66.78,     100.26,    14.4723,    38.5386}
			};
			
			for (int i = 0; i < 10; ++i)
			{
				for (int j = 0; j < 25; ++j)
				{
					object_components[j] = individuals[i][j];
				}
				
				individual_pool[i]->init(object_components);
			}
			
	/*		object_components[0] = 1.0;
			object_components[1] = 150.0;
			object_components[2] = 100.0;
			object_components[3] = 3.5;
			object_components[4] = 0.1; //r2r
			object_components[5] = 20.0;
			object_components[6] = 160.0;
			object_components[7] = 20.0; //vc2r
			object_components[8] = 40.0;
			object_components[9] = 160.0;
			object_components[10] = 160.0; //vf2r
			object_components[11] = 160.0;
			object_components[12] = 160.0;
			object_components[13] = 0.5; //step_distance
			object_components[14] = 5.0;
			object_components[15] = 160.0;
			object_components[16] = 5.0;
			object_components[17] = 7.5;
			object_components[18] = 160.0;
			object_components[19] = 160.0;
			object_components[20] = 160.0;
			object_components[21] = 160.0;
			object_components[22] = 20.0; //r2r_not_reachable
			object_components[23] = 160.0;
			object_components[24] = 160.0;
			individual_pool[9]->init(object_components);*/
		}
	}
	
} //namespace mapgeneration_opt
