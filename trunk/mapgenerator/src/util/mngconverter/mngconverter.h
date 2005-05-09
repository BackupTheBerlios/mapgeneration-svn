/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef MNG_CONVERTER_H
#define MNG_CONVERTER_H

#include <fstream>

namespace mapgeneration_util
{
	
	class MapAndGuideConverter
	{
		
		public:
			
			MapAndGuideConverter();
			
			
			void
			run_converter();
			
			
			void
			set_input_file(const std::string& input_file);
			
			
			void
			set_output_file(const std::string& output_file);
			
			
		private:
			
			std::ifstream _input_file;
			
			std::ofstream _output_file;
	};
	
}

#endif //MNG_CONVERTER_H
