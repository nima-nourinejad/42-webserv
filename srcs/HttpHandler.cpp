/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asohrabi <asohrabi@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:39:26 by asohrabi          #+#    #+#             */
/*   Updated: 2025/01/02 14:00:37 by asohrabi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpHandler.hpp"

HttpHandler::HttpHandler(ServerBlock &serverConfig)
	: _cgiHandler(serverConfig), _rootDir(serverConfig.getLocations()[0]->getRoot())
	, _serverBlock(serverConfig), _maxBodySize(serverConfig.getClientMaxBodySize())
	{
		//creating a default state of map
		_errorPages[404]="default_404.html";
		_errorPages[500]="default_403.html";

		//filling the map with the error pages from the server block
		for (const auto &errorPage : serverConfig.getErrorPages())
			_errorPages[errorPage.first] = errorPage.second;
	}

HttpHandler::~HttpHandler() {}

bool	HttpHandler::_isMethodAllowed(const std::string &method, const std::string &path)
{
	for (const auto &location : _serverBlock.getLocations())
	{
		if (path.find(location->getLocation()) == 0) // Match location prefix
		{
			// const std::vector<std::string>	&allowedMethods = location.getLimitExcept();
			const auto	&allowedMethods = location->getLimitExcept();
			bool		isMethodAllowed = std::find(allowedMethods.begin(), allowedMethods.end(), method) != allowedMethods.end();
			
			return (isMethodAllowed);
		}
	}
	return false;
}

Response	HttpHandler::getErrorPage(const Request &req, int statusCode)
{
	const auto						&errorPages = _serverBlock.getErrorPages();
	Response						response;
	std::shared_ptr<LocationBlock>	matchedLocation;

	for (const auto &location : _serverBlock.getLocations())
	{
		if (req.getPath() == location->getLocation())
		{
			matchedLocation = location;
			break;
		}
	}

	if (!matchedLocation)
	{
		// std::cout << "No matching location found" << std::endl;
		return getErrorPage(req, 404); // Not found
	}
	
	if (errorPages.find(statusCode) != errorPages.end())
	{
		response.setStatusLine("HTTP/1.1 " + std::to_string(statusCode) + " " + getStatusMessage(statusCode) + "\r\n");
		response.setBody(readFileError(_rootDir + "/" + errorPages.at(statusCode)));
		return response;
	}
	response.setStatusLine("HTTP/1.1 " + std::to_string(statusCode) + " " + getStatusMessage(statusCode) + "\r\n");
	response.setBody(readFileError(_rootDir + "/" + "dummy.html")); // needed to make better
	return response;
}

std::string	HttpHandler::_validateRequest(const Request &req)
{
	std::string	method = req.getMethod();
	std::string	path = req.getPath();

	if (!_isMethodAllowed(method, path))
		return "HTTP/1.1 405 Method Not Allowed\r\n\r\nMethod Not Allowed\n";
	for (const auto &location : _serverBlock.getLocations())
	{
		if (req.getPath().find(location->getLocation()) == 0)
		{
			if (!location->getCgiPath().empty() && !std::filesystem::exists(location->getCgiPath()))
				return "Invalid CGI Path"; // should be like a http response

			if (!location->getRoot().empty() && !std::filesystem::exists(location->getRoot()))
				return "Invalid root path"; // should be like a http response
		}
	}
	return "Ok";
}

Response	HttpHandler::createResponse(const std::string &request)
{
	Request	req(request);

	return handleRequest(req);
}

Response	HttpHandler::handleRequest(const Request &req)
{
	try
	{
		// std::string validation = _validateRequest(req);
		// if (validation != "Ok")
		// 	return validation;

		// std::cout << "beginning of handle request" << std::endl;
		
		std::shared_ptr<LocationBlock> matchedLocation;

		for (const auto &location : _serverBlock.getLocations())
		{
			// if (req.getPath().find(location->getLocation()) == 0)
			if (req.getPath() == location->getLocation())
			{
				matchedLocation = location;
				std::cout << "Matched location: " << location->getLocation() << std::endl;
				std::cout << "req.getpath: " << req.getPath()<< std::endl;
				break;
			}
		}

		if (!matchedLocation)
		{
			std::cout << "No matching location found" << std::endl;
			return getErrorPage(req, 404); // Not found
		}
		
		// if (_serverBlock.getRoot().empty() && matchedLocation->getRoot().empty())
		// 	return getErrorPage(req, 404); // Not found
		
		// Override root if location-specific root is defined
		if (!matchedLocation->getAlias().empty())
			// _rootDir = matchedLocation->getAlias();
			_filePath = matchedLocation->getAlias();
		else
			_filePath = _rootDir + req.getPath();
		
		if (!matchedLocation->getRoot().empty())
			_rootDir = matchedLocation->getRoot();

		// Override error pages if location-specific error pages are defined
		if (!matchedLocation->getErrorPages().empty())
			_errorPages = matchedLocation->getErrorPages();

		// Handle client_max_body_size for the specific location
		if (matchedLocation->getClientMaxBodySize() > 0)
			_maxBodySize = matchedLocation->getClientMaxBodySize(); //maybe not needed
		
		std::cout << "Max body size in Http Handler: " << _maxBodySize << std::endl;

		if (!matchedLocation->getReturn().second.empty())
		{
			Response			response;
			const auto			&redirectInfo = matchedLocation->getReturn();
			int					statusCode = redirectInfo.first;
			const std::string	&redirectPath = redirectInfo.second;

			response.setStatusLine("HTTP/1.1 " + std::to_string(statusCode) + " Redirect" + "\r\n");
			response.setHeader("Location", redirectPath + "\r\n");
			response.setBody("Redirecting to " + redirectPath);
			// response.setStatusLine("HTTP/1.1 " + std::to_string(matchedLocation->getReturn().first) + " Redirect" + "\r\n");
			// response.setHeader("Location", matchedLocation->getReturn().second + "\r\n");
			// response.setBody("Redirecting to " + matchedLocation->getReturn().second);
			return response;
			// return response.toString();
		}

		

		if (!matchedLocation->getUploadPath().empty())
		{
			// Handle file uploads logic (POST requests)
			std::filesystem::path	uploadPath = matchedLocation->getUploadPath(); // may need to be handled better

			if (!std::filesystem::exists(uploadPath))
				std::filesystem::create_directories(uploadPath);

			if (!std::filesystem::is_directory(uploadPath))
				return getErrorPage(req, 500); // Internal server error

			// Test write by attempting to create a temporary file
			std::ofstream	testFile((uploadPath / "test.tmp").string());

			if (!testFile.is_open())
				// throw std::runtime_error("Upload path is not writable");
				return getErrorPage(req, 500); // Internal server error

			testFile.close();
			std::filesystem::remove(uploadPath / "test.tmp");
		}
		std::cout << "Path: " << req.getPath() << std::endl;
		std::cout << "CGI Path: " << matchedLocation->getCgiPath() << std::endl;
		if (!matchedLocation->getCgiPath().empty())
			return handleCGI(req);
		std::cout << "Method: " << req.getMethod() << std::endl;
		if (req.getMethod() == "GET")
			return handleGET(req);
		else if (req.getMethod() == "POST")
			return handlePOST(req);
		else if (req.getMethod() == "DELETE")
			return handleDELETE(req);

		return getErrorPage(req, 405); // Method not allowed
	}
	catch(const SystemCallError &e)
	{
		return getErrorPage(req, 500); // Internal server error
	}
	catch (const std::runtime_error &e)
	{
		std::cout << "Error: " << e.what() << std::endl;
		return getErrorPage(req, 405); // Method not allowed
		// return e.what(); // Handle runtime errors (e.g., method not allowed)
	}
	
	
	
}
std::string HttpHandler::readFileError(std::string const &path)
{
	std::ifstream	file(path.c_str());

	if (!file.is_open())
		throw SystemCallError("Failed to open file"); //
	std::stringstream	read;

	read << file.rdbuf();
	file.close();
	return read.str();
}
Response	HttpHandler::handleGET(const Request &req)
{
	std::shared_ptr<LocationBlock> matchedLocation;

	for (const auto &location : _serverBlock.getLocations())
	{
		if (req.getPath() == location->getLocation())
		{
			matchedLocation = location;
			break;
		}
	}

	if (!matchedLocation->getIndex().empty())
	{
		// std::string	indexFilePath = _rootDir + req.getPath() + matchedLocation->getIndex();
		std::string	indexFilePath = _filePath + matchedLocation->getIndex();
		std::cout << "Matchedlocation.alias: " << matchedLocation->getAlias() << std::endl;
		std::cout << "Index file path: " << indexFilePath << std::endl;
		if (std::filesystem::exists(indexFilePath)
			&& std::filesystem::is_regular_file(indexFilePath))
			return handleFileRequest(req, indexFilePath); //maybe just in index requested location
	}

	// std::string	filePath = _rootDir + req.getPath();
	// std::string	filePath;

	// if (!matchedLocation->getAlias().empty())
	// 	filePath = matchedLocation->getAlias();
	// else
	// 	filePath = _rootDir + req.getPath();
	std::cout << "File path: " << _filePath << std::endl;
	Response	response;

	// Check if the path is a directory
	if (std::filesystem::is_directory(_filePath))
	{
		std::shared_ptr<LocationBlock> matchedLocation;

		for (const auto &location : _serverBlock.getLocations())
		{
			if (req.getPath() == location->getLocation())
			{
				matchedLocation = location;
				break;
			}
		}

		if (matchedLocation->getAutoindex())
		{
			std::ostringstream	directoryListing;

			directoryListing << "<html><body><h1>Index of " << req.getPath() << "</h1><ul>";
			for (const auto &entry : std::filesystem::directory_iterator(_filePath))
			{
				directoryListing << "<li><a href=\"" << entry.path().filename().string() << "\">";
				directoryListing << entry.path().filename().string() << "</a></li>";
			}
			directoryListing << "</ul></body></html>";

			response.setStatusLine("HTTP/1.1 200 " + getStatusMessage(200) + "\r\n");
			response.setBody(directoryListing.str());
			response.setHeader("Content-Type", "text/html");
			
			return response;
			// return response.toString();
		}
	}

	return handleFileRequest(req, _filePath);
}

// Adding a helper function for file requests
Response	HttpHandler::handleFileRequest(const Request &req, const std::string &filePath)
{
	Response	response;

	int fd = open(filePath.c_str(), O_RDONLY);

	if (fd == -1)
	{
		int			statusCode = (errno == EACCES) ? 403 : 404;
		std::string	errorPage = _errorPages.at(statusCode);

		fd = open((_rootDir + "/" + errorPage).c_str(), O_RDONLY);
	}

	try
	{
		char		buffer[1024];
		std::string	content;
		ssize_t		bytesRead;

		while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0)
			content.append(buffer, bytesRead);

		if (close(fd) == -1)
			handleError("close file descriptor");

		response.setStatusLine("HTTP/1.1 200 " + getStatusMessage(200) + "\r\n");
		response.setBody(content);
		response.setHeader("Content-Type", "text/html");
		response.setHeader("Content-Length", std::to_string(content.size()) + "\r\n");
		return response;
		// return response.toString();
	}

	catch (const SystemCallError &e)
	{
		if (close(fd) == -1)
			handleError("close file descriptor");
		// throw;
		return getErrorPage(req, 500); // Internal server error
	}
}

Response	HttpHandler::handlePOST(const Request &req)
{
	std::string	contentType = req.getHeader("Content-Type");
	Response	response;

	if (contentType.find("multipart/form-data") != std::string::npos)
	{
		// Extract the boundary parameter
		std::string			boundary = "--" + contentType.substr(contentType.find("boundary=") + 9);
		std::istringstream	bodyStream(req.getBody());
		std::string			line;

		while (std::getline(bodyStream, line))
		{
			if (line == boundary)
			{
				std::string	disposition, partContentType;

				std::getline(bodyStream, disposition);
				std::getline(bodyStream, partContentType);
				std::getline(bodyStream, line); // Empty line before part content

				// Parse disposition to check for filename(file upload)
				bool	isFileUpload = disposition.find("filename=") != std::string::npos;
				
				std::ostringstream	partData;

				// Read part data until boundary is reached
				while (std::getline(bodyStream, line)
						&& line != boundary && line != boundary + "--")
					partData << line << "\n";

				if (isFileUpload)
				{
					std::string	filename = extractFilename(disposition);

					saveFile(filename, partData.str());
				}
				else
					response.setBody(partData.str());
			}
		}

		response.setStatusLine("HTTP/1.1 200 " + getStatusMessage(200) + "\r\n");
		response.setHeader("Content-Type", "multipart/form-data");
		response.setHeader("Content-Length", std::to_string(response.getBody().size()) + "\r\n");
	}
	else if (!req.getBody().empty())
	{
		response.setStatusLine("HTTP/1.1 200 " + getStatusMessage(200) + "\r\n");
		response.setBody(req.getBody());
		response.setHeader("Content-Length", std::to_string(req.getBody().size()) + "\r\n");
		response.setHeader("Content-Type", "text/plain");
	}
	else
	{
		response.setStatusLine("HTTP/1.1 400 " + getStatusMessage(400) + "\r\n");
		response.setBody("Empty body in POST request\n");
	}
	return response;
	// return response.toString();
}

std::string	HttpHandler::extractFilename(const std::string& disposition)
{
	size_t	pos = disposition.find("filename=");

	if (pos != std::string::npos)
	{
		std::string	filename = disposition.substr(pos + 10); // Skip "filename=\""
		size_t		endPos = filename.find('"');

		return filename.substr(0, endPos);
	}
	return "uploaded_file";
}

void	HttpHandler::saveFile(const std::string &filename, const std::string &fileData)
{
	std::ofstream	file(filename, std::ios::binary);

	file.write(fileData.c_str(), fileData.size());
	file.close();
}

Response	HttpHandler::handleDELETE(const Request &req)
{
	// std::string filePath = _rootDir + req.getPath();
	(void)req;
	Response	response;

	if (unlink(_filePath.c_str()) == 0)
	{
		response.setStatusLine("HTTP/1.1 200 " + getStatusMessage(200) + "\r\n");
		response.setBody("File deleted successfully\n");
	}
	else if (errno == EACCES)
	{
		response.setStatusLine("HTTP/1.1 403 " + getStatusMessage(403) + "\r\n");
		response.setBody("Permission denied\n");
	}
	else if (errno == ENOENT)
	{
		response.setStatusLine("HTTP/1.1 404 " + getStatusMessage(404) + "\r\n");
		response.setBody("File not found\n");
	}
	return response;
	// return response.toString();
}

Response HttpHandler::handleCGI(const Request &req)
{
	std::cout << "Handling CGI" << std::endl;
	return _cgiHandler.execute(req);
}

std::string	HttpHandler::getStatusMessage(int statusCode)
{
	    // Map of status codes to their messages
    static const std::unordered_map<int, std::string>	statusMessages = {
        {100, "Continue"},
        {101, "Switching Protocols"},
        {102, "Processing"},
        {200, "OK"},
        {201, "Created"},
        {202, "Accepted"},
        {203, "Non-Authoritative Information"},
        {204, "No Content"},
        {205, "Reset Content"},
        {206, "Partial Content"},
        {207, "Multi-Status"},
        {300, "Multiple Choices"},
        {301, "Moved Permanently"},
        {302, "Found"},
        {303, "See Other"},
        {304, "Not Modified"},
        {305, "Use Proxy"},
        {307, "Temporary Redirect"},
        {308, "Permanent Redirect"},
        {400, "Bad Request"},
        {401, "Unauthorized"},
        {402, "Payment Required"},
        {403, "Forbidden"},
        {404, "Not Found"},
        {405, "Method Not Allowed"},
        {406, "Not Acceptable"},
        {407, "Proxy Authentication Required"},
        {408, "Request Timeout"},
        {409, "Conflict"},
        {410, "Gone"},
        {411, "Length Required"},
        {412, "Precondition Failed"},
        {413, "Payload Too Large"},
        {414, "URI Too Long"},
        {415, "Unsupported Media Type"},
        {416, "Range Not Satisfiable"},
        {417, "Expectation Failed"},
        {418, "I'm a teapot"}, // Easter egg from RFC 2324
        {422, "Unprocessable Entity"},
        {426, "Upgrade Required"},
        {500, "Internal Server Error"},
        {501, "Not Implemented"},
        {502, "Bad Gateway"},
        {503, "Service Unavailable"},
        {504, "Gateway Timeout"},
        {505, "HTTP Version Not Supported"}
    };

    // Find the status code in the map and return the corresponding message
    auto	it = statusMessages.find(statusCode);

    if (it != statusMessages.end())
        return it->second;

    return "Unknown Status Code";
}

size_t	HttpHandler::getMaxBodySize(const std::string &request)
{
	// std::cout << "Max body size in Http Handler own method: " << _maxBodySize << std::endl;
	Request							req(request);
	std::shared_ptr<LocationBlock>	matchedLocation;

	for (const auto &location : _serverBlock.getLocations())
	{
		// if (req.getPath().find(location->getLocation()) == 0)
		if (req.getPath() == location->getLocation())
		{
			matchedLocation = location;
			break;
		}
	}

	if (!matchedLocation)
	{
		return _serverBlock.getClientMaxBodySize();
	}
	return _maxBodySize;
}
