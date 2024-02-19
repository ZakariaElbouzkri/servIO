/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abizyane <abizyane@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/12 22:03:16 by abizyane          #+#    #+#             */
/*   Updated: 2024/02/18 12:20:46 by abizyane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "PostRequest.hpp"

PostRequest::PostRequest(std::string &method, std::string &uri, std::string &version, ProcessRequest& parse)
    : _method(method), _uri(uri), _version(version), _parse(parse){
	_contentLength = 0;
	_isChunked = false;
}

std::string		PostRequest::getMethod( void ) const{
	return _method;
}

std::string		PostRequest::getUri( void ) const{
	return _uri;
}

std::map<std::string, std::string>	PostRequest::getHeaders( void ) const{
	return _headers;
}

std::string		PostRequest::getBody( void ) const{
	return _body;
}

ProcessRequest&	PostRequest::getParse( void ) const{
	return _parse;
}

e_statusCode	PostRequest::parseHeader(std::string &line){
	try{
		if (line.find(":") == std::string::npos)
			return HTTP_BAD_REQUEST;
		std::string key = line.substr(0, line.find(":"));
		key.erase(key.find(":"));
		line.erase(0, line.find(":") + 1);
		if (line.find_first_of(" \t\n\r\f\v") == 0)
			return HTTP_BAD_REQUEST; //value cannot start with a whitespace
		line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
		std::string value = line;
		line.clear();
        std::string allowedChars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-!#$%&'*+-.^_|~");
		if (key.empty() || value.empty() || _headers.find(key) != _headers.end() ||
			key.find_first_not_of(allowedChars) != std::string::npos)
			return HTTP_BAD_REQUEST; //invalid header
		_headers[key] = value;
	}catch(const std::exception &){
		return HTTP_BAD_REQUEST;
	}
	return HTTP_OK;
}

e_statusCode	PostRequest::checkHeaders(void){
	if (_headers.find("Host") == _headers.end() || _headers.find("Content-Type") == _headers.end())
		return (HTTP_BAD_REQUEST);
	if (_headers.find("Content-Length") == _headers.end() && _headers.find("Transfer-Encoding") == _headers.end())
		return (HTTP_BAD_REQUEST);
	(_headers.find("Transfer-Encoding") != _headers.end()) ? _isChunked = true : _isChunked = false;
	(_isChunked) ? _contentLength = 0 : _contentLength = strtoll(_headers["Content-Length"].c_str(), NULL, 10);
	return HTTP_OK;
}

e_statusCode	PostRequest::parseBody(std::string &line){
	std::stringstream ss(line);
	std::string	str;

	try{
		if (!_isChunked){
			str = ss.str();
			size_t i = _bodyIndex;
			for (; i < _contentLength && i < str.size(); i++)
				_body += str[i];
			_bodyIndex = i;
			if(i == _contentLength)
				_parse.setParseState(Done);
		}
		else{
			std::getline(ss, str, '\n');
			str.erase(str.find_last_not_of(" \t\n\r\f\v") + 1);
			size_t	chunkLen = strtoll(str.c_str(), NULL, 16);
			str.clear();
			str = ss.str();
			size_t i = _bodyIndex;
			for (; i < chunkLen && i < str.size(); i++)
				_body += str[i];
			_bodyIndex = i;
			if (chunkLen == 0)
				_parse.setParseState(Done);
		}
	}catch(const std::exception &){
		_parse.setParseState(Error);
		return HTTP_BAD_REQUEST;
	}
	line.clear();
    return HTTP_OK;
}

PostRequest::~PostRequest( void ){
}