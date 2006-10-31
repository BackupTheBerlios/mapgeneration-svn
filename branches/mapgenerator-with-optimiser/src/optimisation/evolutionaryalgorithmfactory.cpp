/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "evolutionaryalgorithmfactory.h"

//#include <iostream>
#include <map>

namespace mapgeneration_opt
{
	
	void
	EvolutionaryAlgorithmFactory::deterministic_comma_selector(
		const std::vector<Individual*>& current_parents,
		std::vector<bool>& selected_parents,
		const std::vector<Individual*>& current_descendants,
		std::vector<bool>& selected_descendants)
	{
		int current_parents_size = current_parents.size();
		int current_descendants_size = current_descendants.size();
		
		if (current_descendants_size < current_parents_size)
			throw "Invalid initialisation: descendants less then parents!";
		
		std::multimap< double, int, std::greater<double> >
			fitness_individual_mapper;
		
		for (int i = 0; i < current_descendants_size; ++i)
		{
			double fitness = current_descendants[i]->get_fitness();
			fitness_individual_mapper.insert( std::make_pair(fitness, i) );
		}
		
		std::multimap< double, int, std::greater<double> >::iterator iter
			= fitness_individual_mapper.begin();
		for (int i = 0; i < current_parents_size; ++i, ++iter)
			selected_descendants[i] = true;
	}
	
	
	void
	EvolutionaryAlgorithmFactory::deterministic_plus_selector(
		const std::vector<Individual*>& current_parents,
		std::vector<bool>& selected_parents,
		const std::vector<Individual*>& current_descendants,
		std::vector<bool>& selected_descendants)
	{
		int current_parents_size = current_parents.size();
		int current_descendants_size = current_descendants.size();
		
		std::multimap< double, std::pair<int, bool>, std::greater<double> >
			fitness_individual_mapper;
		
		for (int i = 0; i < current_parents_size; ++i)
		{
			double fitness = current_parents[i]->get_fitness();
			fitness_individual_mapper.insert(std::make_pair(
				fitness, std::make_pair(i, true) ));
		}
		
		for (int j = 0; j < current_descendants_size; ++j)
		{
			double fitness = current_descendants[j]->get_fitness();
			fitness_individual_mapper.insert(std::make_pair(
				fitness, std::make_pair(j, false) ));
		}
		
		std::multimap< double, std::pair<int, bool>, std::greater<double> >::iterator
			iter = fitness_individual_mapper.begin();
		for (int i = 0; i < current_parents_size; ++i, ++iter)
		{
			if (iter->second.second == true)
				selected_parents[iter->second.first] = true;
			else
				selected_descendants[iter->second.first] = true;
		}
	}
	
}
