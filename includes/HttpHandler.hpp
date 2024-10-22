/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asohrabi <asohrabi@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:39:07 by asohrabi          #+#    #+#             */
/*   Updated: 2024/10/22 17:49:53 by asohrabi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

#include "Request.hpp"
#include "SystemCallError.hpp"
#include "CGIHandler.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class HttpHandler
{

	private:
		CGIHandler _cgiHandler;
		std::string	_rootDir;

	public:
		HttpHandler();
		HttpHandler(const std::string &rootDir);
		~HttpHandler();

		std::string	handleRequest(const Request &req);
		std::string	handleGET(const Request &req);
		std::string	handlePOST(const Request &req);
		std::string	handleDELETE(const Request &req);
		std::string	handleCGI(const Request &req);
};

#endif
