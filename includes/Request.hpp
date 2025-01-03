/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 16:31:09 by asohrabi          #+#    #+#             */
/*   Updated: 2025/01/03 13:31:28 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "SystemCallError.hpp"
#include <string>
#include <map>
#include <sstream>
#include <iostream>

class Request
{
	private:
		std::string							_method;
		std::string 						_path;
		std::string 						_httpVersion;
		std::map<std::string, std::string>	_headers;
		std::string							_body;

	public:
		Request();
		Request(const std::string &rawRequest, int errorStatus);
		~Request();

		const std::string					&getMethod() const;
		const std::string					&getPath() const;
		const std::string					&getHttpVersion() const;
		const std::string					&getHeader(const std::string& key) const;
		const std::string					&getBody() const;

		void								parse(const std::string& rawRequest);
};

#endif
