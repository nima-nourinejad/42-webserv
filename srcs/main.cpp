/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 09:37:19 by nnourine          #+#    #+#             */
/*   Updated: 2024/11/14 09:47:33 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <sys/epoll.h>

int main ()
{
	Server server(9001, "127.0.0.3", 1000);

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
