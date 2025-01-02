/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asohrabi <asohrabi@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 17:26:44 by asohrabi          #+#    #+#             */
/*   Updated: 2025/01/02 13:59:02 by asohrabi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>
#include <sstream>

class Response
{
	private:
		std::string							_statusLine;
		std::map<std::string, std::string>	_headers;
		std::string							_body;
		// size_t								_maxBodySize;

	public:
		Response();
		Response(const std::string &statusLine);
		Response(const Response &other);
		Response &operator=(const Response &other);
		~Response();

		void								setStatusLine(const std::string &statusLine);
		void								setHeader(const std::string &key, const std::string &value);
		void								setBody(const std::string body);

		std::string							getStatusLine() const;
		std::string							getRawHeader() const;
		std::string							getBody() const;
		// size_t								getMaxBodySize() const;
		std::string							toString() const;
};

#endif
