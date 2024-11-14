/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationBlock.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akovalev <akovalev@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/24 18:46:15 by akovalev          #+#    #+#             */
/*   Updated: 2024/11/14 21:05:33 by akovalev         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONBLOCK_HPP
#define LOCATIONBLOCK_HPP

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <set>
#include <regex>
#include <filesystem>

class LocationBlock
{
	private:
		std::string _location; // /path/to/resource
		std::string _root; // /var/www/html
		std::string _index; // index.html
		bool _autoindex; // on | off
		std::vector<std::string> _cgi_extension;	// .php, .py, .pl, .rb
		std::string _cgi_path; // /usr/bin/php-cgi
		std::string _upload_path; // /var/www/html/uploads
		std::string _proxy_pass; // http:// may actually not be needed as per subject
		std::pair<int, std::string> _return;	// defines a return code and a URL to redirect to
		std::string _alias;
		size_t _client_max_body_size; // size in bytes, needs to be converted if in human-readable format
		std::map<int, std::string> _error_pages;
		std::vector<std::string> _limit_except;
	public:
		LocationBlock(/* args */);
		~LocationBlock();
		LocationBlock(std::string location);
		std::string getLocation() const;
		std::string getRoot() const;
		std::string getIndex() const;
		bool getAutoindex() const;
		std::vector<std::string> getCgiExtension() const;
		std::string getCgiPath() const;
		std::string getUploadPath() const;
		std::string getProxyPass() const;
		std::pair<int, std::string> getReturn() const;
		std::string getAlias() const;
		size_t getClientMaxBodySize() const;
		std::vector<std::string> getLimitExcept() const;
		std::map<int, std::string> getErrorPages() const;
		void setLocation(const std::string& location);
		void setRoot(const std::string& root);
		void setIndex(const std::string& index);
		void setAutoindex(const std::string& autoindex);
		void setCgiExtension(const std::vector<std::string>& cgi_extension);
		void setCgiPath(const std::string& cgi_path);
		void setUploadPath(const std::string& upload_path);
		void setProxyPass(const std::string& proxy_pass);
		void setErrorPage(int code, const std::string& page);
		void setReturn(std::string return_val, const std::string& url);
		void setAlias(const std::string& alias);
		void setClientMaxBodySize(std::string& client_max_body_size);
		void printLocationBlock();
		void setLimitExcept(const std::vector<std::string>& vals);
};

#endif