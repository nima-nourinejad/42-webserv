// /* ************************************************************************** */
// /*                                                                            */
// /*                                                        :::      ::::::::   */
// /*   main2.cpp                                          :+:      :+:    :+:   */
// /*                                                    +:+ +:+         +:+     */
// /*   By: asohrabi <asohrabi@student.hive.fi>        +#+  +:+       +#+        */
// /*                                                +#+#+#+#+#+   +#+           */
// /*   Created: 2024/10/22 16:43:26 by asohrabi          #+#    #+#             */
// /*   Updated: 2024/11/13 16:09:41 by asohrabi         ###   ########.fr       */
// /*                                                                            */
// /* ************************************************************************** */

// #include "HttpHandler.hpp"
// #include "CGIHandler.hpp"

// int main()
// {
// 	std::string	rootDir = "./www";
// 	std::string	cgiPath = "./www/cgi-bin/script.cgi";

// 	HttpHandler	httpHandler(rootDir);
// 	CGIHandler	cgiHandler(cgiPath);

// 	// Example raw requests for testing
// 	std::string	rawGetRequest =
// 		"GET /index.html HTTP/1.1\r\n"
// 		"Host: localhost\r\n"
// 		"\r\n";

// 	std::string	rawPostRequest =
// 		"POST /cgi-bin/script.cgi HTTP/1.1\r\n"
// 		"Host: localhost\r\n"
// 		"Content-Length: 4\r\n"
// 		"\r\n"
// 		"test";

	// std::string	boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
	// std::string	rawFileUploadRequest =
	// 	"POST /upload HTTP/1.1\r\n"
	// 	"Host: localhost\r\n"
	// 	"Content-Type: multipart/form-data; boundary=" + boundary + "\r\n"
	// 	"Content-Length: 214\r\n"
	// 	"\r\n"
	// 	"--" + boundary + "\r\n"
	// 	"Content-Disposition: form-data; name=\"file\"; filename=\"test.txt\"\r\n"
	// 	"Content-Type: text/plain\r\n"
	// 	"\r\n"
	// 	"Hello, this is a test file content.\r\n"
	// 	"--" + boundary + "--\r\n";


// 	try
// 	{
// 		// Handle GET request
// 		Request		getRequest(rawGetRequest);
// 		std::string	getResponse = httpHandler.handleRequest(getRequest);

// 		std::cout << "GET HTTP Response:\n" << getResponse << std::endl;

// 		// Handle POST request
// 		Request		postRequest(rawPostRequest);
// 		std::string	postResponse = httpHandler.handleRequest(postRequest);

// 		std::cout << "POST HTTP Response:\n" << postResponse << std::endl;
// 		std::cout << std::endl;

// 		// Execute CGI handler
// 		std::string	cgiResponse = cgiHandler.execute(postRequest);

// 		std::cout << "CGI Response:\n" << cgiResponse << std::endl;

// 		// Handle file upload request
// 		Request		fileUploadRequest(rawFileUploadRequest);
// 		std::string	fileUploadResponse = httpHandler.handleRequest(fileUploadRequest);

// 		std::cout << "File Upload HTTP Response:\n" << fileUploadResponse << std::endl;
// 	}
// 	catch (const std::exception &e)
// 	{
// 		std::cerr << "Error: " << e.what() << std::endl;
// 	}

// 	return 0;
// }
