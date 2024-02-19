/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abizyane <abizyane@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/06 23:07:10 by abizyane          #+#    #+#             */
/*   Updated: 2024/02/18 12:51:45 by abizyane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#pragma once

#include "ProcessRequest.hpp"
#include "../Configuration/MainConf.hpp"
#include <iostream>

class Response {
	private :
		IRequest*							_request;
		std::string							_requestMethod;
		std::string							_requestUri;
		std::map<std::string, std::string>	_requestHeaders;
		std::string							_requestBody;
		MainConf*							_mainConf;
		e_statusCode						_status;
		bool								_good;
		std::string							_response;
		std::map<std::string, std::string>  _headers;
		std::vector<char>					_body;
		void								_prepareResponse( void );
	
	public :
		Response(IRequest& request);

		bool			good( void );

		std::string		GetResponse(void);
		~Response();

};

		// std::string& 						GetResponse(void);
		// size_t		 						GetLength(void);
		// std::string& 						GetBody(void);
		// std::string& 						GetContentType(void);
		// e_statusCode 						GetStatusCode(void);
		// std::map<std::string, std::string>& GetHeaders(void);
		// std::string& 						GetHeader(const std::string& key);