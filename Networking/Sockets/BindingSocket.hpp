#pragma once 

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
// #include "ASocketinglibc-sockets.hpp"
#include "./SimpleSocket.hpp"


namespace Socketing
{
	class BindingSocket : public SimpleSocket
	{
		public:
			/* Constructor */
			BindingSocket(int domain, int service, int protocol, int port, u_long interface);

			/* Function to connect a network */
			int	connect_to_network(int sock, struct sockaddr_in addr);

	};
}