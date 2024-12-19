/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   eventData.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nima <nnourine@student.hive.fi>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/15 13:05:42 by nnourine          #+#    #+#             */
/*   Updated: 2024/12/19 09:08:51 by nima             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

enum eventTypes
{
	LISTENING,
	CLIENT,
	PIPE,
};

struct eventData
{
	int index;
	int fd;
	int type;
};
