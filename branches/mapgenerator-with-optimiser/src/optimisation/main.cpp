/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include <iostream>

#include "mapgenerationevolutionaryalgorithm.h"

using namespace mapgeneration;
using namespace mapgeneration_opt;

int main()
{
	std::cout << "MapGenerationEvolutionaryAlgorithm: a test" << std::endl;
	
	EvolutionaryAlgorithm* ea
		= new MapGenerationEvolutionaryAlgorithm(20, 40);
	ea->run();
	
	for (int i = 0; i < ea->parents().size(); ++i)
	{
		Individual* ind = (ea->parents())[i];
		
		MapGenerationIndividual* saa_ind
			= dynamic_cast<MapGenerationIndividual*>(ind);
//		if (saa_ind != 0)
//			std::cout << *saa_ind << std::endl;
//		else
			std::cout << *ind << std::endl;
	}
	
	delete ea;
}
	
/*	
	RandomNumberFactory::register_user();
	int pos = 0;
	int less_than_one = 0;
	for (int i = 0; i < 10000000; ++i)
	{
		double rn = RandomNumberFactory::Ui(0, 9);
		
		if (rn > 4.1)
		{
			++pos;
			
			if (rn > 1.0)
				++less_than_one;
		}
		
	}
	
	std::cout << "pos = " << pos << ", less_than_one = " << less_than_one << std::endl;
	RandomNumberFactory::unregister_user();
*/
//}
