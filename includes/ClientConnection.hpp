/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 09:37:51 by nnourine          #+#    #+#             */
/*   Updated: 2025/02/10 13:11:44 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTCONNECTION_HPP
#define CLIENTCONNECTION_HPP

#include <ctime>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <future>
#include <thread>
#include <chrono>

#include "Request.hpp"
#include "Response.hpp"
#include "HttpHandler.hpp"
#include "CGIHandler.hpp"
#include "SocketException.hpp"
#include "eventData.hpp"

#include <algorithm>

enum ClientStatus
{
	DISCONNECTED,
	WAITFORREQUEST,
	RECEIVINGUNKOWNTYPE,
	RECEIVINGNONCHUNKED,
	RECEIVINGCHUNKED,
	RECEIVED,
	PREPARINGRESPONSE,
	READYTOSEND,
	FAILSENDING,
};


class ClientConnection
{
	private:

	// Constants
	const size_t				MAX_HEADER_SIZE = 32768;
	
	const int					MAX_RETRY = 3;

	
    public:

	// Public Attributes
	int							index;
	int							fd;
	int							status;
	bool						keepAlive;
	time_t						connectTime;
	std::string					request;
	std::vector<std::string>	responseParts;
	struct eventData			eventData;
	HttpHandler					*responseMaker;
	int							pipe[2];
	std::string					body;
	pid_t						pid;
	int							errorStatus;
	struct eventData			pipeEventData;
	bool						isCGI;
	int							serverFailureRetry;
	std::string					serverName;
	bool 						foundStatusLine;
	bool						foundHeader;
	size_t						limitSize;
    bool						server_error_in_recv;

	// Public Methods
	ClientConnection();

	void						changeRequestToBadRequest();
	void						changeRequestToRequestTimeout();
	void						changeRequestToOverload();
	void						changeRequestToBigHeader();
    void						changeRequestToServerTimeout();
	void						changeRequestToServerError();
	bool						finishedReceivingNonChunked();
	bool						finishedReceivingChunked();
	bool						finishedReceiving();
	size_t						receivedLength() const;
	void						findRequestType();
	void						connectionType(std::string statusLine);
	size_t						getChunkedSize(std::string & unProcessed);
	void						grabChunkedData(std::string & unProcessed, size_t chunkedSize);
	void						grabChunkedHeader(std::string & unProcessed, std::string & header);
	void						handleChunkedEncoding();
	void						createResponseParts_nonCGI();
	void						createResponseParts_CGI();
	void						createResponseParts();
	void						readResponseFromPipe();
	time_t						getPassedTime() const;
	void						setCurrentTime();
	void						chunckBody(std::string statusLine, std::string rawHeader, std::string connection, size_t maxBodySize);
	void 						processInforamtionAfterFork(std::string &statusLine, std::string &rawHeader, size_t &maxBodySize);
	void 						logError(std::string const & message);
	void						readFromPipe();
	void						setCGI();
	void						CGI_child();
	void 						printMessage(std::string const & message) const;
	const std::chrono::seconds	NON_CGI_TIMEOUT = std::chrono::seconds(5);
};

#endif
