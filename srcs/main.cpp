/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 09:37:19 by nnourine          #+#    #+#             */
/*   Updated: 2025/01/08 17:27:04 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//replace plain with html
//if node script run with pdf after stucking in the middle of the pdf if close the script the server will shutdown

//for downloading files. first read the subject if we need to handle downloads, if not, try to somehow show and delete a file

//check fore accetable MIME types
// {".pdf", "application/pdf"},
// {".zip", "application/zip"},
// {".jpg", "image/jpeg"},
// {".jpeg", "image/jpeg"},
// {".png", "image/png"},
// {".txt", "text/plain"},
// {".html", "text/html"},
// {".css", "text/css"},
// {".js", "application/javascript"},
// {".json", "application/json"}
// reject the upload and return a 400 Bad Request or 415 Unsupported Media Type response

//for downloads we need to check if the file exists. Then find the MIME type and send the file with the correct MIME type with this header:
// Content-Type: application/MIME_type

//Make sure css is working properly since static website is mandatory



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
			servers.push_back(std::make_unique<Server>(config.getServerBlocks().at(i)));
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
