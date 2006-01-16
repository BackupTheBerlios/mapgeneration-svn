/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "dbconnection/filedbconnection.h"

#include <istream>
#include <string>

#include "edge.h"
#include "tile.h"
#include "util/serializer.h"

using namespace std;
using namespace mapgeneration;

int main() 
{
	#ifdef DEBUG
	
		string* data;
		bool stop = false;
	
		// construct...
		mlog(MLog::debug, "test_dbconnection") << "Testing FileDBConnection!\n";
		mlog(MLog::debug, "test_dbconnection") << "Call DBConnection::DBConnection.\n";		
		FileDBConnection dbc;	
		// constructor called.
	
		try
		{
			// connect...
			mlog(MLog::debug, "test_dbconnection") << "Call DBConnection::connect.\n";
			dbc.set_parameters("testdb1234");
			size_t test_table_id = dbc.register_table("test");
			dbc.connect();
			// connected.
	
			// init...
	//		mlog(MLog::debug, "test_dbconnection") << "Call DBConnection::init.\n";		
	//		dbc.init();
			//inited.
			
			// connect...
	//		mlog(MLog::debug, "test_dbconnection") << "Call DBConnection::connect.\n";
	//		dbc.connect();
			// connected.
		
			// build some input...
			mlog(MLog::debug, "test_dbconnection") << "Build some input.\n";
			string data_string = "";
			int j;
			for (int i = 0; i < 1000000; ++i)
			{
				j = (i % 256);
				data_string.append((const char*)&j, 1);
			}
			mlog(MLog::debug, "test_dbconnection") << "Size of input = 1,000,000\n";
			
			mlog(MLog::debug, "test_dbconnection") << "Call DBConnection::save_tile.\n";
			dbc.save(test_table_id, -100, data_string);
			mlog(MLog::debug, "test_dbconnection") << "Press \"x\" to continue.\n";
			while (!stop)
			{
				if (getchar() == 'x') stop = true;
			}
			stop = false;
			// built it.
			
			//load some data...
			mlog(MLog::debug, "test_dbconnection") << "Call DBConnection::load_tile.\n";
			string* temp = dbc.load(test_table_id, -100);
	
			if (temp->compare(data_string) == 0)
			{
				mlog(MLog::debug, "test_dbconnection") << "Save -> Load: Data equal.\n";
			} else
			{
				for (int i = 0; i < temp->size(); ++i)
				{
					string s = temp->substr(i, 1);
					string t = data_string.substr(i, 1);
					if (s.compare(t) != 0)
					{
						cout << "Fehler: " << i << endl;
					}
				}
				mlog(MLog::debug, "test_dbconnection") << "Save -> Load: DATA CORRUPTED!\n";
			}
			delete temp;
			mlog(MLog::debug, "test_dbconnection") << "Press \"x\" to continue.\n";
			while (!stop)
			{
				if (getchar() == 'x') stop = true;
			}
			stop = false;
			// loaded.
	
			// delete data...
			mlog(MLog::debug, "test_dbconnection") << "Call DBConnection::delete_tile.\n";
			dbc.remove(test_table_id, -100);
			mlog(MLog::debug, "test_dbconnection") << "Press \"x\" to continue.\n";
			while (!stop)
			{
				if (getchar() == 'x') stop = true;
			}
			stop = false;
			// loaded.
			
			// perform some more tests
			for (int i = -10; i < 0; ++i)
			{
				dbc.save(test_table_id, i, data_string);
			}
			
			for (int i = -10; i < 0; i = i + 3)
			{
				string* temp = dbc.load(test_table_id, i);
				delete temp;
			}
			
			for (int i = -10; i < 0; i = i + 4)
			{
				string test = "hallo";
				dbc.save(test_table_id, i, test);
			}
			mlog(MLog::debug, "test_dbconnection") << "Press \"x\" to continue.\n";
			while (!stop)
			{
				if (getchar() == 'x') stop = true;
			}
			stop = false;
			// performed
			
			// drop tables
			mlog(MLog::debug, "test_dbconnection") << "Delete tables.\n";
			dbc.drop_tables();
		}
		catch (string error_message)
		{
			cout << error_message << endl;
			
			// drop tables
			mlog(MLog::debug, "test_dbconnection") << "Delete tables.\n";
			dbc.drop_tables();
		}
	
	#endif
}
