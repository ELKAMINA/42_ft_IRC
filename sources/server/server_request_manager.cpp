/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server_request_manager.cpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jcervoni <jcervoni@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/27 23:08:50 by jcervoni          #+#    #+#             */
/*   Updated: 2023/03/04 08:39:35 by jcervoni         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::handle_request(char *buf, std::vector<Client>::iterator client, int readBytes, int i)
{
	size_t pos;
	std::string input;
	std::string client_buffer = "";
	std::vector<Client>::iterator it;
	buf[readBytes] = '\0';
	client_buffer += buf;
	std::cout << "Ce qu'envoie IRSSI : " << client_buffer << std::endl;
	while ((pos = client_buffer.find("\n")) != std::string::npos)
	{
		if (client_buffer[pos - 1] == '\r')
			input = client_buffer.substr(0, pos);
		else
			input = client_buffer.substr(0, pos + 1);
		Request req = Request(input.c_str(), (*client).getName());
		if (treating_req(req) == 1)
		{
			close(client_events[i].fd);
			client_events[i] = client_events[_online_clients - 1];
			_online_clients--;
			it = find_obj(req.origin, all_clients);
			all_clients.erase(it);
			break;
		}
		client_buffer.erase(0, pos + 1);
	}
	client_buffer.clear();
	return;
}

int Server::treating_req(Request &req)
{
	if (req.check_validity() != 1)
	{
		req.format_entries();
		return (req.requestLexer(this));
	}
	return 0;
}

void Server::chan_requests(Request &req)
{
	size_t i = 0;
	std::vector<Client>::iterator it;
	std::vector<Client>::iterator it_sender;

	it_sender = find_obj(req.origin, all_clients);
	if (req.response != "UNDEFINED")
	{
		req.response += "\n";
		while (i < req.target.size())
		{
			it = find_obj(req.target[i], all_clients);
			if (send(it->getFdClient(), req.response.c_str(), req.response.length(), MSG_DONTWAIT) == -1)
				return (perror("Send"));
			i++;
		}
	}
	if (req.reply != "UNDEFINED")
	{
		req.reply += "\r\n";
		if (send(it_sender->getFdClient(), req.reply.c_str(), req.reply.length(), 0) == -1)
		{
			return (perror("Send"));
		}
	}
}