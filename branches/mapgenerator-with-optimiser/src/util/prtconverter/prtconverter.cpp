/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "prtconverter.h"

#include <cmath>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sqlext.h>
#include <sstream>

namespace mapgeneration_util
{
	
	PeterRoosenTracksConverter::PeterRoosenTracksConverter()
	: _connection(), _environment(),
		_dns("PeterRoosenTracks"), _user("mapgeneration"), _password("mg"),
		_output_file("peter_roosen_tracks"), _output_file_append(false),
		_max_output_file_size(10000000)
	{
	}
	
	
	bool
	PeterRoosenTracksConverter::connect_db()
	{
		SQLRETURN sql_return;
		sql_return = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_environment);
		if (sql_return != SQL_SUCCESS)
		{
			std::cout << "Allocating handle for environment failed. Exiting.\n";
			return false;
		}

		sql_return = SQLSetEnvAttr(_environment, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
		if (sql_return != SQL_SUCCESS)
		{
			std::cout << "Setting environment attributes failed. Exiting.\n";
			return false;
		}

		sql_return = SQLAllocHandle(SQL_HANDLE_DBC, _environment, &_connection);
		if (sql_return != SQL_SUCCESS)
		{
			std::cout << "Allocating handle for connection failed. Exiting.\n";
			return false;
		}
		
		//inited. Now connect:
		SQLCHAR* sql_dns = (SQLCHAR*)_dns.c_str();
		SQLCHAR* sql_user = (SQLCHAR*)_user.c_str();
		SQLCHAR* sql_password = (SQLCHAR*)_password.c_str();
		
		sql_return = SQLConnect(_connection, sql_dns, SQL_NTS,
			sql_user, SQL_NTS, sql_password, SQL_NTS);
		if (sql_return != SQL_SUCCESS)
		{
			std::cout << "Connecting to DB failed. Exiting.\n";
			return false;
		}
		
		std::cout << "Connect to DB: OKAY." << std::endl;
		return true;
	}
	
