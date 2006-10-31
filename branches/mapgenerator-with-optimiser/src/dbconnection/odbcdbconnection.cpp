/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "odbcdbconnection.h"

#include <iostream>
#include <sstream>
#include <sqlext.h>

namespace mapgeneration
{
	
	ODBCDBConnection::ODBCDBConnection() 
	{
		_connected =false;
		_initialized = false;
		
		#ifdef DEBUG
			log(MLog::debug, "ODBCDBConnection", "ODBCDBConnection constructed.");
		#endif
	}
	
	
	ODBCDBConnection::~ODBCDBConnection()
	{
		try 
		{
			SQLRETURN sql_return;
			string stored_error_messages = "";
			
			if (_connected)
			{
				try 
				{
					disconnect(true);
				} catch(string level2_error_message)
				{
					if (stored_error_messages != "")
					{
						stored_error_messages.append(" ");
					}
					stored_error_messages.append(level2_error_message);
				} // level 2 try-catch
			}
		
		
			if (_initialized)
			{
				try 
				{
					destroy(true);
				} catch(string level2_error_message)
				{
					if (stored_error_messages != "")
					{
						stored_error_messages.append(" ");
					}
					stored_error_messages.append(level2_error_message);
				} // level 2 try-catch
			}
			

			if (stored_error_messages != "") 
			{
				throw (stored_error_messages);
			}
		} catch (string error_message)
		{
			throw_error_message("~ODBCDBConnection", error_message);
		} // level 1 try-catch

		#ifdef DEBUG
			log(MLog::debug, "~ODBCDBConnection", "ODBCDBConnection destructed.");
		#endif
	}
	
	
	bool
	ODBCDBConnection::check_db_structure()
	{		
		std::vector<Table>::iterator search_iterator;
		
		SQLHSTMT statement;
		SQLRETURN sql_return;

		SQLCHAR* sql_search_table_name;
		SQLCHAR sql_search_table_type[] = "TABLE";
		
		sql_return =  SQLAllocHandle(SQL_HANDLE_STMT, _connection, &statement);
		evaluate_sql_return(sql_return, "check_db_structure", "Error allocating statement handle!");
		
		int found_tables = 0;

		// every item of SHOULD-exist-table-name vector is searched in database
		for (search_iterator = _tables.begin(); search_iterator != _tables.end(); ++search_iterator)
		{
			try 
			{
				sql_search_table_name = (SQLCHAR*)(search_iterator->_name.c_str());
				sql_return = SQLTables(statement,
														 NULL, 0,
														 NULL, 0,
														 sql_search_table_name, SQL_NTS,
														 sql_search_table_type, SQL_NTS
														);
				evaluate_sql_return(sql_return, "check_db_structure", "Error retrieving table information!");
	
				// got table information. Bind variables to columns...
				SQLCHAR sql_found_table_name[MAX_SQL_CHAR_LENGTH + 1];
				SQLCHAR sql_found_table_type[MAX_SQL_CHAR_LENGTH + 1];
				SQLINTEGER sql_table_name_pointer;
				SQLINTEGER sql_table_type_pointer;
	
				sql_return = SQLBindCol(statement, 3, SQL_C_CHAR, sql_found_table_name,
									 MAX_SQL_CHAR_LENGTH, &sql_table_name_pointer);
				evaluate_sql_return(sql_return, "check_db_structure", "Error binding column to variable!");
			
				sql_return = SQLBindCol(statement, 4, SQL_C_CHAR, sql_found_table_type,
									 MAX_SQL_CHAR_LENGTH, &sql_table_type_pointer);
				evaluate_sql_return(sql_return, "check_db_structure", "Error binding column to variable!");

				// look in every column and compare it to SHOULD-exist-table-names...
				sql_return = SQLFetch(statement);
				evaluate_sql_return(sql_return, "check_db_structure", "Error fetching data from result set!");
				while (sql_return != SQL_NO_DATA)
				{
					std::cout << "Found table: " << (char*)sql_found_table_name << "\n";
					++found_tables;
					sql_return = SQLFetch(statement);
					evaluate_sql_return(sql_return, "check_db_structure", "Error fetching data from result set!");
				}
			} catch (string error_message)
			{
				try
				{
					sql_return = SQLFreeHandle(SQL_HANDLE_STMT, statement);
					evaluate_sql_return(sql_return, "check_db_structure", "Error freeing SQL statement handle");
				} catch (string level2_error_message)
				{
					throw (error_message + " " + level2_error_message);
				} // level 2 try-catch
					
				throw (error_message);
			} // level 1 try-catch
			
			sql_return = SQLFreeStmt(statement, SQL_RESET_PARAMS);
			evaluate_sql_return(sql_return, "check_db_structure", "Error resetting SQL statement parameters");
			
			sql_return = SQLFreeStmt(statement, SQL_UNBIND);
			evaluate_sql_return(sql_return, "check_db_structure", "Error unbinding SQL statement columns");
			
			sql_return = SQLFreeStmt(statement, SQL_CLOSE);
			evaluate_sql_return(sql_return, "check_db_structure", "Error closing SQL statement cursor");
		}

		sql_return = SQLFreeHandle(SQL_HANDLE_STMT, statement);
		evaluate_sql_return(sql_return, "check_db_structure", "Error freeing SQL statement handle");
	
		if (found_tables != _tables.size())
		{
			#ifdef DEBUG
				log(MLog::debug, "check_db_structure", "failed!");
			#endif
			
			return false;
		}
		
		#ifdef DEBUG
			log(MLog::debug, "check_db_structure", "okay.");
		#endif
		
		return true;
	}
	
	
	void
	ODBCDBConnection::connect()
	{
		if (!_dns.length() || !_user.length() || !_password.length())
		{
			throw ("Parameters missing when calling connect!");
		}
		
		init();
		
		//exits when not initialized!
		if(_initialized == false) return;
		
		//exits when already connected!
		if (_connected == true) return;
		
		
		string stored_error_messages = "";

		try 
		{
			SQLRETURN sql_return;
			SQLCHAR* sql_dns = (SQLCHAR*)_dns.c_str();
			SQLCHAR* sql_user = (SQLCHAR*)_user.c_str();
			SQLCHAR* sql_password = (SQLCHAR*)_password.c_str();
		
			sql_return = SQLConnect(_connection,
														sql_dns, SQL_NTS,
														sql_user, SQL_NTS,
														sql_password, SQL_NTS);
													
			evaluate_sql_return(sql_return, "connect", "Error connecting to DB \"" + _dns + "\"!");


			log(MLog::info, "connect", "Connected to DB \"" + _dns + "\" with user \"" + _user + "\" and password \"*****\"");
			_connected = true;

			
			// connected! Now check DB structure...		
			if (check_db_structure() == false) 
			{
				if (_correct_structure == true)
				{
					log(MLog::warning, "connect", "Wrong DB structure! Try to correct...");
					correct_db_structure();
					if (check_db_structure() == false)
					{
						log(MLog::error, "connect", "Wrong DB structure!");
						throw ("Wrong DB structure!");
					}
				
				} else
				{
					log(MLog::error, "connect", "Wrong DB structure!");
					throw ("Wrong DB structure!");
				}
			}

			// successfully checked! Now prepare statements...
			prepare_statements();
		} catch (string error_message)
		{
			if (_connected ==true)
			{
				try
				{
					disconnect(true);
				} catch (string level2_error_message)
				{
					if (stored_error_messages != "")
					{
						stored_error_messages.append("  ");
					}
					stored_error_messages.append(level2_error_message);
				}
			}

			if (stored_error_messages != "")
			{
				stored_error_messages.append("  ");
			}
			stored_error_messages.append(error_message);
			throw_error_message("connect", stored_error_messages);
		}
	}
	
	
	void
	ODBCDBConnection::correct_db_structure()
	{
		// Simple tries to create the necessary tables.

		SQLRETURN sql_return;
		SQLHSTMT sql_statement;
		SQLCHAR* sql_text;
		
		sql_return = SQLAllocHandle(SQL_HANDLE_STMT, _connection, &sql_statement);
		evaluate_sql_return(sql_return, "correct_db_structure", "Error allocating statement handle!");
		
		try
		{
			std::vector<Table>::iterator tables_iter = _tables.begin();
			std::vector<Table>::iterator tables_iter_end = _tables.end();
			for (; tables_iter != tables_iter_end; ++tables_iter)
			{
				std::string command_start = "CREATE TABLE ";
				std::string command_end = " (id INTEGER NOT NULL PRIMARY KEY, data LONG VARBINARY);";
				int command_length = command_start.length() + 
					command_end.length() + tables_iter->_name.length() + 128;					
				char* buffer = new char[command_length];
				sprintf(buffer, "%s%s%s;", command_start.c_str(),
					tables_iter->_name.c_str(), command_end.c_str());
					
				//sql_text = (SQLCHAR*)"CREATE TABLE tiles (id INTEGER NOT NULL PRIMARY KEY, data LONG VARBINARY);";
				sql_text = (SQLCHAR*)buffer;
				sql_return = SQLExecDirect(sql_statement, sql_text, SQL_NTS);
				evaluate_sql_return(sql_return, "correct_DB_structure", "Error executing \"CREATE TABLE\" - statement! (1)");
	
				sql_return = SQLFreeStmt(sql_statement, SQL_RESET_PARAMS);
				evaluate_sql_return(sql_return, "correct_db_structure", "Error resetting SQL statement parameters");
				
				sql_return = SQLFreeStmt(sql_statement, SQL_UNBIND);
				evaluate_sql_return(sql_return, "correct_db_structure", "Error unbinding SQL statement columns");
				
				sql_return = SQLFreeStmt(sql_statement, SQL_CLOSE);
				evaluate_sql_return(sql_return, "correct_db_structure", "Error closing SQL statement cursor");
			}

			log(MLog::info, "correct_db_structure", "Correcting tables... successful!");
		} catch (string error_message)
		{
			try
			{
				sql_return = SQLFreeHandle(SQL_HANDLE_STMT, sql_statement);
				evaluate_sql_return(sql_return,
										"correct_db_structure",
										"Error freeing SQL statement handle!");
			} catch (string level2_error_message)
			{
				throw (error_message + " " + level2_error_message);
			}// level 2 try-catch
			
			throw (error_message);
		} // level 1 try-catch
	
		sql_return = SQLFreeHandle(SQL_HANDLE_STMT, sql_statement);
		evaluate_sql_return(sql_return, "correct_db_structure",
										"Error freeing SQL statement handle!");
										
		#ifdef DEBUG
			log(MLog::debug, "correct_db_structure", "DB Structure corrected successfully.");
		#endif
	}
	
	
	void
	ODBCDBConnection::destroy()
	{
		destroy(false);
	}
	
	
	void
	ODBCDBConnection::disconnect()
	{
		disconnect(false);
	}

	
	void
	ODBCDBConnection::drop_tables()
	{
		SQLRETURN sql_return;
		SQLHSTMT sql_statement;
		SQLCHAR* sql_text;
	
		sql_return = SQLAllocHandle(SQL_HANDLE_STMT, _connection, &sql_statement);
		evaluate_sql_return(sql_return, "destroy", "Error allocating SQL statement handle!");
	
		std::vector<Table>::iterator tables_iter = _tables.begin();
		std::vector<Table>::iterator tables_iter_end = _tables.end();
		for (; tables_iter != tables_iter_end; ++tables_iter)
		{
			std::string command_start = "DROP TABLE ";
			std::string command_end = ";";				
			int command_length = command_start.length() + 
				command_end.length() + tables_iter->_name.length() + 128;					
			char* buffer = new char[command_length];
			sprintf(buffer, "%s%s%s;", command_start.c_str(),
				tables_iter->_name.c_str(), command_end.c_str());
				
			//sql_text = (SQLCHAR*)"DROP TABLE tiles;";
			sql_text = (SQLCHAR*)buffer;
			sql_return = SQLExecDirect(sql_statement, sql_text, SQL_NTS);
			evaluate_sql_return(sql_return, "destroy", "Error executing \"DROP TABLE\" statement! (1)");
			
			delete buffer;
		}

		log(MLog::debug, "dropTables", "Tables destroyed!");
	}
	
	
	void
	ODBCDBConnection::init()
	{
		// exits when already initialized!
		if (_initialized == true) return;
			
		try
		{
			#ifdef DEBUG
				log(MLog::debug, "ODBCDBConnection", "Starting...");
			#endif
		
			SQLRETURN sql_return;

			#ifdef DEBUG
				log(MLog::debug, "ODBCDBConnection", "Allocate environment handle...");
			#endif
			sql_return = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_environment);
			evaluate_sql_return(sql_return, "ODBCDBConnection", "Error allocating SQL environment handle!");

			try
			{
				#ifdef DEBUG		
					log(MLog::debug, "ODBCDBConnection", "Set ODBC version...");
				#endif
				sql_return = SQLSetEnvAttr(_environment, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
				evaluate_sql_return(sql_return, "ODBCDBConnection", "Error setting ODBC version!");
		
				#ifdef DEBUG		
					log(MLog::debug, "ODBCDBConnection", "Allocating connection handle...");
				#endif
				sql_return = SQLAllocHandle(SQL_HANDLE_DBC, _environment, &_connection);
				evaluate_sql_return(sql_return, "ODBCDBConnection", "Error allocating SQL connection!");

			} catch (string level2_error_message)
			{
				try
				{
					sql_return = SQLFreeHandle(SQL_HANDLE_ENV, _environment);
					evaluate_sql_return(sql_return,
											"ODBCDBConnection",
											"Error freeing SQL environment handle!");
				} catch (string level3_error_message)
				{
					throw (level2_error_message + " " + level3_error_message);
				} // level 3 try-catch
											
				throw (level2_error_message);
			} // level 2 try-catch
		} catch(string error_message)
		{
			throw_error_message("ODBCDBConnection", error_message);
		} // level 1 try-catch


		_initialized =true;
		log(MLog::notice, "init", "Init ODBCDBConnection... successful finished.");
	}

	
	void
	ODBCDBConnection::destroy(bool internal_call)
	{
		//exits when not initialized!
		if (_initialized == false) return;
		
		
		string stored_error_messages = "";

		if (_connected == true)
		{
			try 
			{
				disconnect(true);
			} catch (string error_message)
			{
				if (stored_error_messages != "")
				{
					stored_error_messages.append(" ");
				}
				stored_error_messages.append(error_message);
			}
		}
		
		
		SQLRETURN sql_return;
		
		try
		{
			try
			{		
				sql_return = SQLFreeHandle(SQL_HANDLE_DBC, _connection);
				evaluate_sql_return(sql_return, 
									"destroy",
									"Error freeing SQL connection handle!");
			} catch(string level2_error_message)
			{
				if (stored_error_messages != "")
				{
					stored_error_messages.append(" ");
				}
				stored_error_messages.append(level2_error_message);
			} // level 2 try-catch
			
			try
			{
				sql_return = SQLFreeHandle(SQL_HANDLE_ENV, _environment);
				evaluate_sql_return(sql_return,
										"destroy", 
										"Error freeing SQL environment handle!");
			} catch (string level2_error_message)
			{
				if (stored_error_messages != "")
				{
					stored_error_messages.append(" ");
				}
				stored_error_messages.append(level2_error_message);
			} // level 2 try-catch
			
			if (stored_error_messages != "") 
			{
				throw (stored_error_messages);
			}
		} catch (string error_message)
		{
			if (internal_call)
			{
				throw (error_message);
			} else
			{
				throw_error_message("destroy", error_message);
			}
		} // level 1 try-catch

		_initialized = false;
		log(MLog::notice, "destroy", "Destroy ODBCDBConnection... successful finished.");
	}
	

