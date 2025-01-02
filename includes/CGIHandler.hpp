/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:52:42 by asohrabi          #+#    #+#             */
/*   Updated: 2025/01/02 15:02:15 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "Request.hpp"
#include "Response.hpp"
#include "SystemCallError.hpp"
#include "LocationBlock.hpp"
#include "ServerBlock.hpp"

#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sstream>

class HttpHandler;

class CGIHandler
{
	private:
		ServerBlock	_serverBlock;
		pid_t		_pid;

	public:
		CGIHandler();
		CGIHandler(ServerBlock &serverConfig);
		~CGIHandler();

		pid_t		getPid() const { return _pid; } // maybe not needed
		Response	execute(const Request &req);
};


#endif
