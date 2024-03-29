#include "Parser.hpp"

Parser::Parser( void ) 
{
	if (!_lex.is_open()) {
		throw std::runtime_error((std::string(CONF_PATH) + " could not be opened").c_str());
	}
	conf = MainConf::getConf();
	_currTok = _lex.getNextToken();
}

Parser::~Parser( void ){
}

Parser::Parser( Parser const& rhs )
{
	*this = rhs;
}

Parser&	Parser::operator=( const Parser& rhs )
{
	(void)rhs;
	return (*this);
}

void	Parser::_error( const std::string& message )
{
	throw std::runtime_error(message.c_str());
}

void	Parser::_advance( Token tok ) 
{
	if (_currTok != tok)
		_error(slog("unexpected token %s while expecting %s at: %s:%d", to_str(_currTok).c_str(), to_str(tok).c_str(), CONF_PATH, _lex.line()));
	if (_currTok != Token::_EOF)
		_currTok = _lex.getNextToken();
}

void	Parser::parse( void )
{
	_advance(Token::HTTP);
	_advance(Token::OPEN_CURLY);
	while (_currTok != Token::CLOSE_CURLY) {
		switch (_currTok.type()) {
			case Token::ROOT:		_parseRoot(*conf); break;
			case Token::ERR_PAGE:	_parseErrorPage(*conf); break;
			case Token::ALLOW:		_parseMethods(*conf); break;
			case Token::DENY:		_parseMethods(*conf); break;
			case Token::INDEX:		_parseIndex(*conf); break;
			case Token::UPLOAD:		_parseUpload(*conf); break;
			case Token::AUTO:		_parseAutoIndex(*conf); break;
			case Token::BODY_SIZE:	_parseClientBody(*conf); break;
			case Token::SERVER:		conf->addServer(_parseServer()); break;
			default:
				_error(slog("unexpected token %s at: %s:%d", to_str(_currTok).c_str(), CONF_PATH, _lex.line()));
		}
	}
	if (conf->number_of_servers() == 0)
		_error("config file should contain at least one server config");
	_advance(Token::CLOSE_CURLY);
	_advance(Token::_EOF);
	conf->passDirectiveToServers();
}


void	Parser::_duplicateError(const std::string& directive)
{
	std::stringstream	ss;
	ss << "duplicated direcive " << directive;
	ss << " at " << CONF_PATH << ":" << _lex.line();
	_error(ss.str());
}

void	Parser::_parseRoot(HTTP& httpConf)
{
	_advance(Token::ROOT);
	if (httpConf.hasDirective("root"))
		_duplicateError("root");
	httpConf.setRoot(_currTok.data());
	httpConf.markDirective("root");
	_advance(Token::WORD);
	_advance(Token::SEMICOLEN);
}

void	Parser::_parseErrorPage(HTTP& httpConf)
{
	_advance(Token::ERR_PAGE);
	std::stringstream	ss1(_currTok.data());
	_advance(Token::WORD);
	std::stringstream	ss2(_currTok.data());
	_advance(Token::WORD);
	unsigned int code; ss1 >> code;
	std::string	page; ss2 >>  page;
	if (!ss1.eof() || code < 400 || code > 599)
		_error(slog("invalid http error code %d at: %s:%d", code, CONF_PATH, _lex.line()));

	std::string	directive = slog("error_page:%d", code);
	if (httpConf.hasDirective(directive))
		_duplicateError(directive);
	httpConf.addErrorPage(code, page);
	httpConf.markDirective(directive);
	_advance(Token::SEMICOLEN);
}


void	Parser::_parseMethods(HTTP& httpConf)
{
	bool	isAllow = _currTok == Token::ALLOW;
	_advance(_currTok);
	std::deque<std::string>	methods;
	while (_currTok == Token::WORD)
	{
		methods.push_back(_currTok.data());
		_advance(Token::WORD);
	}
	if (methods.empty()) _advance(Token::WORD);
	for (std::deque<std::string>::iterator it = methods.begin(); it != methods.end(); it++)
	{
		std::string	directive = (*it);
		if (httpConf.hasDirective(directive))
			_duplicateError(directive);
		if (isAllow) {
			httpConf.allowMethod(*it);
		} else {
			httpConf.denyMethod(*it);
		}
		httpConf.markDirective(directive);
	}
	_advance(Token::SEMICOLEN);
}