	void
	PeterRoosenTracksConverter::disconnect_db()
	{
		SQLRETURN sql_return;
		
		sql_return = SQLDisconnect(_connection);
		if (sql_return != SQL_SUCCESS)
		{
			std::cout << "Disconnecting from DB failed." << std::endl;
		}
	
		sql_return = SQLFreeHandle(SQL_HANDLE_DBC, _connection);
		if (sql_return != SQL_SUCCESS)
		{
			std::cout << "Freeing connection handle failed." << std::endl;
		}
	
		sql_return = SQLFreeHandle(SQL_HANDLE_ENV, _environment);
		if (sql_return != SQL_SUCCESS)
		{
			std::cout << "Freeing environment handle failed." << std::endl;
		}
		
		std::cout << "Disconnect from DB: OKAY." << std::endl;
	}

	
	void
	PeterRoosenTracksConverter::run_converter(bool status)
	{
		SQLRETURN sql_return;
		SQLHSTMT hstmt;
		
		sql_return = SQLAllocHandle(SQL_HANDLE_STMT, _connection, &hstmt);
		if (sql_return != SQL_SUCCESS)
		{
			std::cout << "Allocating statement handle failed." << std::endl;
		}
		
		double x1, y1, x2, y2;
		SQLINTEGER x1_ind, y1_ind, x2_ind, y2_ind;
		
		sql_return = SQLBindCol(hstmt, 1, SQL_C_DOUBLE, &x1, sizeof(double), &x1_ind);
		if (sql_return != SQL_SUCCESS)
		{
			std::cout << "Binding to column 1 failed." << std::endl;
		}
		sql_return = SQLBindCol(hstmt, 2, SQL_C_DOUBLE, &y1, sizeof(double), &y1_ind);
		if (sql_return != SQL_SUCCESS)
		{
			std::cout << "Binding to column 2 failed." << std::endl;
		}
		sql_return = SQLBindCol(hstmt, 3, SQL_C_DOUBLE, &x2, sizeof(double), &x2_ind);
		if (sql_return != SQL_SUCCESS)
		{
			std::cout << "Binding to column 3 failed." << std::endl;
		}
		sql_return = SQLBindCol(hstmt, 4, SQL_C_DOUBLE, &y2, sizeof(double), &y2_ind);
		if (sql_return != SQL_SUCCESS)
		{
			std::cout << "Binding to column 4 failed." << std::endl;
		}
		
		std::string stmt_string("SELECT x1, y1, x2, y2 FROM tracks WHERE id < 100000 ORDER BY id ASC;");
		sql_return = SQLExecDirect(hstmt, (SQLCHAR*)(stmt_string.c_str()), SQL_NTS);
		if (sql_return != SQL_SUCCESS)
		{
			std::cout << "Executing SQL statement failed." << std::endl;
		}
		
		// init positions:
		double old_x1 = -1.0;
		double old_x2 = -1.0;
		double old_y1 = -1.0;
		double old_y2 = -1.0;
		
		enum CoordOrder
		{
			COORD_ORDER_1TO2,
			COORD_ORDER_2TO1,
			UNKNOWN
		};
		
		CoordOrder coord_order = UNKNOWN;
		// done.
		
		// init time (to 2000-01-01 00:00:00):
		tm* current_time = new tm;
		current_time->tm_sec = 0;
		current_time->tm_min = 0;
		current_time->tm_hour = 0;
		current_time->tm_mday = 1;
		current_time->tm_mon = 0;
		current_time->tm_year = 100;
		
		time_t count = mktime(current_time);
		// done.
		
		// init and open output file:
		std::ofstream output_file;
		output_file.setf(std::ios::fixed);
		output_file.precision(4);
		
		std::string init_filename(_output_file);
		init_filename.append("0");
		output_file.open(init_filename.c_str());
		
		int file_counter = 1;
		// done.
		
		// init step size:
		const int STEP_SIZE = 10;
		// done.
		
		// loop over result set:
		while ( (sql_return = SQLFetch(hstmt)) != SQL_NO_DATA)
		{
			if ( (status) && (file_counter % 10000 == 0) )
				std::cout << file_counter << std::endl;
			
			if (sql_return == SQL_ERROR)
				std::cout << "Fetching column " << count + 1
					<< " failed." << std::endl;
			
/*			if (output_file.tellp() >= _max_output_file_size)
			{
				output_file.close();
				
				std::stringstream number;
				number << file_counter;
				++file_counter;
				
				std::string filename(_output_file);
				filename.append(number.str());
				output_file.open(filename.c_str());
				
				coord_order = UNKNOWN;
			}*/
			
			if (coord_order == UNKNOWN)
			{
				if ( (old_x2 == x1) && (old_y2 == y1) )
				{
					coord_order = COORD_ORDER_1TO2;
					
					new_file(output_file, file_counter);
					
					write_to_file(output_file, count, x1, y1);
					count += STEP_SIZE;
				} else if ( (old_x1 == x2) && (old_y1 == y2) )
				{
					coord_order = COORD_ORDER_2TO1;
					
					new_file(output_file, file_counter);
					
					write_to_file(output_file, count, x2, y2);
					count += STEP_SIZE;
				} else
				{
					// Coordinates do not belong together. That would result
					// in a 2 point track. Just forget it!
					
					// set coord_order to UNKNOWN. Actually not needed. But
					// you known where to continue immediately.
					coord_order = UNKNOWN;
				}
			}
			
			if (coord_order == COORD_ORDER_1TO2)
			{
				if ( (old_x2 == x1) && (old_y2 == y1) )
				{
					write_to_file(output_file, count, x2, y2);
					count += STEP_SIZE;
				} else if ( (old_x2 == x2) && (old_y2 == y2) )
				{
					// coord_order changed:
					output_file << "\t/* Coordinate order changed. ";
					output_file << "Now it is from 2 to 1. */";
					output_file << std::endl;
					coord_order = COORD_ORDER_2TO1;
					
					write_to_file(output_file, count, x1, y1);
					count += STEP_SIZE;
				} else
				{
					coord_order = UNKNOWN;
				}
			} else // if (coord_order == COORD_ORDER_2TO1)
			{
				if ( (old_x1 == x2) && (old_y1 == y2) )
				{
					write_to_file(output_file, count, x1, y1);
					count += STEP_SIZE;
				} else if ( (old_x1 == x1) && (old_y1 == y1) )
				{
					// coord_order changed:
					output_file << "\t/* Coordinaten order changed. ";
					output_file << "Now it is from 1 to 2. */";
					output_file << std::endl;
					coord_order = COORD_ORDER_1TO2;
					
					write_to_file(output_file, count, x2, y2);
					count += STEP_SIZE;
				} else
				{
					coord_order = UNKNOWN;
				}
			}
			
			old_x1 = x1;
			old_y1 = y1;
			old_x2 = x2;
			old_y2 = y2;
		}
		
		output_file.close();
		
		delete current_time;
	}
	
	
	void
	PeterRoosenTracksConverter::set_db_attributes(const std::string& dns,
		const std::string& user, const std::string& password)
	{
		_dns = dns;
		_user = user;
		_password = password;
	}
	
	
	void
	PeterRoosenTracksConverter::set_output_file(
		const std::string& output_file, int max_size, bool append)
	{
		_output_file = output_file;
		_output_file_append = append;
		_max_output_file_size = max_size;
	}
	
	
	std::string
	PeterRoosenTracksConverter::decimal_to_nmea_coordinate(
		const double& decimal, int digits_left_of_decimal_point)
	{
		double int_part = floor(decimal);
		int_part *= 100.0;
		
		double remainder = decimal - floor(decimal);
		remainder *= 60.0;
		
		double nmea = int_part + remainder;
		
		std::stringstream sstream;
		sstream << nmea;
		std::string nmea_string(sstream.str());
		
		std::string::size_type index = nmea_string.find('.', 0);
		if (index == std::string::npos)
			nmea_string.append(".0");
			
		index = nmea_string.find('.', 0);
		if (index < digits_left_of_decimal_point)
			nmea_string.insert(0, digits_left_of_decimal_point - index, '0');
		else if (index > digits_left_of_decimal_point)
			nmea_string.erase(0, index - digits_left_of_decimal_point);
		
//		std::cout << decimal << " " << int_part << " " << remainder << " " << nmea << " '" << nmea_string << "'\n";
		return nmea_string;
	}
	
