/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 09:37:19 by nnourine          #+#    #+#             */
/*   Updated: 2024/11/15 17:24:31 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <sys/epoll.h>

int main ()
{
	ConfigParser config;
	std::ifstream file("config/webserv.conf");;
	if(!file.is_open())
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
	
	Server server(config.getServerBlocks().at(0));

	try
	{
		while (server.signal_status != SIGINT)
		{
			server.handleEvents ();
		}
	}
	catch(SocketException const & e)
	{
		e.log ();
	}
	catch(std::exception const & e)
	{
		Server::logError (e.what ());
	}
	catch(...)
	{
		Server::logError ("Unknown exception");
	}
	server.closeSocket ();
	return 0;
}
