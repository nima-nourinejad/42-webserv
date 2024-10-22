/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SystemCallError.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asohrabi <asohrabi@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 17:10:38 by asohrabi          #+#    #+#             */
/*   Updated: 2024/10/22 17:23:18 by asohrabi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SystemCallError.hpp"

SystemCallError::SystemCallError(const std::string &msg) : std::runtime_error(msg) {}

void	handleError(const std::string &context)
{
    throw SystemCallError(context + " failed.");
}
