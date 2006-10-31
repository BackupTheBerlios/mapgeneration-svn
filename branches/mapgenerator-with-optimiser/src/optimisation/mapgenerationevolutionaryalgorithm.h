/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef MAPGENERATIONEVOLUTIONARYALGORITHM_H
#define MAPGENERATIONEVOLUTIONARYALGORITHM_H

#include "asymmetricadaptiveevolutionaryalgorithm.h"

#include <fstream>

#include "mapgenerationindividual.h"

namespace mapgeneration_opt
{
	
	class MapGenerationEvolutionaryAlgorithm
		: public AsymmetricAdaptiveEvolutionaryAlgorithm<MAP_GENERATION_INDIVIDUAL_DIMENSION>
	{
		
		public:
			
			MapGenerationEvolutionaryAlgorithm(int parents_count,
				int descendants_count);
			
			virtual
			~MapGenerationEvolutionaryAlgorithm();
			
			
			virtual void
			apply_selector(
				const std::vector<Individual*>& current_parents,
				std::vector<bool>& selected_parents,
				const std::vector<Individual*>& current_descendants,
				std::vector<bool>& selected_descendants);
			
			
		protected:
			
			virtual bool
			cancel_condition(const std::vector<Individual*>* individuals);
			
			
			virtual MapGenerationIndividual*
			create_individual();
			
			
			virtual double
			get_random_number_for_object_component(int index);
			
			
			virtual void
			init_individuals(std::vector<Individual*>& individual_pool);
			
			
		private:
			
			static const unsigned int _MAX_AGE = 25;
			
			int _counter;
			static const int _MAX_ITERATION = 500;
			
			double _best_fitness;
			int _best_fitness_counter;
			
//			int _individual_counter;
			
			std::vector<FilteredTrace> _test_pool;
			
			std::ifstream _optimisation_results_ifstream;
			bool _previous_optimisation_results_used;
			
	};
	
} //namespace mapgeneration_opt

#endif //MAPGENERATIONEVOLUTIONARYALGORITHM_H
