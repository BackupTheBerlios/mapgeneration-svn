/*******************************************************************************
* MapGeneration Project - Creating a road map for the world.                   *
*                                                                              *
* Copyright (C) 2004-2005 by Rene Bruentrup and Bjoern Scholz                  *
* Licensed under the Academic Free License version 2.1                         *
*******************************************************************************/


#include "configuration.h"

#include "mlog.h"


namespace mapgeneration_util
{		

	Configuration::Configuration(std::string filename, 
		pubsub::ServiceList* service_list,
		std::string entry_header,
		std::vector<Parameter>* default_parameter_vector)
	: _default_parameter_map(),	
		_default_parameters_given(false),
		_filename(filename), 
		_service_list(service_list), _entry_header(entry_header)
	{
		if (default_parameter_vector)
		{
			_default_parameters_given = true;
			
			std::vector<Parameter>::iterator defaults_iter = 
				default_parameter_vector->begin();
			std::vector<Parameter>::iterator defaults_iter_end = 
				default_parameter_vector->end();
			for (; defaults_iter != defaults_iter_end; ++defaults_iter)
			{			
				_default_parameter_map.insert(
					std::pair<std::string, Parameter>
					(
						defaults_iter->_name,
						*defaults_iter
					)
				);
			}
		}
		
	}
	
	
	bool
	Configuration::read_configuration()
	{  
		bool parse_result = parse(_filename.c_str());
		
		if (parse_result == false)
		{
			mlog(MLog::warning, "Configuration")
				<< "Could not parse configuration from file.\n";
			return false;
		}

		D_ParameterMap::iterator parameter_iter = _parameter_map.begin();
		D_ParameterMap::iterator parameter_iter_end = _parameter_map.end();
		for (; parameter_iter != parameter_iter_end; ++parameter_iter)
		{
			std::string type;			
			std::map< std::string, std::string>::iterator attributes_iter =
				parameter_iter->second._attribute_map.find("type");
			if (attributes_iter != parameter_iter->second._attribute_map.end())
				type = attributes_iter->second;
			else
			{
				mlog(MLog::warning, "Configuration")
					<< "Parameter without type attribute found, assuming "
					<< "string!\n";
				type = "string";
			}
			
			if (type!="bool" && type!="double" && type!="int" && type!="string")
			{
				mlog(MLog::error, "Configuration")
					<< "Parameter with unknown type found. Assuming string!\n";
				type = "string";
			}
			
			parameter_iter->second._type = type;
			
			if (_default_parameters_given)
			{
				D_ParameterMap::iterator default_parameter_iter =
					_default_parameter_map.find(parameter_iter->first);
				if (default_parameter_iter == _default_parameter_map.end())
				{
					mlog(MLog::warning, "Configuration")
						<< "Unknown parameter:\n" << parameter_iter->first
						<< ", " << parameter_iter->second._type << ", "
						<< parameter_iter->second._value << "\n";
				} else 
				{
					if (default_parameter_iter->second._type != 
					parameter_iter->second._type)
					{
						mlog(MLog::error, "Configuration")
							<< "Parameter " << parameter_iter->first << " has wrong "
							<< "type:\nIs: " << parameter_iter->second._type
							<< "  Should be: " << default_parameter_iter->second._type
							<< "\n";
						
						parse_result = false;
					}
					
					_default_parameter_map.erase(default_parameter_iter);
				}
			}
			
		}


		D_ParameterMap::iterator default_parameter_iter = 
			_default_parameter_map.begin();
		D_ParameterMap::iterator default_parameter_iter_end = 
			_default_parameter_map.end();
		for (; default_parameter_iter != default_parameter_iter_end; 
			++default_parameter_iter)
		{
			mlog(MLog::warning, "Configuration")
				<< "Parameter " << default_parameter_iter->first << " not "
				<< "found in configuration file. Will insert default of "
				<< default_parameter_iter->second._value << "("
				<< default_parameter_iter->second._type << ")\n";
			_parameter_map.insert(*default_parameter_iter);
		}


		parameter_iter = _parameter_map.begin();
		parameter_iter_end = _parameter_map.end();
		for (; parameter_iter != parameter_iter_end; ++parameter_iter)
		{			
			pubsub::GenericService* generic_service;

			if (parameter_iter->second._type == "bool")
			{
				bool value = false;
				if (parameter_iter->second._value == "true")
					value = true;
				
				generic_service = new pubsub::Service<bool>
				(
					parameter_iter->second._name,
					value
				);
			} else if (parameter_iter->second._type == "double")
			{
				double value = atof(parameter_iter->second._value.c_str());
				generic_service = new pubsub::Service<double>
				(
					parameter_iter->second._name,
					value
				);
			} else if (parameter_iter->second._type == "int")
			{
				int value = atoi(parameter_iter->second._value.c_str());
				generic_service = new pubsub::Service<int>
				(
					parameter_iter->second._name,
					value
				);
			} else if (parameter_iter->second._type == "string")
			{
				generic_service = new pubsub::Service<std::string>
				(
					parameter_iter->second._name,
					parameter_iter->second._value
				);
			} else 
			{
				mlog(MLog::error, "Configuration")
					<< "Program error in type list!\n";
			}
			
			_service_list->add(generic_service);
		}
		
		//std::cout << *_service_list << std::endl;

		return parse_result;
	}

    
	void
	Configuration::characters(const unsigned char *text, unsigned int len)
	{
		std::string value(reinterpret_cast<const char*>(text), len);
		_parse_stack.back()._value = value;
	}
  

