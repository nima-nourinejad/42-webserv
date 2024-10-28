/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asohrabi <asohrabi@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:31:01 by asohrabi          #+#    #+#             */
/*   Updated: 2024/10/28 13:25:28 by asohrabi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

Request::Request() : _method(""), _path(""), _httpVersion(""), _headers(), _body("") {}

Request::Request(const std::string &rawRequest)
{
	try
	{
		parse(rawRequest);
	}
	catch (const SystemCallError &e)
	{
		std::cerr << "Error while parsing request: " << e.what() << std::endl;
		throw;
	}
}

Request::~Request() {}

std::string	Request::getMethod() const
{
	return _method;
}

std::string	Request::getPath() const
{
	return _path;
}

std::string	Request::getHttpVersion() const
{
	return _httpVersion;
}

std::string	Request::getHeader(const std::string &key) const
{
	std::map<std::string, std::string>::const_iterator	it = _headers.find(key);
	
	if (it != _headers.end())
		return it->second;
	return "";
}
std::string	Request::getBody() const
{
	return _body;
}

void	Request::parse(const std::string &rawRequest)
{
	std::istringstream	stream(rawRequest);
	std::string			line;

	if (!std::getline(stream, line))
		handleError("Failed to read request line");

	std::istringstream	requestLine(line);
	if (!(requestLine >> _method >> _path >> _httpVersion))
		handleError("Invalid request format");

	while (std::getline(stream, line) && line != "\r")
	{
		std::size_t	colon = line.find(':');

		if (colon != std::string::npos)
		{
			std::string	headerKey = line.substr(0, colon);
			std::string	headerValue = line.substr(colon + 2);  // Skip colon and space
			_headers[headerKey] = headerValue;
		}
	}

	if (_headers["Content-Length"] != "")
	{
		try
		{
			std::size_t	contentLength = std::stoi(_headers["Content-Length"]);

			_body.resize(contentLength);
			stream.read(&_body[0], contentLength);
			if (stream.gcount() != static_cast<std::streamsize>(contentLength))
				handleError("Content length mismatch");

		}
		catch (const std::invalid_argument &e)
		{
			handleError("Invalid Content-Length header");
		}
		catch (const std::out_of_range &e)
		{
			handleError("Content-Length out of range");
		}
	}
}
