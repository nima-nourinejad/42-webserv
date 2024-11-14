/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Proxy.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 09:53:12 by nnourine          #+#    #+#             */
/*   Updated: 2024/11/14 11:34:23 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef PROXY_HPP
#define PROXY_HPP

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <vector>
#include <unistd.h>
#include <cstring>

#include "SocketException.hpp"

enum ProxyStatus
{
	PROXY_DISCONNECTED,
	PROXY_INTERNAL_ERROR,
	PROXY_BAD_GATEWAY,
	PROXY_GATEWAY_TIMEOUT,
	PROXY_CONNECTED,
	PROXY_SENDING_REQUEST,
	PROXY_SENT_REQUEST,
	PROXY_RECEIVING_UNKOWN_TYPE,
	PROXY_RECEIVING_NONCHUNKED,
	PROXY_RECEIVING_CHUNKED,
	PROXY_RECEIVED,
};

class Proxy
{
	public:
		Proxy();
		std::string host;
		int port;
		int fd;
		struct sockaddr_in	address;
		int status;
		std::vector<std::string> requestParts;
		std::string response;
		
		
		void setHost(std::string const & host);
		void setPort(int port);
		void setAdress();
		void createSocket();
		void connectToUpstream();
		void startProxy(std::string const & host, int port);
		void communicateWithUpstream(std::vector<std::string> requestParts, std::string const & host, int port);
		void setRequest(std::vector<std::string> requestParts);
		void sendRequest();
		void receiveResponse();
		void closeProxy();
		int findProxyStatus() const;
		bool sameUpstream(std::string const & host, int port) const;
		std::string getResponse() const;
		
};

#endif