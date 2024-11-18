/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asohrabi <asohrabi@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:39:26 by asohrabi          #+#    #+#             */
/*   Updated: 2024/11/18 16:15:01 by asohrabi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpHandler.hpp"

HttpHandler::HttpHandler() : _rootDir("") {}

HttpHandler::HttpHandler(const std::string &rootDir, const ServerBlock &serverConfig)
	: _rootDir(rootDir), _serverBlock(serverConfig) {}

HttpHandler::~HttpHandler() {}

bool	HttpHandler::_isMethodAllowed(const std::string &method, const std::string &path)
{
	// Check if the method is allowed for the requested path in the server configuration
	for (const auto &location : _serverBlock.getLocations())
	{
		if (path.find(location.getLocation()) == 0) // Match location prefix
		{
			const std::vector<std::string>	&allowedMethods = location.getLimitExcept();
			
			return (std::find(allowedMethods.begin(), allowedMethods.end(), method) != allowedMethods.end());
		}
	}
	return false;
}

std::string	HttpHandler::_getErrorPage(int statusCode)
{
	const std::map<int, std::string>	&errorPages = _serverBlock.getErrorPages();
	
	if (errorPages.find(statusCode) != errorPages.end())
		return errorPages.at(statusCode); // Custom error page
	return "default_error.html";         // Default fallback
}

void HttpHandler::_validateRequest(const Request &req)
{
	const std::string	&method = req.getMethod();
	const std::string	&path = req.getPath();

	if (!_isMethodAllowed(method, path))
		throw std::runtime_error("HTTP/1.1 405 Method Not Allowed\r\n\r\nMethod Not Allowed\n");
}

std::string	HttpHandler::handleRequest(const Request& req)
{
	try
	{
		_validateRequest(req);

		if (req.getMethod() == "GET")
			return handleGET(req);
		else if (req.getMethod() == "POST")
			return handlePOST(req);
		else if (req.getMethod() == "DELETE")
			return handleDELETE(req);
		else if (req.getMethod() == "CGI") // check if for the CGI, it should start with CGI
			return handleCGI(req);

		return "HTTP/1.1 405 Method Not Allowed\r\n\r\nMethod Not Allowed\n";
	}
	catch (const std::runtime_error &e)
    {
        return e.what(); // Handle runtime errors (e.g., method not allowed)
    }
	catch(const SystemCallError &e)
	{
		return "HTTP/1.1 500 Internal Server Error\r\n\r\nError: " + std::string(e.what()) + "\n";
	}
}

std::string	HttpHandler::handleGET(const Request &req)
{
	std::string	filePath = _rootDir + req.getPath();
	Response	response;

	int fd = open(filePath.c_str(), O_RDONLY);

	if (fd == -1)
	{
		int			statusCode = (errno == EACCES) ? 403 : 404;
		std::string	errorPage = _getErrorPage(statusCode);

		response.setStatusLine("HTTP/1.1 " + std::to_string(statusCode));
		response.setBody("Error: " + errorPage + "\n");
		return response.toString();
	}

	try
	{
		char		buffer[1024];
		std::string	content;
		ssize_t		bytesRead;

		while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0)
			content.append(buffer, bytesRead);

		if (close(fd) == -1)
			handleError("close file descriptor");

		if (bytesRead < 0)
		{
			response.setStatusLine("HTTP/1.1 500 Internal Server Error");
			response.setBody("Error reading file\n");
			return response.toString();
		}
		response.setStatusLine("HTTP/1.1 200 OK");
		response.setBody(content);
		response.setHeader("Content-Type", "text/html");
		return response.toString();
	}
	catch(const SystemCallError &e)
	{
		if (close(fd) == -1)
			handleError("close file descriptor");
		throw;
	}
}

std::string	HttpHandler::handlePOST(const Request &req)
{
	std::string	contentType = req.getHeader("Content-Type");
	Response	response;

	if (contentType.find("multipart/form-data") != std::string::npos)
	{
		// Extract the boundary parameter
		std::string			boundary = "--" + contentType.substr(contentType.find("boundary=") + 9);
		std::istringstream	bodyStream(req.getBody());
		std::string			line;

		while (std::getline(bodyStream, line))
		{
			if (line == boundary)
			{
				std::string	disposition, partContentType;

				std::getline(bodyStream, disposition);
				std::getline(bodyStream, partContentType);
				std::getline(bodyStream, line); // Empty line before part content

				// Parse disposition to check for filename(file upload)
				bool	isFileUpload = disposition.find("filename=") != std::string::npos;
				
				std::ostringstream	partData;

				// Read part data until boundary is reached
				while (std::getline(bodyStream, line)
						&& line != boundary && line != boundary + "--")
					partData << line << "\n";

				if (isFileUpload)
				{
					std::string	filename = extractFilename(disposition);

					saveFile(filename, partData.str());
				}
				else
					response.setBody(partData.str());
			}
		}

		response.setStatusLine("HTTP/1.1 200 OK");
		response.setHeader("Content-Type", "multipart/form-data");
		response.setHeader("Content-Length", std::to_string(response.getBody().size()));
	}
	else if (!req.getBody().empty())
	{
		response.setStatusLine("HTTP/1.1 200 OK");
		response.setBody(req.getBody());
		response.setHeader("Content-Length", std::to_string(req.getBody().size()));
		response.setHeader("Content-Type", "text/plain");
	}
	else
	{
		response.setStatusLine("HTTP/1.1 400 Bad Request");
		response.setBody("Empty body in POST request\n");
	}
	return response.toString();
}

std::string	HttpHandler::extractFilename(const std::string& disposition)
{
	size_t	pos = disposition.find("filename=");

	if (pos != std::string::npos)
	{
		std::string	filename = disposition.substr(pos + 10); // Skip "filename=\""
		size_t		endPos = filename.find('"');

		return filename.substr(0, endPos);
	}
	return "uploaded_file";
}


void	HttpHandler::saveFile(const std::string &filename, const std::string &fileData)
{
	std::ofstream	file(filename, std::ios::binary);

	file.write(fileData.c_str(), fileData.size());
	file.close();
}

std::string	HttpHandler::handleDELETE(const Request &req)
{
	std::string filePath = _rootDir + req.getPath();
	Response	response;

	if (unlink(filePath.c_str()) == 0)
	{
		response.setStatusLine("HTTP/1.1 200 OK");
		response.setBody("File deleted successfully\n");
	}
	else if (errno == EACCES)
	{
		response.setStatusLine("HTTP/1.1 403 Forbidden");
		response.setBody("Permission denied\n");
	}
	else if (errno == ENOENT)
	{
		response.setStatusLine("HTTP/1.1 404 Not Found");
		response.setBody("File not found\n");
	}
	return response.toString();
}

std::string HttpHandler::handleCGI(const Request &req)
{
	return _cgiHandler.execute(req);
}
