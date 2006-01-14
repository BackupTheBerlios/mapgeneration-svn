/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef PRT_CONVERTER_H
#define PRT_CONVERTER_H

#include <sql.h>
#include <string>

namespace mapgeneration_util
{
	
	class PeterRoosenTracksConverter
	{
		
		public:
			
			PeterRoosenTracksConverter();
			
			
			bool
			connect_db();
			
			
			void
			disconnect_db();
			
			
			void
			run_converter(bool status);
			
			
			void
			set_db_attributes(const std::string& dns, const std::string& user,
				const std::string& password);
			
			
			void
			set_output_file(const std::string& output_file,
				int max_size = 10000000, bool append = false);
			
			
		private:
			
			SQLHDBC _connection;

			SQLHENV _environment;

			std::string _dns;

			std::string _user;

			std::string _password;
			
			std::string _output_file;
			
			bool _output_file_append;
			
			int _max_output_file_size;
			
			std::string
			decimal_to_nmea_coordinate(const double& decimal,
				int digits_left_to_decimal_point);
			
			void
			new_file(std::ofstream& file, int& file_number);
			
			
			void
			write_to_file(std::ofstream& output_file, const time_t& time,
				const double& x, const double& y);
	};
	
}

#endif //PRT_CONVERTER_H
