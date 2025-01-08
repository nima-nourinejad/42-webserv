/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 09:37:28 by nnourine          #+#    #+#             */
/*   Updated: 2025/01/08 14:01:13 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(ServerBlock & serverBlock)
    : _socket_fd(-1), _fd_epoll(-1), _config(serverBlock.getListen(),
	serverBlock.getHost(), serverBlock.getClientMaxBodySize(), serverBlock.getServerName()), _num_clients(0)
	, _responseMaker(serverBlock)
{
	
	applyCustomSignal();
	createEpoll();
	startListeningSocket();
	eventData.type = LISTENING;
	eventData.index = MAX_CONNECTIONS;
	eventData.fd = -1;
	
	createClientConnections(serverBlock);
};

void Server::connectToSocket()
{
	if (signal_status != SIGINT)
	{
		if (bind(_socket_fd,(struct sockaddr *)&_address, sizeof(_address)) == -1)
			throw SocketException("Failed to bind socket");
	}
	if (signal_status != SIGINT)
	{
		if (listen(_socket_fd, BACKLOG) == -1)
			throw SocketException("Failed to listen on socket");
	}
	printMessage("Server is listening on host " + _config.host + " and port " + std::to_string(_config.port));
	addEpoll(_socket_fd, MAX_CONNECTIONS);
}

bool Server::serverFull() const
{
	if (_num_clients >= MAX_CONNECTIONS)
	{
		printMessage("Max clients reached");
		return true;
	}
	return false;
}

int Server::findAvailableSlot() const
{
	for (int i = 0; i < MAX_CONNECTIONS; ++i)
	{
		if (_clients[i].status == DISCONNECTED && _clients[i].fd == -1 && _clients[i].index == -1)
			return i;
	}
	return -1;
}

void Server::occupyClientSlot(int availableSlot, int fd)
{
	printMessage("Accepted client " + std::to_string(availableSlot + 1) + ". Waiting for the request");
	_clients[availableSlot].fd = fd;
	_clients[availableSlot].index = availableSlot;
	_clients[availableSlot].status = WAITFORREQUEST;
	_clients[availableSlot].setCurrentTime();
}

void Server::handlePendingConnections()
{
	while (true)
	{
		int availableSlot = findAvailableSlot();
		if (availableSlot == -1)
			throw SocketException("Failed to find available slot for client");
		int fd = accept(_socket_fd, nullptr, nullptr);
		if (fd == -1)
		{
			if (errno != EAGAIN)
				throw SocketException("Failed to accept client");
			else
			{
				break;
			}
		}
		else
		{
			addEpoll(fd, availableSlot);
			++_num_clients;
			occupyClientSlot(availableSlot, fd);
		}
	}
}

void Server::acceptClient()
{
	if (serverFull())
	{
		sendServiceUnavailable(_socket_fd);
		return;
	}
	try
	{
		handlePendingConnections();
	}
	catch(SocketException const & e)
	{
		e.log();
		if (e.type == ADD_EPOLL)
		{
			if (e.open_fd != -1)
				sendServerError(e.open_fd);
		}
	}
}

void Server::closeSocket()
{
	printMessage("Server is shutting down");
	signal(SIGINT, SIG_DFL);
	closeClientSockets();
	removeEpoll(_socket_fd);
	eventData.fd = -1;
	if (_fd_epoll != -1)
		close(_fd_epoll);
	if (_socket_fd != -1)
		close(_socket_fd);
}

void Server::closeClientSocket(int index)
{
	try
	{
		if (_clients[index].fd != -1 && index < MAX_CONNECTIONS && index >= 0)
		{
			removeEpoll(_clients[index].fd);
			close(_clients[index].fd);
			_clients[index].fd = -1;
			if (_clients[index].pid != -1)
			{
				waitpid(_clients[index].pid, 0, 0);
				_clients[index].pid = -1;
			}
			if (_clients[index].pipe[0] != -1)
			{
				removeEpoll(_clients[index].pipe[0]);
				close(_clients[index].pipe[0]);
				_clients[index].pipe[0] = -1;
			}
			if (_clients[index].pipe[1] != -1)
			{
				close(_clients[index].pipe[1]);
				_clients[index].pipe[1] = -1;
			}
			_clients[index].keepAlive = true;
			_clients[index].connectTime = 0;
			_clients[index].request.clear();
			_clients[index].responseParts.clear();
			_clients[index].body.clear();
			_clients[index].index = -1;
			_clients[index].eventData.fd = -1;
			_clients[index].eventData.index = -1;
			_clients[index].pipeEventData.fd = -1;
			_clients[index].pipeEventData.index = -1;
			_clients[index].errorStatus = 0;
			--_num_clients;
			_clients[index].status = DISCONNECTED;
		}
	}
	catch(const std::exception& e)
	{
		std::string error1 = "Failed to close and clean client " + std::to_string(index + 1) + " : ";
		std::string error2 = e.what();
		_clients[index].logError(error1 + error2);
	}
}

void Server::closeClientSockets()
{
	for (int i = 0; i < MAX_CONNECTIONS; ++i)
		closeClientSocket(i);
}

int Server::waitForEvents() ///////////////////////////////////////////// throw
{
	int n_ready_fds = epoll_wait(_fd_epoll, _ready, TOTAL_EVENTS, 0);
	if (n_ready_fds == -1)
	{
		if (errno == EINTR)
			return 0;
		else
			throw SocketException("Failed to wait for events");
	}
	return n_ready_fds;
}

void Server::sendResponseParts(int index) ////////////////////////////////// throw
{
	if (_clients[index].fd == -1 || index >= MAX_CONNECTIONS || index < 0 || signal_status == SIGINT)
		return;
	ssize_t bytes_sent;
	bytes_sent = send(_clients[index].fd, _clients[index].responseParts[0].c_str(), _clients[index].responseParts[0].size(), MSG_DONTWAIT);
	if (bytes_sent == 0)
	{
		printMessage("Client " + std::to_string(index + 1) + " disconnected");
		closeClientSocket(index);
		return;
	}
	else if (bytes_sent > 0)
	{
		if (bytes_sent < static_cast<ssize_t>(_clients[index].responseParts[0].size()))
		{
			std::string remainPart = _clients[index].responseParts[0].substr(bytes_sent);
			_clients[index].responseParts[0] = remainPart;
			return;
		}
		else
		{
			_clients[index].responseParts.erase(_clients[index].responseParts.begin());
			if (_clients[index].responseParts.empty())
			{
				if (_clients[index].keepAlive == false)
				{
					printMessage("Client " + std::to_string(index + 1) + " requested to close connection");
					closeClientSocket(index);
				}
				else
				{
					printMessage("Client " + std::to_string(index + 1) + " requested to keep connection alive. Waiting for a new request");
					_clients[index].request.clear();
					_clients[index].status = WAITFORREQUEST;
					_clients[index].setCurrentTime();
				}
			}
		}
	}
}

void Server::receiveMessage(int index) ////////////////////////////////// throw
{
	if (_clients[index].fd == -1 || index >= MAX_CONNECTIONS || index < 0 || signal_status == SIGINT)
		return;
	char buffer[16384] = {};
	ssize_t bytes_received;
	bytes_received = recv(_clients[index].fd, buffer, sizeof(buffer), MSG_DONTWAIT);
	if (bytes_received == 0)
	{
		printMessage("Client " + std::to_string(index + 1) + " disconnected");
		closeClientSocket(index);
	}
	else if (bytes_received > 0)
	{
		printMessage("Received message from client " + std::to_string(index + 1));
		_clients[index].setCurrentTime();
		if (_clients[index].status == WAITFORREQUEST)
			_clients[index].status = RECEIVINGUNKOWNTYPE;
		_clients[index].request.append(buffer, bytes_received);
		_clients[index].checkRequestSize();
		if (_clients[index].status == RECEIVINGUNKOWNTYPE)
			_clients[index].findRequestType();
		if (_clients[index].finishedReceiving())
		{
			if (_clients[index].status == RECEIVINGCHUNKED)
				_clients[index].handleChunkedEncoding();
			_clients[index].status = RECEIVED;
		}
	}
}

int Server::getClientStatus(struct epoll_event const & event) const
{
	if (eventType(event) == LISTENING)
		return -1;
	struct eventData * target =(struct eventData *)event.data.ptr;
	int index = target->index;
	return _clients[index].status;
}

int Server::getClientIndex(struct epoll_event const & event) const
{
	if (eventType(event) == LISTENING)
		return -1;
	struct eventData * target =(struct eventData *)event.data.ptr;
	return target->index;
}

int Server::eventType(struct epoll_event const & event) const
{
	if (event.data.ptr == nullptr)
		return -1;
	struct eventData * target =(struct eventData *)event.data.ptr;
	return target->type;
}

void Server::handleTimeout(int index)
{
	printMessage("Client " + std::to_string(index + 1) + " timed out");
	if (_clients[index].request.empty() == false)
	{
		_clients[index].changeRequestToBadRequest();
	}
	else
		closeClientSocket(index);
}

void Server::handleTimeouts()
{
	for (int i = 0; i < MAX_CONNECTIONS; ++i)
	{
		if (_clients[i].status > DISCONNECTED && _clients[i].status < RECEIVED && _clients[i].getPassedTime() > TIMEOUT)
			handleTimeout(i);
	}
}

void Server::prepareResponses()
{
	for (int i = 0; i < MAX_CONNECTIONS; ++i)
	{
		if (_clients[i].status == RECEIVED)
		{
			int result = pipe(_clients[i].pipe);
			if (result == -1)
				_clients[i].setPlain500Response();
			else
			{
				try
				{
					addEpoll(_clients[i].pipe[0], i + MAX_CONNECTIONS + 1);
					_clients[i].createResponseParts();
				}
				catch(const std::exception& e)
				{
					_clients[i].setPlain500Response();
					close(_clients[i].pipe[0]);
					close(_clients[i].pipe[1]);
				}
			}
		}
	}
}

void Server::handleErr(struct epoll_event const & event)
{
	if (eventType(event) == LISTENING)
	{
		std::cerr << "Error on listening socket" << std::endl;
		removeEpoll(_socket_fd);
		eventData.fd = -1;
		close(_socket_fd);
		startListeningSocket();
	}
	else if (eventType(event) == CLIENT)
	{
		int index = getClientIndex(event);
		if (index == -1)
			return;
		printMessage("Client " + std::to_string(index + 1) + " disconnected");
		closeClientSocket(index);
	}
	else if (eventType(event) == PIPE)
	{
		int index = getClientIndex(event);
		if (index == -1)
			return;
		_clients[index].logError("Pipe error");
		_clients[index].setPlain500Response();
		if (_clients[index].pid != -1)
		{
			waitpid(_clients[index].pid, 0, 0);
			_clients[index].pid = -1;
		}
		if (_clients[index].pipe[0] != -1)
		{
			removeEpoll(_clients[index].pipe[0]);
			close(_clients[index].pipe[0]);
			_clients[index].pipe[0] = -1;
		}
		if (_clients[index].pipe[1] != -1)
		{
			close(_clients[index].pipe[1]);
			_clients[index].pipe[1] = -1;
		}
		
	}
}

void Server::handleClientEvents(struct epoll_event const & event)
{
	if (event.events &(EPOLLHUP | EPOLLERR))
		handleErr(event);
	else
	{
		if (getClientStatus(event) < RECEIVED &&(event.events & EPOLLIN))
			receiveMessage(getClientIndex(event));
		else if (getClientStatus(event) == READYTOSEND &&(event.events & EPOLLOUT))
		{
			int index = getClientIndex(event);
			int pipe_read_fd = _clients[index].pipe[0];
			if (pipe_read_fd != -1)
			{
				removeEpoll(pipe_read_fd);
				close(pipe_read_fd);
				_clients[index].pipe[0] = -1;
			}
			sendResponseParts(getClientIndex(event));
		}
	}
}

void Server::handleListeningEvents(struct epoll_event const & event)
{
	if (event.events &(EPOLLHUP | EPOLLERR))
		handleErr(event);
	else if (event.events & EPOLLIN)
		acceptClient();
}

void Server::handlePipeEvents(struct epoll_event const & event)
{
	if (event.events & (EPOLLHUP | EPOLLERR) & getClientStatus(event) == PREPARINGRESPONSE)
		handleErr(event);
	else if  (event.events & EPOLLIN)
	{
		int index = getClientIndex(event);
		if (index == -1)
			return;
		if (_clients[index].status == PREPARINGRESPONSE)
		{
			try
			{
				_clients[index].accumulateResponseParts();
			}
			catch(const std::exception& e)
			{
				_clients[index].setPlain500Response();
				std::string errorMessage = e.what();
				_clients[index].logError("Accumulating response failed: " + errorMessage);
				try
				{
					if (_clients[index].pid != -1)
					{
						waitpid(_clients[index].pid, 0, 0);
						_clients[index].pid = -1;
					}
				}
				catch(const std::exception& e)
				{
					std::string error1 = "Failed to wait for child process: ";
					std::string error2 = e.what();
					_clients[index].logError(error1 + error2);
				}
				try 
				{
					if (_clients[index].pipe[0] != -1)
					{
						removeEpoll(_clients[index].pipe[0]);
						close(_clients[index].pipe[0]);
						_clients[index].pipe[0] = -1;
					}
				}
				catch(const std::exception& e)
				{
					std::string error1 = "Failed to close and remove pipe: ";
					std::string error2 = e.what();
					_clients[index].logError(error1 + error2);
					
				}
				
				try
				{
					if (_clients[index].pipe[1] != -1)
					{
						close(_clients[index].pipe[1]);
						_clients[index].pipe[1] = -1;
					}
				}
				catch(const std::exception& e)
				{
					std::string error1 = "Failed to close pipe: ";
					std::string error2 = e.what();
					_clients[index].logError(error1 + error2);
				}
			}
		}
	}
}

int getFd(struct epoll_event const & event)
{
	if (event.data.ptr == nullptr)
	{
		return -1;
	}
	struct eventData * target =(struct eventData *)event.data.ptr;
	return target->fd;
}

int getType(struct epoll_event const & event)
{
	if (event.data.ptr == nullptr)
	{
		return -1;
	}
	struct eventData * target =(struct eventData *)event.data.ptr;
	return target->type;
}

int getIndex(struct epoll_event const & event)
{
	if (event.data.ptr == nullptr)
	{
		return -1;
	}
	struct eventData * target =(struct eventData *)event.data.ptr;
	return target->index;
}

void Server::handleSocketEvents()
{
	try
	{
		int n_ready_fds = waitForEvents();
		for (int i = 0; i < n_ready_fds; i++)
		{
			if (eventType(_ready[i]) == LISTENING)
			{
				handleListeningEvents(_ready[i]);
			}
			else if (eventType(_ready[i]) == CLIENT)
			{
				handleClientEvents(_ready[i]);
			}
			else if (eventType(_ready[i]) == PIPE)
			{
				handlePipeEvents(_ready[i]);
			}
		}
	}
	catch(const std::exception& e)
	{
		std::string error1 = "Failed to handle socket events: ";
		std::string error2 = e.what();
		printMessage(error1 + error2);
	}
	
	
}

void Server::handleEvents()
{
	try
	{
		prepareResponses();
	}
	catch(const std::exception& e)
	{
		std::string error1 = "Failed to prepare responses: ";
		std::string error2 = e.what();
		printMessage(error1 + error2);
	}
	
	try
	{
		handleSocketEvents();
	}
	catch(const std::exception& e)
	{
		std::string error1 = "Failed to handle socket events: ";
		std::string error2 = e.what();
		printMessage(error1 + error2);
	}
	
	try
	{
		handleTimeouts();
	}
	catch(const std::exception& e)
	{
		std::string error1 = "Failed to handle timeouts: ";
		std::string error2 = e.what();
		printMessage(error1 + error2);
	}
}

void Server::addEpoll(int fd, int index)
{
	if (index == MAX_CONNECTIONS)
	{
		eventData.fd = fd;
		_events[index].data.ptr = &eventData;
		_events[index].events = EPOLLIN | EPOLLHUP | EPOLLERR;
	}
	else if (index > MAX_CONNECTIONS)
	{
		int clientIndex = index - MAX_CONNECTIONS - 1;
		_clients[clientIndex].pipeEventData.fd = fd;
		_clients[clientIndex].pipeEventData.index = clientIndex;
		_events[index].data.ptr = &(_clients[clientIndex].pipeEventData);
		_events[index].events = EPOLLIN | EPOLLHUP | EPOLLERR;
	}
	else
	{
		_events[index].events = EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLERR;
		_clients[index].eventData.fd = fd;
		_clients[index].eventData.index = index;
		_events[index].data.ptr = &(_clients[index].eventData);
	}
	if (epoll_ctl(_fd_epoll, EPOLL_CTL_ADD, fd, _events + index) == -1)
		throw SocketException("Failed to add to epoll", fd);
}

void Server::createSocket()
{
	_socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (_socket_fd == -1)
		throw SocketException("Failed to create socket");
}

void Server::setAddress()
{
	struct addrinfo hints = {};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	struct addrinfo * result = nullptr;

	int error = getaddrinfo(_config.host.c_str(), nullptr, &hints, &result);
	if (error != 0 || result == nullptr || result->ai_addr == nullptr || result->ai_family != AF_INET)
	{
		if (_config.host != "")
			printMessage("Failed to get address info. Using default host as fallback");
		else
			printMessage("No host provided. Using default host as fallback");
		if (result != nullptr)
			freeaddrinfo(result);
		_config.host = "0.0.0.0";
		error = getaddrinfo(_config.host.c_str(), nullptr, &hints, &result);
		if (error != 0)
			throw SocketException("Failed to get address info (fallback host)");
		if (result == nullptr || result->ai_addr == nullptr)
			throw SocketException("No address info returned (fallback host)");
		if (result->ai_family != AF_INET)
		{
			freeaddrinfo(result);
			throw SocketException("Invalid address family returned (fallback host)");
		}
	}
	_address = *reinterpret_cast<struct sockaddr_in *>(result->ai_addr);
	freeaddrinfo(result);
	_address.sin_port = htons(_config.port);
}

void Server::signalHandler(int signal)
{
	if (signal == SIGINT)
		signal_status = SIGINT;
}

void Server::applyCustomSignal()
{
	if (signal(SIGINT, &signalHandler) == SIG_ERR)
		throw SocketException("Failed to set signal handler");
}

volatile sig_atomic_t Server::signal_status = 0;

void Server::createEpoll()
{
	_fd_epoll = epoll_create1(0);
	if (_fd_epoll == -1)
		throw SocketException("Failed to create epoll");
}

void Server::removeEpoll(int fd)
{
	if (epoll_ctl(_fd_epoll, EPOLL_CTL_DEL, fd, nullptr) == -1)
		throw SocketException("Failed to remove epoll event");
}

void Server::makeSocketReusable()
{
	int reusable = 1;
	if (setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &reusable, sizeof(reusable)) == -1)
		throw SocketException("Failed to make socket reusable");
}

void Server::createClientConnections(ServerBlock & serverBlock)
{
	(void)serverBlock;
	for (int i = 0; i < MAX_CONNECTIONS; ++i)
	{
		_clients.push_back(ClientConnection());
		_clients[i].responseMaker = &_responseMaker;
		_clients[0].maxBodySize = _config.maxBodySize;
	}
	
}

void Server::startListeningSocket()
{
	_retry = 0;
	bool success = false;

	while (signal_status != SIGINT && !success && _retry < MAX_RETRY)
	{
		try
		{
			createSocket();
			makeSocketReusable();
			setAddress();
			connectToSocket();
			success = true;
		}
		catch(SocketException const & e)
		{
			e.log();
			if (_socket_fd != -1)
			{
				close(_socket_fd);
				_socket_fd = -1;
			}
			++_retry;
		}
	}
	if (!success)
	{
		if (_socket_fd != -1)
		{
			close(_socket_fd);
			_socket_fd = -1;
		}
		throw SocketException("Failed to start listening socket");
	}
}

void Server::logError(std::string const & message)
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

Server::~Server()
{
	closeSocket();
}

void Server::printMessage(std::string const & message) const
{
	std::cout << _config.name << " : " << message << std::endl;
}

void Server::sendServiceUnavailable(int socket_fd)
{
	int temp_fd = accept(socket_fd, nullptr, nullptr);
	if (temp_fd == -1)
		return;
	std::string statusLine = "HTTP/1.1 503 Service Unavailable\r\n";
	std::string contentType = "Content-Type: text/plain\r\n";
	std::string connection = "Connection: close\r\n";
	std::string body = "503 Service Unavailable";
	size_t maxBodySize = _responseMaker.getServerBlock().getClientMaxBodySize();
	std::string header;
	std::vector<std::string> response;
	
	if (body.size() > maxBodySize)
	{
		std::string transferEncoding = "Transfer-Encoding: chunked\r\n";
		header = statusLine + contentType + transferEncoding + connection;
		response.push_back(header + "\r\n");
		size_t chunkSize;
		std::string chunk;
		std::stringstream sstream;
		while (body.size() > 0)
		{
			chunkSize = std::min(body.size(), maxBodySize);
			chunk = body.substr(0, chunkSize);
			sstream.str("");
			sstream << std::hex << chunkSize << "\r\n";
			sstream << chunk << "\r\n";
			response.push_back(sstream.str());
			body = body.substr(chunkSize);
		}
		response.push_back("0\r\n\r\n");
	}
	else
	{
		std::string contentLength = "Content-Length: " + std::to_string(body.size()) + "\r\n";
		header = statusLine + contentType + contentLength + connection;
		response.push_back(header + "\r\n" + body);
	}

	while (response.size() > 0)
	{
		ssize_t bytes_sent;
		bytes_sent = send(temp_fd, response[0].c_str(), response[0].size(), MSG_DONTWAIT);
		if (bytes_sent <= 0)
			break;
		if (bytes_sent < static_cast<ssize_t>(response[0].size()))
		{
			std::string remainPart = response[0].substr(bytes_sent);
			response[0] = remainPart;
		}
		else
			response.erase(response.begin());
	}
	close(temp_fd);
}


void Server::sendServerError(int fd)
{
	std::string statusLine = "HTTP/1.1 500 Internal Server Error\r\n";
	std::string contentType = "Content-Type: text/plain\r\n";
	std::string connection = "Connection: close\r\n";
	std::string body = "500 Internal Server Error";
	size_t maxBodySize = _responseMaker.getServerBlock().getClientMaxBodySize();
	std::string header;
	std::vector<std::string> response;
	
	if (body.size() > maxBodySize)
	{
		std::string transferEncoding = "Transfer-Encoding: chunked\r\n";
		header = statusLine + contentType + transferEncoding + connection;
		response.push_back(header + "\r\n");
		size_t chunkSize;
		std::string chunk;
		std::stringstream sstream;
		while (body.size() > 0)
		{
			chunkSize = std::min(body.size(), maxBodySize);
			chunk = body.substr(0, chunkSize);
			sstream.str("");
			sstream << std::hex << chunkSize << "\r\n";
			sstream << chunk << "\r\n";
			response.push_back(sstream.str());
			body = body.substr(chunkSize);
		}
		response.push_back("0\r\n\r\n");
	}
	else
	{
		std::string contentLength = "Content-Length: " + std::to_string(body.size()) + "\r\n";
		header = statusLine + contentType + contentLength + connection;
		response.push_back(header + "\r\n" + body);
	}

	while (response.size() > 0)
	{
		ssize_t bytes_sent;
		bytes_sent = send(fd, response[0].c_str(), response[0].size(), MSG_DONTWAIT);
		if (bytes_sent <= 0)
			break;
		if (bytes_sent < static_cast<ssize_t>(response[0].size()))
		{
			std::string remainPart = response[0].substr(bytes_sent);
			response[0] = remainPart;
		}
		else
			response.erase(response.begin());
	}
	close(fd);
}
