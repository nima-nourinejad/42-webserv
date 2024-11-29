/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 09:37:19 by nnourine          #+#    #+#             */
/*   Updated: 2024/11/29 11:39:46 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// ServerBlock needs its own getRoot function so it searches for the root in the locations vector

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
	ConfigParser config;
	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv <config_file>" << std::endl;
		return 1;
	}
	// std::ifstream file("config/webserv.conf");
	std::ifstream file(argv[1]);
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
	std::cout << "Successfully parsed config" << std::endl;
	config.printServerConfig();

	//// Creating servers
	std::vector<std::unique_ptr<Server>> servers;
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
	
	//// Handling events
	std::size_t serverNum;
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
