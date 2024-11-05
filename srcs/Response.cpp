/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asohrabi <asohrabi@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 17:26:33 by asohrabi          #+#    #+#             */
/*   Updated: 2024/11/05 12:51:46 by asohrabi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response() : _statusLine("HTTP/1.1 200 OK") {}

Response::Response(const std::string &statusLine) : _statusLine(statusLine) {}

Response::~Response() {}

void	Response::setStatusLine(const std::string &statusLine)
{
	_statusLine = statusLine;
}

void	Response::setHeader(const std::string &key, const std::string &value)
{
	_headers[key] = value;
}

void	Response::setBody(const std::string &body)
{
	_body = body;
	_headers["Content-Length"] = std::to_string(body.size());
}

std::string	Response::getBody() const
{
	return _body;
}

std::string	Response::toString() const
{
	std::ostringstream	response;

	response << _statusLine << "\r\n";
	for (const auto &header : _headers)
		response << header.first << ": " << header.second << "\r\n";
	response << "\r\n" << _body;
	return response.str();
}
