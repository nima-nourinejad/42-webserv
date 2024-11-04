/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asohrabi <asohrabi@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:53:02 by asohrabi          #+#    #+#             */
/*   Updated: 2024/11/04 17:58:12 by asohrabi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"

CGIHandler::CGIHandler() : _cgiPath("") {}

CGIHandler::CGIHandler(const std::string &cgiPath) : _cgiPath(cgiPath) {}

CGIHandler::~CGIHandler() {}

std::string	CGIHandler::execute(const Request &req)
{
	try
	{
		int	pipefd[2];

		if (pipe(pipefd) == -1)
			handleError("pipe");

		pid_t	pid = fork();

		if (pid == -1)
			handleError("fork");

		if (pid == 0) // child
		{
			if (close(pipefd[0]) == -1)
				handleError("close read-end in child");

			if (dup2(pipefd[1], STDOUT_FILENO) == -1)
				handleError("dup2");
			if (close(pipefd[1]) == -1)
				handleError("close write-end in child");

			char *argv[] = { const_cast<char *>(_cgiPath.c_str()), NULL };
			
			// Set up environment variables (for simplicity, only CONTENT_LENGTH)
			std::string	contentLength = "CONTENT_LENGTH=" + std::to_string(req.getBody().size());
			char *envp[] = { const_cast<char *>(contentLength.c_str()), NULL };

			if (execve(_cgiPath.c_str(), argv, envp) == -1)
				handleError("execve");
			_exit(1);
		}
		else
		{
			if (close(pipefd[1]) == -1)
				handleError("close write-end in parent");

			int	status;
			if (waitpid(pid, &status, 0) == -1)
				handleError("waitpid");

			char				buffer[1024];
			std::ostringstream	output;
			ssize_t				bytesRead;

			while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
				output.write(buffer, bytesRead);

			if (close(pipefd[0]) == -1)
				handleError("close read-end in parent");

			// std::ostringstream	response;
			Response	response;

			if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
			{
				std::string	cgiOutput = output.str();
				response.setStatusLine("HTTP/1.1 200 OK");
				response.setBody(cgiOutput);
				response.setHeader("Content-Length", std::to_string(cgiOutput.size()));
				response.setHeader("Content-Type", "text/plain");
				// response << "HTTP/1.1 200 OK\r\n"
				// 		<< "Content-Length: " << cgiOutput.size() << "\r\n"
				// 		<< "Content-Type: text/plain\r\n"
				// 		<< "\r\n"
				// 		<< cgiOutput;
			}
			else
			{
				response.setStatusLine("HTTP/1.1 500 Internal Server Error");
				response.setBody("CGI script error\n");
			}
				// response << "HTTP/1.1 500 Internal Server Error\r\n\r\nCGI script error\n";

			return response.toString();
		}
	}
	catch (const SystemCallError& e)
	{
		// std::ostringstream	response;
		Response	response;
		
		response.setStatusLine("HTTP/1.1 500 Internal Server Error");
		response.setBody("Error: " + std::string(e.what()) + "\n");
		// response << "HTTP/1.1 500 Internal Server Error\r\n\r\nError: " << e.what() << "\n";
		return response.toString();
	}

	Response	unexpectedResponse;

	unexpectedResponse.setStatusLine("HTTP/1.1 500 Internal Server Error");
	unexpectedResponse.setBody("Unexpected error\n");
	return unexpectedResponse.toString();
	// return "HTTP/1.1 500 Internal Server Error\r\n\r\nUnexpected error\n";
}
