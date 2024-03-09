/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetRequest.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abizyane <abizyane@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/12 21:58:16 by abizyane          #+#    #+#             */
/*   Updated: 2024/03/09 13:27:47 by abizyane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "GetRequest.hpp"


GetRequest::GetRequest(std::string &method, std::string &uri, ProcessRequest& parse)
	: _method(method), _uri(uri), _version("HTTP/1.1"), _parse(parse){
	_isChunked = false;
	_contentLength = 0;
	_hasBody = false;
	_bodyIndex = 0;
	_fileName = ".requestbody";
}

std::string		GetRequest::getMethod( void ) const{
	return _method;
}

std::string		GetRequest::getUri( void ) const{
	return _uri;
}

std::map<std::string, std::string>	GetRequest::getHeaders( void ) const{
	return _headers;
}

std::string		GetRequest::getBody( void ) {
	std::string line;
	_body >> line;
	return line;
}

ProcessRequest&	GetRequest::getParse( void ) const{
	return _parse;
}

e_statusCode	GetRequest::checkHeaders(void){
	if (_headers.find("Host") == _headers.end())
		return HTTP_BAD_REQUEST;
	
	if (_headers.find("Content-Length") != _headers.end() || _headers.find("Transfer-Encoding") != _headers.end()){
		_hasBody = true;
		if (_headers.find("Transfer-Encoding") != _headers.end()){
			if (_headers.find("Transfer-Encoding")->second != "chunked")
				return HTTP_NOT_IMPLEMENTED;
			_isChunked = true;
		} 
		if (!_isChunked){
			if (_headers["Content-Length"].find_first_not_of("0123456789") != std::string::npos)
				return HTTP_BAD_REQUEST;
			_contentLength = strtoll(_headers["Content-Length"].c_str(), NULL, 10);
		}
		std::srand(std::time(0));
		for (size_t i = 0; i < 20; i++)
			_fileName.push_back(to_str(std::rand())[0]);
		_body.open(_fileName.c_str(), std::ios::out | std::ios::in | std::ios::trunc);
		if (!_body.is_open()){
			_parse.setParseState(Error);
			return HTTP_INTERNAL_SERVER_ERROR;
		}
	}
	else
		_parse.setParseState(Done);
	if (_hasBody)
		_parse.setParseState(Body);
	return HTTP_OK;
}

e_statusCode	GetRequest::parseHeader(std::string &line){
	try{
		if (line.find(":") == std::string::npos)
			return HTTP_BAD_REQUEST;
		std::string key = line.substr(0, line.find(":"));
		line.erase(0, line.find(":") + 1);
		line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
		line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
		std::string value = line;
		line.clear();
        std::string allowedChars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-!#$%&'*+-.^_|~");
		if (key.empty() || value.empty() || _headers.find(key) != _headers.end() ||
			key.find_first_not_of(allowedChars) != std::string::npos)
			return HTTP_BAD_REQUEST;
		_headers[key] = value;
	}catch(const std::exception &){
		return HTTP_BAD_REQUEST;
	}
	return HTTP_OK;
}

e_statusCode	GetRequest::parseBody(std::string &line){ // TODO: i think that we don't need this function
	std::stringstream ss(line);
	std::string	str;
	size_t	i = 0;
	try{
		if (!_isChunked){
			str = ss.str();
			for (; _bodyIndex + i < _contentLength && i < str.size(); i++);
			_body.write(str.c_str(), i);
			_bodyIndex += i;
			if(_bodyIndex == _contentLength)
				_parse.setParseState(Done);
		}
		else{
			std::getline(ss, str, '\n');
			size_t	chunkLen = strtoll(str.c_str(), NULL, 16);
			str.clear();
			str = ss.str();
			for (; _bodyIndex + i < chunkLen && i < str.size(); i++);
			_body.write(str.c_str(), i);
			_bodyIndex += i;
			if (chunkLen == 0)
				_parse.setParseState(Done);
			else if (_bodyIndex == chunkLen)
				_bodyIndex = 0;
		}
	}catch(const std::exception &){
		_parse.setParseState(Error);
		return HTTP_BAD_REQUEST;
	}
	line.clear();
	return HTTP_OK;
}

GetRequest::~GetRequest( void ){
	if (_body.is_open()){
		_body.close();
		std::remove(_fileName.c_str());
	}
}