	void
	ODBCDBConnection::disconnect(bool internal_call)
	{
		//exits when not connected
		if (_connected == false) return;
		
		try
		{
			SQLRETURN sql_return;
			string stored_error_messages = "";
			
			try
			{
				free_prepared_statements();
			} catch(string level2_error_message)
			{
				if (stored_error_messages != "")
				{
					stored_error_messages.append(" ");
				}
				stored_error_messages.append(level2_error_message);
			}
		
			try 
			{
				sql_return = SQLDisconnect(_connection);
				evaluate_sql_return(sql_return, "disconnect", "Error disconnecting from database!");
			} catch (string level2_error_message)
			{
				if (stored_error_messages != "")
				{
					stored_error_messages.append(" ");
				}
				stored_error_messages.append(level2_error_message);
			}
			
			if (stored_error_messages != "")
			{
				throw (stored_error_messages);
			}
		} catch (string error_message)
		{
			if (internal_call)
			{
				throw (error_message);
			} else
			{
				throw_error_message("disconnect", error_message);
			}
		}

		_connected = false;
		
		log(MLog::info, "disconnect", "Disconnected from DB.");
		
		destroy();
	}
	
	
	void
	ODBCDBConnection::free_prepared_statements()
	{
		SQLRETURN sql_return;
		string stored_error_messages = "";
		bool error_occured = false;
		
		std::vector<Table>::iterator tables_iter = _tables.begin();
		std::vector<Table>::iterator tables_iter_end = _tables.end();
		for(; tables_iter != tables_iter_end; ++tables_iter)
		{
			std::vector<SQLHSTMT>::iterator statement_iter = 
				tables_iter->_prepared_statements.begin();
			std::vector<SQLHSTMT>::iterator statement_iter_end = 
				tables_iter->_prepared_statements.end();
			for(; statement_iter != statement_iter_end; ++statement_iter)
			{
				try
				{
					sql_return = SQLFreeHandle(SQL_HANDLE_STMT,
														&*statement_iter);
					evaluate_sql_return(sql_return, "free_prepared_statements",
													"Error freeing SQL statement handle!");
				} catch (string error_message)
				{
					if (stored_error_messages != "")
					{
						stored_error_messages.append(" ");
					}
					stored_error_messages.append(error_message);
					error_occured = true;
				}
			}
		}

		
		if (error_occured == true)
		{
			throw (stored_error_messages);
		}
		
		#ifdef DEBUG
			log(MLog::debug, "free_prepared_statements", "Prepared statements freed.");
		#endif
	}


