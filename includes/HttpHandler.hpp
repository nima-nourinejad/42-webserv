/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asohrabi <asohrabi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:39:07 by asohrabi          #+#    #+#             */
/*   Updated: 2025/02/06 18:45:53 by asohrabi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPHANDLER_HPP
#define HTTPHANDLER_HPP

#include "Request.hpp"
#include "Response.hpp"
#include "SystemCallError.hpp"
#include "CGIHandler.hpp"
#include "ServerBlock.hpp"
#include "Configuration.hpp" //

#include <string>
#include <fstream>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <unordered_map>

const int DEF_MAX_BODY_SIZE = 1000000;

class HttpHandler
{
	private:
		CGIHandler						_cgiHandler;
		std::string						_rootDir;
		ServerBlock						_serverBlock;
		std::map<int, std::string>		_errorPages;
		size_t							_maxBodySize;
		std::string						_filePath;
		std::filesystem::path			_uploadPath;
		std::string						_serverName;
		int								_port;
		int								_locationFlag;

		bool							_isMethodAllowed(const Request &req);
		int								_validateRequest(const Request &req);
		std::string						_getFileName(const Request &req);
		bool							_isDownload(const Request &req);

	public:
		HttpHandler(ServerBlock &serverConfig, int port);
		~HttpHandler();

		Response						createResponse(const std::string &request);
		Response						handleRequest(const Request &req);
		Response						handleDownload(const Request &req);
		Response						handleGET(const Request &req);
		Response						handleFileRequest(const Request &req, const std::string &filePath);
		Response						handlePOST(const Request &req);
		Response						handleOPTIONS(const Request &req);
		Response						handleDELETE(const Request &req);
		Response						handleCGI(const Request &req);

		CGIHandler						getCGIHandler() const { return _cgiHandler; } // maybe not needed
		Response						getErrorPage(const Request &req, int statusCode);
		std::string						getCurrentTime();
		std::string						extractFileName(const std::string &disposition);
		void							saveFile(const std::string &filename, const std::string &fileData);
		std::string						readFileError(std::string const& path);
		std::string						getStatusMessage(int statusCode);
		size_t							getMaxBodySize(const std::string &request, int errorStatus);
		ServerBlock						getServerBlock() const { return _serverBlock; } // maybe not needed
		std::shared_ptr<LocationBlock>	findMatchedLocation(const Request &req);
		bool							isValidLines(const std::string &request);
};

#endif