	void 
	Configuration::close(void)
	{
		_filestream.close();
	}


	void
	Configuration::comment(const unsigned char *text, unsigned len)
	{
	}


	void
	Configuration::endDocument(void)
	{
	}


	void
	Configuration::endElement(const unsigned char *name)
	{
		/* We should have all parts of a parameter now and
		 * will construct an entry for the _parameter_map.
		 */
		if (_parse_stack.back()._is_node)
		{
			Parameter parameter;
			
			std::string parameter_name;
			D_ParseStack::const_iterator
				parse_stack_iter = _parse_stack.begin();
			D_ParseStack::const_iterator 
				parse_stack_iter_end = _parse_stack.end();
	
			if (parse_stack_iter != parse_stack_iter_end)
				++parse_stack_iter;
	
			for (; parse_stack_iter != parse_stack_iter_end;
				++parse_stack_iter)
			{
				parameter_name.append(parse_stack_iter->_name);
				parameter_name.append(".");
			}
	
			if (parameter_name.size())
				parameter_name.erase(parameter_name.size()-1, 1);
			
			parameter._name = parameter_name;
			parameter._attribute_map = _parse_stack.back()._attribute_map;
			parameter._value = _parse_stack.back()._value;
			
			/* Each parameter is only saved once. All further appearances are
			 * ignored.*/
			_parameter_map.insert(
				std::pair<std::string, Parameter>
				(
					parameter._name, parameter
				)
			);
		}
		
		_parse_stack.pop_back();
	}


	bool
	Configuration::open(const char *resource)
	{
		_filestream.open(resource, std::fstream::in);
		if (_filestream.good())
			return true;
		else
			return false;
	}


	int
	Configuration::read(unsigned char *buffer, int len)
	{
		_filestream.read(reinterpret_cast<char*>(buffer), len);
		return _filestream.gcount();
	}
	
	
	int
	Configuration::read(unsigned char *buffer, size_t len)
	{		
		return read(buffer, (int)len);
	}


	void
	Configuration::startDocument(void)
	{
	}


	void
	Configuration::startElement(const unsigned char *name,
		const unsigned char **attr)
	{
		StackEntry stack_entry;
		
		if (_parse_stack.size())
			_parse_stack.back()._is_node = false;

		stack_entry._is_node = true;

		std::string parameter_name(reinterpret_cast<const char*>(name));
		stack_entry._name = parameter_name;
		
		const unsigned char** attributes = attr;		
		while(attributes && *attributes)
		{
			std::string attribute(reinterpret_cast<const char*>(*attributes));
			++attributes;
			std::string value(reinterpret_cast<const char*>(*attributes));
			++attributes;			

			/* If attributes appear more than once, only the first
			 * appearance is used. */
			stack_entry._attribute_map.insert
			(
				std::pair<std::string, std::string>
				(
					attribute,
					value
				)
			);
		}
		
		_parse_stack.push_back(stack_entry);
	}
        
} // namespace mapgeneration_util
