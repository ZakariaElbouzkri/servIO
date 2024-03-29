/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConf.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zel-bouz <zel-bouz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/12 11:36:53 by zel-bouz          #+#    #+#             */
/*   Updated: 2024/03/26 21:08:47 by zel-bouz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConf.hpp"
#include "../utils/utils.hpp"
#include <fstream>

LocationConf::LocationConf( void ) : HTTP(), _locations(NULL), _extentions(NULL)
	, _redirect(NULL)
{
}

LocationConf::LocationConf( HTTP const& rhs ) : HTTP(rhs), _locations(NULL), _extentions(NULL)
	, _redirect(NULL)
{
}

LocationConf::~LocationConf( void )
{
	delete	_extentions;
	delete 	_redirect;
	if (_locations != NULL) {
		for (std::map<std::string, LocationConf*>::iterator it = _locations->begin(); it != _locations->end(); it++){
			delete it->second;
		}
	}
	delete _locations;
}


LocationConf::LocationConf( LocationConf const& rhs ) : HTTP(rhs), _locations(NULL), _extentions(NULL)
	, _redirect(NULL)
{
}

LocationConf&	LocationConf::operator=( const LocationConf& rhs )
{
	if (this != &rhs){
		HTTP::operator=(rhs);
	}
	return (*this);
}

void	LocationConf::addLocation( const std::string& url, LocationConf* location )
{
	if (_locations == NULL)
		_locations = new std::map<std::string, LocationConf*>();
	(*_locations)[url] = location;
}

void	LocationConf::addExtention( const std::string& extention )
{
	if (_extentions == NULL)
		_extentions = new std::set<std::string>();
	_extentions->insert(extention);
}

void	LocationConf::addRedirectpage( int code, const std::string& page )
{
	if (_redirect == NULL)
		_redirect = new std::pair<int, std::string> ();
	_redirect->first = code;
	_redirect->second = page;
}

bool	LocationConf::hasCgi( void ) const
{
	return _extentions != NULL;
}

bool	LocationConf::hasRedirect( void ) const
{
	return	_redirect != NULL; 	
}

// new implemented :

bool	LocationConf::isCgi( const std::string& extention )
{
	return _extentions != NULL && _extentions->find(extention) != _extentions->end();
}

std::string		LocationConf::getRoot( void ) const
{
	return ((_root != NULL) ? *_root : "");
}

bool	LocationConf::hasUpload( void ) const {
	return _uploadStore != NULL;
}

std::string		LocationConf::getUploadStore( void ) const 
{
	return ((_uploadStore != NULL) ? *_uploadStore : "");
}

std::string		LocationConf::getErrPage( int code, const std::string& defaultPag )
{
	if (_errorPage == NULL)
		return (defaultPag);
	std::string ans;
	std::map<int, std::string>::iterator it = _errorPage->find(code);
	if (it != _errorPage->end()) {
		std::string path = ((this->_root == NULL) ? "/" :normPath(*this->_root) + "/") + it->second;
		std::ifstream	file(path.c_str(), std::ios::in | std::ios::binary);
		if (file.is_open() && std::getline(file, ans, '\0'))
			return ans;
	}
	return defaultPag;
}

bool	LocationConf::methodIsAllowed( const std::string& method) const
{
	if (_allowed == NULL)
		return (false);
	std::set<std::string>::iterator it = _allowed->find( method );
	return (it != _allowed->end());
}

bool	LocationConf::dirListingEnabled() const
{
	return (_autoIndex);
}

size_t	LocationConf::getClientBodySize( void ) const
{
	return (_clientMaxBody);
}

void	LocationConf::passDirectiveToRoutes( void )
{
	if (_locations == NULL)
		return ;
	std::map<std::string, LocationConf*>::iterator first = _locations->begin();
	std::map<std::string, LocationConf*>::iterator last = _locations->end();
	for (; first != last; first++) {
		if (this->hasDirective("root") && !first->second->hasDirective("root")) {
			first->second->setRoot(*this->_root);
			first->second->markDirective("root");
		}
		if (this->_uploadStore != NULL && !first->second->hasDirective("upload_store")) {
			first->second->setUploadStore(*this->_uploadStore);
			first->second->markDirective("upload_store");
		}
		if (!first->second->hasDirective("autoindex")) {
			first->second->setAutoIndex(this->_autoIndex);
			first->second->markDirective("autoindex");
		}
		if (!first->second->hasDirective("clientBody")) {
			first->second->setClientBody(this->_clientMaxBody);
			first->second->markDirective("clientBody");
		}
		if (this->_errorPage != NULL) {
			std::map<int, std::string>::iterator it = _errorPage->begin();
			std::map<int, std::string>::iterator ite = _errorPage->end();
			for (; it != ite; it++) {
				if (!first->second->hasDirective("error_page:" + to_str(it->first))) {
					first->second->addErrorPage(it->first, it->second);
					first->second->markDirective("error_page:" + to_str(it->first));
				}
			}
		}
		if (this->_allowed != NULL) {
			std::set<std::string>::iterator it = _allowed->begin();
			std::set<std::string>::iterator ite = _allowed->end();
			for (; it != ite; it++) {
				if (first->second->hasDirective(*it) == false)
					first->second->allowMethod(*it);
			}
		}
		if (this->_index != NULL) {
			std::vector<std::string>::iterator it = _index->begin();
			std::vector<std::string>::iterator ite = _index->end();
			for (; it != ite; it++)
				first->second->addIndex(*it);
		}
		if (_extentions != NULL) {
			std::set<std::string>::iterator it = _extentions->begin();
			std::set<std::string>::iterator ite = _extentions->end();
			for (; it != ite; it++)
				first->second->addExtention(*it);
		}
		first->second->passDirectiveToRoutes();
	}
}

bool	LocationConf::hasIndex( void ) const
{
	return _index != NULL;
}

std::pair<int, std::string>	LocationConf::getRedirectPage( void ) const 
{
	return *_redirect;
}
const std::vector<std::string>&	LocationConf::getIndex( void ) const
{
	return *_index;
}

std::pair<std::string, LocationConf*>	LocationConf::getUri( std::string uri ) const
{
	std::string	resultUri;

    uri = normPath(uri);
	if (_locations == NULL) {
		return std::make_pair(to_str(""), (LocationConf*)NULL);
	}
    while (uri != "") {
		if (_locations->find(uri) != _locations->end())
			return std::make_pair(resultUri, (*_locations)[uri]);

		std::map<std::string, LocationConf*>::iterator it = _locations->begin();
		std::map<std::string, LocationConf*>::iterator ite = _locations->end();

		for (; it != ite; it++) {
			std::pair<std::string, LocationConf*>	ans = it->second->getUri(uri);
			if (ans.second != NULL) return ans;
		}

        std::size_t pos = uri.find_last_of('/');
        if (pos != std::string::npos) {
			resultUri = normPath(uri.substr(pos)) + normPath(resultUri);
            uri = uri.substr(0, pos + (pos == 0));
        }
    }
	return std::make_pair(to_str(""), (LocationConf*)NULL);
}