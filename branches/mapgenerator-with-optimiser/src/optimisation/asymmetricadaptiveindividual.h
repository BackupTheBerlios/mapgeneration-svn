/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef ASYMMETRICADAPTIVEINDIVIDUAL_H
#define ASYMMETRICADAPTIVEINDIVIDUAL_H

#include "individual.h"

namespace mapgeneration_opt
{
	
	template<int dimension>
	class AsymmetricAdaptiveIndividual;
	
	
	template<int dimension>
	std::ostream&
	operator<<(std::ostream& o_stream,
		AsymmetricAdaptiveIndividual<dimension>& aa_individual);
	
	template<int dimension>
	std::ostream&
	operator<<(std::ostream& o_stream,
		const AsymmetricAdaptiveIndividual<dimension>& aa_individual);
	
	
	template<int dimension>
	class AsymmetricAdaptiveIndividual : public Individual
	{
		
		friend std::ostream&
		operator<< <>(std::ostream& o_stream,
			AsymmetricAdaptiveIndividual<dimension>& aa_individual);
		
		friend std::ostream&
		operator<< <>(std::ostream& o_stream,
			const AsymmetricAdaptiveIndividual<dimension>& aa_individual);
		
		
		public:
			
			AsymmetricAdaptiveIndividual();
			
			
			virtual
			~AsymmetricAdaptiveIndividual();
			
			
			virtual void
			deserialize(std::istream& i_stream);
			
			
			virtual void
			init(const std::vector< std::vector<double> >& components);
			
			
			virtual AsymmetricAdaptiveIndividual&
			operator=(const Individual& individual);
			
			
			virtual void
			serialize(std::ostream& o_stream) const;
			
			
		protected:
			
			virtual void
			impl_mutate();
			
			
			virtual void
			impl_recombinate(const std::vector<Individual*>& parents);
			
			
		private:
			
			std::vector<double> _step_sizes;
			std::vector<double> _asymmetry_measures;
			
	};
	
} // namespace mapgeneration_opt


#include <iomanip>

namespace mapgeneration_opt
{
	
	template<int dimension>
	std::ostream&
	operator<<(std::ostream& o_stream,
		AsymmetricAdaptiveIndividual<dimension>& aa_individual)
	{
		Individual& ind = dynamic_cast<Individual&>(aa_individual);
		o_stream << ind;
		
		o_stream << "Sigma = {";
		for (int i = 0; i < dimension; ++i)
		{
			o_stream << "[" << i << "] = "
				<< std::setw(7) << aa_individual._step_sizes[i];
			
			if (i < dimension - 1)
				o_stream << ", ";
		}
		o_stream << "}. ";
		
		o_stream << "Asym = {";
		for (int i = 0; i < dimension; ++i)
		{
			o_stream << "[" << i << "] = "
				<< std::setw(7) << aa_individual._asymmetry_measures[i];
			
			if (i < dimension - 1)
				o_stream << ", ";
		}
		o_stream << "}. ";
		
		return o_stream;
	}
	
	
	template<int dimension>
	std::ostream&
	operator<<(std::ostream& o_stream,
		const AsymmetricAdaptiveIndividual<dimension>& aa_individual)
	{
		const Individual& ind = dynamic_cast<const Individual&>(aa_individual);
		o_stream << ind;
		
		o_stream << "Sigma = {";
		for (int i = 0; i < dimension; ++i)
		{
			o_stream << "[" << i << "] = "
				<< std::setw(7) << aa_individual._step_sizes[i];
			
			if (i < dimension - 1)
				o_stream << ", ";
		}
		o_stream << "}. ";
		
		o_stream << "Asym = {";
		for (int i = 0; i < dimension; ++i)
		{
			o_stream << "[" << i << "] = "
				<< std::setw(7) << aa_individual._asymmetry_measures[i];
			
			if (i < dimension - 1)
				o_stream << ", ";
		}
		o_stream << "}. ";
		
		return o_stream;
	}
	
} //namespace mapgeneration_opt