	std::vector<unsigned int>
	ODBCDBConnection::get_all_used_ids(size_t table_id)
	{
		SQLRETURN sql_return;
		SQLHSTMT sql_statement;
		SQLINTEGER sql_indicator;
		
		unsigned int col_value;
		std::vector<unsigned int> ids;

		sql_statement = _tables[table_id]._prepared_statements[select_ids];
		
		sql_return = SQLExecute(sql_statement);
		evaluate_sql_return(sql_return, "get_all_used_ids", "Error executing prepared SQL command!");

		sql_return = SQLBindCol(sql_statement, 1, SQL_C_ULONG, &col_value,
														0, &sql_indicator);
		evaluate_sql_return(sql_return, "get_all_used_ids", "Error binding variable to SQL column!");

		while ((sql_return = SQLFetch(sql_statement)) != SQL_NO_DATA)
		{
			evaluate_sql_return(sql_return, "get_all_used_ids", "Error fetching SQL rowset!");
			ids.push_back(col_value);
		}

		sql_return = SQLFreeStmt(sql_statement, SQL_RESET_PARAMS);
		evaluate_sql_return(sql_return, "get_all_used_ids", "Error resetting SQL statement parameters");
			
		sql_return = SQLFreeStmt(sql_statement, SQL_UNBIND);
		evaluate_sql_return(sql_return, "get_all_used_ids", "Error unbinding SQL statement columns");
			
		sql_return = SQLFreeStmt(sql_statement, SQL_CLOSE);
		evaluate_sql_return(sql_return, "get_all_used_ids", "Error closing SQL statement cursor");
				
		return ids;
	}


