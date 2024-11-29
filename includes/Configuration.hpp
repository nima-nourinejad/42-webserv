/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Configuration.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 09:37:55 by nnourine          #+#    #+#             */
/*   Updated: 2024/11/29 10:09:13 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include <map>
#include <string>

class Configuration
{
	public:
		int			port;
		std::string	host;
		size_t		maxBodySize;
		std::string	name;

		Configuration(int port, std::string const & host, size_t maxBodySize, std::string const & name);
};

#endif
