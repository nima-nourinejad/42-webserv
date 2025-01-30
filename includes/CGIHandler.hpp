/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asohrabi <asohrabi@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:52:42 by asohrabi          #+#    #+#             */
/*   Updated: 2025/01/30 13:30:47 by asohrabi         ###   ########.fr       */
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
#include <thread>

class HttpHandler;

class CGIHandler
{
	private:
		static constexpr int	TIMEOUT = 60;
		ServerBlock				_serverBlock;
		pid_t					_pid;

	public:
		CGIHandler();
		CGIHandler(ServerBlock &serverConfig);
		~CGIHandler();

		pid_t					getPid() const;
		Response				execute(const Request &req);
};


#endif
