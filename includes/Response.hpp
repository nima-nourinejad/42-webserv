/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asohrabi <asohrabi@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 17:26:44 by asohrabi          #+#    #+#             */
/*   Updated: 2024/12/28 11:36:01 by asohrabi         ###   ########.fr       */
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
		size_t								_maxBodySize;

	public:
		Response();
		Response(const std::string &statusLine);
		~Response();

		void								setStatusLine(const std::string &statusLine);
		void								setHeader(const std::string &key, const std::string &value);
		void								setBody(const std::string body);

		std::string							getStatusLine() const;
		std::string							getRawHeader() const;
		std::string							getBody() const;
		size_t								getMaxBodySize() const;
		std::string							toString() const;
};

#endif
