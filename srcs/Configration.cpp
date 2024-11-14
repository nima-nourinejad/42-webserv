/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Configration.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/14 09:37:08 by nnourine          #+#    #+#             */
/*   Updated: 2024/11/14 09:37:09 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Configration.hpp"

Configration::Configration (int port, std::string const & host, size_t maxBodySize)
    : port (port), host (host), maxBodySize (maxBodySize){};