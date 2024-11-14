/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Proxy.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 10:14:08 by nnourine          #+#    #+#             */
/*   Updated: 2024/11/14 11:36:29 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Proxy.hpp"

Proxy::Proxy() : host(""), port(0), fd(-1), status(PROXY_DISCONNECTED) {}

void Proxy::setHost(std::string const & host)
{
	this->host = host;
}

void Proxy::setPort(int port)
{
	this->port = port;
}

void Proxy::setAdress()
{
	struct addrinfo hints = {};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	struct addrinfo * result = nullptr;

	int error = getaddrinfo(host.c_str(), nullptr, &hints, &result);
	if (error != 0)
		throw SocketException("Proxy : Failed to get address info");
	if (result == nullptr || result->ai_addr == nullptr)
		throw SocketException("Proxy: No address info returned");
	if (result->ai_family != AF_INET)
	{
		freeaddrinfo(result);
		throw SocketException("Proxy: Invalid address family returned");
	}
	address = *reinterpret_cast<struct sockaddr_in *>(result->ai_addr);
	freeaddrinfo(result);
	address.sin_port = htons(port);
}

void Proxy::createSocket()
{
	fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (fd == -1)
		throw SocketException("Proxy: Failed to create socket");
}

void Proxy::connectToUpstream()
{
	if (connect(fd, reinterpret_cast<struct sockaddr *>(&address), sizeof(address)) == -1)
	{
		if (errno != EINPROGRESS)
			throw SocketException("Proxy: Failed to connect to upstream", fd);
	}
	else
		status = PROXY_CONNECTED;
}

bool Proxy::sameUpstream(std::string const & host, int port) const
{
	if (this->host == host && this->port == port)
		return true;
	return false;
}

void Proxy::closeProxy()
{
	if (fd != -1)
	{
		close(fd);
		fd = -1;
	}
	status = PROXY_DISCONNECTED;
	requestParts.clear();
	response.clear();
	std::memset(&address, 0, sizeof(address));
	port = 0;
	host.clear();
}

void Proxy::startProxy(std::string const & host, int port)
{
	try
	{
		if (status == PROXY_CONNECTED)
		{
			if (sameUpstream(host, port))
				return;
			closeProxy();
		}
		setHost(host);
		setPort(port);
		setAdress();
		createSocket();
		connectToUpstream();
	}
	catch (SocketException const & e)
	{
		e.log();
		closeProxy();
		if (e.type == PROXY_INTERNAL_ERROR)
			status = PROXY_INTERNAL_ERROR;
		else if (e.type == PROXY_BAD_GATEWAY)
			status = PROXY_BAD_GATEWAY;
	}
	catch (...)
	{
		closeProxy();
		status = PROXY_INTERNAL_ERROR;
	}
}


void Proxy::setRequest(std::vector<std::string> requestParts)
{
	this->requestParts = requestParts;
}

void Proxy::sendRequest()
{
	if (status != PROXY_CONNECTED && status != PROXY_SENDING_REQUEST)
		return;
	ssize_t bytes_sent = send(fd, requestParts[0].c_str(), requestParts[0].size(), MSG_DONTWAIT);
	if (bytes_sent == 0)
		throw SocketException("Proxy: connection closed by upstream");
	else if (bytes_sent > 0)
	{
		status = PROXY_SENDING_REQUEST;
		if (bytes_sent < static_cast<ssize_t>(requestParts[0].size()))
		{
			std::string remainPart = requestParts[0].substr(bytes_sent);
			requestParts[0] = remainPart;
		}
		else
			requestParts.erase(requestParts.begin());
	}
	if (requestParts.empty())
		status = PROXY_SENT_REQUEST;
}

void Proxy::receiveResponse()
{
	if (status != PROXY_SENT_REQUEST && status != PROXY_RECEIVING_UNKOWN_TYPE && status != PROXY_RECEIVING_NONCHUNKED && status != PROXY_RECEIVING_CHUNKED)
		return;
	char buffer[16384] = {};
	ssize_t bytes_received = recv(fd, buffer, sizeof(buffer), MSG_DONTWAIT);
	if (bytes_received == 0)
		throw SocketException("Proxy: connection closed by upstream");
	else if (bytes_received > 0)
	{
		response += std::string(buffer, bytes_received);
		if (status == PROXY_SENT_REQUEST)
		{
			if (response.find("Transfer-Encoding: chunked") != std::string::npos)
				status = PROXY_RECEIVING_CHUNKED;
			else
				status = PROXY_RECEIVING_NONCHUNKED;
		}
		if (status == PROXY_RECEIVING_CHUNKED)
		{
			if (response.find("\r\n0\r\n\r\n") != std::string::npos)
				status = PROXY_RECEIVED;
		}
		else if (status == PROXY_RECEIVING_NONCHUNKED)
		{
			if (response.find("\r\n\r\n") != std::string::npos)
				status = PROXY_RECEIVED;
		}
	}
}

void Proxy::communicateWithUpstream(std::vector<std::string> requestParts, std::string const & host, int port)
{
	try
	{
		startProxy(host, port);
		if (status != PROXY_CONNECTED)
			return;
		setRequest(requestParts);
		sendRequest();
		receiveResponse();
	}
	catch (SocketException const & e)
	{
		e.log();
		closeProxy();
		if (e.type == INTERNAL_ERROR)
			status = PROXY_INTERNAL_ERROR;
		else if (e.type == BAD_GATEWAY)
			status = PROXY_BAD_GATEWAY;
	}
	catch (...)
	{
		closeProxy();
		status = PROXY_INTERNAL_ERROR;
	}
}

int Proxy::findProxyStatus() const
{
	return status;
}

std::string Proxy::getResponse() const
{
	return response;
}