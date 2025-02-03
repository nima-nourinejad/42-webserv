/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 09:37:28 by nnourine          #+#    #+#             */
/*   Updated: 2025/02/03 18:39:40 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(ServerBlock & serverBlock, int port, int max_fd)
    : _socket_fd(-1), _fd_epoll(-1), _config(port, serverBlock.getHost(),
	serverBlock.getClientMaxBodySize(), serverBlock.getServerName()), _num_clients(0)
	, _responseMaker(serverBlock), fd_num(0), max_fd(max_fd), max_connections((max_fd - 1) / 2), total_events(max_fd),
	_events(max_fd), _ready(max_fd), _clients(max_connections)
{
	
	applyCustomSignal();
	createEpoll();
	startListeningSocket();
	
	eventData.type = LISTENING;
	eventData.index = max_connections;
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
		if (listen(_socket_fd, backlog) == -1)
			throw SocketException("Failed to listen on socket");
	}
	printMessage("Server is listening on host " + _config.host + " and port " + std::to_string(_config.port));
	addEpoll(_socket_fd, max_connections);
}

bool Server::serverFull() const
{
	if (_num_clients >= max_connections)
	{
		printMessage("Max clients reached");
		return true;
	}
	return false;
}

int Server::findAvailableSlot() const
{
	for (int i = 0; i < max_connections; ++i)
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
		if ((fd_num + 1) > max_fd || serverFull())
			return;
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
			fd_num++;
			addEpoll(fd, availableSlot);
			++_num_clients;
			occupyClientSlot(availableSlot, fd);
		}
	}
}

void Server::acceptClient()
{

	if ((fd_num + 1) > max_fd || serverFull())
		return;
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
	{
		close(_fd_epoll);
		fd_num--;
	}
	if (_socket_fd != -1)
	{
		close(_socket_fd);
		fd_num--;
	}
}

void Server::closeClientSocket(int index)
{
	try
	{
		if (_clients[index].fd != -1 && index < max_connections && index >= 0)
		{
			_clients[index].status = DISCONNECTED;
			try
			{
				removeEpoll(_clients[index].fd);
			}
			catch(const std::exception& e)
			{
				std::string error1 = "Failed remove client fd from epoll " + std::to_string(index + 1) + " : ";
				std::string error2 = e.what();
				_clients[index].logError(error1 + error2);
			}
			
			
			if (_clients[index].fd != -1)
			{
				close(_clients[index].fd);
				fd_num--;
				_clients[index].fd = -1;
			}
			if (_clients[index].pid != -1)
			{
				waitpid(_clients[index].pid, 0, 0);
				_clients[index].pid = -1;
			}
			if (_clients[index].pipe[0] != -1)
			{
				try
				{
					removeEpoll(_clients[index].pipe[0]);
				}
				catch (const std::exception& e)
				{
					std::string error1 = "Failed to remove pipe from epoll " + std::to_string(index + 1) + " : ";
					std::string error2 = e.what();
					_clients[index].logError(error1 + error2);
				}
				close(_clients[index].pipe[0]);
				fd_num--;
				_clients[index].pipe[0] = -1;
			}
			if (_clients[index].pipe[1] != -1)
			{
				close(_clients[index].pipe[1]);
				fd_num--;
				_clients[index].pipe[1] = -1;
			}
			_clients[index].keepAlive = true;
			_clients[index].isCGI = false;
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
	for (int i = 0; i < max_connections; ++i)
		closeClientSocket(i);
}

int Server::waitForEvents()
{
	int n_ready_fds = epoll_wait(_fd_epoll, _ready.data(), total_events, 0);
	if (n_ready_fds == -1)
	{
		if (errno != EINTR)
			logError("Failed to wait for events");
		return 0;
	}
	return n_ready_fds;
}

void Server::sendResponseParts(int index)
{
	try
	{
		if (_clients[index].fd == -1 || index >= max_connections || index < 0 || signal_status == SIGINT)
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
						_clients[index].isCGI = false;
						_clients[index].status = WAITFORREQUEST;
						_clients[index].setCurrentTime();
					}
				}
			}
		}
	}
	catch(const std::exception& e)
	{
		std::string error1 = "Failed to send response to client " + std::to_string(index + 1) + " : ";
		std::string error2 = e.what();
		_clients[index].logError(error1 + error2);
		if (_clients[index].status != FAILSENDING)
		{
			_clients[index].setCurrentTime();
			_clients[index].status = FAILSENDING;
		}
	}
}

void Server::receiveMessage(int index)
{
	try 
	{
		if (_clients[index].fd == -1 || index >= max_connections || index < 0 || signal_status == SIGINT)
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
				printMessage("Request fully received from client " + std::to_string(index + 1));
				std::cout << _clients[index].request << std::endl;
			}
		}
	}
	catch(const std::exception& e)
	{
		std::string error1 = "Failed to receive message from client " + std::to_string(index + 1) + " : ";
		std::string error2 = e.what();
		_clients[index].logError(error1 + error2);
		_clients[index].changeRequestToServerError();
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
	if (_clients[index].status == FAILSENDING)
		closeClientSocket(index);
	else if (_clients[index].request.empty() == false)
		_clients[index].changeRequestToRequestTimeout();
	else
		closeClientSocket(index);
}

void Server::handleTimeouts()
{
	for (int i = 0; i < max_connections; ++i)
	{
		if ((_clients[i].status > DISCONNECTED && _clients[i].status < RECEIVED) || _clients[i].status == FAILSENDING)
		{
			if (_clients[i].getPassedTime() > TIMEOUT)
				handleTimeout(i);
		}
	}
}

void Server::prepareResponses()
{
	for (int i = 0; i < max_connections; ++i)
	{
		if (_clients[i].status == RECEIVED)
		{
			_clients[i].setCGI();
			if (_clients[i].isCGI == false)
			{
				try
				{
					_clients[i].pipe[0] = -1;
					_clients[i].pipe[1] = -1;
					_clients[i].createResponseParts();
				}
				catch(const std::exception& e)
				{
					_clients[i].errorStatus = 500;
					_clients[i].status = RECEIVED;
					_clients[i].logError("Failed to create non CGI response parts: " + std::string(e.what()));
				}
				
				
			}
			else
			{
				if ((fd_num + 2) > max_fd)
					return;
				int result = pipe(_clients[i].pipe);
				if (result == -1)
				{
					_clients[i].errorStatus = 500;
					_clients[i].status = RECEIVED;
					_clients[i].pipe[0] = -1;
					_clients[i].pipe[1] = -1;
					std::string error = "Failed to create pipe at fd_num: " + std::to_string(fd_num);
					_clients[i].logError(error);
				}
				else
				{
					bool added_to_epoll = false;
					fd_num+=2;
					try
					{
						addEpoll(_clients[i].pipe[0], i + max_connections + 1);
						added_to_epoll = true;
						_clients[i].createResponseParts();
					}
					catch(const std::exception& e)
					{
						_clients[i].logError("Failed to create CGI response parts or add the pipe to epoll: " + std::string(e.what()));
						_clients[i].errorStatus = 500;
						_clients[i].status = RECEIVED;
						if (_clients[i].pipe[0] != -1)
						{
							try
							{
								if (added_to_epoll)
									removeEpoll(_clients[i].pipe[0]);
							}
							catch(const std::exception& e)
							{
								_clients[i].logError("Failed to remove pipe from epoll after failure of creating CGI response: " + std::string(e.what()));
							}
							close(_clients[i].pipe[0]);
							fd_num--;
							_clients[i].pipe[0] = -1;
						}
						if (_clients[i].pipe[1] != -1)
						{
							close(_clients[i].pipe[1]);
							fd_num--;
							_clients[i].pipe[1] = -1;
						}
						if(_clients[i].pid != -1)
						{
							kill(_clients[i].pid, SIGKILL);
							waitpid(_clients[i].pid, 0, 0);
							_clients[i].pid = -1;
						}
					}
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
		fd_num--;
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
		_clients[index].changeRequestToServerError();
		_clients[index].logError("Pipe error");
		if (_clients[index].pid != -1)
		{
			waitpid(_clients[index].pid, 0, 0);
			_clients[index].pid = -1;
		}
		if (_clients[index].pipe[0] != -1)
		{
			removeEpoll(_clients[index].pipe[0]);
			close(_clients[index].pipe[0]);
			fd_num--;
			_clients[index].pipe[0] = -1;
		}
		if (_clients[index].pipe[1] != -1)
		{
			close(_clients[index].pipe[1]);
			fd_num--;
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
				try
				{
					removeEpoll(pipe_read_fd);
				}
				catch(const std::exception& e)
				{
					std::string error1 = "Failed to remove pipe from epoll: ";
					std::string error2 = e.what();
					_clients[index].logError(error1 + error2);
				}
				
				try
				{
					close(pipe_read_fd);
					fd_num-= 2;
				}
				catch(const std::exception& e)
				{
					std::string error1 = "Failed to close pipe: ";
					std::string error2 = e.what();
					_clients[index].logError(error1 + error2);
				}
				
				
				_clients[index].pipe[0] = -1;
			}
			sendResponseParts(getClientIndex(event));
		}
		else if (getClientStatus(event) == FAILSENDING && (event.events & EPOLLOUT))
			sendResponseParts(getClientIndex(event));
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
				_clients[index].readResponseFromPipe();
			}
			catch(const std::exception& e)
			{
				_clients[index].changeRequestToServerError();
				std::string errorMessage = e.what();
				_clients[index].logError("Reading response form pipe failed: " + errorMessage);
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
						fd_num--;
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
						fd_num--;
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
		logError(error1 + error2);
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
		logError(error1 + error2);
	}
	catch(...)
	{
		logError("Failed to prepare responses");
	}
	
	try
	{
		handleSocketEvents();
	}
	catch(const std::exception& e)
	{
		std::string error1 = "Failed to handle socket events: ";
		std::string error2 = e.what();
		logError(error1 + error2);
	}
	catch(...)
	{
		logError("Failed to handle socket events");
	}
	
	try
	{
		handleTimeouts();
	}
	catch(const std::exception& e)
	{
		std::string error1 = "Failed to handle timeouts: ";
		std::string error2 = e.what();
		logError(error1 + error2);
	}
	catch(...)
	{
		logError("Failed to handle timeouts");
	}
}

void Server::addEpoll(int fd, int index)
{
	if (index == max_connections)
	{
		eventData.fd = fd;
		_events[index].data.ptr = &eventData;
		_events[index].events = EPOLLIN | EPOLLHUP | EPOLLERR;
	}
	else if (index > max_connections)
	{
		int clientIndex = index - max_connections - 1;
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
	if (epoll_ctl(_fd_epoll, EPOLL_CTL_ADD, fd, _events.data() + index) == -1)
		throw SocketException("Failed to add to epoll", fd);
}

void Server::createSocket()
{
	_socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	fd_num++;
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
	fd_num++;
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
	for (int i = 0; i < max_connections; ++i)
	{
		_clients[i].responseMaker = &_responseMaker;
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
				fd_num--;
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
			fd_num--;
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


void Server::sendServerError(int fd)
{
	std::string body;
	std::string statusLine;
	std::string rawHeader;
	size_t maxBodySize_error;
	try
	{
		maxBodySize_error = _responseMaker.getMaxBodySize("", 500);
		Request		req("", 500);
		Response	response;
		response = _responseMaker.getErrorPage(req, 500);
		body = response.getBody();
		statusLine = response.getStatusLine();
		rawHeader = response.getRawHeader();
	}
	catch(const std::exception& e)
	{
		statusLine = "HTTP/1.1 500 Internal Server Error\r\n";
		rawHeader = "Content-Type: text/plain\r\n";
		body = "500 Internal Server Error";
		maxBodySize_error = body.size();
		std::string errorMessage = e.what();
		logError("Failed to create cutom error response: " + errorMessage);
	}
	std::string connection = "Connection: close\r\n";
	std::string header;
	std::vector<std::string> responseParts;

	if (body.size() > maxBodySize_error)
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
			chunkSize = std::min(body.size(), maxBodySize_error);
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

	while (responseParts.size() > 0)
	{
		ssize_t bytes_sent;
		bytes_sent = send(fd, responseParts[0].c_str(), responseParts[0].size(), MSG_DONTWAIT);
		if (bytes_sent <= 0)
			break;
		if (bytes_sent < static_cast<ssize_t>(responseParts[0].size()))
		{
			std::string remainPart = responseParts[0].substr(bytes_sent);
			responseParts[0] = remainPart;
		}
		else
			responseParts.erase(responseParts.begin());
	}
	close(fd);
	fd_num--;
}


