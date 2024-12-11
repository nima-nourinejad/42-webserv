/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:39:26 by asohrabi          #+#    #+#             */
/*   Updated: 2024/12/11 11:51:27 by nnourine         ###   ########.fr       */
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
			std::cout << "..allowedMethods: " << allowedMethods[0] << std::endl;
			bool		isMethodAllowed = std::find(allowedMethods.begin(), allowedMethods.end(), method) != allowedMethods.end();
			
			std::cout << "..isMethodAllowed: " << isMethodAllowed << std::endl;
			return (isMethodAllowed);
		}
	}
	return false;
}

std::string	HttpHandler::_getErrorPage(int statusCode)
{
	// const std::map<int, std::string>	&errorPages = _serverBlock.getErrorPages();
	const auto	&errorPages = _serverBlock.getErrorPages();
	
	std::cout << "we are here" << std::endl;
	if (errorPages.find(statusCode) != errorPages.end())
	{
		std::cout << "we are here 2" << std::endl;
		return errorPages.at(statusCode); // Custom error page
	}
	std::cout << "we are not here" << std::endl;
	return "dummy.html";         // Default fallback
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

std::string	HttpHandler::createResponse(const std::string &request)
{
	Request	req(request);

	return handleRequest(req);
}

std::string	HttpHandler::handleRequest(const Request &req)
{
	try
	{
		// std::string validation = _validateRequest(req);
		// if (validation != "Ok")
		// 	return validation;

		std::cout << "beginning of handle request" << std::endl;
		
		std::shared_ptr<LocationBlock> matchedLocation;

		// Override root if location-specific root is defined
		// if (!matchedLocation->getRoot().empty())
		// 	_rootDir = matchedLocation->getRoot();

		// // Override error pages if location-specific error pages are defined
		// if (!matchedLocation->getErrorPages().empty())
		// 	_errorPages = matchedLocation->getErrorPages();

		// // Handle client_max_body_size for the specific location
		// if (matchedLocation->getClientMaxBodySize() > 0)
		// 	_maxBodySize = matchedLocation->getClientMaxBodySize(); //maybe not needed

		std::vector<std::shared_ptr<LocationBlock>> _copied_locations;

		std::cout << "Size of copied locations before: " << _copied_locations.size() << std::endl;
		
		_copied_locations = _serverBlock.getLocations();
		std::cout <<_copied_locations[0]->getLocation() << std::endl;

		std::cout << "Size of copied locations: " << _copied_locations.size() << std::endl;

		for (const auto &location : _serverBlock.getLocations())
		{
			if (req.getPath().find(location->getLocation()) == 0)
			{
				matchedLocation = location;
				break;
			}
		}

		if (!matchedLocation->getReturn().second.empty())
		{
			Response	response;

			response.setStatusLine("HTTP/1.1 " + std::to_string(matchedLocation->getReturn().first) + " Redirect");
			response.setHeader("Location", matchedLocation->getReturn().second);
			return response.toString();
		}

		if (!matchedLocation->getAlias().empty())
			_rootDir = matchedLocation->getAlias();

		if (!matchedLocation->getUploadPath().empty())
		{
			// Handle file uploads logic (POST requests)
			std::filesystem::path	uploadPath = matchedLocation->getUploadPath();

			if (!std::filesystem::exists(uploadPath))
				std::filesystem::create_directories(uploadPath);

			if (!std::filesystem::is_directory(uploadPath))
				throw std::runtime_error("Upload path is not writable");

			// Test write by attempting to create a temporary file
			std::ofstream	testFile((uploadPath / "test.tmp").string());

			if (!testFile.is_open())
				throw std::runtime_error("Upload path is not writable");

			testFile.close();
			std::filesystem::remove(uploadPath / "test.tmp");
		}

		if (!matchedLocation->getCgiPath().empty())
			return handleCGI(req);

		if (req.getMethod() == "GET")
			return handleGET(req);
		else if (req.getMethod() == "POST")
			return handlePOST(req);
		else if (req.getMethod() == "DELETE")
			return handleDELETE(req);

		return _getErrorPage(405); // Method not allowed
	}
	catch (const std::runtime_error &e)
	{
		return e.what(); // Handle runtime errors (e.g., method not allowed)
	}
	catch(const SystemCallError &e)
	{
		return _getErrorPage(500); // Internal server error
	}
}
std::string readFileError(std::string const & path)
{
	std::ifstream file(path.c_str());
	if (!file.is_open())
		throw SystemCallError("Failed to open file");
	std::stringstream read;
	read << file.rdbuf();
	file.close();
	return read.str();
}
std::string	HttpHandler::handleGET(const Request &req)
{
	std::shared_ptr<LocationBlock> matchedLocation;

	for (const auto &location : _serverBlock.getLocations())
	{
		if (req.getPath().find(location->getLocation()) == 0)
		{
			matchedLocation = location;
			break;
		}
	}

	if (!matchedLocation->getIndex().empty())
	{
		std::string	indexFilePath = _rootDir + req.getPath() + matchedLocation->getIndex();

		if (std::filesystem::exists(indexFilePath)
			&& std::filesystem::is_regular_file(indexFilePath))
			return handleFileRequest(indexFilePath); //maybe just in index requested location
	}

	std::string	filePath = _rootDir + req.getPath();
	Response	response;

	// Check if the path is a directory
	if (std::filesystem::is_directory(filePath))
	{
		std::shared_ptr<LocationBlock> matchedLocation;

		for (const auto &location : _serverBlock.getLocations())
		{
			if (req.getPath().find(location->getLocation()) == 0)
			{
				matchedLocation = location;
				break;
			}
		}

		if (matchedLocation->getAutoindex())
		{
			std::ostringstream	directoryListing;

			directoryListing << "<html><body><h1>Index of " << req.getPath() << "</h1><ul>";
			for (const auto &entry : std::filesystem::directory_iterator(filePath))
			{
				directoryListing << "<li><a href=\"" << entry.path().filename().string() << "\">";
				directoryListing << entry.path().filename().string() << "</a></li>";
			}
			directoryListing << "</ul></body></html>";

			response.setStatusLine("HTTP/1.1 200 OK");
			response.setBody(directoryListing.str());
			response.setHeader("Content-Type", "text/html");
			return response.toString();
		}
	}

	return handleFileRequest(filePath);
}

// Adding a helper function for file requests
std::string	HttpHandler::handleFileRequest(const std::string &filePath)
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

		response.setStatusLine("HTTP/1.1 200 OK");
		response.setBody(content);
		response.setHeader("Content-Type", "text/html");
		return response.toString();
	}

	catch (const SystemCallError &e)
	{
		if (close(fd) == -1)
			handleError("close file descriptor");
		throw;
	}
}

