/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef EVOLUTIONARYALGORITHM_H
#define EVOLUTIONARYALGORITHM_H

#include <vector>

#include "functor.h"
#include "individual.h"

namespace mapgeneration_opt
{
	
	class EvolutionaryAlgorithm
	{
		
		public:
			
			typedef GenericFunctor< bool, const std::vector<Individual*> >
				CancelConditionFunctor;
			
			
			EvolutionaryAlgorithm(int parents_count, int descendants_count);
			
			
			virtual
			~EvolutionaryAlgorithm();
			
			
			void
			define_cancel_condition(CancelConditionFunctor* functor);
			
			
			const std::vector<Individual*>& 
			parents() const;
			
			
			void
			run();
			
			
		protected:
			
			virtual void
			apply_selector(
				const std::vector<Individual*>& current_parents,
				std::vector<bool>& selected_parents,
				const std::vector<Individual*>& current_descendants,
				std::vector<bool>& selected_descendants) = 0;
			
			
			virtual void
			execute_at_loop_begin();
			
			
			virtual void
			init_individuals(std::vector<Individual*>& individual_pool) = 0;
			
			
//		private:
			
			int _descendants_count;
			
			
			int _parents_count;
			
			
			std::vector<Individual*> _descendants;
			
			
			std::vector<Individual*> _parents;
			
			
			CancelConditionFunctor* _cancel_condition_functor;
			
	};
	
	
	
	
	class OnePlusOneEvolutionaryAlgorithm
		: public EvolutionaryAlgorithm
	{
		
		public:
			
			OnePlusOneEvolutionaryAlgorithm(
				bool use_one_to_five_success_rule = true);
			
			
			virtual
			~OnePlusOneEvolutionaryAlgorithm();
			
			
		protected:
			
			void
			apply_selector(
				const std::vector<Individual*>& current_parents,
				std::vector<bool>& selected_parents,
				const std::vector<Individual*>& current_descendants,
				std::vector<bool>& selected_descendants);
			
			
			virtual void
			execute_at_loop_begin();
			
			
		private:
			
			int _loops_since_last_check;
			
			
			int _selected_descendants_since_last_check;
			
			
			bool _use_one_to_five_success_rule;
			
	};
	
	
	
	
	template<int dimension>
	class SphereOnePlusOneEvolutionaryAlgorithm
		: public OnePlusOneEvolutionaryAlgorithm
	{
		
		public:
			
			SphereOnePlusOneEvolutionaryAlgorithm();
			
			
			virtual
			~SphereOnePlusOneEvolutionaryAlgorithm();
			
			
		protected:
			
			bool
			cancel_condition(const std::vector<Individual*>* individuals);
			
			
			virtual void
			execute_at_loop_begin();
			
			
			virtual void
			init_individuals(std::vector<Individual*>& individual_pool);
			
			
		private:
			
			CancelConditionFunctor*
			_cancel_condition_functor;
			
			
			int
			_just_a_counter;
			
			
			static const int
			_MAX_ITERATION;
			
	};
	
} // namespace mapgeneration_opt


#include <cmath>
#include <iostream>

#include "asymmetricadaptiveindividual.h"
#include "randomnumbergenerators.h"

namespace mapgeneration_opt
{
	
	template<int dimension>
	const int SphereOnePlusOneEvolutionaryAlgorithm<dimension>::_MAX_ITERATION
		= 1001;
	
	template<int dimension>
	SphereOnePlusOneEvolutionaryAlgorithm<dimension>::
		SphereOnePlusOneEvolutionaryAlgorithm()
	: OnePlusOneEvolutionaryAlgorithm(false), _just_a_counter(_MAX_ITERATION)
	{
		this->_cancel_condition_functor
			= new Functor< SphereOnePlusOneEvolutionaryAlgorithm, bool, const std::vector<Individual*> >
			(this, &SphereOnePlusOneEvolutionaryAlgorithm<dimension>::cancel_condition);
	}
	
	
	template<int dimension>
	SphereOnePlusOneEvolutionaryAlgorithm<dimension>::
		~SphereOnePlusOneEvolutionaryAlgorithm()
	{
		delete _cancel_condition_functor;
	}
	
	
	template<int dimension>
	bool
	SphereOnePlusOneEvolutionaryAlgorithm<dimension>::cancel_condition(
		const std::vector<Individual*>* individuals)
	{
		assert(individuals->size() > 0);
		
		--_just_a_counter;
		
		double fitness = ((*individuals)[0])->get_fitness();
		
		bool result = ((_just_a_counter <= 0) || (fitness > -pow(10.0, -20.0)));
		
		if (result)
			std::cout << "FITNESS: " << fitness << ", counter: "
				<< _MAX_ITERATION - _just_a_counter << std::endl;
		
		return result;
	}
	
	
	template<int dimension>
	void
	SphereOnePlusOneEvolutionaryAlgorithm<dimension>::execute_at_loop_begin()
	{
		OnePlusOneEvolutionaryAlgorithm::execute_at_loop_begin();
		
		if (_just_a_counter > _MAX_ITERATION - 10)
		{
			SphereAsymmetricAdaptiveIndividual<dimension>* saa_ind
				= dynamic_cast<SphereAsymmetricAdaptiveIndividual<dimension>*>
					(parents()[0]);
			
			if (saa_ind != 0)
			{
				std::cout << _MAX_ITERATION - _just_a_counter << ": "
					<< *saa_ind << std::endl;
			}
		}
	}
	
	template<int dimension>
	void
	SphereOnePlusOneEvolutionaryAlgorithm<dimension>::init_individuals(
		std::vector<Individual*>& individual_pool)
	{
		for(int i = 0; i < individual_pool.size(); ++i)
		{
			///*
			std::vector<double> object_components(dimension);
			for (int dim = 0; dim < dimension; ++dim)
			{
				object_components[dim]
					= RandomNumberFactory::Ur(-1000.0, 1000.0);
			}
			
			SphereOnePlusOneIndividual<dimension>* sphere_ind
				= new SphereOnePlusOneIndividual<dimension>;
			sphere_ind->init(object_components);
			//*/
			
			/*
			std::vector<double> object_components(dimension);
			std::vector<double> step_sizes(dimension);
			std::vector<double> asymmetry_measures(dimension);
			for (int dim = 0; dim < dimension; ++dim)
			{
				object_components[dim]
					= RandomNumberFactory::Ur(-1000.0, 1000.0);
				step_sizes[dim]
					= RandomNumberFactory::Ur(1.0, 20.0);
				asymmetry_measures[dim]
					= RandomNumberFactory::Ur(-10.0, 10.0);
			}
			
			std::vector< std::vector<double> > components;
			components.resize(3);
			components[0] = object_components;
			components[1] = step_sizes;
			components[2] = asymmetry_measures;
			
			SphereAsymmetricAdaptiveIndividual<dimension>* sphere_ind
				= new SphereAsymmetricAdaptiveIndividual<dimension>;
			
			sphere_ind->init(components);
			//*/
			
			individual_pool[i] = sphere_ind;
		}
	}
	
} //namespace mapgeneration_opt

#endif //EVOLUTIONARYALGORITHM_H
