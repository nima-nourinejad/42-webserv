/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:39:26 by asohrabi          #+#    #+#             */
/*   Updated: 2024/11/29 16:59:50 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpHandler.hpp"

// HttpHandler::HttpHandler() : _rootDir("") , _serverBlock(*(new ServerBlock)) {}

HttpHandler::HttpHandler(ServerBlock &serverConfig)
	: _rootDir(serverConfig.getLocations()[0].getRoot()), _serverBlock(serverConfig)
	{
		//creating a default state of map
		_errorPages[404]="default_404.html";
		_errorPages[500]="default_403.html";

		//filling the map with the error pages from the server block
		// std::map<int, std::string>::iterator errorPages_it = serverConfig.getErrorPages().begin();
		// while (errorPages_it != serverConfig.getErrorPages().end())
		// {
		// 	_errorPages[errorPages_it->first] = errorPages_it->second;
		// 	errorPages_it++;
		// }
		for (const auto &errorPage : serverConfig.getErrorPages())
			_errorPages[errorPage.first] = errorPage.second;
	}

HttpHandler::~HttpHandler() {std::cout << "HttpHandler destructor" << std::endl;}

bool	HttpHandler::_isMethodAllowed(const std::string &method, const std::string &path)
{
	for (const auto &location : _serverBlock.getLocations())
	{
		if (path.find(location.getLocation()) == 0) // Match location prefix
		{
			// const std::vector<std::string>	&allowedMethods = location.getLimitExcept();
			const auto	&allowedMethods = location.getLimitExcept();
			std::cout << "..allowedMethods: " << allowedMethods[0] << std::endl;
			bool		isMethodAllowed = std::find(allowedMethods.begin(), allowedMethods.end(), method) != allowedMethods.end();
			
			std::cout << "..isMethodAllowed: " << isMethodAllowed << std::endl;
			return (isMethodAllowed);
		}
	}
	return false;
}

std::string	HttpHandler::_getErrorPage(int statusCode)
{
	const std::map<int, std::string>	&errorPages = _serverBlock.getErrorPages();
	// const auto	&errorPages = _serverBlock.getErrorPages();
	
	std::cout << "we are here" << std::endl;
	if (errorPages.find(statusCode) != errorPages.end())
	{
		std::cout << "we are here 2" << std::endl;
		return errorPages.at(statusCode); // Custom error page
	}
	std::cout << "we are not here" << std::endl;
	return "dummy.html";         // Default fallback
}

std::string	HttpHandler::_validateRequest(const Request &req)
{
	std::string	method = req.getMethod();
	std::string	path = req.getPath();

	if (!_isMethodAllowed(method, path))
		return "HTTP/1.1 405 Method Not Allowed\r\n\r\nMethod Not Allowed\n";
	for (const auto &location : _serverBlock.getLocations())
	{
		if (req.getPath().find(location.getLocation()) == 0)
		{
			if (!location.getCgiPath().empty() && !std::filesystem::exists(location.getCgiPath()))
				return "Invalid CGI Path"; // should be like a http response

			if (!location.getRoot().empty() && !std::filesystem::exists(location.getRoot()))
				return "Invalid root path"; // should be like a http response
		}
	}
	return "Ok";
}

std::string	HttpHandler::createResponse(const std::string &request)
{
	Request	req(request);

	return handleRequest(req);
}

std::string	HttpHandler::handleRequest(const Request &req)
{
	try
	{
		// std::string validation = _validateRequest(req);
		// if (validation != "Ok")
		// 	return validation;
		if (req.getMethod() == "GET")
			return handleGET(req);
		else if (req.getMethod() == "POST")
			return handlePOST(req);
		else if (req.getMethod() == "DELETE")
			return handleDELETE(req);
		else if (req.getMethod() == "CGI") // check if for the CGI, it should start with CGI
			return handleCGI(req);

		return _getErrorPage(405); // Method not allowed
	}
	catch (const std::runtime_error &e)
    {
        return e.what(); // Handle runtime errors (e.g., method not allowed)
    }
	catch(const SystemCallError &e)
	{
		return _getErrorPage(500); // Internal server error
	}
}
std::string readFileError(std::string const & path)
{
	std::ifstream file(path.c_str());
	if (!file.is_open())
		throw SystemCallError("Failed to open file");
	std::stringstream read;
	read << file.rdbuf();
	file.close();
	return read.str();
}
std::string	HttpHandler::handleGET(const Request &req)
{
	std::cout << "we are at the start handleGet" << std::endl;
	std::string	filePath = _rootDir + req.getPath();
	Response	response;
	std::cout << filePath << std::endl;
	int fd = open(filePath.c_str(), O_RDONLY);

	if (fd == -1)
	{
		std::cout << "fd is -1" << std::endl;
		int			statusCode = (errno == EACCES) ? 403 : 404;
		
		// std::string	errorPage = _getErrorPage(statusCode);
		// std::cout << "errorPage: " << errorPage << std::endl;
		std::string	errorPage = _errorPages.at(statusCode);
		fd = open((_rootDir + "/" + errorPage).c_str(), O_RDONLY);
		// response.setBody(readFileError(_rootDir + "/" + errorPage));

		// response.setStatusLine("HTTP/1.1 " + std::to_string(statusCode));
		// // response.setBody("Error: " + errorPage + "\n");
		// return response.toString();
	}

	try
	{
		std::cout << "we are at the try handleGet" << std::endl;
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

// HttpHandler &HttpHandler::operator=(HttpHandler const &src)
// {
// 	if (this != &src)
// 	{
// 		_cgiHandler = src._cgiHandler;
// 		_rootDir = src._rootDir;
// 		_serverBlock = src._serverBlock;
// 	}
// 	return *this;
// }
