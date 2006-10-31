/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "evolutionaryalgorithm.h"

#include "evolutionaryalgorithmfactory.h"
#include "randomnumberfactory.h"

namespace mapgeneration_opt
{
	
	EvolutionaryAlgorithm::EvolutionaryAlgorithm(int parents_count,
		int descendants_count)
	: _cancel_condition_functor(0), _descendants(descendants_count),
		_descendants_count(descendants_count), _parents(parents_count),
		_parents_count(parents_count)
	{
//		RandomNumberFactory::_simulation_mode = true;
		RandomNumberFactory::register_user();
	}
	
	
	EvolutionaryAlgorithm::~EvolutionaryAlgorithm()
	{
		for (int i = 0; i < _parents_count; ++i)
			delete _parents[i];
		
		for (int i = 0; i < _descendants_count; ++i)
			delete _descendants[i];
		
		RandomNumberFactory::unregister_user();
	}
	
	
	void
	EvolutionaryAlgorithm::define_cancel_condition(
		CancelConditionFunctor* functor)
	{
		_cancel_condition_functor = functor;
	}
	
	
	const std::vector<Individual*>&
	EvolutionaryAlgorithm::parents() const
	{
		return _parents;
	}
	
	
	void
	EvolutionaryAlgorithm::run()
	{
		if (_cancel_condition_functor == 0)
			throw "No cancel condition set. Define cancel condition function!";
		
		std::vector<Individual*> individual_pool(_descendants_count + _parents_count);
		init_individuals(individual_pool);
		
		for (int i = 0; i < _parents_count; ++i)
			_parents[i] = individual_pool[i];
		
		for (int i = 0; i < _descendants_count; ++i)
			_descendants[i] = individual_pool[i + _parents_count];
		
		while( !(*_cancel_condition_functor)(&_parents) )
		{
			execute_at_loop_begin();
			
			// recombination phase
			for (int i = 0; i < _descendants_count; ++i)
				_descendants[i]->recombinate(_parents);
			
			// mutation phase
			for (int i = 0; i < _descendants_count; ++i)
				_descendants[i]->mutate();
			
			// selection phase
			std::vector<bool> selected_parents(_parents_count);
			for (int i = 0; i < _parents_count; ++i)
				selected_parents[i] = false;
			
			std::vector<bool> selected_descendants(_descendants_count);
			for (int j = 0; j < _descendants_count; ++j)
				selected_descendants[j] = false;
			
			apply_selector(_parents, selected_parents,
				_descendants, selected_descendants);
			
			// copy selected parents to _parents
			// and the rest to _descendants
			std::vector<Individual*> temporary_saved_parents(_parents_count);
			std::vector<Individual*> temporary_saved_descendants(_descendants_count);
			int parents_index = 0;
			int descendants_index = 0;
			for (int i = 0; i < _parents_count; ++i)
			{
				if (selected_parents[i] == true)
				{
					temporary_saved_parents[parents_index] = _parents[i];
					++parents_index;
				} else
				{
					temporary_saved_descendants[descendants_index] = _parents[i];
					++descendants_index;
				}
			}
			
			for (int j = 0; j < _descendants_count; ++j)
			{
				if (selected_descendants[j] == true)
				{
					temporary_saved_parents[parents_index] = _descendants[j];
					++parents_index;
				} else
				{
					temporary_saved_descendants[descendants_index] = _descendants[j];
					++descendants_index;
				}
			}
			
			for (int k = 0; k < _parents_count; ++k)
				_parents[k] = temporary_saved_parents[k];
			
			for (int m = 0; m < _descendants_count; ++m)
				_descendants[m] = temporary_saved_descendants[m];
			
		} // end while
		
	}
	
	
	void
	EvolutionaryAlgorithm::execute_at_loop_begin()
	{
	}
	
} // namespace mapgeneration_opt



namespace mapgeneration_opt
{
	
	OnePlusOneEvolutionaryAlgorithm::OnePlusOneEvolutionaryAlgorithm(
		bool use_one_to_five_success_rule)
	: EvolutionaryAlgorithm(1, 1), _loops_since_last_check(0),
		_selected_descendants_since_last_check(0),
		_use_one_to_five_success_rule(use_one_to_five_success_rule)
	{
	}
	
	
	OnePlusOneEvolutionaryAlgorithm::~OnePlusOneEvolutionaryAlgorithm()
	{
	}
	
	
	void
	OnePlusOneEvolutionaryAlgorithm::apply_selector(
		const std::vector<Individual*>& current_parents,
		std::vector<bool>& selected_parents,
		const std::vector<Individual*>& current_descendants,
		std::vector<bool>& selected_descendants)
	{
		EvolutionaryAlgorithmFactory::deterministic_plus_selector(
			current_parents, selected_parents,
			current_descendants, selected_descendants);
		
		// 1/5 success rule:
		if (selected_descendants[0] == true)
			++_selected_descendants_since_last_check;
	}
	
	
	void
	OnePlusOneEvolutionaryAlgorithm::execute_at_loop_begin()
	{
		// 1/5 success rule:
		if (_use_one_to_five_success_rule)
		{
			++_loops_since_last_check;
			
			if (_loops_since_last_check >= 10)
			{
				if (_selected_descendants_since_last_check > 2)
				{
					OnePlusOneIndividual* ind
						= dynamic_cast<OnePlusOneIndividual*>(_parents[0]);
					
					ind->increase_sigma();
						
				} else if (_selected_descendants_since_last_check < 2)
				{
					OnePlusOneIndividual* ind
						= dynamic_cast<OnePlusOneIndividual*>(_parents[0]);
					
					ind->decrease_sigma();
				}
				
				_loops_since_last_check = 0;
				_selected_descendants_since_last_check = 0;
			}
		}
	}
	
} // namespace mapgeneration_opt
