/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef FILEDBCONNECTION_H
#define FILEDBCONNECTION_H


#include <string>
#include <vector>

#include "dbconnection.h"


namespace mapgeneration
{

	/**
	 * @brief Inits database, creates tables (if necessary) and provides method
	 * to access it.
	 * 
	 * @todo save_filteredtrace schreiben!
	 * 
	 */
	class FileDBConnection : public DBConnection
	{

		public:
	
			/**
			 * @brief Empty constructor. Allocated handles.
			 */
			FileDBConnection ();
		

			/**
			 * @brief Destructor. Frees handles.
			 */
			~FileDBConnection();


			/**
			 * @brief Connects to database.
			 */
			void
			connect();
			

			/**
			 * @brief Disconnects from database.
			 */
			void
			disconnect();
			
			
			#ifdef DEBUG
				/**
				 * @brief Drops all registered tables.
				 */
				void
				drop_tables();
			#endif
			
						
			/**
			 * @brief Returns a vector containing all used IDs from the specified
			 * table.
			 * 
			 * @return the vector of unsigned int
			 */
			std::vector<unsigned int>
			get_all_used_ids(size_t table_id);


			/**
			 * @brief Returns a vector containing the free IDs in between plus
			 * the first unused ID form the specified table.
			 * 
			 * @return the vector of unsigned int.
			 */
			std::vector<unsigned int>
			get_free_ids(size_t table_id);


			/**
			 * @brief Returns the ID next to max id
			 * 
			 * @return next to max id
			 */
			unsigned int
			get_next_to_max_id(size_t table_id);
						
			
			/**
			 * @brief Loads a BLOB from DB into a string.
			 * 
			 * @param table the table from which will be loaded
			 * @param id the id
			 * @return string representation of BLOB
			 */
			std::string*
			load(size_t table_id, unsigned int id);
			
			
			/**
			 * @brief Registers a new table.
			 */
			size_t
			register_table(std::string name);
			
			
			/**
			 * @brief Removes an entry from the DB.
			 * 
			 * @param from_table the table where the entry is located
			 * @param id the id
			 */
			void
			remove(size_t table_id, unsigned int id);
		
			
			/**
			 * @brief Save a string as BLOB in DB.
			 * 
			 * @param table the table in which will be saved
			 * @param id the id
			 * @param data the string
			 */
			void 
			save(size_t table_id, unsigned int id, std::string& data);			
			
			
			void
			set_parameters(std::string db_directory);
		
			
		private:
		
			class Table
			{
				public:
				
					std::string _name;
					std::string _directory;
			};
			
		
			std::string _db_directory;
			
			
			std::vector<Table> _tables;
			
			
			std::string
			filename(size_t table_id, unsigned int id);
			
	};

}

#endif //FILEDBCONNECTION_H
