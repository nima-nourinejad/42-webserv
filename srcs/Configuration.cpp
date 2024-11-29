/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Configuration.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 09:37:08 by nnourine          #+#    #+#             */
/*   Updated: 2024/11/29 10:09:57 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Configuration.hpp"

Configuration::Configuration(int port, std::string const & host, size_t maxBodySize, std::string const & name)
    : port(port), host(host), maxBodySize(maxBodySize), name(name) {};
