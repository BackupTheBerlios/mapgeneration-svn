/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef MAPGENERATIONINDIVIDUAL_H
#define MAPGENERATIONINDIVIDUAL_H

namespace mapgeneration
{
	class FilteredTrace;
}

using mapgeneration::FilteredTrace;

#include "asymmetricadaptiveindividual.h"

namespace mapgeneration_opt
{
	
	const int MAP_GENERATION_INDIVIDUAL_DIMENSION = 25;
	
	
	class MapGenerationIndividual
		: public AsymmetricAdaptiveIndividual<MAP_GENERATION_INDIVIDUAL_DIMENSION>
	{
		
		public:
			
			MapGenerationIndividual();
			
			
			MapGenerationIndividual(
				const std::vector<FilteredTrace>& test_pool);
			
			
			virtual void
			deserialize(std::istream& i_stream);
			
			
			unsigned int
			get_age() const;
			
			
			const std::vector<FilteredTrace>&
			get_test_pool() const;
			
			
			void
			increment_age();
			
			
			virtual MapGenerationIndividual&
			operator=(const Individual& individual);
			
			
			void
			reset_age();
			
			
			virtual void
			serialize(std::ostream& o_stream) const;
			
			
			void
			set_test_pool(const std::vector<FilteredTrace>& test_pool);
			
			
		protected:
			
			virtual double
			impl_calculate_fitness() const;
			
			
			virtual void
			impl_mutate();
			
			
		private:
			
			
			unsigned int _age;
			
			mutable unsigned int _total_number_of_crossings;
			mutable unsigned int _total_number_of_nodes;
			mutable double _total_surface_area;
//			int _id, _adder;
//			mutable int _file_counter;
			
			double _a, _b;
			
			mutable std::vector<FilteredTrace> _test_pool;
	};
	
} // namespace mapgeneration_opt

#endif //MAPGENERATIONINDIVIDUAL_H
