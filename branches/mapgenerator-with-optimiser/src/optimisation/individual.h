/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

#include <ostream>
#include <vector>

#include "functor.h"

namespace mapgeneration_opt
{
	
	class Individual;
	
	
	std::ostream&
	operator<<(std::ostream& o_stream, Individual& individual);
	std::ostream&
	operator<<(std::ostream& o_stream, const Individual& individual);
	
	
	std::ostream&
	operator<<(std::ostream& o_stream,
		const std::vector<Individual*>& individuals);
	
	
	class Individual
	{
		
		friend std::ostream&
		operator<<(std::ostream& o_stream, Individual& individual);
		friend std::ostream&
		operator<<(std::ostream& o_stream, const Individual& individual);
		
		
		public:
			
			Individual();
			
			
			virtual
			~Individual();
			
			
			virtual void
			deserialize(std::istream& i_stream);
			
			
			double
			get_fitness();
			
			
			void
			init(const std::vector<double>& object_components);
			
			
			// convention:
			// components[0] = object_components !!!
			virtual void
			init(const std::vector< std::vector<double> >& components);
			
			
			void
			mutate();
			
			
			const std::vector<double>&
			object_components() const;
			
			
			virtual Individual&
			operator=(const Individual& individual);
			
			
			void
			recombinate(const std::vector<Individual*>& parents);
			
			
			virtual void
			serialize(std::ostream& o_stream) const;
			
			
		protected:
			
			std::vector<double> _object_components;
			
			
			virtual double
			impl_calculate_fitness() const = 0;
			
			
			virtual void
			impl_mutate();
			
			
			virtual void
			impl_recombinate(const std::vector<Individual*>& parents);
			
			
			void
			set_fitness(double fitness);
			
			
		private:
			
			double _fitness;
			
			
			bool _valid_fitness;
			
			
			void
			calculate_fitness();
			
	};
	
	
	
	
	class OnePlusOneIndividual : public Individual
	{
		
		public:
			
			typedef GenericFunctor<double, double> RandomNumberGeneratorFunctor;
			
			
			OnePlusOneIndividual();
			
			
			virtual
			~OnePlusOneIndividual();
			
			
			void
			decrease_sigma();
			
			
			virtual void
			deserialize(std::istream& i_stream);
			
			
			double
			get_sigma() const;
			
			
			void
			increase_sigma();
			
			
			virtual OnePlusOneIndividual&
			operator=(const Individual& individual);
			
			
			virtual void
			serialize(std::ostream& o_stream) const;
			
			
		protected:
			
			virtual void
			impl_mutate();
			
			
			virtual void
			impl_recombinate(const std::vector<Individual*>& parents);
			
			
		private:
			
			static const double _Cd = 0.82;
			static const double _Ci = 1.0 / 0.82;
			
			double _sigma;
			
	};
	
	
	template<int dimension>
	class SphereOnePlusOneIndividual : public OnePlusOneIndividual
	{
		
		protected:
			
			virtual double
			impl_calculate_fitness() const;
			
	};
		
}


namespace mapgeneration_opt
{
	
	template<int dimension>
	double
	SphereOnePlusOneIndividual<dimension>::impl_calculate_fitness() const
	{
		assert(this->_object_components.size() == dimension);
		
		// sphere model ?!
		double value = 0.0;
		for (int i = 0; i < dimension; ++i)
			value += this->_object_components[i] * this->_object_components[i];
		
		return -value;
	}
		
} // namespace mapgeneration_opt

#endif //INDIVIDUAL_H