	void
	PeterRoosenTracksConverter::new_file(std::ofstream& file, int& file_number)
	{
		file.close();
		
		std::stringstream number;
		number << file_number;
		++file_number;
		
		std::string filename(_output_file);
		filename.append(number.str());
		file.open(filename.c_str());
		
		file << std::endl;
		file << "\t/* New track information. */";
		file << std::endl;
		file << "\t/* Initial coordinate order: 1 to 2. */";
		file << std::endl;
	}
	
	
	void
	PeterRoosenTracksConverter::write_to_file(std::ofstream& output_file,
		const time_t& time, const double& x, const double& y)
	{
		tm* current_time = localtime(&time);
		
		output_file << "$GPRMC,";
		output_file << std::setw(2) << std::setfill('0') << current_time->tm_hour;
		output_file << std::setw(2) << std::setfill('0') << current_time->tm_min;
		output_file << std::setw(2) << std::setfill('0') << current_time->tm_sec;
		
		output_file << ",A,";
		output_file << decimal_to_nmea_coordinate(y, 4);
		output_file << (y < 0.0 ? ",S," : ",N,");
		output_file << decimal_to_nmea_coordinate(x, 5);
		output_file << (x < 0.0 ? ",W," : ",E,");
		
		output_file << ",,";
		output_file << std::setw(2) << std::setfill('0') << current_time->tm_mday;
		output_file << std::setw(2) << std::setfill('0') << current_time->tm_mon + 1;
		output_file << std::setw(2) << std::setfill('0') << current_time->tm_year % 100;
		output_file << ",," << std::endl;
	}
	
}
