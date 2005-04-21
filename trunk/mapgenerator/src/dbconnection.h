/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef DBCONNECTION_H
#define DBCONNECTION_H

#ifdef CYGWIN
	#include <windows.h>
#endif

#include <sql.h>
#include <string>
#include <vector>

#include "util/mlog.h"

#define MAX_SQL_CHAR_LENGTH (255)
#define SQL_BINARY_BUFFER (8192)

using std::string;
using std::vector;
using mapgeneration_util::MLog;

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
		
		
			class Table
			{
				public:
				
					std::string _name;
					
					/**
					 * @brief Prepared SQL statement handles.
					 */
					std::vector<SQLHSTMT> _prepared_statements;
			};

	
			/**
			 * @brief Empty constructor. Allocated handles.
			 */
			DBConnection ();
		

			/**
			 * @brief Destructor. Frees handles.
			 */
			~DBConnection();


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
			void
			connect(string dns, string user, string password, bool correct_structure = false);
			
			
			/**
			 * @brief Deletes entry from DB.
			 * 
			 * @param from_table the table where the entry is located
			 * @param id the id
			 */
			void
			remove(size_t table_id, unsigned int id);
			
			 

			/**
			 * @brief Destroy database handles.
			 * 
			 * Needed for restarting.
			 */		
			void
			destroy();
		

			/**
			 * @brief Disconnects from database.
			 */
			void
			disconnect();
			
			
			#ifdef DEBUG
				/**
				 * @brief Drops every table.
				 */
				void
				dropTables();
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
			 * @brief Inits database handles.
			 */
			void 
			init();
			
			
			/**
			 * @brief Loads a BLOB from DB into a string.
			 * 
			 * @param table the table from which will be loaded
			 * @param id the id
			 * @return string representation of BLOB
			 */
			string*
			load(size_t table_id, unsigned int id);
			
			
			/**
			 * @brief Registers a new table.
			 */
			size_t
			register_table(std::string name);
			

			/**
			 * @brief Save a string as BLOB in DB.
			 * 
			 * @param table the table in which will be saved
			 * @param id the id
			 * @param data the string
			 */
			void 
			save(size_t table_id, unsigned int id, string& data);
		
		
		private:

			/**
			 * @brief Simple enumeration to code the prepared statements.
			 */	
			enum
			{
				select = 0,
				update,
				insert,
				select_ids,
				select_free_ids,
				select_max_id
			};
			

			static const int number_of_statements = 6;


			/**
			 * @brief Flag. Set true when connected.
			 */			
			bool _connected;
			
	
			/**
			 * @brief Handle for SQL connection.
			 */
			SQLHDBC _connection;
			
		
			/**
			 * @brief Handle for SQL environment.
			 */
			SQLHENV _environment;

		
			/**
			 * @brief Flag. Set true when inited.
			 */
			bool _inited;
			
			
			/**
			 * @brief Collection of all registered tables.
			 */
			std::vector<Table>
			_tables;
			
		
			/**
			 * @brief Checks database structure.
			 * 
			 * @return true, if check passes successfully
			 */
			inline bool
			check_db_structure();
		
	
			/**
			 * @brief Tries to corrects database structure.
			 * 
			 * Throws an exception on failure.
			 */
			inline void
			correct_db_structure();
			 
	
			/**
			 * @see destroy()
			 */
			void
			destroy (bool internal_call);

	
			/**
			 * @see disconnect()
			 */
			void
			disconnect (bool internal_call);
			
			
			/**
			 * @brief Evaluates SQL return value.
			 * 
			 * When sqlReturn == SQL_ERROR, then the error is logged and an string 
			 * exception is thrown.
			 * 
			 * @param sql_return the return status from a SQL command
			 * @param caller identifier for the calling method
			 * @param message the message throw in the string exception, also used
			 * for logging
			 */
			inline void
			evaluate_sql_return(SQLRETURN sql_return, string caller, string message);
		

			/**
			 * @brief Frees prepared statements.
			 */			
			inline void
			free_prepare_statements();

		
			/**
			 * @brief Small logger.
			 * 
			 * Uses mainly the MLog functionality.
			 * 
			 * @param level the MLog::Level
			 * @param caller identifier for the calling method
			 * @param message the message for logging
			 */
			inline void 
			log(MLog::Level level, string caller, string message);

	
			/**
			 * @brief Prepares prepared_statements.
			 */
			inline void
			prepare_statements();
		
	
			/**
			 * @brief Shows error messages.
			 */
			inline void
			show_error();

			
			/**
			 * @brief Shows error messages.
			 * 
			 * @param sql_handle_type sql handle type
			 * @param sql_handle sql handle
			 */
			inline void
			show_error(SQLSMALLINT sql_handle_type, SQLHANDLE& sql_handle);
		
		
			/**
			 * @brief Used for comfortable exception handling.
			 * 
			 * @param caller identifier for the calling method
			 * @param error_message the error message
			 */
			inline void 
			throw_error_message(string caller, string error_message);

	};


	inline void
	DBConnection::evaluate_sql_return(SQLRETURN sqlReturn, string caller, string message)
	{
		if (sqlReturn == SQL_ERROR) 
		{
			log(MLog::error, caller, message);
				
			#ifdef DEBUG
				show_error();
			#endif
			
			throw(message);
		}
	}


	inline void
	DBConnection::log(MLog::Level level, string caller, string message)
	{
		mlog(level, "DBConnection")
		#ifdef DEBUG
			<< "::" << caller << " : "
		#endif
		<< message << "\n";
	}


	inline void
	DBConnection::show_error(SQLSMALLINT sql_handle_type, SQLHANDLE& sql_handle)
	{
		SQLCHAR stat[10];
		SQLCHAR msg[501];
		SQLINTEGER err;
		SQLSMALLINT mlen;
		
		log(MLog::debug, "show_error", "und los!");
		
		int i = 1;
		while (SQLGetDiagRec(sql_handle_type, sql_handle, i, stat, &err,
					msg, 500, &mlen) != SQL_NO_DATA)
		{
			mlog(MLog::debug, "DBConnection") <<" ::show_error : stat = \"" << stat << "\"\n";
			mlog(MLog::debug, "DBConnection") <<" ::show_error : err = \"" << err << "\"\n";
			mlog(MLog::debug, "DBConnection") << " ::show_error : msg = \"" << msg << "\"\n";
			mlog(MLog::debug, "DBConnection") << " ::show_error: mlen = \"" << mlen << "\"\n";
			mlog(MLog::debug, "") << "_\n";
			i++;
		}
	}


	inline void
	DBConnection::show_error()
	{
		SQLCHAR stat[10];
		SQLCHAR msg[501];
		SQLINTEGER err;
		SQLSMALLINT mlen;
		
		int i = 1;
		while (SQLGetDiagRec(SQL_HANDLE_DBC, _connection, i, stat, &err,
					msg, 500, &mlen) != SQL_NO_DATA)
		{
			mlog(MLog::debug, "DBConnection") <<" ::show_error : stat = \"" << stat << "\"\n";
			mlog(MLog::debug, "DBConnection") <<" ::show_error : err = \"" << err << "\"\n";
			mlog(MLog::debug, "DBConnection") << " ::show_error : msg = \"" << msg << "\"\n";
			mlog(MLog::debug, "DBConnection") << " ::show_error: mlen = \"" << mlen << "\"\n";
			mlog(MLog::debug, "") << "_\n";
			i++;
		}
	}


	inline void
	DBConnection::throw_error_message(string caller, string error_message)
	{	
		string throw_message = "";
		
		throw_message.append("DBConnection::");
		throw_message.append(caller);
		throw_message.append(" : ");
		throw_message.append(error_message);
		throw_message.append("\n");
			
		throw (throw_message);
	}

} // namespace mapgeneration_util

#endif //DBCONNECTION_H
