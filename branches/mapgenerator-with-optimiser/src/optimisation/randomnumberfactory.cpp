/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "randomnumberfactory.h"

namespace mapgeneration_opt
{
	
	RandomNumberFactory* RandomNumberFactory::_factory = 0;
	bool RandomNumberFactory::_simulation_mode = false;
	int RandomNumberFactory::_users = 0;
	
	
	RandomNumberFactory::RandomNumberFactory()
	: _asymmetric_rng(0), _lognormal_rng(0)
	{
		if (_simulation_mode)
			_mersenne_twister = new MTRand(0U);
		else
			_mersenne_twister = new MTRand;
	}
	
	
	RandomNumberFactory::~RandomNumberFactory()
	{
		if (_asymmetric_rng != 0)
			delete _asymmetric_rng;
		
		if (_lognormal_rng != 0)
			delete _lognormal_rng;
		
		if (_mersenne_twister != 0)
			delete _mersenne_twister;
	}
	
	
	double
	RandomNumberFactory::asymmetric_random_number(double step_size,
		double asymmetry, double gamma)
	{
		if (_factory->_mersenne_twister == 0)
		{
			if (_simulation_mode)
				_factory->_mersenne_twister = new MTRand(0U);
			else
				_factory->_mersenne_twister = new MTRand;
		}
		
		if (_factory->_asymmetric_rng == 0)
		{
			_factory->_asymmetric_rng = new AsymmetricRandomNumberGenerator(
				_factory->_mersenne_twister);
		}
		
		return _factory->_asymmetric_rng->rand(step_size, asymmetry, gamma);
	}
	
	
	double
	RandomNumberFactory::lognormal_random_number(double mean, double variance)
	{
		if (_factory->_mersenne_twister == 0)
		{
			if (_simulation_mode)
				_factory->_mersenne_twister = new MTRand(0U);
			else
				_factory->_mersenne_twister = new MTRand;
		}
		
		if (_factory->_lognormal_rng == 0)
		{
			_factory->_lognormal_rng = new LogNormalRandomNumberGenerator(
				_factory->_mersenne_twister);
		}
		
		return _factory->_lognormal_rng->rand(mean, variance);
	}
	
	
	double
	RandomNumberFactory::normal_random_number(double mean, double variance)
	{
		if (_factory->_mersenne_twister == 0)
		{
			if (_simulation_mode)
				_factory->_mersenne_twister = new MTRand(0U);
			else
				_factory->_mersenne_twister = new MTRand;
		}
		
		return _factory->_mersenne_twister->randNorm(mean, variance);
	}
	
	
	void
	RandomNumberFactory::register_user()
	{
		if (_users == 0)
		{
			if (_factory == 0)
				_factory = new RandomNumberFactory;
			else
				assert(_factory == 0);
		}
		
		++_users;
	}
	
	
	unsigned long
	RandomNumberFactory::uniform_integer_random_number(unsigned long start_incl,
		unsigned long end_incl)
	{
		if (_factory->_mersenne_twister == 0)
		{
			if (_simulation_mode)
				_factory->_mersenne_twister = new MTRand(0U);
			else
				_factory->_mersenne_twister = new MTRand;
		}
		
		unsigned long uniform_integer_random_number
			= _factory->_mersenne_twister->randInt(end_incl - start_incl);
		
		return uniform_integer_random_number + start_incl;
	}
	
	
	double
	RandomNumberFactory::uniform_real_random_number(double start_incl,
		double end_incl)
	{
		if (_factory->_mersenne_twister == 0)
		{
			if (_simulation_mode)
				_factory->_mersenne_twister = new MTRand(0U);
			else
				_factory->_mersenne_twister = new MTRand;
		}
		
		double uniform_real_random_number
			= _factory->_mersenne_twister->rand(end_incl - start_incl);
		
		return uniform_real_random_number + start_incl;
	}
	
	
	void
	RandomNumberFactory::unregister_user()
	{
		if (_users > 0)
			--_users;
		
		if (_users == 0)
		{
			assert (_factory != 0);
			
			delete _factory;
			_factory = 0;
		}
	}
	
} //namespace mapgeneration_opt
