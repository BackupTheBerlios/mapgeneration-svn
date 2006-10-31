/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "randomnumbergenerators.h"

#include <cmath>

// -------------------------------------------------------------------------- //
// AsymmetricRandomNumberGenerator                                            //
// -------------------------------------------------------------------------- //
namespace mapgeneration_opt
{
	
	AsymmetricRandomNumberGenerator::AsymmetricRandomNumberGenerator(
		MTRand* external_generator)
	{
		if (external_generator == 0)
		{
			_mersenne_twister = new MTRand;
			_uses_external_generator = false;
		} else
		{
			_mersenne_twister = external_generator;
			_uses_external_generator = true;
		}
	}
	
	
	AsymmetricRandomNumberGenerator::~AsymmetricRandomNumberGenerator()
	{
		if (!_uses_external_generator)
			delete _mersenne_twister;
	}
	
	
	double
	AsymmetricRandomNumberGenerator::erf_inverse(double x)
	{
		return ( (1.0 / sqrt(2.0)) * gause_inverse((x+1.0)/2.0) );
	}
	
	
	double
	AsymmetricRandomNumberGenerator::freq(double x)
	{
		double v = fabs(x);
		double v_2 = v*v;
		double v_3 = v*v_2;
		
		double h;
		if (v < 0.5)
		{
			double h = (3.6767877*v - 0.097970465*v_3)
				/ (3.2584593 + v_2);
			
		} else if (v < 4.0)
		{
			double v_4 = v*v_3;
			
			double ap = 7.37389 + 6.86502*v + 3.0318*v_2 + 0.56317*v_3
				+ 0.0000431878*v_4;
			double aq = 7.37396 + 15.1849*v + 12.7955*v_2 + 5.35422*v_3
				+ v_4;
			double h = 1.0 - exp(-v_2) * (ap/aq);
			
		} else
		{
			double v_4 = v*v_3;
			double v_5 = v*v_4;
			
			double h = 1.0 - exp(-v_2)
				* (-0.096821 + 0.439821*v_2 + 0.248761*v_4)
				/ (v_3 + 0.440917*v_5);
		}
		
		if (x > 0.0)
		{
			return (0.5 * h + 0.5);
		} else
		{
			double hc = 1.0 - h;
			return 0.5 * hc;
		}
	}
	
	
	double
	AsymmetricRandomNumberGenerator::gause_inverse(double z)
	{
		if ( !(z>0 && z<1) )
			throw "Wrong z!";
		
		if (z == 0.5)
			return 0.0;
		
		double x = z;
		if (z > 0.5)
			x = 1.0 - z;
		
		x = sqrt(-2.0 * log(x));
		x = x - (1271.06 + 450.636*(x*x) + 7.45551*(x*x))
			/ (500.756 + 750.365*x + 110.421*(x*x) + (x*x*x));
		
		if (z < 0.5)
			x = -x;
		
		return ( x + 2.50662827 * (z - freq(x)) * exp(0.5 * (x*x)) );
	}
	
	
	double
	AsymmetricRandomNumberGenerator::rand(double step_size, double asymmetry,
		double gamma)
	{
		double G = _mersenne_twister->rand();
		
		if ( G < 1.0 / (1.0+pow(1.0+asymmetry, 0.5*gamma)) )
		{
			return ( sqrt(2.0*pow(step_size, gamma))
				* erf_inverse(pow(1.0+asymmetry, 0.5*gamma) * G + G - 1.0) );
			
		} else
		{
			double a = pow(1.0+asymmetry, -0.5*gamma);
			
			return ( sqrt(2.0*pow( (1.0+asymmetry)*step_size, gamma ))
				* erf_inverse(-a + a*G + G) );
		}
	}
	
	
	void
	AsymmetricRandomNumberGenerator::seed(unsigned long seed)
	{
		_mersenne_twister->seed(seed);
	}
	
} //namespace mapgeneration_opt


// -------------------------------------------------------------------------- //
// LogNormalRandomNumberGenerator                                            //
// -------------------------------------------------------------------------- //
namespace mapgeneration_opt
{
	
	LogNormalRandomNumberGenerator::LogNormalRandomNumberGenerator(
		MTRand* external_generator)
	{
		if (external_generator == 0)
		{
			_mersenne_twister = new MTRand;
			_uses_external_generator = false;
		} else
		{
			_mersenne_twister = external_generator;
			_uses_external_generator = true;
		}
	}
	
	
	LogNormalRandomNumberGenerator::~LogNormalRandomNumberGenerator()
	{
		if (!_uses_external_generator)
			delete _mersenne_twister;
	}
	
	
	double
	LogNormalRandomNumberGenerator::rand(double mean, double variance)
	{
		double normal_random_number
			= _mersenne_twister->randNorm(mean, variance);
		
		return exp(normal_random_number);
	}
	
	
	void
	LogNormalRandomNumberGenerator::seed(unsigned long seed)
	{
		_mersenne_twister->seed(seed);
	}
	
} //namespace mapgeneration_opt
