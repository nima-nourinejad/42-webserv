/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SystemCallError.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 17:10:38 by asohrabi          #+#    #+#             */
/*   Updated: 2025/01/03 13:12:09 by nnourine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SystemCallError.hpp"

SystemCallError::SystemCallError(const std::string &msg) : std::runtime_error(msg) {}

void	handleError(const std::string &context)
{
    throw SystemCallError(context);
}
