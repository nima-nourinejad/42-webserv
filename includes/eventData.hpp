/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   eventData.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asohrabi <asohrabi@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/15 13:05:42 by nnourine          #+#    #+#             */
/*   Updated: 2024/11/18 12:42:59 by asohrabi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

enum eventTypes
{
	LISTENING,
	CLIENT,
};

struct eventData
{
	int index;
	int fd;
	int type;
};
