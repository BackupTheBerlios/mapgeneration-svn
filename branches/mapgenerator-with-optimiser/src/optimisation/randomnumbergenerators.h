/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef RANDOMNUMBERGENERATOR_H
#define RANDOMNUMBERGENERATOR_H

#include "mersennetwister/MersenneTwister.h"

namespace mapgeneration_opt
{
	
	class AsymmetricRandomNumberGenerator
	{
		
		public:
			
			AsymmetricRandomNumberGenerator(MTRand* external_generator = 0);
			
			
			~AsymmetricRandomNumberGenerator();
			
			
			double
			rand(double step_size = 1.0, double asymmetry = 0.0,
				double gamma = 2.0);
			
			
			void
			seed(unsigned long seed);
			
			
		private:
			
			MTRand*
			_mersenne_twister;
			
			
			bool
			_uses_external_generator;
			
			
			double
			erf_inverse(double x);
			
			
			double
			freq(double x);
			
			
			double
			gause_inverse(double z);
			
	};
	
	
	class LogNormalRandomNumberGenerator
	{
		
		public:
			
			LogNormalRandomNumberGenerator(MTRand* external_generator = 0);
			
			
			~LogNormalRandomNumberGenerator();
			
			
			double
			rand(double mean = 0.0, double variance = 1.0);
			
			
			void
			seed(unsigned long seed);
			
		private:
			
			MTRand*
			_mersenne_twister;
			
			
			bool
			_uses_external_generator;
			
	};
	
} //namespace mapgeneration_opt

#endif //RANDOMNUMBERGENERATOR_H
