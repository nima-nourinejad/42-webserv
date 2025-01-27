/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 09:37:19 by nnourine          #+#    #+#             */
/*   Updated: 2025/01/27 13:18:16 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//replace plain with html
//if node script run with pdf after stucking in the middle of the pdf if close the script the server will shutdown

//for downloading files. first read the subject if we need to handle downloads, if not, try to somehow show and delete a file


//for downloads we need to check if the file exists. Then find the MIME type and send the file with the correct MIME type with this header:
// Content-Type: application/MIME_type

//OPTIONS method is mandatory for deleting files
// before the actual DELETE we receive an OPTIONS request for CORS
// OPTIONS /uploads/map HTTP/1.1
// Host: 127.1.0.0:4242
// User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:132.0) Gecko/20100101 Firefox/132.0
// Accept: */*
// Accept-Language: en-US,en;q=0.5
// Accept-Encoding: gzip, deflate, br, zstd
// Access-Control-Request-Method: DELETE
// Origin: https://nima-nourinejad.github.io
// Connection: keep-alive
// Sec-Fetch-Dest: empty
// Sec-Fetch-Mode: cors
// Sec-Fetch-Site: cross-site
// Priority: u=4
//
// we should respond with somthing like this:
// HTTP/1.1 204 No Content
// Access-Control-Allow-Origin: *
// Access-Control-Allow-Methods: GET, POST, DELETE, OPTIONS
// Access-Control-Allow-Headers: Content-Type
// Access-Control-Max-Age: 86400

// for uploads/downloads we should do prefix match



#include "Server.hpp"

bool sigInt(std::vector<std::unique_ptr<Server>> const & servers)
{
	for (std::size_t i = 0; i < servers.size(); i++)
	{
		if (servers[i]->signal_status == SIGINT)
			return true;
	}
	return false;
}

int main(int argc, char **argv)
{
	
	std::string configPath;
	if (argc > 2)
	{
		std::cerr << "Error: too many arguments" << std::endl;
		return 1;
	}
	//Double check below with the subject
	if (argc ==1)
		configPath = "config/config.conf";
	else
		configPath = argv[1];
	
	ConfigParser config;

	// std::ifstream file("config/webserv.conf");
	std::ifstream file(configPath.c_str());
	if (!file.is_open())
	{
		std::cerr << "Error: could not open file" << std::endl;
		return 1;
	}

	try
	{
		config.parseConfig(file);
	}

	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return 1;
	}

	// std::cout << "Successfully parsed config" << std::endl;
	// config.printServerConfig();

	// Creating servers
	std::vector<std::unique_ptr<Server>>	servers;

	for(std::size_t i = 0; (i < config.getServerBlocks().size() && !sigInt(servers)); i++)
	{
		try
		{
			// servers.push_back(std::make_unique<Server>(config.getServerBlocks().at(i)));
			for (unsigned int j = 0; j < config.getServerBlocks().at(i).getListen().size(); j++)
			{
				try
				{
					servers.push_back(std::make_unique<Server>(config.getServerBlocks().at(i), config.getServerBlocks().at(i).getListen().at(j)));
				}
				catch(SocketException const & e)
				{
					e.log();
				}
				catch(std::exception const & e)
				{
					Server::logError(e.what());
				}
				catch(...)
				{
					Server::logError("Unknown exception during server creation for config block " + std::to_string(i + 1) + " and port " + std::to_string(config.getServerBlocks().at(i).getListen().at(j)));
				}
				
			}
		}
		catch(SocketException const & e)
		{
			e.log();
		}
		catch(std::exception const & e)
		{
			Server::logError(e.what());
		}
		catch(...)
		{
			Server::logError("Unknown exception during server creation for config block " + std::to_string(i + 1));
		}
	}
	// Handling events
	std::size_t	serverNum;

	while (!sigInt(servers))
	{
		if (servers.empty())
			break;
		if (serverNum >= servers.size())
			serverNum = 0;
		try
		{
			servers[serverNum]->handleEvents();
			serverNum++;
		}
		catch(SocketException const & e)
		{
			e.log();
			servers.erase(servers.begin() + serverNum);
		}
		catch(std::exception const & e)
		{
			Server::logError(e.what());
			servers.erase(servers.begin() + serverNum);
		}
		catch(...)
		{
			Server::logError("Unknown exception during event handling");
			servers.erase(servers.begin() + serverNum);
		}
	}
}
