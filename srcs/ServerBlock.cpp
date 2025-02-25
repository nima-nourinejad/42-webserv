/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerBlock.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akovalev <akovalev@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/24 18:45:23 by akovalev          #+#    #+#             */
/*   Updated: 2025/02/14 14:05:53 by akovalev         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerBlock.hpp"

ServerBlock::ServerBlock()
{;
	_client_max_body_size = 0;
}

ServerBlock::~ServerBlock()
{
}

ServerBlock::ServerBlock(const ServerBlock& original)
{
	_server_name = original._server_name;
	_listen = original._listen;
	_locations = original._locations;
	_error_pages = original._error_pages;
	_host = original._host;
	_root = original._root;
	_client_max_body_size = original._client_max_body_size;
}

ServerBlock& ServerBlock::operator=(const ServerBlock& original)
{
	if (this != &original)
	{
		_server_name = original._server_name;
		_listen = original._listen;
		_locations = original._locations;
		_error_pages = original._error_pages;
		_host = original._host;
		_root = original._root;
		_client_max_body_size = original._client_max_body_size;
	}
	return *this;
}

std::string ServerBlock::getServerName() const
{
	return _server_name;
}

std::vector<uint16_t> ServerBlock::getListen() const
{
	return _listen;
}

std::vector<std::shared_ptr<LocationBlock>>& ServerBlock::getLocations() {
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

std::string ServerBlock::getRoot() const
{
	return _root;
}

void ServerBlock::setServerName(const std::string& server_name)
{
	if (server_name.empty())
		throw std::invalid_argument("Configuration error: Server name is empty");
	_server_name = server_name;
}

void ServerBlock::setListen(const std::vector<uint16_t>& listen)
{
	if (_listen.size() != 0 && listen.size() != 0)
		throw std::invalid_argument("Configuration error: Listen is already set");
	if (listen.empty())
		throw std::invalid_argument("Configuration error: Listen is empty");
	for (const uint16_t& port : listen)
	{
		if (port < 1 || port > 65535)
			throw std::invalid_argument("Configuration error: Port is out of range (1-65535)");
	}
	std::set<uint16_t> set(listen.begin(), listen.end()); 
	_listen.assign(set.begin(), set.end()); 
}

void ServerBlock::setLocations(const std::vector<std::shared_ptr<LocationBlock>>& locations)
{
	_locations = locations;
}


void ServerBlock::setErrorPage(int code, const std::string& page)
{
	if (page.empty())
		throw std::invalid_argument("Configuration error: Error page is empty");
	if (code < 100 || code > 599)
		throw std::invalid_argument("Configuration error: Error code is out of range");
	_error_pages[code] = page;
}

void ServerBlock::setHost(const std::string& host)
{
	std::regex ip_pattern("^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}$");

	if (!std::regex_match(host, ip_pattern))
		throw std::invalid_argument("Configuration error: Incorrect host format");
	size_t start = 0;
	for (int i = 0; i < 4; i++) 
	{
		size_t end = host.find('.', start);
		std::string octet = host.substr(start, end - start);
		int octet_int = std::stoi(octet);
		if (octet_int < 0 || octet_int > 255)
			throw std::invalid_argument("Configuration error: Incorrect host format");
		start = end + 1;
	}
	_host = host;
}

void ServerBlock::setClientMaxBodySize(std::string& client_max_body_size)
{
	if (client_max_body_size.empty() || !std::all_of(client_max_body_size.begin(), client_max_body_size.end(), ::isdigit))
		throw std::invalid_argument("Configuration error: Incorrect client_max_body_size format");

	try {
		int size = std::stoi(client_max_body_size);
		if (size < 0 || size > 1000000000)
			throw std::invalid_argument("Configuration error: client_max_body_size is out of range (0-1000000000)");
		_client_max_body_size = size;
	}
	catch (const std::exception&) {
		throw std::invalid_argument("Configuration error: Incorrect client_max_body_size format");
	}
}

void ServerBlock::setRoot(const std::string& root)
{
	if (root.empty())
		throw std::invalid_argument("Configuration error: Root is empty");
	if (!std::filesystem::exists(root) || !std::filesystem::is_directory(root))
		throw std::invalid_argument("Configuration error: Root is not a valid directory");
	_root = root;
}