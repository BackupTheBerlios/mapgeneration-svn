/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef ASYMMETRICADAPTIVEEVOLUTIONARYALGORITHM_H
#define ASYMMETRICADAPTIVEEVOLUTIONARYALGORITHM_H

#include "asymmetricadaptiveindividual.h"
#include "evolutionaryalgorithm.h"

namespace mapgeneration_opt
{
	
	template<int dimension>
	class AsymmetricAdaptiveEvolutionaryAlgorithm
		: public EvolutionaryAlgorithm
	{
		
		public:
			
			AsymmetricAdaptiveEvolutionaryAlgorithm(int parents_count,
				int descendants_count);
			
			
			virtual
			~AsymmetricAdaptiveEvolutionaryAlgorithm();
			
			
			virtual void
			apply_selector(
				const std::vector<Individual*>& current_parents,
				std::vector<bool>& selected_parents,
				const std::vector<Individual*>& current_descendants,
				std::vector<bool>& selected_descendants);
			
			
			
		protected:
			
			virtual bool
			cancel_condition(const std::vector<Individual*>* individuals) = 0;
			
			
			virtual AsymmetricAdaptiveIndividual<dimension>*
			create_individual() = 0;
			
			
			virtual double
			get_random_number_for_asymmetry_measure(int index);
			
			
			virtual double
			get_random_number_for_object_component(int index);
			
			
			virtual double
			get_random_number_for_step_size(int index);
			
			
			virtual void
			init_individuals(std::vector<Individual*>& individual_pool);
			
	};
	
/*	
	template<int dimension>
	class SphereAsymmetricAdaptiveEvolutionaryAlgorithm
		: public AsymmetricAdaptiveEvolutionaryAlgorithm<dimension>
	{
		
		public:
			
			SphereAsymmetricAdaptiveEvolutionaryAlgorithm(
				int parents_count, int descendants_count)
			: AsymmetricAdaptiveEvolutionaryAlgorithm<dimension>(
				parents_count, descendants_count), _counter(0)
			{
			}
			
			
		protected:
			
			virtual bool
			cancel_condition(const std::vector<Individual*>* individuals)
			{
///*				std::cout << _counter << ":" << std::endl;
				for (int i = 0; i < individuals->size(); ++i)
				{
					std::cout
						<< *dynamic_cast<AsymmetricAdaptiveIndividual<dimension>*>
							( (*individuals)[i] )
						<< std::endl;
				}
				std::cout << std::endl;
// /				
				double best_fitness = (*individuals)[0]->get_fitness();
				
				for (int i = 1; i < individuals->size(); ++i)
				{
					double fitness = (*individuals)[i]->get_fitness();
					
					if (fitness > best_fitness)
						best_fitness = fitness;
				}
				
				bool result = (best_fitness > -std::pow(10.0, -6.0)) || (++_counter > 1000);
				if (result) std::cout << _counter << std::endl;
					
				return result;
			}
			
			
		private:
			
			int _counter;
			
	};
*/			
	
} //namespace mapgeneration_opt


#include "evolutionaryalgorithmfactory.h"

namespace mapgeneration_opt
{
	
	template<int dimension>
	AsymmetricAdaptiveEvolutionaryAlgorithm<dimension>
		::AsymmetricAdaptiveEvolutionaryAlgorithm
		(int parents_count, int descendants_count)
	: EvolutionaryAlgorithm(parents_count, descendants_count)
	{
		this->_cancel_condition_functor = new Functor
			< AsymmetricAdaptiveEvolutionaryAlgorithm, bool, const std::vector<Individual*> >
			(this, &AsymmetricAdaptiveEvolutionaryAlgorithm<dimension>::cancel_condition);
	}
	
	
	template<int dimension>
	AsymmetricAdaptiveEvolutionaryAlgorithm<dimension>
		::~AsymmetricAdaptiveEvolutionaryAlgorithm()
	{
		delete this->_cancel_condition_functor;
	}
	
	
	template<int dimension>
	void
	AsymmetricAdaptiveEvolutionaryAlgorithm<dimension>::apply_selector(
		const std::vector<Individual*>& current_parents,
		std::vector<bool>& selected_parents,
		const std::vector<Individual*>& current_descendants,
		std::vector<bool>& selected_descendants)
	{
		EvolutionaryAlgorithmFactory::deterministic_plus_selector(
			current_parents, selected_parents,
			current_descendants, selected_descendants);
	}
	
	
	template<int dimension>
	double
	AsymmetricAdaptiveEvolutionaryAlgorithm<dimension>
		::get_random_number_for_asymmetry_measure(int index)
	{
		return RandomNumberFactory::Ur(-0.5, 0.5);
	}
	
	
	template<int dimension>
	double
	AsymmetricAdaptiveEvolutionaryAlgorithm<dimension>
		::get_random_number_for_object_component(int index)
	{
		return 0.0;
	}
	
	
	template<int dimension>
	double
	AsymmetricAdaptiveEvolutionaryAlgorithm<dimension>
		::get_random_number_for_step_size(int index)
	{
		return 1.0;
	}
	
	
	template<int dimension>
	void
	AsymmetricAdaptiveEvolutionaryAlgorithm<dimension>::init_individuals(
		std::vector<Individual*>& individual_pool)
	{
		for (int i = 0; i < individual_pool.size(); ++i)
		{
			std::vector<double> object_components(dimension);
			std::vector<double> step_sizes(dimension);
			std::vector<double> asymmetry_measures(dimension);
			for (int dim = 0; dim < dimension; ++dim)
			{
				object_components[dim]
					= get_random_number_for_object_component(dim);
				step_sizes[dim]
					= get_random_number_for_step_size(dim);
				asymmetry_measures[dim]
					= get_random_number_for_asymmetry_measure(dim);
			}
			
			std::vector< std::vector<double> > components;
			components.resize(3);
			components[0] = object_components;
			components[1] = step_sizes;
			components[2] = asymmetry_measures;
			
			AsymmetricAdaptiveIndividual<dimension>* sphere_ind
				= create_individual();
			
			sphere_ind->init(components);
			
			individual_pool[i] = sphere_ind;
		}
	}
	
} //namespace mapgeneration_opt

#endif //ASYMMETRICADAPTIVEEVOLUTIONARYALGORITHM_H
