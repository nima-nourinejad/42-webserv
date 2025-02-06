/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asohrabi <asohrabi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:39:26 by asohrabi          #+#    #+#             */
/*   Updated: 2025/02/06 18:45:16 by asohrabi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpHandler.hpp"

HttpHandler::HttpHandler(ServerBlock &serverConfig, int port)
	: _cgiHandler(serverConfig), _serverBlock(serverConfig), _maxBodySize(0)
	, _serverName(serverConfig.getServerName()), _port(port), _locationFlag(0)
{
	if (!serverConfig.getLocations().empty())
		_rootDir = serverConfig.getLocations()[0]->getRoot();
	else
	{
		_rootDir = serverConfig.getRoot();
		_locationFlag = 1;
	}

	
	_errorPages[400] = "html/default_400.html";
	_errorPages[401] = "html/default_401.html";
	_errorPages[402] = "html/default_402.html";
	_errorPages[403] = "html/default_403.html";
	_errorPages[404] = "html/default_404.html";
	_errorPages[405] = "html/default_405.html";
	_errorPages[406] = "html/default_406.html";
	_errorPages[407] = "html/default_407.html";
	_errorPages[408] = "html/default_408.html";
	_errorPages[409] = "html/default_409.html";
	_errorPages[410] = "html/default_410.html";
	_errorPages[411] = "html/default_411.html";
	_errorPages[412] = "html/default_412.html";
	_errorPages[413] = "html/default_413.html";
	_errorPages[414] = "html/default_414.html";
	_errorPages[415] = "html/default_415.html";
	_errorPages[416] = "html/default_416.html";
	_errorPages[417] = "html/default_417.html";
	_errorPages[418] = "html/default_418.html";
	_errorPages[422] = "html/default_422.html";
	_errorPages[426] = "html/default_426.html";
	_errorPages[500] = "html/default_500.html";
	_errorPages[501] = "html/default_501.html";
	_errorPages[502] = "html/default_502.html";
	_errorPages[503] = "html/default_503.html";
	_errorPages[504] = "html/default_504.html";
	_errorPages[505] = "html/default_505.html";

	for (const std::pair<const int, std::string> &errorPage : serverConfig.getErrorPages())
		_errorPages[errorPage.first] = errorPage.second;
}

HttpHandler::~HttpHandler() {}

std::shared_ptr<LocationBlock>	HttpHandler::findMatchedLocation(const Request &req)
{
	std::string						path;
	std::shared_ptr<LocationBlock>	matchedLocation = nullptr;
	
	for (const std::shared_ptr<LocationBlock> &location : _serverBlock.getLocations())
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
	std::shared_ptr<LocationBlock>	matchedLocation = findMatchedLocation(req);
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
	std::shared_ptr<LocationBlock>	matchedLocation = findMatchedLocation(req);
	std::string						location_uri = matchedLocation->getLocation();
	
	if (location_uri != req.getPath())
		return true;
	return false;
}

bool	HttpHandler::_isMethodAllowed(const Request &req)
{
	std::shared_ptr<LocationBlock>	matchedLocation = findMatchedLocation(req);

	if (!matchedLocation)
		return false;

	const std::vector<std::string>	&allowedMethods = matchedLocation->getLimitExcept();
	bool							isMethodAllowed = std::find(allowedMethods.begin(), allowedMethods.end(), req.getMethod())
															!= allowedMethods.end();
		
		return isMethodAllowed;
}

Response	HttpHandler::getErrorPage(const Request &req, int statusCode)
{
	Response						response;
	std::shared_ptr<LocationBlock>	matchedLocation = findMatchedLocation(req);

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

int	HttpHandler::_validateRequest(const Request &req)
{
	if (req.getHttpVersion() != "HTTP/1.1")
	{
		if (req.getHttpVersion() == "HTTP/0.9" || req.getHttpVersion() == "HTTP/1.0" || req.getHttpVersion() == "HTTP/2" || req.getHttpVersion() == "HTTP/3")
			return 505;
		return 400;
	}
	std::string	method = req.getMethod();
	if (method != "GET" && method != "POST" && method != "OPTIONS" && method != "DELETE" && method != "PUT" && method != "HEAD" && method != "TRACE" && method != "CONNECT" && method != "PATCH")
		return 400;

	std::shared_ptr<LocationBlock>	matchedLocation = findMatchedLocation(req);

	if (!matchedLocation)
		return 404;

	if (!_isMethodAllowed(req))
		return 405;

	if (req.getHeader("Host").empty()
	|| ((req.getHeader("Host") != (_serverName + ":" + std::to_string(_port)))
	&& (req.getHeader("Host") != (_serverBlock.getHost() + ":" + std::to_string(_port)))))
		return 400;

	if (method == "POST" && req.getHeader("Content-Length").empty())
		return 411;

	size_t	contentLength;

	if (req.getHeader("Content-Length").empty())
		contentLength = 0;
	else
	{
		try
		{
			contentLength = (std::stoul)(req.getHeader("Content-Length"));
		}
		catch(...)
		{
			return 400;
		}
	}

	if (req.getHeader("Transfer-Encoding") != "chunked" && contentLength != req.getBody().size())
		return 400;

	if(req.getHeader("Transfer-Encoding") == "chunked" && !req.getHeader("Content-Length").empty())
		return 400;

	if (req.getBody().size() > _maxBodySize)
		return 413;

	for (const std::shared_ptr<LocationBlock> &location : _serverBlock.getLocations())
	{
		if (req.getPath() == location->getLocation())
		{
			if (!location->getCgiPath().empty() && !std::filesystem::exists(location->getCgiPath()))
				return 500;

			if (!(location->getRoot().empty()) && !std::filesystem::exists(location->getRoot()))
				return 500;
		}
	}
	return 200;
}

bool	HttpHandler::isValidLines(const std::string &request)
{
	std::istringstream	stream(request);
	std::string			line;

	if (!std::getline(stream, line))
		return false;

	std::istringstream	requestLine(line);
	std::string			method;
	std::string			path;
	std::string			httpVersion;

	if (!(requestLine >> method >> path >> httpVersion))
		return false;

	if (method.empty() || path.empty() || httpVersion.empty())
		return false;

	while (std::getline(stream, line) && line != "\r")
	{
		std::size_t	colon = line.find(':');

		if (colon == std::string::npos)
			return false;
		
		if (line.size() != 0 && line[line.size() - 1] != '\r')
			return false;
	}
	if (line != "\r")
		return false;

	return true;
}

Response	HttpHandler::createResponse(const std::string &request)
{	
	Request	req;

	// if (isValidLines(request))
		req = Request(request, 0);
	// else
	// 	req = Request(request, 400);

	_maxBodySize = getMaxBodySize(request, 0);

	return handleRequest(req);
}

Response	HttpHandler::handleRequest(const Request &req)
{
	try
	{
		if (_locationFlag == 1)
			return getErrorPage(req, 404);

		// int validation = _validateRequest(req);
		// if (validation != 200)
		// 	return getErrorPage(req, validation);
		
		std::shared_ptr<LocationBlock>	matchedLocation = findMatchedLocation(req);

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
			Response							response;
			const std::pair<int, std::string>	&redirectInfo = matchedLocation->getReturn();
			int									statusCode = redirectInfo.first;
			const std::string					&redirectPath = redirectInfo.second;

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
		throw SystemCallError("Failed to open file");

	std::stringstream	read;

	read << file.rdbuf();
	file.close();
	return read.str();
}

std::string getFileExtension(std::string const &filename)
{
	std::string extension = filename.substr(filename.find_last_of(".") + 1);
	return extension;
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
	std::string						extention = getFileExtension(fileName);
	std::string						contentType = getContentType(extention);
	std::shared_ptr<LocationBlock>	matchedLocation = findMatchedLocation(req);
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
	std::shared_ptr<LocationBlock> matchedLocation = findMatchedLocation(req);

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
			for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(_filePath))
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

void ensureUploadPathExists(const std::string& uploadPath)
{
    std::filesystem::path directory(uploadPath);

    if (std::filesystem::exists(directory)) 
		return;
	if (std::filesystem::create_directories(directory))
		return;
	throw SystemCallError("Failed to create upload directory");
}

Response HttpHandler::handlePOST(const Request &req)
{
	std::string contentType = req.getHeader("Content-Type");
	Response response;

	std::shared_ptr<LocationBlock> matchedLocation = findMatchedLocation(req);

	if (!matchedLocation)
		return getErrorPage(req, 404);
	
	try
	{
		ensureUploadPathExists(_uploadPath);
	}
	catch(...)
	{
		return getErrorPage(req, 500);
	}
	

	bool		isMultiPart = contentType.find("multipart/form-data") != std::string::npos;
	std::string boundary;
	std::string data;
	// bool		isFileUpload = false;
	std::string disposition;
	std::string fileName;

	if (isMultiPart)
	{
		size_t pos = contentType.find("boundary=");

		if (pos == std::string::npos)
			return getErrorPage(req, 400);
		
		boundary = "--" + contentType.substr(contentType.find("boundary=") + 9);
		std::istringstream bodyStream(req.getBody());
		std::string line;

		while (std::getline(bodyStream, line))
		{
			if (line == boundary)
			{
				std::string partContentType;
				std::getline(bodyStream, disposition);
				
				fileName = extractFileName(disposition);
				
				if (std::filesystem::exists((std::string)(_uploadPath) + "/" + fileName))
					return getErrorPage(req, 409);
				
				std::getline(bodyStream, partContentType);
				std::getline(bodyStream, line);

				// isFileUpload = disposition.find("filename=") != std::string::npos;
				std::ostringstream partData;

				while (std::getline(bodyStream, line) && line != boundary)
					partData << line << "\n";

				data = partData.str();

				if (data.find(boundary) != std::string::npos)
					data = data.substr(0, data.find(boundary));
			}
		}
		if (isMultiPart)
		{
			boundary = boundary.substr(0, boundary.length() - 1);
			if (data.find(boundary) != std::string::npos)
				data = data.substr(0, data.find(boundary));
		}
		// if (isFileUpload)
		// {
			// std::string filename = extractFileName(disposition);
			std::string	filePath = matchedLocation->getUploadPath() + "/" + fileName;
			saveFile(filePath, data);

			response.setStatusLine("HTTP/1.1 201 " + getStatusMessage(201) +"\r\n");
			response.setHeader("Content-Type", "text/plain");
			response.setBody("File uploaded successfully. File Name: " + fileName);
			return response;
		// }
		// else
		// {
		// 	response.setStatusLine("HTTP/1.1 200 " + getStatusMessage(200) +"\r\n");
		// 	response.setHeader("Content-Type", "text/plain");
		// 	response.setBody("Non-file data processed successfully.");
		// 	return response;
		// }
		return response;
	}

	else if (!req.getBody().empty())
	{
		std::string fileName = getCurrentTime(); // timestamp function
		saveFile(matchedLocation->getUploadPath() + "/" + fileName, req.getBody()); //maybe without "/"

		response.setStatusLine("HTTP/1.1 200 " + getStatusMessage(200) +"\r\n");
		response.setHeader("Content-Type", req.getHeader("Content-Type"));
		// response.setBody(req.getBody()); 
		response.setBody("File uploaded successfully. File Name: " + fileName); //
		response.setHeader("Content-Length", std::to_string(req.getBody().size())); ///maybe this should be deleted because Nima adds it
		return response;
	}
	else
	{
		response.setStatusLine("HTTP/1.1 400 " + getStatusMessage(400) +"\r\n");
		response.setHeader("Content-Type", "text/plain");
		response.setBody("Empty body in POST request.");
		return response;
	}
}

std::string	HttpHandler::getCurrentTime()
{
	// time_t	current_time = time(nullptr);

	// if (current_time == -1)
	// 	throw SystemCallError("Failed to get current time");

	// return current_time;
	std::chrono::time_point<std::chrono::system_clock> timePoint = std::chrono::system_clock::now();
	std::time_t timeInSeconds = std::chrono::system_clock::to_time_t(timePoint);
	std::stringstream name;
	name << std::put_time(std::localtime(&timeInSeconds), "%Y-%m-%d-%H:%M:%S");
	return name.str();
	
}

std::string	HttpHandler::extractFileName(const std::string& disposition)
{
	size_t	pos = disposition.find("filename=");

	if (pos != std::string::npos)
	{
		std::string	filename = disposition.substr(pos + 10);
		size_t		endPos = filename.find('"');

		return filename.substr(0, endPos);
	}
	return (getCurrentTime());
}

void	HttpHandler::saveFile(const std::string &filename, const std::string &fileData)
{
	std::ofstream	file(filename, std::ios::binary);

	if (!file.is_open())
		throw SystemCallError("Failed to open file");

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
	
	if (std::filesystem::remove(path))
	{
		response.setStatusLine("HTTP/1.1 200 " + getStatusMessage(200) + "\r\n");
		// response.setHeader("Content-Type", "text/plain"); Nima adds this but because he is not sure commented it
		response.setBody("File deleted successfully. File URL: /uploads/" + fileName);
	}
	else if (errno == EACCES)
	{
		return getErrorPage(req, 403);
		// response.setStatusLine("HTTP/1.1 403 " + getStatusMessage(403) + "\r\n");
		// response.setBody("Permission denied\n");
	}
	else if (errno == ENOENT)
	{
		return getErrorPage(req, 404);
		// response.setStatusLine("HTTP/1.1 404 " + getStatusMessage(404) + "\r\n");
		// response.setBody("File not found\n");
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

	std::unordered_map<int, std::string>::const_iterator	it = statusMessages.find(statusCode);

	if (it != statusMessages.end())
		return it->second;

	return "Unknown Status Code";
}

size_t	HttpHandler::getMaxBodySize(const std::string &request, int errorStatus)
{
	Request							req(request, errorStatus);
	std::shared_ptr<LocationBlock>	matchedLocation = findMatchedLocation(req);

	if (matchedLocation && matchedLocation->getClientMaxBodySize() > 0)
		return matchedLocation->getClientMaxBodySize();
	if (_serverBlock.getClientMaxBodySize() > 0)
		return _serverBlock.getClientMaxBodySize();
		
	return DEF_MAX_BODY_SIZE;
}
