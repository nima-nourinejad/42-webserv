/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerBlock.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akovalev <akovalev@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/24 17:53:25 by akovalev          #+#    #+#             */
/*   Updated: 2025/01/09 18:15:33 by akovalev         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERBLOCK_HPP
#define SERVERBLOCK_HPP

#include <iostream>
#include <vector>
#include <map>
#include "LocationBlock.hpp"
#include <stdint.h>
#include <string>
#include <algorithm>

class ServerBlock
{
	private:
		std::string					_server_name;
		std::vector<uint16_t>		_listen; //ports, range 1-65535
		//std::vector<LocationBlock>	_locations;
		std::vector<std::shared_ptr<LocationBlock>> _locations;
		std::map<int, std::string>	_error_pages; // the way map is used, it overwrites the value if the key is the same, so maybe additional check is needed
		std::string					_host; // IP address, e.g. 127.0.0.1
		size_t						_client_max_body_size; // size in bytes, needs to be converted if in human-readable format
		std::string					_root; // path to the root directory
	public:
		ServerBlock(/* args */);
		~ServerBlock();
		ServerBlock(const ServerBlock& original); // copy constructor
		ServerBlock& operator=(const ServerBlock& copy); // assignment operator
		
		std::string					getServerName() const;
		std::vector<uint16_t>		getListen() const;
		std::vector<std::shared_ptr<LocationBlock>>& getLocations();	
		std::map<int, std::string>	getErrorPages() const;
		std::string					getHost() const;
		size_t						getClientMaxBodySize() const;
		std::string					getRoot() const;
		void						setServerName(const std::string& server_name);
		void 						setListen(const std::vector<uint16_t>& listen);
 		void 						setLocations(const std::vector<std::shared_ptr<LocationBlock>>& locations);		void						setErrorPage(int code, const std::string& page);
		void						setHost(const std::string& host);
		void						setClientMaxBodySize(std::string& client_max_body_size);
		void						setRoot(const std::string& root);
};

#endif