std::string	HttpHandler::handlePOST(const Request &req)
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

		response.setStatusLine("HTTP/1.1 200 OK");
		response.setHeader("Content-Type", "multipart/form-data");
		response.setHeader("Content-Length", std::to_string(response.getBody().size()));
	}
	else if (!req.getBody().empty())
	{
		response.setStatusLine("HTTP/1.1 200 OK");
		response.setBody(req.getBody());
		response.setHeader("Content-Length", std::to_string(req.getBody().size()));
		response.setHeader("Content-Type", "text/plain");
	}
	else
	{
		response.setStatusLine("HTTP/1.1 400 Bad Request");
		response.setBody("Empty body in POST request\n");
	}
	return response.toString();
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

std::string	HttpHandler::handleDELETE(const Request &req)
{
	std::string filePath = _rootDir + req.getPath();
	Response	response;

	if (unlink(filePath.c_str()) == 0)
	{
		response.setStatusLine("HTTP/1.1 200 OK");
		response.setBody("File deleted successfully\n");
	}
	else if (errno == EACCES)
	{
		response.setStatusLine("HTTP/1.1 403 Forbidden");
		response.setBody("Permission denied\n");
	}
	else if (errno == ENOENT)
	{
		response.setStatusLine("HTTP/1.1 404 Not Found");
		response.setBody("File not found\n");
	}
	return response.toString();
}

std::string HttpHandler::handleCGI(const Request &req)
{
	return _cgiHandler.execute(req);
}
