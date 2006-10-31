/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef EVOLUTIONARYALGORITHMFACTORY_H
#define EVOLUTIONARYALGORITHMFACTORY_H

#include <vector>

#include "individual.h"

namespace mapgeneration_opt
{
	
	class EvolutionaryAlgorithmFactory
	{
		
		public:
			
			static void
			deterministic_comma_selector(
				const std::vector<Individual*>& current_parents,
				std::vector<bool>& selected_parents,
				const std::vector<Individual*>& current_descendants,
				std::vector<bool>& selected_descendants);
			
			
			static void
			deterministic_plus_selector(
				const std::vector<Individual*>& current_parents,
				std::vector<bool>& selected_parents,
				const std::vector<Individual*>& current_descendants,
				std::vector<bool>& selected_descendants);
			
	};
	
}

#endif //EVOLUTIONARYALGORITHMFACTORY_H
