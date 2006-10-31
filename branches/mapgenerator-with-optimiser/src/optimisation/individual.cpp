/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "individual.h"

#include <iomanip>
#include <iostream>

#include "util/serializer.h"
using mapgeneration_util::Serializer;

namespace mapgeneration_opt
{
	
	std::ostream&
	operator<<(std::ostream& o_stream, Individual& individual)
	{
		o_stream << "Fitness = " << std::setw(8) << individual.get_fitness()
			<< ". ";
		
		o_stream << "X = {";
		for (int i = 0; i < individual._object_components.size(); ++i)
		{
			o_stream << "[" << i << "] = " << individual._object_components[i];
			
			if (i < individual._object_components.size() - 1)
				o_stream << ", ";
		}
		o_stream << "}. ";
		
		try
		{
			OnePlusOneIndividual& opoi
				= dynamic_cast<OnePlusOneIndividual&>(individual);
			o_stream << "{sigma = " << opoi.get_sigma() << "}. ";
		} catch (...)
		{}
		
		return o_stream;
	}
	
	
	std::ostream&
	operator<<(std::ostream& o_stream, const Individual& individual)
	{
		o_stream << "Fitness = " << std::setw(8) << "NN"
			<< ". ";
		
		o_stream << "X = {";
		for (int i = 0; i < individual._object_components.size(); ++i)
		{
			o_stream << "[" << i << "] = " << individual._object_components[i];
			
			if (i < individual._object_components.size() - 1)
				o_stream << ", ";
		}
		o_stream << "}. ";
		
		try
		{
			const OnePlusOneIndividual& opoi
				= dynamic_cast<const OnePlusOneIndividual&>(individual);
			o_stream << "{sigma = " << opoi.get_sigma() << "}. ";
		} catch (...)
		{}
		
		return o_stream;
	}
	
	
	std::ostream&
	operator<<(std::ostream& o_stream,
		const std::vector<Individual*>& individuals)
	{
		o_stream << "--------------------------------------------------------"
			<< std::endl;
		
		for (int i = 0; i < individuals.size(); ++i)
		{
			o_stream << "Individual " << i << ": "
				<< individuals[i] << std::endl;
		};
		
		o_stream << "--------------------------------------------------------"
			<< std::endl;
		
		return o_stream;
	}
	
} // namespace mapgeneration_opt



namespace mapgeneration_opt
{
	
	Individual::Individual()
	: _fitness(0.0), _object_components(), _valid_fitness(false)
	{
	}
	
	
	Individual::~Individual()
	{
	}
	
	
	void
	Individual::calculate_fitness()
	{
		_fitness = impl_calculate_fitness();
		
		_valid_fitness = true;
	}
	
	
	void
	Individual::deserialize(std::istream& i_stream)
	{
		Serializer::deserialize(i_stream, _object_components);
		
		_valid_fitness = false;
	}
	
	
	double
	Individual::get_fitness()
	{
		if (!_valid_fitness)
			calculate_fitness();
		
		return _fitness;
	}
	
	
	void
	Individual::impl_mutate()
	{
	}
	
	
	void
	Individual::impl_recombinate(
		const std::vector<Individual*>& parents)
	{
	}
	
	
	void
	Individual::init(const std::vector<double>& object_components)
	{
		_valid_fitness = false;
		
		int dimensions = object_components.size();
		_object_components.resize(dimensions);
		for (int i = 0; i < dimensions; ++i)
			_object_components[i] = object_components[i];
	}
	
	
	void
	Individual::init(
		const std::vector< std::vector<double> >& components)
	{
		if (components.size() != 1)
			throw "Invalid components size!";
		
		init(components[0]);
	}
	
	
	void
	Individual::mutate()
	{
		_valid_fitness = false;
		
		impl_mutate();
	}
	
	
	const std::vector<double>&
	Individual::object_components() const
	{
		return _object_components;
	}
	
	
	Individual&
	Individual::operator=(const Individual& individual)
	{
		_object_components = individual._object_components;
		
		_fitness = individual._fitness;
		_valid_fitness = individual._valid_fitness;
		
		return *this;
	}
	
	
	void
	Individual::recombinate(const std::vector<Individual*>& parents)
	{
		_valid_fitness = false;
		
		impl_recombinate(parents);
	}
	
	
	void
	Individual::serialize(std::ostream& o_stream) const
	{
		Serializer::serialize(o_stream, _object_components);
	}
	
	
	void
	Individual::set_fitness(double fitness)
	{
		_valid_fitness = false;
		
		_fitness = fitness;
	}
	
} // namespace mapgeneration_opt


#include "randomnumberfactory.h"

namespace mapgeneration_opt
{
	
	OnePlusOneIndividual::OnePlusOneIndividual()
	: Individual(), _sigma(1.0)
	{
	}
	
	
	OnePlusOneIndividual::~OnePlusOneIndividual()
	{
	}
	
	
	void
	OnePlusOneIndividual::decrease_sigma()
	{
		_sigma = _sigma * _Cd;
	}
	
	
	void
	OnePlusOneIndividual::deserialize(std::istream& i_stream)
	{
		Individual::deserialize(i_stream);
		Serializer::deserialize(i_stream, _sigma);
	}
	
	
	double
	OnePlusOneIndividual::get_sigma() const
	{
		return _sigma;
	}
	
	
	void
	OnePlusOneIndividual::increase_sigma()
	{
		_sigma = _sigma * _Ci;
	}
	
	
	void
	OnePlusOneIndividual::impl_mutate()
	{
		for (int i = 0; i < this->_object_components.size(); ++i)
		{
			this->_object_components[i]
				+= RandomNumberFactory::N(0.0, _sigma);
		}
	}
	
	
	void
	OnePlusOneIndividual::impl_recombinate(
		const std::vector<Individual*>& parents)
	{
		const OnePlusOneIndividual* opo_individual
			= dynamic_cast<const OnePlusOneIndividual*>(parents[0]);
		
		if (opo_individual == 0)
			throw "Invalid parent!";
		
		for (int i = 0; i < this->_object_components.size(); ++i)
		{
			this->_object_components[i]
				= opo_individual->object_components()[i];
		}
		
		_sigma = opo_individual->get_sigma();
	}
	
	
	OnePlusOneIndividual&
	OnePlusOneIndividual::operator=(const Individual& individual)
	{
		Individual::operator=(individual);
		
		try
		{
			const OnePlusOneIndividual& opo_individual
				= dynamic_cast<const OnePlusOneIndividual&>(individual);
			
			_sigma = opo_individual._sigma;
		} catch (...)
		{
			std::cout << "WARUM???????????????????????????????" << std::endl;
		}
		
		return *this;
	}
	
	
	void
	OnePlusOneIndividual::serialize(std::ostream& o_stream) const
	{
		Individual::serialize(o_stream);
		Serializer::serialize(o_stream, _sigma);
	}
	
} // namespace mapgeneration_opt
