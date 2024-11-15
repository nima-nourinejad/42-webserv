/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   eventData.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnourine <nnourine@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/15 13:05:42 by nnourine          #+#    #+#             */
/*   Updated: 2024/11/15 14:12:09 by nnourine         ###   ########.fr       */
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
} ;