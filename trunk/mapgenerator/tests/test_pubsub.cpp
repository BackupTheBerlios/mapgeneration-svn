/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include <iostream>

#include "util/pubsub/servicesystem.h"
#include "util/pubsub/publisher.h"
#include "util/pubsub/subscriber.h"
#include "util/pubsub/callsubscriber.h"
#include "util/pubsub/callpublisher.h"
#include "util/pubsub/classcallpublisher.h"
#include "util/pubsub/classcallsubscriber.h"


using namespace std;
using namespace pubsub;


class RandomPublisher : public Publisher<int>
{
	
	public:
	
		RandomPublisher(Service<int>* service)
		: Publisher<int>::Publisher(service, Publisher<int>::PULL)
		{		
		}
		
		
		int
		publish()
		{
			return rand();
		}
		
		
		void
		remove_subscriber(Subscriber<int>* subscriber)
		{
			delete this;
		}
		
		
		void
		set_random_seed(int seed)
		{
			cout << "RandomPublisher: Setting random seed to: " << seed << "\n";
			srand(seed);
		}
};


void
show_int(int i)
{
	cout << "New integer send: " << i << "\n";
}


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
		} else
		{
			cout << "UNKNOWN TYPE";			
		}
		cout << "\n";
	}
}


int main()
{
	cout << "\nTesting the publish subscribe system!\n\n";
	
	cout << "Creating new ServiceList service_list: ";
	ServiceList service_list;
	cout << "Ok\n\n";
	
	cout << "Creating new service:\n";	
	Service<string>* message_service = new Service<string>("test_pubsub.messages", "Initial message");	
	cout << "  Name: " << message_service->get_name() << "\n";
	cout << "  Id:   " << message_service->get_id() << "\n";
	cout << "OK\n\n";
	
	cout << "Creating new service:\n";
	Service<int> random_service("test_pubsub.random_int", 0);
	cout << "  Name: " << random_service.get_name() << "\n";
	cout << "  Id:   " << random_service.get_id() << "\n";
	cout << "OK\n\n";
	
	cout << "Creating new service:\n";
	Service<ServiceList*>* service_list_service = new Service<ServiceList*>("test_pubsub.service_list", &service_list);
	cout << "  Name: " << service_list_service->get_name() << "\n";
	cout << "  Id:   " << service_list_service->get_id() << "\n";
	cout << "OK\n\n";
	
	cout << "Registering services to service_list: ";
	service_list.add(&random_service);	
	service_list.add(message_service);
	service_list.add(service_list_service);
	cout << "OK\n\n";
	
	cout << "Using a little output procedure to show all showable values:\n";
	show_all(service_list);
	cout << "OK\n\n";
	
	cout << "Changing some values and showing all again:\n";
	message_service->receive("New message!");
	random_service.receive(5);
	show_all(service_list);
	cout << "OK\n\n";
	
	cout << "Lets delete message_service and service_list_service, we don't need them:\n";
	delete message_service;
	delete service_list_service;
	show_all(service_list);
	cout << "OK!\n\n";
	
	cout << "Creating great new service for real random numbers, using a RandomPublisher:\n";	
	Service<int> auto_random_service("test_pubsub.auto_random_service", 0);
	RandomPublisher* random_publisher = new RandomPublisher(&auto_random_service);
	cout << "  Name: " << auto_random_service.get_name() << "\n";
	cout << "  Id:   " << auto_random_service.get_id() << "\n";
	cout << "OK\n\n";
	
	service_list.add(&auto_random_service);
	
	cout << "Showing all twice, do you see the difference?:\n";
	show_all(service_list);
	show_all(service_list);
	cout << "OK\n\n";
	
	
	cout << "Ok, we implemented a RandomPublisher that just uses the rand \n"
		<< "function, that is really overhead. Let's just use a CallPublisher:\n\n";
		
	cout << "Deleting old RandomPublisher:";
	delete random_publisher;
	show_all(service_list);
	show_all(service_list);
	cout << "We just get the old random value now! OK!\n\n";
	
	cout << "Create and register (auto) the new CallPublisher:\n";
	new CallPublisher<int>(&auto_random_service, rand);
	show_all(service_list);
	show_all(service_list);	
	cout << "auto_random_service works again! OK!\n\n";
		
	cout << "Registering a CallSubscriber to show changes on random_service: ";
	new CallSubscriber<int>(&random_service, show_int);
	cout << "OK\n\n";
	
	cout << "So lets change the random_service value two times:\n";
	random_service.receive(8);
	random_service.receive(10);
	cout << "Great, isn't it?\nOK\n\n";
	
	
	cout << "If we want to know the number_of_services in our service list \n"
		<< "we have a problem: it's a function inside a class, we have to use \n"
		<< "a ClassCallProvider for this:\n";
	Service<int> services_service("test_pubsub.services", 0);	
	new ClassCallPublisher<ServiceList, int>(&services_service, &service_list, &ServiceList::number_of_services);
	cout << "Registered services in service_list: " << services_service.publish() << "\n";
	cout << "Oh, we did not yet register our new service. Lets do that.\n";
	service_list.add(&services_service);
	cout << "Registered services in service_list: " << services_service.publish() << "\n";
	cout << "OK!\n\n";
	
	
	cout << "Lets add a subscriber to our random_service that sets the random \n"
		<< "seed of a new RandomPublisher (we connect this publisher to \n"
		<< "random_service) and use a ClassCallSubscriber for this:\n";
	random_publisher = new RandomPublisher(&auto_random_service);
	new ClassCallSubscriber<RandomPublisher, int>(&random_service, random_publisher,
		&RandomPublisher::set_random_seed);
	cout << "So lets change the random_service value two times:\n";
	random_service.receive(12);
	random_service.receive(14);
	cout << "OK\n\n";
	
	
	return 0;
}
