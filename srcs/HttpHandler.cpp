/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asohrabi <asohrabi@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:39:26 by asohrabi          #+#    #+#             */
/*   Updated: 2024/11/05 11:33:00 by asohrabi         ###   ########.fr       */
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
		else if (req.getMethod() == "CGI")
			return handleCGI(req);

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
	Response	response;

	int fd = open(filePath.c_str(), O_RDONLY);

	if (fd == -1)
	{
		if (errno == EACCES)
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
		response.setHeader("Content-Type", "text/plain");
		return response.toString();
	}
	catch (const SystemCallError &e)
	{
		if (close(fd) == -1)
			handleError("close file descriptor");
		throw;
	}
}

std::string	HttpHandler::handlePOST(const Request& req)
{
	std::string body = req.getBody();
	Response	response;

	if (body.empty())
	{
		response.setStatusLine("HTTP/1.1 400 Bad Request");
		response.setBody("Empty body in POST request\n");
		return response.toString();
	}

	response.setStatusLine("HTTP/1.1 200 OK");
	response.setBody(body);
	response.setHeader("Content-Length", std::to_string(body.size()));
	response.setHeader("Content-Type", "text/plain");
	return response.toString();
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
