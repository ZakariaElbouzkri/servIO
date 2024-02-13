/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProcessRequest.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abizyane <abizyane@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/12 16:04:49 by abizyane          #+#    #+#             */
/*   Updated: 2024/02/13 16:39:58 by abizyane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <utility>
#include <sstream>
#include <algorithm>

#include "IRequest.hpp"
#include "Response.hpp"
#include "GetRequest.hpp"
#include "PostRequest.hpp"
#include "DeleteRequest.hpp"

typedef enum {
	RequestLine,
	Headers,
	Body,
	Done,
	Error
} e_parseState;

class ProcessRequest {
	private:
		std::string 	_requestBuffer;
		e_parseState	_state;
		e_statusCode	_status;
		IRequest*		_request;
		Response*		_response;
		bool			_good;
		void			_parseRequestLine(std::string&	requestLine);
		void			_generateResponse( void );

	public:
		ProcessRequest();

		// class ErrorException : public std::exception {
		// 	public:
		// 		virtual const char* what() const throw() {
		// 			return "Error: Invalid request";
		// 		}
		// };

		void		parseLine(std::string	request);
		bool		good( void );

		IRequest*	getRequest( void );
		Response*	getResponse( void );
		
		~ProcessRequest(){};
};

/*

requestMethod = GET | POST | DELETE

class   RequestFactory:


parseFirstLine();

class   IRequest:
	void    parse( void ) = 0;

class GETRequest extends IRequest:
	void    parse( void ) = 0;

*/