/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asohrabi <asohrabi@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:53:02 by asohrabi          #+#    #+#             */
/*   Updated: 2025/01/30 13:47:19 by asohrabi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"
#include "HttpHandler.hpp"

CGIHandler::CGIHandler() : _pid(-1) {}

CGIHandler::CGIHandler(ServerBlock &serverConfig) : _serverBlock(serverConfig) {}

CGIHandler::~CGIHandler() {}

pid_t	CGIHandler::getPid() const { return _pid; }

Response	CGIHandler::execute(const Request &req)
{
	try
	{
		HttpHandler						httpHandlerInstance(_serverBlock);
		std::shared_ptr<LocationBlock>	matchedLocation = httpHandlerInstance.findMatchedLocation(req);

		std::string		filePath = matchedLocation->getCgiPath();

		// if (std::find(matchedLocation->getCgiExtension().begin(), matchedLocation->getCgiExtension().end(), extension)
		// 		== matchedLocation->getCgiExtension().end())
		// 	throw std::runtime_error("Unsupported CGI extension");

		if (!std::filesystem::exists(filePath) || !std::filesystem::is_regular_file(filePath))
			throw std::runtime_error("CGI file does not exist or is not a regular file");

		if (access(filePath.c_str(), X_OK) != 0)
			throw std::runtime_error("CGI file is not executable");
		
		int	pipefd[2];

		if (pipe(pipefd) == -1)
			handleError("pipe");

		std::chrono::steady_clock::time_point	start_time = std::chrono::steady_clock::now();

		_pid = fork();

		if (_pid == -1)
			handleError("fork");

		if (_pid == 0)
		{
			if (close(pipefd[0]) == -1)
				handleError("close read-end in child");
			if (dup2(pipefd[1], STDOUT_FILENO) == -1)
				handleError("dup2");
			if (close(pipefd[1]) == -1)
				handleError("close write-end in child");

			char		*argv[] = { const_cast<char *>(filePath.c_str()), NULL };
			std::string	contentLength = "CONTENT_LENGTH=" + std::to_string(req.getBody().size());
			char		*envp[] = { const_cast<char *>(contentLength.c_str()), NULL };

			if (execve(filePath.c_str(), argv, envp) == -1)
				handleError("execve");
			_exit(1);
		}
		else
		{
			if (close(pipefd[1]) == -1)
				handleError("close write-end in parent");

			int	status;

			while (true)
			{
				pid_t	result = waitpid(_pid, &status, WNOHANG);
				
				if (result == _pid)
					break;
				std::chrono::duration<double>	elapsed = std::chrono::steady_clock::now() - start_time;

				if (std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() >= TIMEOUT) {
					kill(_pid, SIGKILL);
					waitpid(_pid, &status, 0);
					close(pipefd[0]);
					throw std::runtime_error("CGI script timed out");
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
        	}

			char				buffer[1024];
			std::ostringstream	output;
			ssize_t				bytesRead;

			while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
				output.write(buffer, bytesRead);

			if (close(pipefd[0]) == -1)
				handleError("close read-end in parent");

			Response	response;

			if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
			{
				std::string	cgiOutput = output.str();
				response.setStatusLine("HTTP/1.1 200 OK\r\n"); // should be changed and be like http handler
				response.setBody(cgiOutput);
				response.setHeader("Content-Length", std::to_string(cgiOutput.size()));
				response.setHeader("Content-Type", "text/html");
			}
			else
			{
				response.setStatusLine("HTTP/1.1 500 Internal Server Error\r\n");
				response.setBody("CGI script error"); // should be changed and be like http handler
			}
			return response;
		}
	}

	catch(const SystemCallError& e)
	{
		Response	response;
		
		response.setStatusLine("HTTP/1.1 500 Internal Server Error");
		response.setBody("Error: " + std::string(e.what()) + "\n");
		return response;
	}

	Response	unexpectedResponse;

	unexpectedResponse.setStatusLine("HTTP/1.1 500 Internal Server Error");
	unexpectedResponse.setBody("Unexpected error\n");
	return unexpectedResponse;
}
