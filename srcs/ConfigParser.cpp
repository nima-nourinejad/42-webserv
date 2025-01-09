/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akovalev <akovalev@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/24 18:51:19 by akovalev          #+#    #+#             */
/*   Updated: 2025/01/09 18:47:42 by akovalev         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"	

ConfigParser::ConfigParser()
{
}

ConfigParser::~ConfigParser()
{
}

std::vector<ServerBlock> ConfigParser::getServerBlocks() const
{
	return _server_blocks;
}

std::string tokenTypeToString(TokenType type)
{
	switch(type)
	{
		case TokenType::OPEN: return "OPEN";
		case TokenType::CLOSE: return "CLOSE";
		case TokenType::SEMICOLON: return "SEMICOLON";
		case TokenType::COMMENT: return "COMMENT";
		case TokenType::END_OF_FILE: return "END_OF_FILE";
		case TokenType::KEY_VALUE: return "KEY_VALUE";
		case TokenType::KEY_MULTI_VALUE: return "KEY_MULTI_VALUE";
		default: return "UNKNOWN";	
	}
}

void printTokens(const std::vector<Token>& tokens)
{
	int i = 0;
	for (const Token& token : tokens)
	{

		std::cout << "Token " << i++ << ": ";
		if (token.type == TokenType::KEY_VALUE)
			std::cout << "KEY_VALUE: " << token.key << " " << token.values[0] << std::endl;
		else if (token.type == TokenType::KEY_MULTI_VALUE)
		{
			std::cout << "KEY_MULTI_VALUE: " << token.key << " ";
			for (const std::string& val : token.values)
				std::cout << val << " ";
			std::cout << std::endl;
		}
		else
			std::cout << tokenTypeToString(token.type) << ": " << token.values[0] << std::endl;
	}
}


void ConfigParser::unexpectedToken(size_t i)
{
	std::cout << "Issue at token " << i	<< " " << tokenTypeToString(_tokens[i].type) << std::endl;
	//printServerConfig();
	throw std::runtime_error("Unexpected token");
}

void ConfigParser::parseServerBlock(size_t& index)
{
	bool serverBlockOpened = false;
	_server_blocks.push_back(ServerBlock());
	// std::cout << "Parsing server block at index " << index << " with token type " << tokenTypeToString(_tokens[index].type) << std::endl;
	index++;
	if (_tokens[index].type != TokenType::OPEN)
		unexpectedToken(index);
	while (index < _tokens.size())
	{
		if (_tokens[index].type == TokenType::OPEN)
		{
			if (serverBlockOpened)
				unexpectedToken(index);
			// std::cout << "Opening server block" << std::endl;
			serverBlockOpened = true;
		}
		else if (_tokens[index].type == TokenType::CLOSE)
		{
			if (!serverBlockOpened)
				unexpectedToken(index);
			// std::cout << "Closing server block" << std::endl;
			//printServerConfig();
			break;
		}
		else if (_tokens[index].key == "server_name")
		{
			_server_blocks[_server_blocks.size() - 1].setServerName(_tokens[index].values[0]);
		}
		else if (_tokens[index].key == "root")
		{
			_server_blocks[_server_blocks.size() - 1].setRoot(_tokens[index].values[0]);
		}
		else if (_tokens[index].key == "client_max_body_size")
		{
			_server_blocks[_server_blocks.size() - 1].setClientMaxBodySize(_tokens[index].values[0]);
		}
		else if (_tokens[index].key == "host")
		{
			_server_blocks[_server_blocks.size() - 1].setHost(_tokens[index].values[0]);
		}
		else if (_tokens[index].key == "error_page")
		{
			if (_tokens[index].values[0].empty() || !std::all_of(_tokens[index].values[0].begin(), _tokens[index].values[0].end(), ::isdigit))
				throw std::invalid_argument("Error code is not a number");
			_server_blocks[_server_blocks.size() - 1].setErrorPage(std::stoi(_tokens[index].values[0]), _tokens[index].values[1]);
		}
		else if (_tokens[index].key == "listen")
		{
			std::vector<uint16_t> ports;
			for (const std::string& port : _tokens[index].values)
			{
				if (port.empty() || !std::all_of(port.begin(), port.end(), ::isdigit))
					throw std::invalid_argument("Port is not a number");
				if (*port.begin() == '0')
					throw std::invalid_argument("Port starts with 0");
				ports.push_back(std::stoi(port));
			}
			_server_blocks[_server_blocks.size() - 1].setListen(ports);
		}
		else if (_tokens[index].key == "location")
		{
			// std::cout << "Parsing location block" << std::endl;
			parseLocationBlock(index);
		}
		else if (_tokens[index].type == TokenType::SEMICOLON)
		{
			if (_tokens[index - 1].type != TokenType::KEY_VALUE && _tokens[index - 1].type != TokenType::KEY_MULTI_VALUE)
				unexpectedToken	(index);
		}
		else if (_tokens[index].type != TokenType::COMMENT)
		{
			unexpectedToken(index);
		}
		index++;
	}
}

void ConfigParser::parseLocationBlock(size_t& index)
{
	bool locationBlockOpened = false;

	if ((_tokens[index].values[0] == ""))
	{
		std::cout << "Location block is empty" << std::endl;
		unexpectedToken(index);
	}
	// else if (_tokens[index].values[0].back() != '/')
	// {
	// 	std::cout << "Location block does not end with /" << std::endl;
	// 	unexpectedToken(index);
	// }

	index++;
	if (_tokens[index].type != TokenType::OPEN)
		unexpectedToken(index);
	auto location_block = std::make_shared<LocationBlock>(_tokens[index - 1].values[0]);
    _server_blocks.back().getLocations().push_back(location_block);
	auto& current_location = *_server_blocks.back().getLocations().back();
	
    while (index < _tokens.size()) {
        const auto& token = _tokens[index];

        if (token.type == TokenType::OPEN) {
            if (locationBlockOpened)
                unexpectedToken(index);
            locationBlockOpened = true;
        } else if (token.type == TokenType::CLOSE) {
            if (!locationBlockOpened)
                unexpectedToken(index);
            // std::cout << "Closing location block" << std::endl;
            return;
        } else if (token.key == "root") {
            current_location.setRoot(token.values[0]);
        } else if (token.key == "index") {
            current_location.setIndex(token.values[0]);
        } else if (token.key == "autoindex") {
            current_location.setAutoindex(token.values[0]);
        } else if (token.key == "cgi_pass") {
            current_location.setCgiPath(token.values[0]);
        } else if (token.key == "upload_path") {
            current_location.setUploadPath(token.values[0]);
        } else if (token.key == "proxy_pass") {
            current_location.setProxyPass(token.values[0]);
        } else if (token.key == "error_page") {
            current_location.setErrorPage(std::stoi(token.values[0]), token.values[1]);
        } else if (token.key == "cgi_ext") {
            current_location.setCgiExtension(token.values);
        } else if (token.key == "return") {
            current_location.setReturn(token.values[0], token.values[1]);
        } else if (token.key == "alias") {
            current_location.setAlias(token.values[0]);
        } else if (token.key == "limit_except") {
            current_location.setLimitExcept(token.values);
        } else if (token.key == "client_max_body_size") {
            current_location.setClientMaxBodySize(token.values[0]);
        } else if (token.type == TokenType::SEMICOLON) {
            if (_tokens[index - 1].type != TokenType::KEY_VALUE && 
                _tokens[index - 1].type != TokenType::KEY_MULTI_VALUE)
                unexpectedToken(index);
        } else if (token.type != TokenType::COMMENT) {
            unexpectedToken(index);
        }
		index++;
	}
}

void ConfigParser::tokenize(std::vector<Token>& tokens, std::ifstream& filepath)
{
	std::vector<std::string> singleValueKeys = {
		"server_name", "location", "client_max_body_size", "proxy_pass", 
		"root", "index", "autoindex", "cgi_pass", "upload_path", 
		"upload_store", "upload_max_file_size", "server", "host", "alias", "root"
	};

	std::vector<std::string> multiValueKeys = {
		"limit_except", "cgi_ext", "error_page", "return", "listen"
	};

	std::string word;
	while (filepath >> word)
	{
		//std::cout << word << std::endl;
		if (word  == "{")
		{
			tokens.push_back(Token(TokenType::OPEN, word));
		}
		else if (word == "}")
		{
			tokens.push_back(Token(TokenType::CLOSE, word));
		}
		else if (word == ";")
		{
			tokens.push_back(Token(TokenType::SEMICOLON, word));
		}
		else if (word == "#" || word.front() == '#')
		{
			if (word.front() == '#')
			{	
				std::string comment;
				std::getline(filepath, comment);
				tokens.push_back(Token(TokenType::COMMENT,(word + comment)));
			}
			else {
				tokens.push_back(Token(TokenType::COMMENT, word));
				std::string comment;
				std::getline(filepath, comment);
				tokens.push_back(Token(TokenType::COMMENT, comment));
			}
		}
		else if (std::find(singleValueKeys.begin(), singleValueKeys.end(), word) != singleValueKeys.end())
		{
			std::string value;
			if (word == "server")
			{
				tokens.push_back(Token(TokenType::KEY_VALUE, word, ""));
			}
			else if (filepath >> value)
			{
				if (value.back() == ';')
				{
					tokens.push_back(Token(TokenType::KEY_VALUE, word,(value.substr(0, value.size() - 1))));
					tokens.push_back(Token(TokenType::SEMICOLON, ";")); 
				}
				else if (word == "location")
				{
					tokens.push_back(Token(TokenType::KEY_VALUE, word, value));
				}
				else
					unexpectedToken(tokens.size());
			}
		}
		else if (std::find(multiValueKeys.begin(), multiValueKeys.end(), word) != multiValueKeys.end())
		{
			std::vector<std::string> values;
			std::string value;
			while (filepath >> value && value != "{")
			{
				if (value.back() == ';')
				{
					values.push_back(value.substr(0, value.size() - 1));
					tokens.push_back(Token(TokenType::KEY_MULTI_VALUE, word, values));
					tokens.push_back(Token(TokenType::SEMICOLON, ";"));
					break;
				}
				else
				{
					values.push_back(value);
				}
			}
			if (value == "{")
			{
				tokens.push_back(Token(TokenType::KEY_MULTI_VALUE, word, values));
				tokens.push_back(Token(TokenType::OPEN, value));
			}
		}		
		else
		{
			tokens.push_back(Token(TokenType::UNKNOWN, word));
			// printTokens(tokens);
			throw std::runtime_error("Unknown token");
		}
		
	}
	if (filepath.eof()) {
 	  tokens.push_back(Token(TokenType::END_OF_FILE, ""));
}
}

std::vector<ServerBlock>	ConfigParser::parseConfig(std::ifstream& filepath)
{
	tokenize(_tokens, filepath);
	// printTokens(_tokens);

	for (size_t i = 0; i < _tokens.size(); i++)
	{
		while (_tokens[i].type == TokenType::COMMENT)
			i++;
		if (_tokens[i].type == TokenType::END_OF_FILE)
			break;
		else
		if (_tokens[i].key != "server")
		{
			std::cout << "Issue at token " << i	<< " " << tokenTypeToString(_tokens[i].type) << std::endl;
			throw std::runtime_error("Expected 'server' block");
		}
		else
			parseServerBlock(i);
	}
	return _server_blocks;
}

void ConfigParser::printServerConfig()
{
	for (ServerBlock& server : _server_blocks)
	{
		std::cout << "--------------------------------" << std::endl;
		std::cout << "Server name: ";
		std::cout << server.getServerName() << std::endl;			
		std::cout << std::endl;
		std::cout << "Root: " << server.getRoot() << std::endl;
		std::cout << "Listen: " << std::endl;
		for (const uint16_t& port : server.getListen())
		{
			std::cout << port << " ";
		}
		std::cout << "Client max body size: " << server.getClientMaxBodySize() << std::endl;
		std::cout << "Host: " << server.getHost() << std::endl;
		std::cout << "Error pages: " << std::endl;
		for (const auto& page : server.getErrorPages())
		{
			std::cout << page.first << " " << page.second << " " << std::endl;
		}
		std::cout << "Locations: " << std::endl;
		for (const std::shared_ptr<LocationBlock>& location : server.getLocations())
		{
			location->printLocationBlock();
			std::cout << "--------------------------------" << std::endl;
			std::cout << std::endl;
		}
	}
}

// int main()
// {
// 	ConfigParser config;
// 	std::ifstream file("config/webserv.conf");;
// 	if (!file.is_open())
// 	{
// 		std::cerr << "Error: could not open file" << std::endl;
// 		return 1;
// 	}
// 	try
// 	{
// 		config.parseConfig(file);
// 	}
// 	catch(const std::exception& e)
// 	{
// 		std::cerr << e.what() << '\n';
// 		return 1;
// 	}
// 	std::cout << "Successfully parsed config" << std::endl;
// 	config.printServerConfig();
// 	return 0;
// }