	std::vector<unsigned int>
	ODBCDBConnection::get_free_ids(size_t table_id)
	{
		SQLRETURN sql_return;
		SQLHSTMT sql_statement;
		SQLINTEGER sql_indicator;
		
		unsigned int col_value;
		std::vector<unsigned int> ids;

		sql_statement = _tables[table_id]._prepared_statements[select_free_ids];
		
		sql_return = SQLExecute(sql_statement);
		evaluate_sql_return(sql_return, "get_free_ids", "Error executing prepared SQL command!");

		sql_return = SQLBindCol(sql_statement, 1, SQL_C_ULONG, &col_value,
														0, &sql_indicator);
		evaluate_sql_return(sql_return, "get_free_ids", "Error binding variable to SQL column!");

		while ((sql_return = SQLFetch(sql_statement)) != SQL_NO_DATA)
		{
			evaluate_sql_return(sql_return, "get_free_ids", "Error fetching SQL rowset!");
			ids.push_back(col_value);
		}

		sql_return = SQLFreeStmt(sql_statement, SQL_RESET_PARAMS);
		evaluate_sql_return(sql_return, "get_free_ids", "Error resetting SQL statement parameters");
			
		sql_return = SQLFreeStmt(sql_statement, SQL_UNBIND);
		evaluate_sql_return(sql_return, "get_free_ids", "Error unbinding SQL statement columns");
			
		sql_return = SQLFreeStmt(sql_statement, SQL_CLOSE);
		evaluate_sql_return(sql_return, "get_free_ids", "Error closing SQL statement cursor");
		
		ids.push_back(get_next_to_max_id(table_id));
		
		return ids;
	}
	
	
	unsigned int
	ODBCDBConnection::get_next_to_max_id(size_t table_id)
	{
		SQLRETURN sql_return;
		SQLHSTMT sql_statement;
		SQLINTEGER sql_indicator;
		
		unsigned int col_value = 0;

		sql_statement = _tables[table_id]._prepared_statements[select_max_id];
		
		sql_return = SQLExecute(sql_statement);
		evaluate_sql_return(sql_return, "get_next_to_max_id", "Error executing prepared SQL command!");

		sql_return = SQLBindCol(sql_statement, 1, SQL_C_ULONG, &col_value,
														0, &sql_indicator);
		evaluate_sql_return(sql_return, "get_next_to_max_id", "Error binding variable to SQL column!");

		if ((sql_return = SQLFetch(sql_statement)) != SQL_NO_DATA)
		{
			evaluate_sql_return(sql_return, "get_next_to_max_id", "Error fetching SQL rowset!");
			++col_value;
		}

		sql_return = SQLFreeStmt(sql_statement, SQL_RESET_PARAMS);
		evaluate_sql_return(sql_return, "get_next_to_max_id", "Error resetting SQL statement parameters");
			
		sql_return = SQLFreeStmt(sql_statement, SQL_UNBIND);
		evaluate_sql_return(sql_return, "get_next_to_max_id", "Error unbinding SQL statement columns");
			
		sql_return = SQLFreeStmt(sql_statement, SQL_CLOSE);
		evaluate_sql_return(sql_return, "get_next_to_max_id", "Error closing SQL statement cursor");
		
		std::cout << "XX:" << col_value << "\n";
		
		return col_value;
		
	}
	
	
	string*
	ODBCDBConnection::load(size_t table_id, unsigned int id)
	{
		if ((_initialized != true) || (_connected != true))
			throw_error_message("load_blob", "Not initialized and/or connected!");
		
		try
		{				
			SQLRETURN sql_return;
			SQLHSTMT sql_statement;

			sql_statement = _tables[table_id]._prepared_statements[select];
			
			SQLINTEGER sql_id = (SQLINTEGER)id;
			SQLCHAR sql_buffer[SQL_BINARY_BUFFER];
			SQLINTEGER sql_buffer_length = (SQLINTEGER)(SQL_BINARY_BUFFER);
			SQLINTEGER sql_indicator;
			
			sql_return = SQLBindParameter(sql_statement, 1, SQL_PARAM_INPUT,
													SQL_C_SLONG, SQL_INTEGER, 10,
													0, &sql_id, 0, 0);
			evaluate_sql_return(sql_return, "load_blob", "Error binding variable to SQL parameter!");
		
			sql_return = SQLExecute(sql_statement);
			evaluate_sql_return(sql_return, "load_blob", "Error executing prepared SQL command!");
	
			sql_return = SQLFetch(sql_statement);
			evaluate_sql_return(sql_return, "load_blob", "Error fetching data from result set!");
			if (sql_return == SQL_NO_DATA)
				return NULL;
	
			sql_return = SQLGetData(sql_statement, 1, SQL_C_BINARY, sql_buffer,
											sql_buffer_length, &sql_indicator);
			evaluate_sql_return(sql_return, "load_blob", "Error executing SQLGetData!");
			if (sql_return == SQL_NO_DATA)
				return NULL;
	
			string* data_representation = new string();
			if (sql_indicator == SQL_NO_TOTAL || sql_indicator >= SQL_BINARY_BUFFER)
			{
				data_representation->append((char*)sql_buffer, SQL_BINARY_BUFFER);
			} else
			{
				if (sql_indicator > 0)
				{
					data_representation->append((char*)sql_buffer, sql_indicator);
				}
			}					
			
			while (SQL_NO_DATA != 
						(sql_return = SQLGetData(sql_statement, 1, SQL_C_BINARY,
															sql_buffer,	sql_buffer_length,
															&sql_indicator)))
			{
				evaluate_sql_return(sql_return, "load_blob", "Error executing SQLGetData!");
	
				if (sql_indicator == SQL_NO_TOTAL || sql_indicator >= SQL_BINARY_BUFFER)
				{
					data_representation->append((char*)sql_buffer, SQL_BINARY_BUFFER);
				} else
				{
					if (sql_indicator > 0)
					{
						data_representation->append((char*)sql_buffer, sql_indicator);
					}
				}					
			}
	
	
			sql_return = SQLFreeStmt(sql_statement, SQL_RESET_PARAMS);
			evaluate_sql_return(sql_return, "load_blob", "Error resetting SQL statement parameters");
				
			sql_return = SQLFreeStmt(sql_statement, SQL_UNBIND);
			evaluate_sql_return(sql_return, "load_blob", "Error unbinding SQL statement columns");
				
			sql_return = SQLFreeStmt(sql_statement, SQL_CLOSE);
			evaluate_sql_return(sql_return, "load_blob", "Error closing SQL statement cursor");
			
			if (*data_representation == "")
				return 0;
			else
				return data_representation;
		
		} catch (string error_message)
		{
			throw_error_message("load_tile", error_message);
		}

	}
	
	
	void 
	ODBCDBConnection::prepare_statements()
	{
		// first allocate the statements...
		SQLRETURN sql_return;
		string stored_error_messages = "";

		bool error_occured = false;
		
		std::vector<Table>::iterator tables_iter = _tables.begin();
		std::vector<Table>::iterator tables_iter_end = _tables.end();
		for(; (tables_iter != tables_iter_end) && (error_occured == false);
			++tables_iter)
		{
			for (int i = 0; (error_occured == false) && 
				(i < _NUMBER_OF_STATEMENTS); ++i)
			{
				tables_iter->_prepared_statements.push_back(SQLHSTMT());
				try
				{
					// allocate _prepared_load_tiles_statement...		
					sql_return = SQLAllocHandle(SQL_HANDLE_STMT, _connection,
						&tables_iter->_prepared_statements.back());
					evaluate_sql_return(sql_return, "prepared_statements",
													"Error allocating SQL statement handle!");
				} catch (string error_message)
				{
					if (stored_error_messages != "")
					{
						stored_error_messages.append(" ");
					}
					stored_error_messages.append(error_message);
					error_occured = true;
				}
			}
		}
						
		// error occured! try to free statement handles!
		if (error_occured)
		{
			free_prepared_statements();
			throw("Error preparing statements.");
		}
		
		#ifdef DEBUG
			log(MLog::debug, "prepared_statements", "Prepared statesments successfully allocated.");
		#endif
		
		std::string sql_commands[_NUMBER_OF_STATEMENTS];
		
		tables_iter = _tables.begin();
		tables_iter_end = _tables.end();
		for(; (tables_iter != tables_iter_end) && (error_occured == false);
			++tables_iter)
		{			
			sql_commands[select] = "SELECT data FROM ";
			sql_commands[select] += tables_iter->_name;
			sql_commands[select] += " WHERE id = ?;";
			
			sql_commands[update] = "UPDATE ";
			sql_commands[update] += tables_iter->_name;
			sql_commands[update] += " SET data = ? WHERE id = ?;";
			
			sql_commands[insert] = "INSERT INTO ";
			sql_commands[insert] += tables_iter->_name;
			sql_commands[insert] += " (data, id) VALUES (?, ?);";
			
			sql_commands[select_ids] = "SELECT id FROM ";
			sql_commands[select_ids] += tables_iter->_name;
			sql_commands[select_ids] += " ORDER BY id ASC;";
			
			sql_commands[select_free_ids] = "SELECT id FROM ";
			sql_commands[select_free_ids] += tables_iter->_name;
			sql_commands[select_free_ids] += " WHERE data = \"\" ORDER BY id ASC;";
			
			sql_commands[select_max_id] = "SELECT MAX(id) FROM ";
			sql_commands[select_max_id] += tables_iter->_name;
			sql_commands[select_max_id] += ";";
		
			try
			{
				for (int i = 0; i < _NUMBER_OF_STATEMENTS; ++i)
				{
					sql_return = SQLPrepare(tables_iter->_prepared_statements[i],
													(SQLCHAR*)(sql_commands[i].c_str()),
													SQL_NTS);
					evaluate_sql_return(sql_return, "prepared_statements",
											"Error preparing SQL statement!");
				}
			} catch (string error_message)
			{
				try
				{
					free_prepared_statements();
				} catch (string level2_error_message)
				{
					throw (error_message + " " + level2_error_message);
				}  // level 2 try-catch
				
				throw (error_message);
			} // level 1 try-catch
		}

		#ifdef DEBUG
			log(MLog::debug, "prepare_statements", "Everything successfully done.");
		#endif
	}
	
	
	size_t
	ODBCDBConnection::register_table(std::string name)
	{
		if (_initialized || _connected)
			throw_error_message("ODBCDBConnection", 
				"Cannot register table when inizialized or connected!");
		
		Table new_table;
		new_table._name = name;
		_tables.push_back(new_table);
		
		return (_tables.size() - 1);
	}
	
	
	void
	ODBCDBConnection::remove(size_t table_id, unsigned int id)
	{
		if ((_initialized != true) || (_connected != true))
			throw_error_message("delete_entry", "Not initialized and/or connected!");

		try
		{
			std::string really_temp;
			save(table_id, id, really_temp);
		} catch (string error_message)
		{
			throw_error_message("delete_entry", error_message);
		}
		
	}
	

