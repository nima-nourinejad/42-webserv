/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:52:42 by asohrabi          #+#    #+#             */
/*   Updated: 2024/12/18 15:23:28 by nnourine         ###   ########.fr       */
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

class CGIHandler
{
	private:
		ServerBlock	_serverBlock;

	public:
		CGIHandler(ServerBlock &serverConfig);
		~CGIHandler();

		Response	execute(const Request &req);
};


#endif
