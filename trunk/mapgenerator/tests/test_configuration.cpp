/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include <iostream>
#include <map>
#include "util/configuration.h"
#include "util/pubsub/servicelist.h"
#include "util/pubsub/service.h"
#include "util/pubsub/publisher.h"
#include "util/pubsub/subscriber.h"
#include "util/pubsub/callsubscriber.h"
#include "util/pubsub/callpublisher.h"
#include "util/pubsub/classcallpublisher.h"
#include "util/pubsub/classcallsubscriber.h"

using namespace std;
using namespace pubsub;
using namespace mapgeneration_util;



void
show_all(ServiceList& service_list)
{
	map<string, GenericService*>::const_iterator service_iterator = 
		service_list.services_by_name().begin();
	map<string, GenericService*>::const_iterator service_iterator_end = 
		service_list.services_by_name().end();
	for (; service_iterator != service_iterator_end; service_iterator++)
	{
		cout << "    " << service_iterator->first << ", ID: " << 
			service_iterator->second->get_id() << ", Type: " <<
			service_iterator->second->get_data_type_name() << ", Value: ";
		if (service_iterator->second->get_data_type_name() == typeid(std::string("")).name())
		{
			cout << static_cast< Service<std::string>* >(service_iterator->second)->publish();
		} else if (service_iterator->second->get_data_type_name() == typeid(int(0)).name())
		{
			cout << static_cast< Service<int>* >(service_iterator->second)->publish();
		} else if (service_iterator->second->get_data_type_name() == typeid(double(0)).name())
		{
			cout << static_cast< Service<double>* >(service_iterator->second)->publish();
		} else 
		{
			cout << "UNKNOWN TYPE";			
		}
		cout << "\n";
	}
}


int main()
{
	cout << "\nTesting configuration!\n\n";
	
	ServiceList service_list;
	
	Configuration configuration("configuration.xml", &service_list, 0);
	bool success = configuration.read_configuration();	
	cout << "Success_read_configuration  = " << success << endl;
	
	return 0;
}

