/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asohrabi <asohrabi@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:52:42 by asohrabi          #+#    #+#             */
/*   Updated: 2024/11/05 11:31:34 by asohrabi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "Request.hpp"
#include "Response.hpp"
#include "SystemCallError.hpp"
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
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