	void
	ODBCDBConnection::save(size_t table_id, unsigned int id, string& data_representation)
	{
		if ((_initialized != true) || (_connected  != true))
			throw_error_message("save_blob", "Not initialized and/or connected!");
		try
		{		
			
			SQLRETURN sql_return;
			SQLHSTMT sql_statement;
	
			sql_statement = _tables[table_id]._prepared_statements[update];
					
			SQLCHAR* sql_data = (SQLCHAR*)((const char*)data_representation.c_str());
			SQLINTEGER sql_data_length = data_representation.size();
				
			SQLINTEGER sql_id = (SQLINTEGER)id;
			SQLPOINTER sql_parameter_indicator = (SQLPOINTER)1; 
			SQLINTEGER sql_execution_indicator;
						
			sql_return = SQLBindParameter(sql_statement, 1, SQL_PARAM_INPUT,
											SQL_C_BINARY, SQL_LONGVARBINARY,
											sql_data_length, 0, sql_data,
											sql_data_length, &sql_data_length);
	
			evaluate_sql_return(sql_return, "save_blob", "Error binding variable to SQL parameter!");
			
			sql_return = SQLBindParameter(sql_statement, 2, SQL_PARAM_INPUT,
													SQL_C_SLONG, SQL_INTEGER, 10,
													0, &sql_id, 0, 0);
			evaluate_sql_return(sql_return, "save_blob", "Error binding variable to SQL parameter!");
		
			sql_return = SQLExecute(sql_statement);
			evaluate_sql_return(sql_return, "save_blob", "Error executing prepared SQL command (1)!");
	
			// checking, how many rows were affected.
			// if 0 then try INSERT...
			SQLINTEGER row_count = (SQLINTEGER)-1;
			sql_return = SQLRowCount(sql_statement, &row_count);
			evaluate_sql_return(sql_return, "save_blob", "Error executing SQLRowCount command!");
			
			sql_return = SQLFreeStmt(sql_statement, SQL_RESET_PARAMS);
			evaluate_sql_return(sql_return, "save_blob", "Error resetting SQL statement parameters");
				
			sql_return = SQLFreeStmt(sql_statement, SQL_UNBIND);
			evaluate_sql_return(sql_return, "save_blob", "Error unbinding SQL statement columns");
				
			sql_return = SQLFreeStmt(sql_statement, SQL_CLOSE);
			evaluate_sql_return(sql_return, "save_blob", "Error closing SQL statement cursor");
			
			
			if (row_count == 0)
			{
				#ifdef DEBUG
				//	log(MLog::debug, "save_blob", "UPDATE failed. Will try INSERT...");
				#endif
	
				sql_statement = _tables[table_id]._prepared_statements[insert];
	
				sql_return = SQLBindParameter(sql_statement, 1, SQL_PARAM_INPUT,
												SQL_C_BINARY, SQL_LONGVARBINARY,
												sql_data_length, 0, sql_data,
												sql_data_length, &sql_data_length);
				
				evaluate_sql_return(sql_return, "save_blob", "Error binding variable to SQL parameter!");
			
				sql_return = SQLBindParameter(sql_statement, 2, SQL_PARAM_INPUT,
														SQL_C_SLONG, SQL_INTEGER, 10,
														0, &sql_id, 0, 0);
				evaluate_sql_return(sql_return, "save_blob", "Error binding variable to SQL parameter!");
	
	
				string stored_error_messages = "";
				sql_return = SQLExecute(sql_statement);
				if (sql_return == SQL_ERROR)
				{
					SQLRETURN sql_diag_return;
					SQLCHAR stat[6];
					SQLINTEGER err = (SQLINTEGER)-1;
	
					try
					{
						sql_diag_return = SQLGetDiagRec(SQL_HANDLE_STMT, sql_statement,
																	1, stat, &err, 0, 0, 0);
						evaluate_sql_return(sql_diag_return, "save_clob", "Error retrieving diagnostics!");
					} catch (string level2_error_message)
					{
						if (stored_error_messages != "")
						{
							stored_error_messages.append(" ");
						}
						stored_error_messages.append(level2_error_message);
					}
					
					if ((strcmp((char*)stat, "23000") == 0) && (err == 1062))
					{
						#ifdef DEBUG
					//		log(MLog::debug, "save_blob", "Data unchanged. No INSERT needed!");
						#endif
					} else
					{
						if (stored_error_messages != "")
						{
							stored_error_messages.append(" ");
						}
						stored_error_messages.append("Error executing prepared SQL command (2)!");
					}
					
					if (stored_error_messages != "")
					{
						throw (stored_error_messages);
					}
				}				
	
				sql_return = SQLFreeStmt(sql_statement, SQL_RESET_PARAMS);
				evaluate_sql_return(sql_return, "save_blob", "Error resetting SQL statement parameters");
				
				sql_return = SQLFreeStmt(sql_statement, SQL_UNBIND);
				evaluate_sql_return(sql_return, "save_blob", "Error unbinding SQL statement columns");
				
				sql_return = SQLFreeStmt(sql_statement, SQL_CLOSE);
				evaluate_sql_return(sql_return, "save_blob", "Error closing SQL statement cursor");
			
			}
			
		} catch (string error_message)
		{
			throw_error_message("save_blob", error_message);
		}		
		
	}
	
	
	void
	ODBCDBConnection::set_parameters(string dns, string user, string password, 
		bool correct_structure)
	{
		_dns = dns;
		_user = user;
		_password = password;
		_correct_structure = correct_structure;
	}

} // namespace mapgeneration
