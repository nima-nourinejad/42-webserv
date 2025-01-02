/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:53:02 by asohrabi          #+#    #+#             */
/*   Updated: 2025/01/02 16:57:42 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"

CGIHandler::CGIHandler() : _pid(-1) {}

CGIHandler::CGIHandler(ServerBlock &serverConfig) : _serverBlock(serverConfig) {}

CGIHandler::~CGIHandler() 
{
	// std::cout << "Hi from CGIHandler destructor" << std::endl;
	// std::cout << "pid: " << _pid << std::endl;
	// if (_pid != -1)
	// {
	// 	std::cout << "Killing CGI child process" << std::endl;
	// 	kill(_pid, SIGKILL);
	// 	_pid = -1;
	// 	std::cout << "Killed CGI child process" << std::endl;
	// }
	// std::cout << "Bye from CGIHandler destructor" << std::endl;
}

Response	CGIHandler::execute(const Request &req)
{
	try
	{
		// Check for allowed CGI extensions
		// std::string		filePath = req.getPath();
		// std::cout << "file path: " << filePath << std::endl;
		// std::string		extension = filePath.substr(filePath.find_last_of("."));
		// std::cout << "extension: " << extension << std::endl;
		std::shared_ptr<LocationBlock>	matchedLocation;

		std::cout << "Hi from CGIHandler" << std::endl;
		for (const auto &location : _serverBlock.getLocations())
		{
			std::cout << "location: " << location->getLocation() << std::endl;
			if (req.getPath() == location->getLocation())
			{
				matchedLocation = location;
				break;
			}
		}
		std::cout << "1Matched location: " << matchedLocation->getLocation() << std::endl;
		std::string		filePath = matchedLocation->getCgiPath();
		std::cout << "file path: " << filePath << std::endl;
		// std::cout << "1Matched location: " << matchedLocation->getLocation() << std::endl;
		// if (std::find(matchedLocation->getCgiExtension().begin(), matchedLocation->getCgiExtension().end(), extension)
		// 		== matchedLocation->getCgiExtension().end())
		// 	throw std::runtime_error("Unsupported CGI extension");
		// std::cout << "extension: " << extension << std::endl;
		// Check file existence and permissions
		if (!std::filesystem::exists(filePath) || !std::filesystem::is_regular_file(filePath))
			throw std::runtime_error("CGI file does not exist or is not a regular file");
		std::cout << "file path: " << filePath << std::endl;
		if (access(filePath.c_str(), X_OK) != 0)
			throw std::runtime_error("CGI file is not executable");
		
		std::cout << "Hi from part 2" << std::endl;
		int	pipefd[2];

		if (pipe(pipefd) == -1)
			handleError("pipe");

		// pid_t	pid = fork();
		_pid = fork();
		std::cout << "pid: " << _pid << std::endl;
		if (_pid == -1)
			handleError("fork");

		if (_pid == 0) // child
		{
			std::cout << "Hi from child" << std::endl;
			if (close(pipefd[0]) == -1)
				handleError("close read-end in child");

			if (dup2(pipefd[1], STDOUT_FILENO) == -1)
				handleError("dup2");
			if (close(pipefd[1]) == -1)
				handleError("close write-end in child");

			char *argv[] = { const_cast<char *>(filePath.c_str()), NULL };
			
			// Set up environment variables(for simplicity, only CONTENT_LENGTH)
			std::string	contentLength = "CONTENT_LENGTH=" + std::to_string(req.getBody().size());
			char *envp[] = { const_cast<char *>(contentLength.c_str()), NULL };

			if (execve(filePath.c_str(), argv, envp) == -1)
				handleError("execve");
			_exit(1);
		}
		else
		{
			if (close(pipefd[1]) == -1)
				handleError("close write-end in parent");

			int	status;
			std::cout << "PID in parent: " << _pid << std::endl;
			if (waitpid(_pid, &status, 0) == -1)
				handleError("waitpid");
			std::cout << "pid after waitpid: " << _pid << std::endl;

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
			// return response.toString();
		}
	}

	catch(const SystemCallError& e)
	{
		Response	response;
		
		response.setStatusLine("HTTP/1.1 500 Internal Server Error");
		response.setBody("Error: " + std::string(e.what()) + "\n");
		return response;
		// return response.toString();
	}

	Response	unexpectedResponse;

	unexpectedResponse.setStatusLine("HTTP/1.1 500 Internal Server Error");
	unexpectedResponse.setBody("Unexpected error\n");
	return unexpectedResponse;
	// return unexpectedResponse.toString();
}
