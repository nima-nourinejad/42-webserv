/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:39:07 by asohrabi          #+#    #+#             */
/*   Updated: 2025/01/28 18:34:49 by nnourine         ###   ########.fr       */
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
#include <unordered_map>

class HttpHandler
{
	private:
		CGIHandler						_cgiHandler;
		std::string						_rootDir;
		ServerBlock						_serverBlock;
		// last one might be better to be const, then
		// getlocations function in serverblock needs to change too
		std::map<int, std::string>		_errorPages;
		size_t							_maxBodySize; // maybe not needed
		std::string						_filePath;
		std::filesystem::path			_uploadPath;

		bool							_isMethodAllowed(const std::string &method, const std::string &path);
		std::string						_validateRequest(const Request &req);
		// std::shared_ptr<LocationBlock>	_findMatchedLocation(const Request &req);
		std::string						_getFileName(const Request &req);
		bool							_isDownload(const Request &req);

	public:
		HttpHandler(ServerBlock &serverConfig); //if serverblock became const, here too
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
		std::string						extractFilename(const std::string &disposition);
		void							saveFile(const std::string &filename, const std::string &fileData);
		std::string						readFileError(std::string const& path);
		std::string						getStatusMessage(int statusCode);
		size_t							getMaxBodySize(const std::string &request, int errorStatus);
		ServerBlock						getServerBlock() const { return _serverBlock; } // maybe not needed
		std::shared_ptr<LocationBlock>	_findMatchedLocation(const Request &req);
};

#endif