void	Parser::_parseIndex(HTTP& httpConf)
{
	_advance(Token::INDEX);
	std::deque<std::string>	indexes;
	while (_currTok == Token::WORD)
	{
		indexes.push_back(_currTok.data());
		_advance(Token::WORD);
	}
	if (indexes.empty()) _advance(Token::WORD);
	for (std::deque<std::string>::iterator it = indexes.begin(); it != indexes.end(); it++)
	{
		std::string	directive = "index:" + (*it);
		if (httpConf.hasDirective(directive))
			_duplicateError(directive);
		httpConf.addIndex(*it);
		httpConf.markDirective(directive);
	}
	_advance(Token::SEMICOLEN);
}

void	Parser::_parseUpload(HTTP& httpConf)
{
	_advance(Token::UPLOAD);
	std::string	upload = normPath(_currTok.data());
	_advance(Token::WORD);
	if (httpConf.hasDirective("upload_store"))
		_duplicateError("upload_store");
	httpConf.setUploadStore(upload);
	httpConf.markDirective("upload_store");
	_advance(Token::SEMICOLEN);
}

void	Parser::_parseAutoIndex(HTTP& httpConf)
{
	_advance(_currTok);
	bool	value = false;
	if (_currTok.data() == "on")
		value = true;
	else if (_currTok.data() != "off") {
		_error(slog("autoindex expected [on] or [off] but found %s at: %s:%d", to_str(_currTok).c_str(), CONF_PATH, _lex.line()));
	}
	_advance(Token::WORD);
	if (httpConf.hasDirective("autoindex"))
		_duplicateError("autoindex");
	httpConf.setAutoIndex(value);
	httpConf.markDirective("autoindex");
	_advance(Token::SEMICOLEN);
}

void	Parser::_parseClientBody(HTTP& httpConf)
{
	_advance(_currTok);
	std::stringstream	ss(_currTok.data());
	_advance(Token::WORD);
	unsigned int code;
	if (!(ss >> code) || !ss.eof())
		_error(slog("invalid client_body_max_size number %d at: %s:%d", to_str(_currTok).c_str(), CONF_PATH, _lex.line()));
	if (httpConf.hasDirective("clientBody"))
		_duplicateError("clientBody");
	httpConf.markDirective("clientBody");
	httpConf.setClientBody(code);
	_advance(Token::SEMICOLEN);
}

ServerConf*	Parser::_parseServer( void )
{
	_advance(Token::SERVER);
	_advance(Token::OPEN_CURLY);
	ServerConf*	server = new ServerConf();
	while (_currTok != Token::CLOSE_CURLY)  {
		switch (_currTok.type()) {
			case Token::ROOT:		_parseRoot(*server); break;
			case Token::ERR_PAGE:	_parseErrorPage(*server); break;
			case Token::ALLOW:		_parseMethods(*server); break;
			case Token::DENY:		_parseMethods(*server); break;
			case Token::INDEX:		_parseIndex(*server); break;
			case Token::UPLOAD:		_parseUpload(*server); break;
			case Token::AUTO:		_parseAutoIndex(*server); break;
			case Token::BODY_SIZE:	_parseClientBody(*server); break;
			case Token::HOST:		_parseHost(*server); break;
			case Token::PORT:		_parsePort(*server); break;
			case Token::LOCATION: {
				std::pair<std::string, LocationConf*>	loc = _parseLocation( *server );
				server->addLocation(loc.first, loc.second);
				break;
			}
			default:
				_error(slog("unexpected token %s at: %s:%d", to_str(_currTok).c_str(), CONF_PATH, _lex.line()));
		}
	}
	_advance(Token::CLOSE_CURLY);
	return server;
}

