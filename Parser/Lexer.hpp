/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zel-bouz <zel-bouz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/12 12:32:57 by zel-bouz          #+#    #+#             */
/*   Updated: 2024/02/21 20:47:10 by zel-bouz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _Lexer_HPP_
#define _Lexer_HPP_

#include <iostream>
#include "Token.hpp"
#include <fstream>
#include <exception>
#include "../includes/Constants.hpp"
#include <cstring>


class	Lexer
{
	private:
		ssize_t		_line;
		ssize_t		_pos;
		std::string	_data;
		char		_curr;
		
		bool	isOpen;

		static std::map<std::string, Token::token>	_keywords;


		void	_advance( void );
		void	_skipComment( void );
		Token	_parseWord( void );

	public:
		Lexer( const std::string& filePath = CONF_PATH );
		~Lexer( void );
		Token	getNextToken( void );
		ssize_t	line( void ) const;
		
		bool	is_open( void ) const;
};

#endif
