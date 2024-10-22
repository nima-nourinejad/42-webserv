/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asohrabi <asohrabi@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:43:26 by asohrabi          #+#    #+#             */
/*   Updated: 2024/10/22 18:13:09 by asohrabi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// #include "Request.hpp"
// #include "HttpHandler.hpp"

// int	main()
// {
// 	std::string	rawRequest = "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n";

// 	Request	request(rawRequest);

// 	HttpHandler	handler;
// 	std::string	response = handler.handleRequest(request);

// 	std::cout << response;

// 	return 0;
// }

#include "HttpHandler.hpp"
#include "CGIHandler.hpp"

int main()
{
	std::string rootDir = "./www";
	std::string cgiPath = "./www/cgi-bin/script.cgi";

	HttpHandler httpHandler(rootDir);
	CGIHandler cgiHandler(cgiPath);

	std::string rawRequest =
		"POST /cgi-bin/script.cgi HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Content-Length: 4\r\n"
		"\r\n"
		"test";	
	
	try
	{
		Request		req(rawRequest);
		std::string	response = httpHandler.handleRequest(req);

		std::cout << "HTTP Response:\n" << response << std::endl;

		std::cout << std::endl;
		
		std::string	cgiResponse = cgiHandler.execute(req);

		std::cout << "CGI Response1:\n" << cgiResponse << std::endl;

	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}
