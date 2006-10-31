/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "filedbconnection.h"

#include <fstream>
#include <sstream>

//#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
//#include <unistd.h>

//#include <cc++/thread.h>

#include "util/mlog.h"


using namespace mapgeneration_util;


namespace mapgeneration
{
	
	FileDBConnection::FileDBConnection() 
	{
	}
	
	
	FileDBConnection::~FileDBConnection()
	{
	}
	
	
	void
	FileDBConnection::connect()
	{
	}
	
	
	void
	FileDBConnection::disconnect()
	{
	}
	
	
	void
	FileDBConnection::drop_tables()
	{
		std::vector<Table>::iterator tables_iter = _tables.begin();
		std::vector<Table>::iterator tables_iter_end = _tables.end();
		for (; tables_iter != tables_iter_end; ++tables_iter)
		{
			std::string directory_name = _db_directory;
			directory_name += "/";
			directory_name += tables_iter->_name;
			
			DIR *dir;
			struct dirent *direntp;
			dir = opendir (directory_name.c_str());
			if (dir == NULL)
				continue;

			readdir(dir); readdir(dir);
			while ((direntp = readdir(dir)) != NULL)
			{
				std::string file_name = directory_name;
				file_name += "/";
				file_name += direntp->d_name;
				unlink(file_name.c_str());
			}
			closedir (dir);
			
			rmdir(directory_name.c_str());
			
			// We also delete the db directory. Is this a good idea?
			rmdir(_db_directory.c_str());
		}
	}
	
	
	std::string
	FileDBConnection::filename(size_t table_id, unsigned int id)
	{
		std::stringstream filename_stream;
		filename_stream << _db_directory << "/" <<
			_tables[table_id]._directory << "/" << id;
		std::string filename = filename_stream.str();
		
		return filename;
	}
	
				
	std::vector<unsigned int>
	FileDBConnection::get_all_used_ids(size_t table_id)
	{
		std::vector<unsigned int> result;
				
		DIR *dir;
		struct dirent *direntp;
		
		std::string directory_name = _db_directory;
		directory_name += "/";
		directory_name += _tables[table_id]._name;
		dir = opendir (directory_name.c_str());
		if (dir == NULL)
			return result;

		readdir(dir); readdir(dir);
		while ((direntp = readdir(dir)) != NULL)
		{
			unsigned int id = atoi(direntp->d_name);
			result.push_back(id);
		}
		closedir (dir);
		
		return result;
	}
	
	
	std::vector<unsigned int>
	FileDBConnection::get_free_ids(size_t table_id)
	{
		std::vector<unsigned int> result;
		
		return result;
	}	
	
	
	unsigned int
	FileDBConnection::get_next_to_max_id(size_t table_id)
	{
		return 0;
	}
				
	
	std::string*
	FileDBConnection::load(size_t table_id, unsigned int id)
	{
		std::ifstream if_stream;		
		std::string fname = filename(table_id, id);
		
		if_stream.open(fname.c_str(), std::ios::in| std::ios::binary);
		
		if (!if_stream)
			return 0;
		
		std::string* loaded_string = new std::string();

		char* buffer = new char[4097];
		do {
			if_stream.read(buffer, 4096);
			loaded_string->append(buffer, if_stream.gcount());
		} while (if_stream);
		delete buffer;
				
		if_stream.close();
		
		return loaded_string;
	}
	
	
	size_t
	FileDBConnection::register_table(std::string name)
	{
		std::string directory_name = _db_directory;
		directory_name += "/";
		directory_name += name;
		mkdir(directory_name.c_str(), 0700);
		
		Table new_table;
		new_table._name = name;
		new_table._directory = name;
		
		_tables.push_back(new_table);
		
		return (_tables.size() - 1);
	}
	
	
	void
	FileDBConnection::remove(size_t table_id, unsigned int id)
	{
		std::string fname = filename(table_id, id);
		unlink(fname.c_str());
	}
	
	
	void 
	FileDBConnection::save(size_t table_id, unsigned int id, std::string& data)
	{
		std::ofstream of_stream;
		
		std::string fname = filename(table_id, id);
		
		of_stream.open(fname.c_str(), std::ios::in |
			std::ios::binary | std::ios::trunc);		
		of_stream << data;
		of_stream.close();
	}
	
	
	void
	FileDBConnection::set_parameters(std::string db_directory)
	{
		_db_directory = db_directory;
		
		if (_db_directory[_db_directory.length() - 1] == '/')
			_db_directory.erase(_db_directory.end());
		
		std::string directory_name = _db_directory;
		mkdir(directory_name.c_str(), 0700);
	}
			
			
} // namespace mapgeneration
