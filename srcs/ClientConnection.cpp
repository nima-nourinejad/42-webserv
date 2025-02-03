/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 09:33:24 by nnourine          #+#    #+#             */
/*   Updated: 2025/02/03 18:32:29 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ClientConnection.hpp"

ClientConnection::ClientConnection()
    : index(-1), fd(-1), status(DISCONNECTED), keepAlive(true),responseMaker(nullptr), pipe{ -1, -1 }, pid(-1), errorStatus(0), isCGI(false)
	{
		eventData.type = CLIENT;
		eventData.fd = -1;
		eventData.index = -1;
		pipeEventData.type = PIPE;
		pipeEventData.fd = -1;
		pipeEventData.index = -1;
	};

void ClientConnection::changeRequestToBadRequest()
{
	errorStatus = 408;
	status = RECEIVED;
}

void ClientConnection::changeRequestToRequestTimeout()
{
	errorStatus = 408;
	status = RECEIVED;
}

void ClientConnection::changeRequestToServerError()
{
	errorStatus = 500;
	status = RECEIVED;
}

bool ClientConnection::finishedReceivingNonChunked()
{
	size_t contentLength;
	std::string contentLengthString;
	if (request.find("Content-Length: ") == std::string::npos)
	{
		return true;
	}
	contentLengthString = request.substr(request.find("Content-Length: ") + 16);
	if (contentLengthString.find("\r\n") == std::string::npos)
	{
		std::cerr << "Failed to find end of content length" << std::endl;
		changeRequestToBadRequest();
		return true;
	}
	contentLengthString = contentLengthString.substr(0, contentLengthString.find("\r\n"));
	try
	{
		contentLength = std::stoul(contentLengthString);
	}
	catch(...)
	{
		std::cerr << "Failed to convert content length to number" << std::endl;
		changeRequestToBadRequest();
		return true;
	}
	if (receivedLength() > contentLength)
	{
		std::cerr << "Received more data than expected" << std::endl;
		changeRequestToBadRequest();
		return true;
	}
	if (receivedLength() == contentLength)
		return true;
	return false;
}

bool ClientConnection::finishedReceivingChunked()
{
	if (request.find("\r\n0\r\n\r\n") != std::string::npos)
		return true;
	return false;
}

bool ClientConnection::finishedReceiving()
{
	if (status == RECEIVINGUNKOWNTYPE)
		return false;
	else if (status == RECEIVINGCHUNKED)
		return finishedReceivingChunked();
	else
		return finishedReceivingNonChunked();
}

size_t ClientConnection::receivedLength() const
{
	size_t headerLength = request.find("\r\n\r\n") + 4;
	size_t receivedLength = request.size() - headerLength;
	return receivedLength;
}

void ClientConnection::findRequestType()
{
	if (request.find("\r\n\r\n") == std::string::npos)
	{
		if (request.size() > MAX_HEADER_SIZE)
		{
			logError("Header size exceeded the limit");
			changeRequestToBadRequest();
		}
	}
	else
	{
		if (request.find("Transfer-Encoding: chunked") != std::string::npos)
			status = RECEIVINGCHUNKED;
		else
			status = RECEIVINGNONCHUNKED;
	}
}

void ClientConnection::connectionType()
{
	if (request.find("Connection: close") != std::string::npos)
		keepAlive = false;
}

size_t ClientConnection::getChunkedSize(std::string & unProcessed)
{
	size_t chunkedSize;
	std::string sizeString;
	sizeString = unProcessed.substr(0, unProcessed.find("\r\n"));
	unProcessed = unProcessed.substr(unProcessed.find("\r\n") + 2);
	try
	{
		chunkedSize = std::stoul(sizeString, nullptr, 16);
	}
	catch(...)
	{
		changeRequestToBadRequest();
		return 0;
	}
	if (unProcessed.size() < chunkedSize + 2)
	{
		changeRequestToBadRequest();
		return 0;
	}
	return chunkedSize;
}

void ClientConnection::grabChunkedData(std::string & unProcessed, size_t chunkedSize)
{
	std::string data;
	data = unProcessed.substr(0, chunkedSize);
	request += data;
	unProcessed = unProcessed.substr(chunkedSize + 2);
}

void ClientConnection::grabChunkedHeader(std::string & unProcessed, std::string & header)
{
	
	header = unProcessed.substr(0, unProcessed.find("\r\n\r\n") + 4);
	unProcessed = unProcessed.substr(unProcessed.find("\r\n\r\n") + 4);
	request = header;
}

void ClientConnection::handleChunkedEncoding()
{
	std::string unProcessed = request;
	
	if (unProcessed.find("Transfer-Encoding: chunked") == std::string::npos)
		return(changeRequestToServerError());
	if (unProcessed.find("\r\n0\r\n\r\n") == std::string::npos)
		return(changeRequestToBadRequest());
	request.clear();
	std::string header = "";
	grabChunkedHeader(unProcessed, header);

	size_t chunkedSize;
	while (true)
	{
		if (unProcessed.find("\r\n") == std::string::npos)
			return(changeRequestToBadRequest());
		chunkedSize = getChunkedSize(unProcessed);
		if (chunkedSize == 0)
			return;
		else
			grabChunkedData(unProcessed, chunkedSize);
	}
}

void ClientConnection::createResponseParts_nonCGI()
{
	try
	{
		std::string statusLine, rawHeader;
		size_t maxBodySize = responseMaker->getMaxBodySize(request, errorStatus);
		Response	response;
		if (!errorStatus)
			response = responseMaker->createResponse(request);
		else
		{
			Request		req(request, errorStatus);
			response = responseMaker->getErrorPage(req, errorStatus);
		}
		body = response.getBody();
		statusLine = response.getStatusLine();
		rawHeader = response.getRawHeader();
		connectionType();
		std::string connection;
		if (keepAlive)
			connection = "Connection: keep-alive\r\n";
		else
			connection = "Connection: close\r\n";
		chunckBody(statusLine, rawHeader, connection, maxBodySize);
		errorStatus = 0;
		status = READYTOSEND;
	}
	catch(const std::exception& e)
	{
		std::string errorMessage = e.what();
		logError("Creating non CGI response failed: " + errorMessage);
		changeRequestToServerError();
	}
}

void ClientConnection::CGI_child()
{
	std::string body, statusLine, rawHeader, maxBodySizeString;
	size_t		maxBodySize;
	Response	response;
	if (pipe[0] != -1)
		close(pipe[0]);
	try
	{
		
		maxBodySize = responseMaker->getMaxBodySize(request, errorStatus);
		maxBodySizeString = std::to_string(maxBodySize) + "\r\n";
		response = responseMaker->createResponse(request);
		body = response.getBody();
		statusLine = response.getStatusLine();
		rawHeader = response.getRawHeader();
	}
	catch(const std::exception& e)
	{
		std::string errorMessage = e.what();
		logError("Child process for creating response failed: " + errorMessage);
		try
		{
			maxBodySize = responseMaker->getMaxBodySize(request, errorStatus);
			maxBodySizeString = std::to_string(maxBodySize) + "\r\n";
			Request		req(request, errorStatus);
			response = responseMaker->getErrorPage(req, errorStatus);
			body = response.getBody();
			statusLine = response.getStatusLine();
			rawHeader = response.getRawHeader();
		}
		catch(const std::exception& e)
		{
			std::string errorMessage = e.what();
			logError("Failed to create error response in child process: " + errorMessage);
			statusLine = "HTTP/1.1 500 Internal Server Error\r\n";
			rawHeader = "Content-Type: text/plain\r\n";
			body = "500 Internal Server Error";
			maxBodySizeString = std::to_string(body.size()) + "\r\n";
		}
	}
	std::string fullMessage = maxBodySizeString + statusLine + rawHeader + "\r\n" + body;
	write(pipe[1], fullMessage.c_str(), fullMessage.size());
	close(pipe[1]);
	exit(0);
}

void ClientConnection::createResponseParts_CGI()
{
	pid = fork();
	if (pid == -1)
	{
		if (pipe[1] != -1)
			close(pipe[1]);
		changeRequestToServerError();
		logError("Failed to fork");		
	}
	if (pid == 0)
		CGI_child();
	
}

void ClientConnection::createResponseParts()
{
	if (status != RECEIVED)
		return;
	responseParts.clear();
	status = PREPARINGRESPONSE;	
	if (!isCGI)
		createResponseParts_nonCGI();
	else
		createResponseParts_CGI();
}

size_t grabMaxBodySize(std::string &response)
{
	size_t maxBodySize;
	std::string maxBodySizeString = response.substr(0, response.find("\r\n"));
	response = response.substr(response.find("\r\n") + 2);
	maxBodySize = std::stoul(maxBodySizeString);
	return maxBodySize;
}

std::string grabStatusLine(std::string & response)
{
	std::string statusLine = response.substr(0, response.find("\r\n") + 2);
	response = response.substr(response.find("\r\n") + 2);
	return statusLine;
}

std::string grabRawHeader(std::string & modifiedResponse)
{
	std::string rawHeader = modifiedResponse.substr(0, modifiedResponse.find("\r\n\r\n") + 2);
	modifiedResponse = modifiedResponse.substr(modifiedResponse.find("\r\n\r\n") + 4);
	return rawHeader;
}

void ClientConnection::chunckBody(std::string statusLine, std::string rawHeader, std::string connection, size_t maxBodySize)
{
	responseParts.clear();
	std::string header;
	if (body.size() > maxBodySize)
	{
		responseParts.push_back(statusLine);
		std::string transferEncoding = "Transfer-Encoding: chunked\r\n";
		rawHeader = rawHeader + transferEncoding + connection;
		responseParts.push_back(rawHeader + "\r\n");
		size_t				chunkSize;
		std::string			chunk;
		std::stringstream	sstream;

		while (body.size() > 0)
		{
			chunkSize = std::min(body.size(), maxBodySize);
			chunk = body.substr(0, chunkSize);
			sstream.str("");
			sstream << std::hex << chunkSize << "\r\n";
			sstream << chunk << "\r\n";
			responseParts.push_back(sstream.str());
			body = body.substr(chunkSize);
		}
		responseParts.push_back("0\r\n\r\n");
	}
	else
	{
		std::string contentLength = "Content-Length: " + std::to_string(body.size()) + "\r\n";
		header = statusLine + rawHeader + contentLength + connection;
		responseParts.push_back(header + "\r\n" + body);
	}
}

void ClientConnection::processInforamtionAfterFork(std::string &statusLine, std::string &rawHeader, std::string &connection, size_t &maxBodySize)
{
	maxBodySize = grabMaxBodySize(body);
	statusLine = grabStatusLine(body);
	rawHeader = grabRawHeader(body);
	if (keepAlive)
		connection = "Connection: keep-alive\r\n";
	else
		connection = "Connection: close\r\n";
}

void ClientConnection::readFromPipe()
{
	body.clear();
	char buffer[16384] = {};
	ssize_t bytes_received;
	while (true)
	{
		std::memset(buffer, 0, sizeof(buffer));
		bytes_received = read(pipe[0], buffer, sizeof(buffer));
		if (bytes_received > 0)
			body.append(buffer, bytes_received);
		else if (bytes_received == 0)
			break;
	}
}

void ClientConnection::readResponseFromPipe()
{
	if (pipe[1] != -1)
		close(pipe[1]);
	pipe[1] = -1;
	readFromPipe();
	if (pid != -1)
		waitpid(pid, 0, 0);
	pid = -1;
	connectionType();
	size_t		maxBodySize;
	std::string statusLine, rawHeader, connection;
	processInforamtionAfterFork(statusLine, rawHeader,connection, maxBodySize);
	chunckBody(statusLine, rawHeader, connection, maxBodySize);
	
	errorStatus = 0;
	status = READYTOSEND;
}

time_t getCurrentTime()
{
	time_t current_time = time(nullptr);
	if (current_time == -1)
		throw SocketException("Failed to get current time");
	return current_time;
}

time_t ClientConnection::getPassedTime() const
{
	time_t current_time = getCurrentTime();
	if (current_time == -1)
		throw SocketException("Failed to get passed time");
	return(difftime(current_time, connectTime));
}

void ClientConnection::setCurrentTime()
{
	connectTime = getCurrentTime();
}

void ClientConnection::logError(std::string const & message)
{
	try
	{
		std::chrono::time_point<std::chrono::system_clock> timePoint = std::chrono::system_clock::now();
		std::time_t timeInSeconds = std::chrono::system_clock::to_time_t(timePoint);
		std::ofstream logFile("server_error.log", std::ios::app);
		if (!logFile.is_open())
			throw std::runtime_error("Failed to open log file");
		logFile << std::put_time(std::localtime(&timeInSeconds), "%Y-%m-%d %H:%M:%S") << " : ";
		logFile << message << std::endl;
		logFile.close();
	}
	catch(std::exception const & e)
	{
		std::cerr << "Failed to log exception : " << e.what() << std::endl;
		std::cerr << "Original exception : " << message << std::endl;
	}
}

void ClientConnection::checkRequestSize()
{
	if (request.size() > MAX_REQUEST_SIZE)
		changeRequestToBadRequest();
}

void ClientConnection::setCGI()
{
	Request req(request, errorStatus);
	isCGI= (!errorStatus && responseMaker->findMatchedLocation(req) && !((responseMaker->findMatchedLocation(req))->getCgiPath().empty()));
}
