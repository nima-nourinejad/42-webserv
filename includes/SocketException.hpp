/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketException.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 09:38:03 by nnourine          #+#    #+#             */
/*   Updated: 2024/11/14 11:35:50 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKETEXCEPTION_HPP
#define SOCKETEXCEPTION_HPP

#include <stdexcept>
#include <string>
#include <cstring>
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>

enum ErrorType
{
	FIND_EMPTY_SLOT,
	ADD_EPOLL,
	ACCEPT_CLIENT,
	INTERNAL_ERROR,
	BAD_GATEWAY,
};



class SocketException : public std::runtime_error
{
	  public:
	SocketException (std::string const & message);
	SocketException (std::string const & message, int open_fd);
	void log() const;
	int type;
	int open_fd;
};

#endif