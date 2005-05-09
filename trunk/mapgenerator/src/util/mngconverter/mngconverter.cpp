/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include <string.h>
#include <iostream>

#include "mngconverter.h"

namespace mapgeneration_util
{
	
	MapAndGuideConverter::MapAndGuideConverter()
	: _input_file(), _output_file()
	{
	}
	
	
	void
	MapAndGuideConverter::run_converter()
	{
		if (_input_file == false || _output_file == false)
		{
			std::cout << "ERROR!" << std::endl;
			throw std::string("Something is wrong with the streams!");
		}
		
		const int NEXT_IS_GGA = 0;
		const int NEXT_IS_RMC = 1;
		
		int status = NEXT_IS_RMC;
		
		while ( !_input_file.eof() )
		{
			char buffer[1000];
			buffer[0] = '\0';
			
			_input_file.getline(buffer, 1000);
			
			if (status == NEXT_IS_GGA)
			{
				char* index_pointer = strstr(buffer, "GPGGA");
				if (index_pointer != 0)
				{
					size_t length_of_buffer = strlen(buffer);
					size_t index = (index_pointer - buffer) / sizeof(char);
					
					_output_file.put('$');
					_output_file.write(index_pointer, length_of_buffer - index);
					_output_file.put('\n');
					
					status = NEXT_IS_RMC;
				}
				
			} else if (status == NEXT_IS_RMC)
			{
				char* index_pointer = strstr(buffer, "GPRMC");
				if (index_pointer != 0)
				{
					size_t length_of_buffer = strlen(buffer);
					size_t index = (index_pointer - buffer) / sizeof(char);
					
					_output_file.put('$');
					_output_file.write(index_pointer, length_of_buffer - index);
					_output_file.put('\n');
					
					status = NEXT_IS_GGA;
				}
			}
		}
		_output_file.flush();
		
		_input_file.close();
		_output_file.close();
	}
	
	
	void
	MapAndGuideConverter::set_input_file(const std::string& input_file)
	{
		_input_file.open(input_file.c_str());
	}
	
	
	void
	MapAndGuideConverter::set_output_file(const std::string& output_file)
	{
		_output_file.open(output_file.c_str());
	}
	
}
