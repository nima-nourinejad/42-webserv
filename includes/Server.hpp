/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 09:37:59 by nnourine          #+#    #+#             */
/*   Updated: 2025/01/02 19:19:54 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <cerrno>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/epoll.h>

#include "Configuration.hpp"
#include "ClientConnection.hpp"
#include "Response.hpp"
#include "ConfigParser.hpp"

class Server
{
    private:

		// Constants
		static constexpr int			MAX_CONNECTIONS = 5;
		static constexpr int			BACKLOG =(2 * MAX_CONNECTIONS);
		static constexpr int			TIMEOUT = 10;
		static constexpr int			MAX_RETRY = 5;
		static constexpr int			TOTAL_EVENTS = 2 * MAX_CONNECTIONS + 1;

		// Private Attributes
		int								_socket_fd;
		int								_fd_epoll;
		struct sockaddr_in				_address;
		Configuration					_config;
		int								_num_clients;
		std::vector<ClientConnection>	_clients;
		struct epoll_event				_events[TOTAL_EVENTS];
		struct epoll_event				_ready[TOTAL_EVENTS];
		int								_retry;
		Response						_response;
		struct eventData 				eventData;
		HttpHandler						_responseMaker;

		// ClientConnection Methods
		void							occupyClientSlot(int availbleSlot, int fd);
		void							closeClientSockets();
		void							handleTimeout(int index);
		void							closeClientSocket(int index);
		void							createClientConnections(ServerBlock & serverBlock);
		void							receiveMessage(int index);
		void							sendResponseParts(int index);

		// Event Handling Methods
		int								waitForEvents();
		int								findAvailableSlot() const;
		int								getClientStatus(struct epoll_event const & event) const;
		int								getClientIndex(struct epoll_event const & event) const;
		bool							serverFull() const;
		void							createEpoll();
		void							handleTimeouts();
		void							prepareResponses();
		void							removeEpoll(int fd);
		void							handleSocketEvents();
		void							handlePendingConnections();
		void							addEpoll(int fd, int index);
		void							handleErr(struct epoll_event const & event);
		void							handleClientEvents(struct epoll_event const & event);
		void							handleListeningEvents(struct epoll_event const & event);
		void							handlePipeEvents(struct epoll_event const & event);
		int								eventType(struct epoll_event const & event) const;
		
		// Signal Methods
		void							applyCustomSignal();
		static void						signalHandler(int signal);

		// Listening Socket Methods
		void							setAddress();
		void							createSocket();
		void							makeSocketReusable();
		void							connectToSocket();
		void							acceptClient();
		void							startListeningSocket();
		void							closeSocket();
		
		// Utility Methods
		void							printMessage(std::string const & message) const;
		void							sendServiceUnavailable(int socket_fd);
		void							sendServerError(int fd);

		
	
    public:
		// Main Methods
		Server(ServerBlock & serverBlock);
		void							handleEvents();
		~Server();

		// Static Methods
		static void 					logError(std::string const & message);

		// Static Attributes
		static volatile sig_atomic_t	signal_status;
};

#endif
