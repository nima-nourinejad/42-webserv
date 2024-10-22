/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asohrabi <asohrabi@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:52:42 by asohrabi          #+#    #+#             */
/*   Updated: 2024/10/22 17:50:18 by asohrabi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "Request.hpp"
#include "SystemCallError.hpp"
#include <string>
#include <unistd.h>   // for fork, execve, pipe, dup2
#include <sys/wait.h> // for waitpid
#include <fcntl.h>    // for open
#include <iostream>
#include <sstream>

class CGIHandler
{
private:
	std::string	_cgiPath;

public:
	CGIHandler();
	CGIHandler(const std::string &cgiPath);
	~CGIHandler();

	std::string	execute(const Request &req);
};


#endif
