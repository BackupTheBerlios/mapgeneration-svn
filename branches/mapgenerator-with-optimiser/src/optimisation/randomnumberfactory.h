/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef RANDOMNUMBERFACTORY_H
#define RANDOMNUMBERFACTORY_H

#include <cassert>
#include "randomnumbergenerators.h"

#include "util/serializer.h"
using mapgeneration_util::Serializer;

namespace mapgeneration_opt
{
	
	class RandomNumberFactory
	{
		
		public:
			
			static bool _simulation_mode;
			
			
			static double
			asymmetric_random_number(double step_size = 1.0,
				double asymmetry = 0.0, double gamma = 2.0);
			
			
			static double
			lognormal_random_number(double mean = 0.0, double variance = 1.0);
			
			
			static double
			normal_random_number(double mean = 0.0, double variance = 1.0);
			
			
			static unsigned long
			uniform_integer_random_number(unsigned long start_incl = 0,
				unsigned long end_incl = 1);
			
			
			static double
			uniform_real_random_number(double start_incl = 0.0,
				double end_incl = 1.0);
			
			
			static inline double
			A(double asymmetry = 0.0, double sigma = 1.0)
			{
				return asymmetric_random_number(2.0, sigma, asymmetry);
			}
			
			
			static inline double
			LN(double a = 0.0, double sigma = 1.0)
			{
				return lognormal_random_number(a, sigma);
			}
			
			
			static inline double
			N(double a = 0.0, double sigma = 1.0)
			{
				return normal_random_number(a, sigma);
			}
			
			
			static inline unsigned long
			Ui(unsigned long start_incl = 0, unsigned long end_incl = 1)
			{
				return uniform_integer_random_number(start_incl, end_incl);
			}
			
			
			static inline double
			Ur(double start_incl = 0.0, double end_incl = 1.0)
			{
				return uniform_real_random_number(start_incl, end_incl);
			}
			
			
			static void
			register_user();
			
			
			static void
			unregister_user();
			
			
			static inline bool
			load(std::istream& i_stream)
			{
				bool flag;
				Serializer::deserialize(i_stream, flag);
				
				if (flag)
				{
					if (_users > 0)
					{
						assert(_factory != 0);
						delete _factory;
						_factory = new RandomNumberFactory;
						
						Serializer::deserialize(i_stream, *_factory);
						
						return true;
						
					} else
					{
						RandomNumberFactory dummy;
						Serializer::deserialize(i_stream, dummy);
					}
				}
				
				return false;
			}
			
			
			static inline void
			save(std::ostream& o_stream)
			{
				if (_factory == 0)
				{
					bool flag = false;
					Serializer::serialize(o_stream, flag);
				} else
				{
					bool flag = true;
					Serializer::serialize(o_stream, flag);
					Serializer::serialize(o_stream, *_factory);
				}
			}
			
			
			inline void
			deserialize(std::istream& i_stream)
			{
				MTRand::uint32 load_array[MTRand::SAVE];
				for (int i = 0; i < MTRand::SAVE; ++i)
					Serializer::deserialize(i_stream, load_array[i]);
				
				this->_mersenne_twister->load(load_array);
			}
			
			
			inline void
			serialize(std::ostream& o_stream) const
			{
				MTRand::uint32 save_array[MTRand::SAVE];
				this->_mersenne_twister->save(save_array);
				
				for (int i = 0; i < MTRand::SAVE; ++i)
					Serializer::serialize(o_stream, save_array[i]);
			}
			
			
		private:
			
			static RandomNumberFactory*
			_factory;
			
			
			static int
			_users;
			
			
			AsymmetricRandomNumberGenerator*
			_asymmetric_rng;
			
			
			LogNormalRandomNumberGenerator*
			_lognormal_rng;
			
			
			MTRand*
			_mersenne_twister;
			
			
			RandomNumberFactory();
			
			
			~RandomNumberFactory();
			
	};
	
} //namespace mapgeneration_opt

#endif //RANDOMNUMBERFACTORY_H
