/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:39:07 by asohrabi          #+#    #+#             */
/*   Updated: 2024/12/11 12:22:15 by nnourine         ###   ########.fr       */
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
		CGIHandler					_cgiHandler;
		std::string					_rootDir;
		ServerBlock					_serverBlock;
		// last one might be better to be const, then
		// getlocations function in serverblock needs to change too
		std::map<int, std::string>	_errorPages;
		size_t						_maxBodySize; // maybe not needed

		bool						_isMethodAllowed(const std::string &method, const std::string &path);
		Response					_getErrorPage(int statusCode);
		std::string					_validateRequest(const Request &req);

	public:
		HttpHandler(ServerBlock &serverConfig); //if serverblock became const, here too
		~HttpHandler();

		Response					createResponse(const std::string &request);
		Response					handleRequest(const Request &req);
		Response					handleGET(const Request &req);
		Response					handleFileRequest(const std::string &filePath);
		Response					handlePOST(const Request &req);
		Response					handleDELETE(const Request &req);
		Response					handleCGI(const Request &req);

		std::string					extractFilename(const std::string &disposition);
		void						saveFile(const std::string &filename, const std::string &fileData);
};

#endif
