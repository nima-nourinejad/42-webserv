/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerBlock.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akovalev <akovalev@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/24 18:45:23 by akovalev          #+#    #+#             */
/*   Updated: 2024/11/12 18:31:36 by akovalev         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerBlock.hpp"

ServerBlock::ServerBlock(/* args */)
{
	_listen = 0;
	_client_max_body_size = 0;
}

ServerBlock::~ServerBlock()
{
}

std::string ServerBlock::getServerName() const
{
	return _server_name;
}

int ServerBlock::getListen() const
{
	return _listen;
}

std::vector<LocationBlock>& ServerBlock::getLocations()
{
	return _locations;
}

std::map<int, std::string> ServerBlock::getErrorPages() const
{
	return _error_pages;
}

std::string ServerBlock::getHost() const
{
	return _host;
}

size_t ServerBlock::getClientMaxBodySize() const
{
	return _client_max_body_size;
}

void ServerBlock::setServerName(const std::string& server_name)
{
	if (server_name.empty())
		throw std::invalid_argument("Server name is empty");
	_server_name = server_name;
}

void ServerBlock::setListen(int listen)
{
	if (listen < 1 || listen > 65535)
		throw std::invalid_argument("Listen port is out of range");
	_listen = listen;
}

void ServerBlock::setLocations(const std::vector<LocationBlock>& locations)
{
	_locations = locations;
}


void ServerBlock::setErrorPage(int code, const std::string& page)
{
	if (page.empty())
		throw std::invalid_argument("Error page is empty");
	if (code < 100 || code > 599)
		throw std::invalid_argument("Error code is out of range");
	_error_pages[code] = page;
}

void ServerBlock::setHost(const std::string& host)
{
	std::regex ip_pattern("^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}$");

	if (!std::regex_match(host, ip_pattern))
		throw std::invalid_argument("Incorrect host format");
	size_t start = 0;
	for (int i = 0; i < 4; i++) {
		size_t end = host.find('.', start);
		std::string octet = host.substr(start, end - start);
		int octet_int = std::stoi(octet);
		if (octet_int < 0 || octet_int > 255)
			throw std::invalid_argument("Incorrect host format");
		start = end + 1;
	}
	_host = host;
}

void ServerBlock::setClientMaxBodySize(std::string& client_max_body_size)
{
	if (client_max_body_size.empty() || !std::all_of(client_max_body_size.begin(), client_max_body_size.end(), ::isdigit)) 
		throw std::invalid_argument("Incorrent client max body size format");
	else
		_client_max_body_size = std::stoi(client_max_body_size);
}