#include "randomnumberfactory.h"

namespace mapgeneration_opt
{
	template<int dimension>
	AsymmetricAdaptiveIndividual<dimension>::AsymmetricAdaptiveIndividual()
	: Individual(), _step_sizes(dimension), _asymmetry_measures(dimension)
	{
		this->_object_components.resize(dimension);
	}
	
	
	template<int dimension>
	AsymmetricAdaptiveIndividual<dimension>::~AsymmetricAdaptiveIndividual()
	{
	}
	
	
	template<int dimension>
	void
	AsymmetricAdaptiveIndividual<dimension>::deserialize(std::istream& i_stream)
	{
//		std::cout << "AAI::deserialize" << std::endl;
		Individual::deserialize(i_stream);
		Serializer::deserialize(i_stream, _step_sizes);
		Serializer::deserialize(i_stream, _asymmetry_measures);
	}
	
	
	template<int dimension>
	void
	AsymmetricAdaptiveIndividual<dimension>::init(
		const std::vector< std::vector<double> >& components)
	{
		if (components.size() != 3)
			throw "Invalid components size!";
		
		// first init the object component:
		if (components[0].size() != dimension)
			throw "Invalid object components size!";
		
		Individual::init(components[0]);
		
		// the step sizes:
		if (components[1].size() != dimension)
			throw "Invalid strategy components size: step sizes!";
		
		for (int i = 0; i < dimension; ++i)
			_step_sizes[i] = components[1][i];
		
		// the asymmetry measures:
		if (components[2].size() != dimension)
			throw "Invalid strategy components size: asymmetry measures!";
		
		for (int i = 0; i < dimension; ++i)
			_asymmetry_measures[i] = components[2][i];
	}
	
	
	template<int dimension>
	AsymmetricAdaptiveIndividual<dimension>&
	AsymmetricAdaptiveIndividual<dimension>::operator=(
		const Individual& individual)
	{
		Individual::operator=(individual);
		
		try
		{
			const AsymmetricAdaptiveIndividual<dimension>& aa_individual
				= dynamic_cast< const AsymmetricAdaptiveIndividual<dimension>& >(individual);
			
			_asymmetry_measures = aa_individual._asymmetry_measures;
			_step_sizes = aa_individual._step_sizes;
			
		} catch (...)
		{
			std::cout << "AsymmetricAdaptiveIndividual::operator= failed!"
				<< std::endl;
		}
		
		return *this;
	}
	
	
	template<int dimension>
	void
	AsymmetricAdaptiveIndividual<dimension>::impl_mutate()
	{
		double tau = 1.0 / sqrt( 2.0 * sqrt(static_cast<double>(dimension)) );
		double tau_hat = 1.0 / sqrt( 2.0 * static_cast<double>(dimension) );
		
		double global_lognormal_random_number
			= RandomNumberFactory::LN(0.0, tau_hat);
		for (int i = 0; i < dimension; ++i)
		{
			double local_lognormal_random_number
				= RandomNumberFactory::LN(0.0, tau);
			_step_sizes[i] = _step_sizes[i]
				* global_lognormal_random_number * local_lognormal_random_number;
		}
		
		double global_normal_random_number
			= RandomNumberFactory::N(0.0, tau_hat);
		for (int i = 0; i < dimension; ++i)
		{
			double local_normal_random_number
				= RandomNumberFactory::N(0.0, tau);
			_asymmetry_measures[i] = _asymmetry_measures[i]
				+ global_normal_random_number + local_normal_random_number;
		}
		
		for (int i = 0; i < dimension; ++i)
		{
			this->_object_components[i] = this->_object_components[i]
				+ RandomNumberFactory::A(_asymmetry_measures[i], _step_sizes[i]);
		}
	}
	
	
	template<int dimension>
	void
	AsymmetricAdaptiveIndividual<dimension>::impl_recombinate(
		const std::vector<Individual*>& parents)
	{
		assert(this->_object_components.size() == dimension);
		assert(_step_sizes.size() == dimension);
		assert(_asymmetry_measures.size() == dimension);
		
		int parents_size = parents.size();
		
		// recombinate object component (global discrete)
		for (int i = 0; i < dimension; ++i)
		{
			unsigned long uniform_random_number
				= RandomNumberFactory::Ui(0, parents_size - 1);
			this->_object_components[i]
				= parents[uniform_random_number]->object_components()[i];
		}
		
		///*
		// recombinate step sizes (global intermediate)
		for (int i = 0; i < dimension; ++i)
		{
			unsigned long uniform_random_number
				= RandomNumberFactory::Ui(0, parents_size - 1);
			const AsymmetricAdaptiveIndividual* aa_parent_1
				= dynamic_cast<const AsymmetricAdaptiveIndividual*>
					(parents[uniform_random_number]);
			if (aa_parent_1 == 0)
				throw "Invalid parent!";
			
			uniform_random_number
				= RandomNumberFactory::Ui(0, parents_size - 1);
			const AsymmetricAdaptiveIndividual* aa_parent_2
				= dynamic_cast<const AsymmetricAdaptiveIndividual*>
					(parents[uniform_random_number]);
			if (aa_parent_2 == 0)
				throw "Invalid parent!";
			
			
			_step_sizes[i] = ( aa_parent_1->_step_sizes[i]
				+ aa_parent_2->_step_sizes[i] ) / 2.0;
		}
		//*/
		
		/*
		// recombinate step sizes (global discrete)
		for (int i = 0; i < dimension; ++i)
		{
			unsigned long uniform_random_number
				= RandomNumberFactory::Ui(0, parents_size - 1);
			
			const AsymmetricAdaptiveIndividual* aa_parent
				= dynamic_cast<const AsymmetricAdaptiveIndividual*>
					(parents[uniform_random_number]);
			if (aa_parent == 0)
				throw "Invalid parent!";
			
			_step_sizes[i] = aa_parent->_step_sizes[i];
		}
		//*/
		
		
		// recombinate asymmetric measures (global intermediate)
		for (int i = 0; i < dimension; ++i)
		{
			unsigned long uniform_random_number
				= RandomNumberFactory::Ui(0, parents_size - 1);
			const AsymmetricAdaptiveIndividual* aa_parent_1
				= dynamic_cast<const AsymmetricAdaptiveIndividual*>
					(parents[uniform_random_number]);
			if (aa_parent_1 == 0)
				throw "Invalid parent!";
			
			uniform_random_number
				= RandomNumberFactory::Ui(0, parents_size - 1);
			const AsymmetricAdaptiveIndividual* aa_parent_2
				= dynamic_cast<const AsymmetricAdaptiveIndividual*>
					(parents[uniform_random_number]);
			if (aa_parent_2 == 0)
				throw "Invalid parent!";
			
			_asymmetry_measures[i] = ( aa_parent_1->_asymmetry_measures[i]
				+ aa_parent_2->_asymmetry_measures[i] ) / 2.0;
		}
	}
	
	
	template<int dimension>
	void
	AsymmetricAdaptiveIndividual<dimension>::serialize(std::ostream& o_stream)
		const
	{
//		std::cout << "AAI::serialize" << std::endl;
		Individual::serialize(o_stream);
		Serializer::serialize(o_stream, _step_sizes);
		Serializer::serialize(o_stream, _asymmetry_measures);
	}
	
} // namespace mapgeneration_opt



namespace mapgeneration_opt
{
	
	template<int dimension>
	class SphereAsymmetricAdaptiveIndividual
		: public AsymmetricAdaptiveIndividual<dimension>
	{
		
		protected:
			
			virtual double
			impl_calculate_fitness() const
			{
				assert(this->_object_components.size() == dimension);
				
				// sphere model ?!
				double value = 0.0;
				for (int i = 0; i < dimension; ++i)
					value += this->_object_components[i] * this->_object_components[i];
				
				return -value;
			}
			
	};

} //namespace mapgeneration_opt

#endif //ASYMMETRICADAPTIVEINDIVIDUAL_H
