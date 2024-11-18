/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asohrabi <asohrabi@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:39:07 by asohrabi          #+#    #+#             */
/*   Updated: 2024/11/18 15:42:00 by asohrabi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

#include "Request.hpp"
#include "Response.hpp"
#include "SystemCallError.hpp"
#include "CGIHandler.hpp"
#include "ServerBlock.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>

class HttpHandler
{
	private:
		CGIHandler	_cgiHandler;
		std::string	_rootDir;
		ServerBlock	_serverBlock;

		bool		_isMethodAllowed(const std::string &method, const std::string &path);
        std::string	_getErrorPage(int statusCode);
        void		_validateRequest(const Request &req); // Validate the request against the configuration

	public:
		HttpHandler();
		HttpHandler(const std::string &rootDir, const ServerBlock &serverConfig);
		~HttpHandler();

		std::string	handleRequest(const Request &req);
		std::string	handleGET(const Request &req);
		std::string	handlePOST(const Request &req);
		std::string	handleDELETE(const Request &req);
		std::string	handleCGI(const Request &req);

		std::string	extractFilename(const std::string &disposition);
		void		saveFile(const std::string &filename, const std::string &fileData);
};

#endif
