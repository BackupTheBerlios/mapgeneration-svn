/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <cc++/common.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "pubsub/servicesystem.h"

namespace mapgeneration_util
{
	class Configuration : public ost::XMLStream
	{
		public:
		
			class Parameter
			{
				public:
				
					Parameter()
					: _name(""), _type(""), _value("")
					{
					}
				
					Parameter(std::string name, std::string type, std::string value)
					: _name(name), _type(type), _value(value)
					{
					}
					
					std::map<std::string, std::string> _attribute_map;
					std::string _name;
					std::string _type;
					std::string _value;
			};

	
			Configuration(std::string filename, 
				pubsub::ServiceList* service_list,
				std::string entry_header = "config.",
				std::vector<Parameter>* default_parameter_vector = 0);


			bool
			read_configuration();


		private:
		
			struct StackEntry
			{
				std::map<std::string, std::string> _attribute_map;
				bool _is_node;
				std::string _name;
				std::string _value;
			};

			typedef std::map<std::string, Parameter> D_ParameterMap;
			typedef std::vector< StackEntry > D_ParseStack;


			D_ParameterMap _default_parameter_map;
			
			bool _default_parameters_given;
			
			std::string _entry_header;

			std::string _filename;
			
			std::fstream _filestream;
			
			D_ParameterMap _parameter_map;

			/**
			 * @brief The stack for the element structur. Needs to
			 * be a vector, because a stack only allows access to the top
			 * element.
			 */			
			D_ParseStack _parse_stack;
			
			
			pubsub::ServiceList* _service_list;


			void
			characters(const unsigned char *text, unsigned int len);
			
			
			void
			close(void);


			void
			comment(const unsigned char *text, unsigned len);


			void
			endDocument(void);
            
            
			void
			endElement(const unsigned char *name);


			bool
			open(const char *resource);


			int
			read(unsigned char *buffer, int len);
			
			
			int
			read(unsigned char *buffer, size_t len);


			void
			startDocument(void);
			
			
			void
			startElement(const unsigned char *name, const unsigned char **attr);

	 };

} // namespace mapgeneration_util

#endif // CONFIGURATION_H
