/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 09:37:51 by nnourine          #+#    #+#             */
/*   Updated: 2025/01/08 12:58:23 by nnourine         ###   ########.fr       */
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

#include "Request.hpp"
#include "Response.hpp"
#include "HttpHandler.hpp"
#include "CGIHandler.hpp"
#include "SocketException.hpp"
#include "eventData.hpp"

enum ClientStatus
{
	DISCONNECTED,
	WAITFORREQUEST,
	RECEIVINGUNKOWNTYPE,
	RECEIVINGNONCHUNKED,
	RECEIVINGCHUNKED,
	RECEIVED,
	PREPARINGRESPONSE,
	READYTOSEND
};


class ClientConnection
{
	private:

	// Constants
	const size_t				MAX_HEADER_SIZE = 32768;
	const size_t				MAX_REQUEST_SIZE = 1048576;
	
    public:

	// Public Attributes
	int							index;
	int							fd;
	int							status;
	bool						keepAlive;
	time_t						connectTime;
	std::string					request;
	std::vector<std::string>	responseParts;
	size_t						maxBodySize;
	struct eventData			eventData;
	HttpHandler					*responseMaker;
	int							pipe[2];
	std::string					body;
	pid_t						pid;
	int							errorStatus;
	struct eventData			pipeEventData;

	// Public Methods
	ClientConnection();

	void						changeRequestToBadRequest();
	void						changeRequestToServerError();
	bool						finishedReceivingNonChunked();
	bool						finishedReceivingChunked();
	bool						finishedReceiving();
	size_t						receivedLength() const;
	void						findRequestType();
	void						connectionType();
	size_t						getChunkedSize(std::string & unProcessed);
	void						grabChunkedData(std::string & unProcessed, size_t chunkedSize);
	void						grabChunkedHeader(std::string & unProcessed, std::string & header);
	void						handleChunkedEncoding();
	void						createResponseParts();
	void						accumulateResponseParts();
	time_t						getPassedTime() const;
	void						setCurrentTime();
	void						chunckBody(std::string statusLine, std::string rawHeader, std::string connection, size_t maxBodySize);
	void 						processInforamtionAfterFork(std::string &statusLine, std::string &rawHeader, std::string &connection, size_t &maxBodySize);
	void						setPlain500Response();
	void 						logError(std::string const & message);
	void						checkRequestSize();
	void						readFromPipe();
};

#endif
