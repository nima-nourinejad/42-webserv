/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Configration.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 09:37:55 by nnourine          #+#    #+#             */
/*   Updated: 2024/11/14 09:37:56 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include <map>
#include <string>

class Configration
{
		public:
	int port;
	std::string host;
	size_t maxBodySize;
	Configration (int port, std::string const & host, size_t maxBodySize);
};

#endif