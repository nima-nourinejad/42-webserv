/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asohrabi <asohrabi@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:43:26 by asohrabi          #+#    #+#             */
/*   Updated: 2024/10/22 18:29:18 by asohrabi         ###   ########.fr       */
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

	// Example raw requests for testing
	std::string rawGetRequest =
		"GET /index.html HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"\r\n";

	std::string rawPostRequest =
		"POST /cgi-bin/script.cgi HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Content-Length: 4\r\n"
		"\r\n"
		"test";

	try
	{
		// Handle GET request
		Request getRequest(rawGetRequest);
		std::string getResponse = httpHandler.handleRequest(getRequest);
		std::cout << "GET HTTP Response:\n" << getResponse << std::endl;

		// Handle POST request
		Request postRequest(rawPostRequest);
		std::string postResponse = httpHandler.handleRequest(postRequest);
		std::cout << "POST HTTP Response:\n" << postResponse << std::endl;

		// Execute CGI handler
		std::string cgiResponse = cgiHandler.execute(postRequest);
		std::cout << "CGI Response:\n" << cgiResponse << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}

