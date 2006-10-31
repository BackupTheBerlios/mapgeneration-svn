/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef DBCONNECTION_H
#define DBCONNECTION_H


#include <vector>


namespace mapgeneration
{

	/**
	 * @brief Inits database, creates tables (if necessary) and provides method
	 * to access it.
	 * 
	 * @todo save_filteredtrace schreiben!
	 * 
	 */
	class DBConnection
	{

		public:
	
			/**
			 * @brief Empty constructor. Allocated handles.
			 */
			DBConnection () {};
		

			/**
			 * @brief Destructor. Frees handles.
			 */
			~DBConnection() {};


			/**
			 * @brief Connects to database.
			 * 
			 * Calls check_db_structure and if correct_structure is set true it tries
			 * to correct the DB structure by calling coorect_db_structure.
			 * 
			 * @param dns name of database
			 * @param user user name
			 * @param password password
			 * @param correct_structure flag (default: false)
			 * @see check_db_structure()
			 * @see correct_db_structure()
			 */
			virtual void
			connect() = 0;
			
			
			/**
			 * @brief Destroy database handles.
			 * 
			 * Needed for restarting.
			 */		
//			void
//			destroy();


			/**
			 * @brief Disconnects from database.
			 */
			virtual void
			disconnect() = 0;
			
			
			/**
			 * @brief Drops all registered tables.
			 */
			virtual void
			drop_tables() = 0;
			
						
			/**
			 * @brief Returns a vector containing all used IDs from the specified
			 * table.
			 * 
			 * @return the vector of unsigned int
			 */
			virtual std::vector<unsigned int>
			get_all_used_ids(size_t table_id) = 0;


			/**
			 * @brief Returns a vector containing the free IDs in between plus
			 * the first unused ID form the specified table.
			 * 
			 * @return the vector of unsigned int.
			 */
			virtual std::vector<unsigned int>
			get_free_ids(size_t table_id) = 0;


			/**
			 * @brief Returns the ID next to max id
			 * 
			 * @return next to max id
			 */
			virtual unsigned int
			get_next_to_max_id(size_t table_id) = 0;
			
			
			/**
			 * @brief Inits database handles.
			 */
//			void 
//			init();
			
			
			/**
			 * @brief Loads a BLOB from DB into a string.
			 * 
			 * @param table the table from which will be loaded
			 * @param id the id
			 * @return string representation of BLOB
			 */
			virtual std::string*
			load(size_t table_id, unsigned int id) = 0;
			
			
			/**
			 * @brief Registers a new table.
			 */
			virtual size_t
			register_table(std::string name) = 0;
			
			
			/**
			 * @brief Removes an entry from the DB.
			 * 
			 * @param from_table the table where the entry is located
			 * @param id the id
			 */
			virtual void
			remove(size_t table_id, unsigned int id) = 0;
		
			
			/**
			 * @brief Save a string as BLOB in DB.
			 * 
			 * @param table the table in which will be saved
			 * @param id the id
			 * @param data the string
			 */
			virtual void 
			save(size_t table_id, unsigned int id, std::string& data) = 0;

	};

}

#endif //DBCONNECTION_H
