/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asohrabi <asohrabi@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:39:26 by asohrabi          #+#    #+#             */
/*   Updated: 2024/10/22 17:53:31 by asohrabi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpHandler.hpp"

HttpHandler::HttpHandler() : _rootDir("") {}

HttpHandler::HttpHandler(const std::string &rootDir) : _rootDir(rootDir) {}

HttpHandler::~HttpHandler() {}

std::string	HttpHandler::handleRequest(const Request& req)
{
	try
	{
		if (req.getMethod() == "GET")
			return handleGET(req);
		else if (req.getMethod() == "POST")
			return handlePOST(req);
		else if (req.getMethod() == "DELETE")
			return handleDELETE(req);

		return "HTTP/1.1 405 Method Not Allowed\r\n\r\nMethod Not Allowed\n";
	}
	catch (const SystemCallError &e)
	{
		return "HTTP/1.1 500 Internal Server Error\r\n\r\nError: " + std::string(e.what()) + "\n";
	}
}

std::string	HttpHandler::handleGET(const Request &req)
{
	std::string	filePath = _rootDir + req.getPath();
	int fd = open(filePath.c_str(), O_RDONLY);

	if (fd == -1)
		return "HTTP/1.1 404 Not Found\r\n\r\nFile not found\n";

	try
	{
		char		buffer[1024];
		std::string	content;
		ssize_t		bytesRead;

		while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0)
			content.append(buffer, bytesRead);

		if (close(fd) == -1)
			handleError("close file descriptor");

		std::ostringstream	response;
		response << "HTTP/1.1 200 OK\r\n"
				<< "Content-Length: " << content.size() << "\r\n"
				<< "Content-Type: text/plain\r\n"
				<< "\r\n"
				<< content;

		return response.str();
	}
	catch (const SystemCallError &e)
	{
		if (close(fd) == -1) {
			// Ensure file descriptor is closed even in case of an error
		}
		throw;  // Rethrow the error to be caught in the parent function
	}
}


std::string HttpHandler::handlePOST(const Request& req)
{
	std::string body = req.getBody();
    
    std::ostringstream responseStream;
    responseStream << "HTTP/1.1 200 OK\r\n"
                   << "Content-Length: " << body.size() << "\r\n"
                   << "Content-Type: text/plain\r\n"
                   << "\r\n"
                   << body;

    return responseStream.str();
}

std::string	HttpHandler::handleDELETE(const Request &req)
{
	std::string filePath = _rootDir + req.getPath();

	if (unlink(filePath.c_str()) == 0)
		return "HTTP/1.1 200 OK\r\n\r\nFile deleted successfully\n";
	else
		return "HTTP/1.1 404 Not Found\r\n\r\nFile not found or permission denied\n";
}

std::string HttpHandler::handleCGI(const Request &req)
{
    return _cgiHandler.execute(req);
}
