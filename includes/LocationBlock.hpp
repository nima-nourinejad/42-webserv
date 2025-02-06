/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationBlock.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akovalev <akovalev@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/24 18:46:15 by akovalev          #+#    #+#             */
/*   Updated: 2025/02/06 18:08:44 by akovalev         ###   ########.fr       */
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
		std::string					_location; // /path/to/resource
		std::string					_root; // /var/www/html
		std::string					_index; // index.html
		bool						_autoindex; // on | off
		std::vector<std::string>	_cgi_extension;	// .php, .py, .pl, .rb
		std::string					_cgi_path; // /usr/bin/php-cgi
		std::string					_upload_path; // path to upload files
		std::pair<int, std::string>	_return;	// defines a return code and a URL to redirect to
		std::string					_alias;
		size_t						_client_max_body_size; // size in bytes
		std::map<int, std::string>	_error_pages;
		std::vector<std::string>	_limit_except;

	public:
		LocationBlock();
		~LocationBlock();
		LocationBlock(std::string location);
		LocationBlock(const LocationBlock& original)
		{
			_location = original._location;
			_root = original._root;
			_index = original._index;
			_autoindex = original._autoindex;
			_cgi_extension = original._cgi_extension;
			_cgi_path = original._cgi_path;
			_upload_path = original._upload_path;
			_return = original._return;
			_alias = original._alias;
			_client_max_body_size = original._client_max_body_size;
			_error_pages = original._error_pages;
			_limit_except = original._limit_except;
		}
		LocationBlock& operator=(const LocationBlock& original) = default;

		std::string 				getLocation() const;
		std::string 				getRoot() const;
		std::string 				getIndex() const;
		bool 						getAutoindex() const;
		std::vector<std::string>	getCgiExtension() const;
		std::string					getCgiPath() const;
		std::string					getUploadPath() const;
		std::pair<int, std::string>	getReturn() const;
		std::string					getAlias() const;
		size_t						getClientMaxBodySize() const;
		std::vector<std::string>	getLimitExcept() const;
		std::map<int, std::string>	getErrorPages() const;
		void						setLocation(const std::string& location);
		void						setRoot(const std::string& root);
		void						setIndex(const std::string& index);
		void						setAutoindex(const std::string& autoindex);
		void						setCgiExtension(const std::vector<std::string>& cgi_extension);
		void						setCgiPath(const std::string& cgi_path);
		void						setUploadPath(const std::string& upload_path);
		void						setErrorPage(int code, const std::string& page);
		void						setReturn(std::string return_val, const std::string& url);
		void						setAlias(const std::string& alias);
		void						setClientMaxBodySize(const std::string& client_max_body_size);
		void						printLocationBlock();
		void						setLimitExcept(const std::vector<std::string>& vals);
};

#endif
