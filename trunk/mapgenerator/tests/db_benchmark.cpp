/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "dbconnection.h"

#include <cstdlib>
#include <string>

using namespace std;
using namespace mapgeneration;

int main() 
{
	cout << "Starte DB-Benchmark." << endl;
	
	char char_data[20001];
	for (int i = 0; i < 20000; ++i)
	{
		char_data[i] = 'a' + (i % 52);
	}
	char_data[20000] = '\0';
	string string_data = string(char_data);
	
	
	DBConnection dbc = DBConnection();
	dbc.connect("MapGeneration", "mapgeneration", "mg");
	
	int i = -10000000;
	int j = 0;
	while (i < 10000000)
	{
		i += (rand() / 300000);
		dbc.save_tile(i, string_data);
		++j;
	}
	cout << j << " Eintraege in die DB geschrieben." << endl;
	
	i = 10000000;
	j = 0;
	while (i > -10000000)
	{
		i-= (rand() / 300000);
		string* test = dbc.load_tile(i);
		delete test;
		++j;
	}
	cout << j << " Eintraege aus der DB gelesen." << endl;

	i = -10000000;
	j = 0;
	while (i < 10000000)
	{
		i += (rand() / 300000);
		dbc.save_tile(i, string_data);
		++j;
	}
	cout << j << " Eintraege in die DB geschrieben." << endl;
	
	i = 10000000;
	j = 0;
	while (i > -10000000)
	{
		i-= (rand() / 300000);
		string* test = dbc.load_tile(i);
		delete test;
		++j;
	}
	cout << j << " Eintraege aus der DB gelesen." << endl;
	
	return 0;
}
