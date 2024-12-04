/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asohrabi <asohrabi@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 09:37:19 by nnourine          #+#    #+#             */
/*   Updated: 2024/12/04 17:02:02 by asohrabi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// Andrey: Check if there needs to be a default root in the server block
//         before defining the locations

// Andrey's comments on git:
// 1) If you specify a root directive in the server block,
//    it acts as the default root for all location blocks within that server.
//
// 2) If a specific location block has its own root directive,
//    that value will override the server block's root
//    for requests that match that location.
//
// 3) If you do not specify a root in the server block,
//    each location block must have its own root directive to serve files.
//
// 4) If no root directive is provided either in the server or location blocks,
//    Nginx will not know where to serve static files from, and requests
//    will likely result in a 404 Not Found or similar error.

// Ali: Each Location block might have its own root, error page, maxbodysize,
//      index
//
// (Solved) - Autoindex, compiling with valgrind works fine but without that, segfault.
// Ali: if the config file is loke this:
	// location /files/ {
	//     root /var/www;
	//     autoindex on;
	// }
//
//     so now in the /files page you should display a directory listing of all
//     the files and directories in /var/www/files/.
// (Solved)
// // Ali: Also add sth for return, alias, upload_path

// new
// Nima: I think maxbodysize in a specific location block should override in your side

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
	if (argc != 2)
	{
		std::cerr << "Usage: ./webserv <config_file>" << std::endl;
		return 1;
	}
	ConfigParser config;

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
