/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketException.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 09:37:31 by nnourine          #+#    #+#             */
/*   Updated: 2024/11/14 11:36:04 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SocketException.hpp"

int findType (std::string const & message)
{
	if (message == "Failed to find available slot for client")
		return FIND_EMPTY_SLOT;
	else if (message == "Failed to accept client")
		return ACCEPT_CLIENT;
	else if (message == "Failed to add to epoll")
		return ADD_EPOLL;
	else if (message == "Proxy : Failed to get address info" || message == "Proxy: No address info returned" || message == "Proxy: Invalid address family returned" || message == "Proxy: Failed to connect to upstream" || message == "Proxy: Connection closed by upstream")
		return BAD_GATEWAY;
	else if (message == "Proxy: Failed to create socket")
		return INTERNAL_ERROR;	
	return -1;
}

SocketException::SocketException (std::string const & message)
    : std::runtime_error (message + " : " + strerror (errno)), type (findType (message)), open_fd (-1){}

SocketException::SocketException (std::string const & message, int open_fd)
	: std::runtime_error (message + " : " + strerror (errno)), type (findType (message)), open_fd (open_fd) {}

void SocketException::log () const
{
	try
	{
		std::chrono::time_point<std::chrono::system_clock> timePoint = std::chrono::system_clock::now ();
		std::time_t timeInSeconds = std::chrono::system_clock::to_time_t (timePoint);
		std::ofstream logFile ("socket_error.log", std::ios::app);
		if (!logFile.is_open ())
		 	throw std::runtime_error ("Failed to open log file");
		logFile << std::put_time (std::localtime (&timeInSeconds), "%Y-%m-%d %H:%M:%S") << " : ";
		logFile << what () << std::endl;
		logFile.close ();
	}
	catch (std::exception const & e)
	{
		std::cerr << "Failed to log exception : " << e.what () << std::endl;
		std::cerr << "Original exception : " << what () << std::endl;
	}
}