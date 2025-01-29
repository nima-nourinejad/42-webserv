/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:39:26 by asohrabi          #+#    #+#             */
/*   Updated: 2025/01/29 14:45:43 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpHandler.hpp"

HttpHandler::HttpHandler(ServerBlock &serverConfig)
	: _cgiHandler(serverConfig), _rootDir(serverConfig.getLocations()[0]->getRoot())
	, _serverBlock(serverConfig), _maxBodySize(serverConfig.getClientMaxBodySize())
{
	//creating a default state of map
	_errorPages[404] = "html/default_404.html";
	_errorPages[500] = "html/default_500.html";
	//more defaults needed

	for (const auto &errorPage : serverConfig.getErrorPages())
		_errorPages[errorPage.first] = errorPage.second;
}

HttpHandler::~HttpHandler() {}

std::shared_ptr<LocationBlock>	HttpHandler::_findMatchedLocation(const Request &req)
{
	std::string						path;
	std::shared_ptr<LocationBlock>	matchedLocation;
	
	for (const auto &location : _serverBlock.getLocations())
	{
		if (!location->getUploadPath().empty() && (req.getPath().length() > location->getLocation().length() && req.getPath().substr(0, location->getLocation().length()) == location->getLocation()))
			path = location->getLocation();
		else
			path = req.getPath();
		if (path == location->getLocation())
		{
			matchedLocation = location;
			break;
		}
	}
	return matchedLocation;
}

std::string	HttpHandler::_getFileName(const Request &req)
{
	std::shared_ptr<LocationBlock>	matchedLocation = _findMatchedLocation(req);
	std::string						location_uri = matchedLocation->getLocation();
	
	if (location_uri != req.getPath())
	{
		std::string			rawFileName = req.getPath().substr(location_uri.length());
		std::ostringstream	decoded;
		
		for (size_t i = 0; i < rawFileName.length(); ++i)
		{
			if (rawFileName[i] == '%' && i + 2 < rawFileName.length())
			{
				std::string	hexValue = rawFileName.substr(i + 1, 2);
				char		decodedChar = static_cast<char>(std::stoi(hexValue, nullptr, 16));
				
				decoded << decodedChar;
				i += 2;
			}
			else
				decoded << rawFileName[i];
		}
		return decoded.str();
	}
	return "";
}

bool HttpHandler::_isDownload(const Request &req)
{
	std::shared_ptr<LocationBlock>	matchedLocation = _findMatchedLocation(req);
	std::string						location_uri = matchedLocation->getLocation();
	
	if (location_uri != req.getPath())
		return true;
	return false;
}

bool	HttpHandler::_isMethodAllowed(const std::string &method, const std::string &path)
{
	for (const auto &location : _serverBlock.getLocations())
	{
		if (path == location->getLocation())
		{
			const auto	&allowedMethods = location->getLimitExcept();
			bool		isMethodAllowed = std::find(allowedMethods.begin(), allowedMethods.end(), method) != allowedMethods.end();
			
			return (isMethodAllowed);
		}
	}
	return false;
}

Response	HttpHandler::getErrorPage(const Request &req, int statusCode)
{
	Response						response;
	std::shared_ptr<LocationBlock>	matchedLocation = _findMatchedLocation(req);

	if (matchedLocation && matchedLocation->getErrorPages().count(statusCode))
	{
		response.setStatusLine("HTTP/1.1 " + std::to_string(statusCode) + " " + getStatusMessage(statusCode) + "\r\n");
		response.setHeader("Content-Type", "text/html");

		if (matchedLocation->getRoot().empty())
			response.setBody(readFileError(_rootDir + "/" + matchedLocation->getErrorPages().at(statusCode)));
		else
			response.setBody(readFileError(matchedLocation->getRoot() + "/" + matchedLocation->getErrorPages().at(statusCode)));
		return response;
	}

	if (_serverBlock.getErrorPages().count(statusCode))
	{
		response.setStatusLine("HTTP/1.1 " + std::to_string(statusCode) + " " + getStatusMessage(statusCode) + "\r\n");
		response.setHeader("Content-Type", "text/html");
		response.setBody(readFileError(_serverBlock.getErrorPages().at(statusCode)));
		return response;
	}

	if (_errorPages.count(statusCode))
	{
		response.setStatusLine("HTTP/1.1 " + std::to_string(statusCode) + " " + getStatusMessage(statusCode) + "\r\n");
		response.setHeader("Content-Type", "text/html");
		response.setBody(readFileError(_errorPages.at(statusCode)));
		return response;
	}

	response.setStatusLine("HTTP/1.1 " + std::to_string(statusCode) + " " + getStatusMessage(statusCode) + "\r\n");
	response.setHeader("Content-Type", "text/html");
	response.setBody("<html><body><h1>Error " + std::to_string(statusCode) + ": " + getStatusMessage(statusCode) + "</h1></body></html>");
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
		if (req.getPath() == location->getLocation())
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
	Request	req(request, 0);

	return handleRequest(req);
}

Response	HttpHandler::handleRequest(const Request &req)
{
	try
	{
		// std::string validation = _validateRequest(req);
		// if (validation != "Ok")
		// 	return validation;
		
		std::shared_ptr<LocationBlock>	matchedLocation = _findMatchedLocation(req);

		if (!matchedLocation)
			return getErrorPage(req, 404);

		// if (_serverBlock.getRoot().empty() && matchedLocation->getRoot().empty())
		// 	return getErrorPage(req, 404); // Not found
		
		if (!matchedLocation->getAlias().empty())
			_filePath = matchedLocation->getAlias();
		else
		{
			if (matchedLocation->getUploadPath().empty())
				_filePath = _rootDir + req.getPath();
			else
				_filePath = matchedLocation->getUploadPath();
		}
		
		if (!matchedLocation->getRoot().empty())
			_rootDir = matchedLocation->getRoot();

		if (!matchedLocation->getErrorPages().empty())
			_errorPages = matchedLocation->getErrorPages();

		if (matchedLocation->getClientMaxBodySize() > 0)
			_maxBodySize = matchedLocation->getClientMaxBodySize();

		if (!matchedLocation->getReturn().second.empty())
		{
			Response			response;
			const auto			&redirectInfo = matchedLocation->getReturn();
			int					statusCode = redirectInfo.first;
			const std::string	&redirectPath = redirectInfo.second;

			response.setStatusLine("HTTP/1.1 " + std::to_string(statusCode) + " Redirect" + "\r\n");
			response.setHeader("Location", redirectPath + "\r\n");
			response.setBody("Redirecting to " + redirectPath);
			return response;
		}	

		if (!matchedLocation->getUploadPath().empty())
		{
			_uploadPath = matchedLocation->getUploadPath();
			
			if (!std::filesystem::exists(_uploadPath))
				std::filesystem::create_directories(_uploadPath);

			if (!std::filesystem::is_directory(_uploadPath))
				return getErrorPage(req, 500);

			std::ofstream	testFile((_uploadPath / "test.tmp").string());

			if (!testFile.is_open())
				return getErrorPage(req, 500);

			testFile.close();
			std::filesystem::remove(_uploadPath / "test.tmp");
		}
		if (!matchedLocation->getCgiPath().empty())
			return handleCGI(req);

		if (req.getMethod() == "GET" && _isDownload(req)) //adding another else if for root of uploads
			return handleDownload(req);
		else if (req.getMethod() == "GET")
			return handleGET(req);
		else if (req.getMethod() == "POST")
			return handlePOST(req);
		else if (req.getMethod() == "OPTIONS")
			return handleOPTIONS(req);
		else if (req.getMethod() == "DELETE")
			return handleDELETE(req);

		return getErrorPage(req, 405);
	}
	catch(const SystemCallError &e)
	{
		return getErrorPage(req, 500);
	}
	catch (const std::runtime_error &e)
	{
		std::cout << "Error: " << e.what() << std::endl;
		return getErrorPage(req, 405);
	}
}

std::string HttpHandler::readFileError(std::string const &path)
{
	std::ifstream	file(path.c_str());

	if (!file.is_open())
	{
		std::cout << "Path: " << path << std::endl;
		throw SystemCallError("Failed to open file");
	}
	std::stringstream	read;

	read << file.rdbuf();
	file.close();
	return read.str();
}

std::string getFileextention(std::string const &filename)
{
	std::string extention = filename.substr(filename.find_last_of(".") + 1);
	return extention;
}

std::string getContentType(const std::string& extension)
{
    if (extension == "html" || extension == "htm")
        return "text/html";
    else if (extension == "jpg" || extension == "jpeg")
        return "image/jpeg";
    else if (extension == "png")
        return "image/png";
    else if (extension == "gif")
        return "image/gif";
    else if (extension == "bmp")
        return "image/bmp";
    else if (extension == "svg")
        return "image/svg+xml";
    else if (extension == "ico")
        return "image/vnd.microsoft.icon";
    else if (extension == "pdf")
        return "application/pdf";
    else if (extension == "zip")
        return "application/zip";
    else if (extension == "txt")
        return "text/plain";
    else if (extension == "css")
        return "text/css";
    else if (extension == "js")
        return "application/javascript";
    else if (extension == "json")
        return "application/json";
    else if (extension == "xml")
        return "application/xml";
    else if (extension == "csv")
        return "text/csv";
    else if (extension == "mp3")
        return "audio/mpeg";
    else if (extension == "wav")
        return "audio/wav";
    else if (extension == "mp4")
        return "video/mp4";
    else if (extension == "webm")
        return "video/webm";
    else if (extension == "ogg")
        return "application/ogg";
    else if (extension == "woff" || extension == "woff2")
        return "font/woff";
    else if (extension == "ttf")
        return "font/ttf";
    else if (extension == "otf")
        return "font/otf";
	else if (extension == "c" || extension == "h" || extension == "cpp" || extension == "hpp")
		return "text/plain";
    else
        return "application/octet-stream";
}

Response	HttpHandler::handleDownload(const Request &req)
{
	std::string						fileName = _getFileName(req);
	std::string						finalFile = "/" + _getFileName(req);
	std::string						extention = getFileextention(fileName);
	std::string						contentType = getContentType(extention);
	std::shared_ptr<LocationBlock>	matchedLocation = _findMatchedLocation(req);
	std::string						filePath = matchedLocation->getUploadPath() + finalFile;
	Response						response;
	int								fd = open(filePath.c_str(), O_RDONLY);

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
		response.setHeader("Content-Type", contentType);
		
	}
	catch (const SystemCallError &e)
	{
		return getErrorPage(req, 500);
	}
	return response;
}

Response	HttpHandler::handleGET(const Request &req)
{
	std::shared_ptr<LocationBlock> matchedLocation = _findMatchedLocation(req);

	if (!matchedLocation->getIndex().empty())
	{
		std::string	filePath = _filePath;
		if (_filePath[filePath.size() - 1] != '/')
			filePath += "/";
			
		std::string	indexFilePath = filePath + matchedLocation->getIndex();

		if (std::filesystem::exists(indexFilePath)
			&& std::filesystem::is_regular_file(indexFilePath))
			return handleFileRequest(req, indexFilePath); //maybe just in index requested location
	}

	Response	response;

	if (std::filesystem::is_directory(_filePath))
	{
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
		}
	}

	return handleFileRequest(req, _filePath);
}

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
	}

	catch (const SystemCallError &e)
	{
		if (close(fd) == -1)
			handleError("close file descriptor");
		return getErrorPage(req, 500);
	}
}

Response	HttpHandler::handlePOST(const Request &req)
{
	std::string	contentType = req.getHeader("Content-Type");
	Response	response;

	std::shared_ptr<LocationBlock> matchedLocation = _findMatchedLocation(req);

	if (!matchedLocation)
		return getErrorPage(req, 404);

	if (contentType.find("multipart/form-data") != std::string::npos)
	{
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
				std::getline(bodyStream, line);

				bool	isFileUpload = disposition.find("filename=") != std::string::npos;
				
				std::ostringstream	partData;

				while (std::getline(bodyStream, line) && line != boundary)
					partData << line << "\n";
				
				std::string data = partData.str();
				boundary = boundary.substr(0, boundary.length() - 1);
				if (data.find(boundary) != std::string::npos)
					data = data.substr(0, data.find(boundary));

				if (isFileUpload)
				{
					std::string	filename = extractFilename(disposition);
					filename = matchedLocation->getUploadPath() + "/" + filename;
					saveFile(filename, data);
				}
				else
					response.setBody(data);
			}
		}
					
		std::cout << "File uploaded successfully" << std::endl;
		response.setStatusLine("HTTP/1.1 200 " + getStatusMessage(200) + "\r\n");
		response.setHeader("Content-Type", "multipart/form-data");
	}
	else if (!req.getBody().empty())
	{
		std::cout << "second state" << std::endl;
		response.setStatusLine("HTTP/1.1 200 " + getStatusMessage(200) + "\r\n");
		response.setBody(req.getBody());
		response.setHeader("Content-Length", std::to_string(req.getBody().size()) + "\r\n");
		response.setHeader("Content-Type", "text/plain");
	}
	else
	{
		std::cout << "Empty body in POST request" << std::endl;
		response.setStatusLine("HTTP/1.1 400 " + getStatusMessage(400) + "\r\n");
		response.setBody("Empty body in POST request\n");
	}
	return response;
}

std::string	HttpHandler::extractFilename(const std::string& disposition)
{
	size_t	pos = disposition.find("filename=");

	if (pos != std::string::npos)
	{
		std::string	filename = disposition.substr(pos + 10);
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

Response	HttpHandler::handleOPTIONS(const Request &req)
{
	(void)req;
	Response	response;

	response.setStatusLine("HTTP/1.1 204 " + getStatusMessage(204) + "\r\n");
	return response;
}



Response	HttpHandler::handleDELETE(const Request &req)
{
	std::string fileName = "/" + _getFileName(req);
	if (fileName.empty())
		return getErrorPage(req, 404);
	std::string				filePath = _uploadPath.c_str() + fileName;
	std::filesystem::path	path(filePath);
	Response				response;
	
	if (std::filesystem::remove(path) == 0)
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
}

Response HttpHandler::handleCGI(const Request &req)
{
	try
	{
		return _cgiHandler.execute(req);
	}
	catch (const std::runtime_error &e)
	{
		std::cerr << "CGI timeout error: " << e.what() << '\n';
		return getErrorPage(req, 504);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return getErrorPage(req, 500);
	}
}

std::string	HttpHandler::getStatusMessage(int statusCode)
{
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

    auto	it = statusMessages.find(statusCode);

    if (it != statusMessages.end())
        return it->second;

    return "Unknown Status Code";
}

size_t	HttpHandler::getMaxBodySize(const std::string &request, int errorStatus)
{
	Request							req(request, errorStatus);
	std::shared_ptr<LocationBlock>	matchedLocation = _findMatchedLocation(req);

	if (!matchedLocation)
	{
		return _serverBlock.getClientMaxBodySize();
	}
	return _maxBodySize;
}
