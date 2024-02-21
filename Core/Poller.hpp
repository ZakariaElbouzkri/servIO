/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Poller.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nakebli <nakebli@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/15 14:32:32 by nakebli           #+#    #+#             */
/*   Updated: 2024/02/20 16:43:11 by nakebli          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../Configuration/MainConf.hpp"
#include <sys/socket.h>
#include <fcntl.h>
#include <poll.h>

typedef std::vector<struct pollfd>::iterator PollIter;

class	Poller {
	public:
		Poller( void );
		~Poller( void );
		void	pushFd( int fd, short events = POLLIN );
		void	erase( int fd );
		bool	poll( int timeout = 10 );
		struct pollfd& operator[]( const int& idx );
		size_t size( void ) const;

		// print

		void	printFds() {
			std::vector<struct pollfd>::iterator bgn = __pollfds.begin();
			while (bgn != __pollfds.end())
			{
				std::cout << bgn->fd << "\t" << bgn->events << std::endl;
				bgn ++;
			}
		}

	private:
		std::vector<struct pollfd>	__pollfds;
};
