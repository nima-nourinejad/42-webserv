/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SystemCallError.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asohrabi <asohrabi@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 17:11:19 by asohrabi          #+#    #+#             */
/*   Updated: 2024/11/04 17:28:12 by asohrabi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SYSTEMCALLERROR_HPP
#define SYSTEMCALLERROR_HPP

#include <stdexcept>
#include <string>

class SystemCallError : public std::runtime_error
{
	public:
		SystemCallError(const std::string &msg);
};

void	handleError(const std::string &context);

#endif
