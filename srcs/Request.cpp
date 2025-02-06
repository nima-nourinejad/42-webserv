/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:31:01 by asohrabi          #+#    #+#             */
/*   Updated: 2025/02/06 20:07:53 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

Request::Request() : _method(""), _path(""), _httpVersion(""), _headers(), _body("") {}

// Request::Request(const std::string &rawRequest)
// {
// 	try
// 	{
// 		parse(rawRequest);
// 	}
// 	catch(const SystemCallError &e)
// 	{
// 		std::cerr << " " << e.what() << std::endl;
// 		// throw;
// 	}
// }

Request::Request(const Request &req)
{
	_method = req._method;
	_path = req._path;
	_httpVersion = req._httpVersion;
	_headers = req._headers;
	_body = req._body;
}

Request	&Request::operator=(const Request &req)
{
	_method = req._method;
	_path = req._path;
	_httpVersion = req._httpVersion;
	_headers = req._headers;
	_body = req._body;
	return *this;
}

Request::Request(const std::string &rawRequest, int errorStatus)
{
	try
	{
		if (!errorStatus)
		{
			try
			{
				parse(rawRequest);
			}
			catch(const SystemCallError &e)
			{
				// std::cerr << " " << e.what() << std::endl;
				// // throw;

				//This part added by nnourine
				//////
				std::istringstream	stream(rawRequest);
				std::string			line;

				if (!std::getline(stream, line))
					handleError("Failed to read request line");

				std::istringstream	requestLine(rawRequest);
				
				if (!(requestLine >> _method >> _path >> _httpVersion))
					handleError("Invalid request format");
				_headers["Content-Length"] = "0";
				_body = "";
				////////
			}
		}
		else
		{
			std::istringstream	stream(rawRequest);
			std::string			line;

			if (!std::getline(stream, line))
				handleError("Failed to read request line");

			std::istringstream	requestLine(rawRequest);
			
			if (!(requestLine >> _method >> _path >> _httpVersion))
				handleError("Invalid request format");
			_headers["Content-Length"] = "0";
			_body = "";
		}
	}
	catch(const std::exception& e)
	{
		//This part added by nnourine
		_method = "UNKNOWN";
		_path = "/";
		_httpVersion = "HTTP/1.1";
		_headers["Content-Length"] = "0";
		_body = "";
	}
}

Request::~Request() {}

const std::string	&Request::getMethod() const { return _method; }

const std::string	&Request::getPath() const { return _path; }

const std::string	&Request::getHttpVersion() const { return _httpVersion; }

const std::string	&Request::getHeader(const std::string &key) const
{
	std::map<std::string, std::string>::const_iterator	it = _headers.find(key);
	
	static const std::string empty_str = "";

	return (it != _headers.end() ? it->second : empty_str);
}

const std::string	&Request::getBody() const { return _body; }

void	Request::parse(const std::string &rawRequest)
{
	std::istringstream	stream(rawRequest);
	std::string			line;

	if (!std::getline(stream, line))
		handleError("Failed to read request line");

	std::istringstream	requestLine(line);

	if (!(requestLine >> _method >> _path >> _httpVersion))
		handleError("Invalid request format");

	// Handle headers
	while (std::getline(stream, line) && line != "\r")
	{
		std::size_t	colon = line.find(':');

		if (colon != std::string::npos)
		{
			std::string	headerKey = line.substr(0, colon);
			std::string	headerValue = line.substr(colon + 1, line.substr(colon + 2).size() - 1);  // Skip colon and space
			_headers[headerKey] = headerValue;
		}
	}
	// Handle body
	if (!_headers["Content-Length"].empty())
	{
		try
		{
			std::size_t	contentLength = std::stoi(_headers["Content-Length"]);

			_body.resize(contentLength);
			stream.read(&_body[0], contentLength);
			if (stream.gcount() != static_cast<std::streamsize>(contentLength))
				handleError("Content length mismatch");
		}
		catch(const std::invalid_argument &e)
		{
			handleError("Invalid Content-Length header");
		}
		catch(const std::out_of_range &e)
		{
			handleError("Content-Length out of range");
		}
	}
}
