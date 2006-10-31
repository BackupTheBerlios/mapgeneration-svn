/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include <iostream>

#include "mapgenerationevolutionaryalgorithm.h"
#include "mapgenerationindividual.h"
#include "filteredtrace.h"

using namespace mapgeneration;
using namespace mapgeneration_opt;

int main(int argc, char* argv[])
{
	std::cout << "Extractor" << std::endl;
	if (argc < 5)
	{
		std::cout << "Usage: " << argv[0]
			<< " Inputfile Outputfile Modus Count" << std::endl;
		
		return 1;
	}
	
	const std::string MODUS_APPEND = "A";
	const std::string MODUS_NEW = "N";
	
	// Read argument.
	std::string inputfile_string(argv[1]);
	std::string outputfile_string(argv[2]);
	std::string modus(argv[3]);
	int count = atoi(argv[4]);
	if (count < 1)
		count = 1;
	if (count > 10)
		count = 10;
	
	std::ifstream inputfile(inputfile_string.c_str());
	if (!inputfile)
	{
		std::cout << "invalid inputfile" << std::endl;
		return 2;
	}
		
	std::ofstream outputfile;
	if (modus == MODUS_APPEND)
		outputfile.open(outputfile_string.c_str(), std::ios::out | std::ios::app);
	else
		outputfile.open(outputfile_string.c_str());
	
	if (!outputfile)
	{
		std::cout << "invalid outputfile" << std::endl;
		return 3;
	}
	
	// Deserialize not-used-parts.
	int int_dummy;
	double double_dummy;
	RandomNumberFactory::load(inputfile);
	Serializer::deserialize(inputfile, int_dummy);
	Serializer::deserialize(inputfile, double_dummy);
	Serializer::deserialize(inputfile, int_dummy);
	
	// Deserialize fitness map.
	std::multimap< double, MapGenerationIndividual, std::greater<double> >
		fitness_individual_map;
	Serializer::deserialize(inputfile, fitness_individual_map);
	
	// Copy the best "count" individuals.
	std::vector< std::vector<double> > individual_object_components;
	
	std::map< double, MapGenerationIndividual, std::greater<double> >::iterator
		iter = fitness_individual_map.begin();
	std::map< double, MapGenerationIndividual, std::greater<double> >::iterator
		iter_end = fitness_individual_map.end();
	int index = 0;
	while ( (index < count) && (iter != iter_end) )
	{
		individual_object_components.push_back(iter->second.object_components());
		
		++index;
		++iter;
	}

	
	
/*	std::vector< std::vector<double> > individual_object_components;
	double individuals[1][25] = {
//		{68.5957, 42.7244, 3.48581, 0, 0, 16.9664, 155.093, 49.208, 33.2389, 32.9913, 149.078, 74.543, 59.2035, 1.69646, 8.50558, 143.366, 10.7197, 12.4299, 73.7539, 65.8542, 42.5241, 155.929, 98.4801, 18.1568, 42.3584}//,
//		{68.5957, 42.7244, 3.48581, 0, 0, 16.9664, 155.093, 49.208, 33.2389, 132.9913, 149.078, 174.543, 159.2035, 1.69646, 8.50558, 143.366, 10.7197, 12.4299, 173.7539, 265.8542, 242.5241, 255.929, 98.4801, 18.1568, 142.3584}//,
		{27.8242,    94.2866,    98.6268,    8.85016,    94.1155,    32.6997,    83.1554,    28.9517,    44.2598,    4.04455,    13.0475,    13.6779,          0,   64.1896,    17.9063,    33.8037,     40.128,    20.3434,    90.5401,    76.2852,    29.6455,    1.96758,    33.9206,     82.329,    8.35487}
//		{3.96896,    150.131,    26.1947,    2.19925,    14.8819,    22.8314,    60.6984,    49.4849,    29.3875,    24.1881,    54.2064,    91.3822,    63.6602,    2.93774,    9.84125,    160.481,    4.78616,    2.43644,    73.7553,    165.914,    166.275,    64.2846,    102.625,    15.0619,     43.234},
//		{0.992823,    75.2857,    34.4227,    5.48386,          0,     25.328,    157.095,    18.8861,    29.5466,     95.549,    152.691,    67.0762,    57.1105,    3.43805,    5.80646,    152.513,    6.98879,    9.76734,    163.048,    162.141,    73.3189,    158.228,    96.7708,    15.2872,    41.0776},
//		{7.22482,    42.2788,    18.7699,    6.32424,          0,    25.3086,    70.8544,    48.4973,    29.0689,    95.8177,     160.24,    160.465,     59.689,   0.819208,    10.1897,    157.029,    6.93193,    2.15125,    75.7963,    71.2481,    60.6413,    166.831,    104.165,    15.7893,    36.1181},
//		{97.0704,    36.5003,    99.7842,    5.01459,          0,    18.7744,    160.098,    44.3202,    42.9525,    23.4893,    153.732,     48.238,    57.8718,          0,   0.745498,    160.318,    3.68146,    12.1972,    156.592,    12.3223,    165.066,    155.754,    91.3953,    15.7809,    39.5929},
//		{4.74501,    148.845,    29.5599,    4.85858,          0,    21.2754,    160.418,    19.1771,    48.7558,    24.2198,     153.93,    77.8226,    64.6569,   0.645786,    6.43761,    161.572,    14.2846,    1.83523,    71.9539,    166.222,    87.7962,    150.135,    47.9147,    20.5676,     40.933},
//		{1.42085,     70.695,    105.116,    9.72403,    1.33904,    25.8266,    158.271,    19.7027,    38.1128,    21.4559,    24.5928,    78.1334,    53.4167,     1.7262,    11.1171,    160.781,    5.54835,    10.2021,    159.721,    160.062,    164.886,    63.6056,    99.2954,    15.0035,    42.6462},
//		{5.1585,    148.567,     38.432,    3.79475,     0.4221,    17.6254,    58.9495,    41.7807,    47.3195,    32.2543,    161.164,    75.9041,    161.406,          0,    6.69195,    161.631,    78.8498,    1.76203,    161.256,    160.675,    165.725,    22.5496,    25.9422,    11.3343,    159.408},
//		{1,        150,        100,        3.5,        0.1,         20,        160,         20,         40,        160,        160,        160,        160,        0.5,          5,        160,          5,        7.5,        160,        160,        160,        160,         20,        160,        160},
//		{0,    151.054,    35.3092,    26.8779,          0,    19.9826,    71.0152,    50.8481,     29.439,    21.6979,    153.508,    57.9603,    63.6669,          0,    5.83828,    161.381,    12.0972,    3.04232,     74.026,    161.889,    163.157,    153.241,    87.1905,    164.496,    44.7923}
	};
	for (int i = 0; i < 1; ++i)
	{
		std::vector<double> ind(25);
		for (int j = 0; j < 25; ++j)
		{
			ind[j] = individuals[i][j];
		}
		
		individual_object_components.push_back(ind);
	}
*/	
	// Write the best individuals to the outputfile.
	Serializer::serialize(outputfile, individual_object_components);
	
	// Ready.
	std::cout << "ready" << std::endl;
	return 0;
}
