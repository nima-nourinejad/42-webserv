/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asohrabi <asohrabi@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 17:26:33 by asohrabi          #+#    #+#             */
/*   Updated: 2025/01/02 13:59:22 by asohrabi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response() : _statusLine("HTTP/1.1 200 OK") {}

Response::Response(const std::string &statusLine) : _statusLine(statusLine) {}

Response::Response(const Response &other)
{
	_statusLine = other._statusLine;
	_headers = other._headers;
	_body = other._body;
}

Response	&Response::operator=(const Response &other)
{
	if (this != &other)
	{
		_statusLine = other._statusLine;
		_headers = other._headers;
		_body = other._body;
	}
	return *this;
}

Response::~Response() {}

void	Response::setStatusLine(const std::string &statusLine)
{
	_statusLine = statusLine;
}

void	Response::setHeader(const std::string &key, const std::string &value)
{
	_headers[key] = value;
}

void	Response::setBody(const std::string body)
{
	_body = body;
	_headers["Content-Length"] = std::to_string(body.size());
}

std::string	Response::getStatusLine() const { return _statusLine; }

std::string	Response::getRawHeader() const
{
	std::ostringstream	header;

	for (const auto &pair : _headers)
	{
		if (pair.first != "Content-Length")
			header << pair.first << ": " << pair.second << "\r\n";
	}
	return header.str();
}

std::string	Response::getBody() const { return _body; }

// size_t	Response::getMaxBodySize() const { return _maxBodySize; }

std::string	Response::toString() const
{
	std::ostringstream	response;

	response << _statusLine << "\r\n";
	for (const auto &header : _headers)
		response << header.first << ": " << header.second << "\r\n";
	response << "\r\n" << _body;
	return response.str();
}