void	Parser::_parseHost(ServerConf& server)
{
	_advance(_currTok);
	std::string	host(_currTok.data());
	_advance(Token::WORD);
	std::string	directive = "host:" + host;
	if (server.hasDirective(directive))
		_duplicateError(directive);
	server.markDirective(directive);
	server.addHostName(host);
	_advance(Token::SEMICOLEN);
}

void	Parser::_parsePort(ServerConf& server)
{
	_advance(Token::PORT);
	std::stringstream		ss(_currTok.data());
	_advance(Token::WORD);
	int port;
	ss >> port;
	if (!ss.eof() || port < 0 || port > 65000)
		_error(slog("invalid port number: %d at: %s:%d", ss.str().c_str(), CONF_PATH, _lex.line()));
	std::stringstream		directive;
	directive << "port:" << port;
	if (server.hasDirective(directive.str()))
		_duplicateError(directive.str());
	server.markDirective(directive.str());
	server.addPort(port);
	_advance(Token::SEMICOLEN);
}

std::pair<std::string, LocationConf*>	Parser::_parseLocation( ServerConf& parentServer, std::string parentUri )
{
	_advance(Token::LOCATION);
	std::pair<std::string, LocationConf*> ans;
	ans.first =  normPath(parentUri + _currTok.data());
	if (parentServer.hasDirective(ans.first))
		_duplicateError(ans.first);
	parentServer.markDirective(ans.first);
	_advance(Token::WORD);
	_advance(Token::OPEN_CURLY);
	LocationConf*	location = new LocationConf();
	while (_currTok != Token::CLOSE_CURLY) {
		switch (_currTok.type()) {
			case Token::ROOT:		_parseRoot(*location);	break;
			case Token::ERR_PAGE:	_parseErrorPage(*location);	break;
			case Token::ALLOW:
			case Token::DENY:		_parseMethods(*location);	break;
			case Token::INDEX:		_parseIndex(*location);	break;
			case Token::UPLOAD:		_parseUpload(*location);	break;
			case Token::AUTO:		_parseAutoIndex(*location);	break;
			case Token::BODY_SIZE:	_parseClientBody(*location);	break;
			case Token::CGI:		_parseCgi(*location);	break;
			case Token::RETURN:		_parseRedirect(*location);	break;
			case Token::LOCATION: {
				std::pair<std::string, LocationConf*>	res = _parseLocation( parentServer, ans.first );
				location->addLocation(res.first, res.second);
				break;
			}
			default: {
				_error(slog("unexpected token %s at: %s:%d", to_str(_currTok).c_str(), CONF_PATH, _lex.line()));
			}
		}
	}
	ans.second = location;
	_advance(Token::CLOSE_CURLY);
	return (ans);
}

void	Parser::_parseCgi( LocationConf& location )
{
	_advance(Token::CGI);
	std::deque<std::string>	extentions;
	while (_currTok == Token::WORD)
	{
		extentions.push_back(_currTok.data());
		_advance(Token::WORD);
	}
	if (extentions.empty()) _advance(Token::WORD);
	for (std::deque<std::string>::iterator it = extentions.begin(); it != extentions.end(); it++)
	{
		std::string	directive = "cgi:" + (*it);
		if (location.hasDirective(directive))
			_duplicateError(directive);
		location.addExtention(*it);
		location.markDirective(directive);
	}
	_advance(Token::SEMICOLEN);
}

void	Parser::_parseRedirect( LocationConf& location )
{
	_advance(Token::RETURN);
	std::stringstream	ss;
	ss << _currTok.data();
	_advance(Token::WORD);
	ss << " " << _currTok.data();
	_advance(Token::WORD);
	int code; std::string page;
	ss >> code >> page;

	if (!ss.eof() || code < 300 || code > 399)
		_error(slog("invalid return value %s at: %s:%d", ss.str().c_str(), CONF_PATH, _lex.line()));
	if (location.hasDirective("return"))
		_duplicateError("return");
	location.markDirective("return");
	location.addRedirectpage(code, page);
	_advance(Token::SEMICOLEN);
}

