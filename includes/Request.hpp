#pragma once

#include "Server.hpp"
# include "./Colors.hpp"
# include <vector>
# include <iostream>
# include <string>
# include <cstring>
# include <sys/socket.h>
# include <netinet/in.h>
# include <poll.h>
# include "ServerSocket.hpp"
# include <map>
// # include "Channels.hpp"
# include "Client.hpp"

class Client;

class Request
{
	public:
		int							_id;
		std::vector<std::string>	entries; // Max 512 caracteres (including the CR-LF)
		char						_prefix; // Optional : ":" used by servers to indicate the true origin of the message
		std::string					_command; 
		std::string					_body;
		std::vector<std::string>	_params; // Command parameters	( may be up to 15)
		// Client&						_origin;
		bool						_entry_isWrong;
		// int							type;

	public:
		Request(char* buf);
		~Request();
		Request( const Request &x );
		Request & operator = ( const Request &rhs );
		std::string getEntries() const;

};