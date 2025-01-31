/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 09:37:19 by nnourine          #+#    #+#             */
/*   Updated: 2025/01/31 18:32:20 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


// For example, check how does server_name work.
// We’ve shared with you a small tester. It’s not mandatory to pass it
// if everything works fine with your browser and tests, but it can help
// you hunt some bugs.


// when a filename that existed in the upload file want to be posted again, throw 409

// check the host from the request and compare it with the server_name in the config file

// add server name with the ip to the /etc/hosts file and after exiting the server, remove it

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
	
	std::string	configPath;

	if (argc > 2)
	{
		std::cerr << "Error: too many arguments" << std::endl;
		return 1;
	}

	if (argc ==1)
		configPath = "config/config.conf";
	else
		configPath = argv[1];
	
	ConfigParser 	config;
	std::ifstream	file(configPath.c_str());

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

	int server_count = 0;
	for (size_t i = 0; i < config.getServerBlocks().size(); i++)
	{
		server_count += config.getServerBlocks().at(i).getListen().size();
	}

	if (server_count == 0)
		return 0;
	static constexpr int TOTAL_FD = 1000;
	
	int max_fd = TOTAL_FD / server_count;

	std::vector<std::unique_ptr<Server>>	servers;

	for(std::size_t i = 0; (i < config.getServerBlocks().size() && !sigInt(servers)); i++)
	{
		try
		{
			for (unsigned int j = 0; j < config.getServerBlocks().at(i).getListen().size(); j++)
			{
				try
				{
					servers.push_back(std::make_unique<Server>(config.getServerBlocks().at(i), config.getServerBlocks().at(i).getListen().at(j), max_fd));
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

	std::size_t	total_servers = servers.size();
	if (total_servers == 0)
	{
		Server::logError("No servers were created");
		return 1;
	}

	std::size_t	serverNum = 0;

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
