/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request_serv_cmds.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jcervoni <jcervoni@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/08 18:20:59 by jcervoni          #+#    #+#             */
/*   Updated: 2023/02/22 18:11:39 by jcervoni         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

int Request::_pass(Client *cli, Server *serv)
{
	if (entries.size() == 1)
	{
		if (entries[0] == serv->get_pass() && cli->loggedIn == false)
		{
			cli->setPwd(serv->get_pass());
			return 0;
		}
		else
		{
			reply = errPasswMismatch(entries[0], "");
			serv->_chan_requests(*this);
		}
	}
	return 1;
}

int Request::_nick(Client *cli, Server *serv)
{
	(void)serv;
	if (cli->getPwd() != serv->get_pass())
		reply = errNotRegistered(cli->getNickName(), "not registered");
	if ((_find(entries[0], serv)) != (serv->all_clients.end()))
	{
		reply = errNicknameInUse(entries[0], "option");
		cli->setNickname("*");

	}
	// else if (wrong_nickname() == 0)
	// 	reply = err
	else
		cli->setNickname(entries[0]);
	serv->_chan_requests(*this);
	return 0;
}

int Request::_user(Client *cli, Server *serv)
{

	if (cli->loggedIn == false)
	{
		cli->setUsername(entries[0]);
		cli->setRealname(entries[3]);
		cli->loggedIn = true;
		reply = "001 " + _origin->getNickName() + " :Welcome to the Internet Relay Network "
		+ _origin->setPrefix() + "\r\n";
	}
	else
		reply = errAlreadyRegistered(cli->getNickName(), "Already registered");
	serv->_chan_requests(*this);
	return 0;
}

int Request::_privmsg(Client *cli, Server *serv)
{
	(void)cli;
	(void)serv;
	if (entries.size() >= 2)
	{
		if (entries[0][0] != '&' && entries[0][0] != '#')
		{
			std::vector<std::string>::iterator it = entries.begin();
			std::string dest = entries[0];
			std::string message;
			if (_find(dest, serv) != (serv->all_clients.end()))
			{
				entries.erase(it);
				if ((*(_find(dest, serv)))->checkMode('a') == 1)
				{
					message = (*(_find(dest, serv)))->getAwayMessage();
					if (send(_origin->getFdClient(), message.c_str(), message.length(), 0) == -1)
					return (-1);
					return 0;
				} // à tester pour NOTICE
				else
				{
					if (entries.size() >= 1)
					{
						size_t i = 0;
						while (i < entries.size())
						{
							message.append(entries[i]);
							message.append(" ");
							i++;
						}
					}
					message.append("\n");
				}
				std::string ToSend =  ":" + _origin->getNickName() + " " + _command + " " + dest + " " + &message[1];
				if (send((*_find(dest, serv))->getFdClient(), ToSend.c_str(), ToSend.length(), 0) == -1)
					return (-1);
				//serv->//replied = true;
			}
			else if (_find(dest, serv) == (serv->all_clients.end()) && _command == "PRIVMSG")
				reply = errNoSuchNick(_origin->getNickName(), entries[0]);
			return 0;
		}
		if (entries[0][0] == '&' || entries[0][0] == '#')
		{
			beginning_with_diez(entries);
			Channel *tmp = existing_chan(&entries[0][1], serv);
			if (!tmp && _command == "PRIVMSG")
			{
				reply = errNoSuchChannel(cli->getNickName(), entries[0]);
				//serv->//replied = true;
			}
			else
			{
				if (message == "")
				{
					message.clear();
					size_t i = jo_nb_chan;
					while (i < entries.size())
					{
						message.append(entries[i]);
						message.append(" ");
						i++;
					}
				}
				tmp->privmsg(*this, serv);
			}
			serv->_chan_requests(*this);
			return 0;
		}
	}
	return 0;
}

int Request::_away(Client *cli, Server *serv)
{
	(void)serv;
	(void)cli;
	if (entries.size() == 0 || (entries.size() == 1 && entries[0] == "")) /* Marche pas sur IRC */
	{
		if (_origin->checkMode('a') == 1)
		{
			reply = rpl_unaway(_origin->getNickName(), ":You are no longer marked as being away!\n");
			_origin->setMode('a', false);
		}
	}
	else if (entries.size() >= 2 && entries[0][0] == ':')
	{
		_origin->setMode('a', true);
		size_t i = 0;
		std::string away;
		away += _origin->getNickName() + " ";
		while (i < entries.size())
		{
			away += entries[i];
			away += " ";
			i++;
		}
		away += "\n";
		_origin->setAwayMessage(&away[1]);
		reply = rpl_away(_origin->getNickName(), "away");
	}
	serv->_chan_requests(*this);
	return 0;
}

int Request::_list(Client *cli, Server *serv)
{

	(void)cli;
	if (_check_lists() != 0)
	{
		string rep = "";

		for (size_t i = 0; i < serv->all_chanels.size(); i++)
		{
			if (!serv->all_chanels[i]->activeMode('s'))
				rep += "#" + serv->all_chanels[i]->getName() + ", ";
		}
		if (rep.size() != 0)
		{
			rep.replace(rep.size() - 2, 2, "\n");
		}
		this->reply = rep;
	}
	else
		reply = "Invalid request \n";
	return 0;
}

int Request::_cap(Client *cli, Server *serv)
{
	(void)cli;
	(void)serv;
	return 0;
}

int Request::_names(Client *cli, Server *serv)
{
	(void)cli;
	if (_check_lists() != 0)
	{
		if (reply == "UNDEFINED")
		{
			removing_sharp(entries);
			reply.clear();
			if (entries.size() == 0 && jo_nb_chan == 0)
				names_noParams(serv);
			else if (entries.size() >= 1)
				names_params(serv);
		}
	}
	serv->_chan_requests(*this);
	return 0;
}

int Request::_invite(Client *cli, Server *serv)
{
	(void)cli;
	Channel *tmp = existing_chan(&entries[1][1], serv);
	if (tmp)
		tmp->invite(*this, serv);
	else
		reply = errNoSuchChannel(_origin->getNickName(), entries[0]);
	serv->_chan_requests(*this);
	//serv->//replied = true;
	return 0;
}

int Request::_wallops(Client *cli, Server *serv)/* A voir si on garde ?*/
{
	if (entries.size() < 2)
		reply = errNeedMoreParams(cli->getNickName(), _command);
	else
	{
		std::vector<Client *>::iterator it = serv->all_clients.begin();
		while (it != serv->all_clients.end())
		{
			if ((*it)->checkMode('w') == true)
			{
				for (size_t i = 0; i < entries.size(); i++)
				{
					reply += entries[i];
					reply += " ";
				}
				serv->_chan_requests(*this);
			}
			it++;
		}
	}
	return 0;
}

int Request::_kill(Client *cli, Server *serv)
{
	(void)cli;
	if (_origin->checkMode('o') == false)
		reply = errNoPrivileges(_origin->setPrefix() + " :Permission Denied - You're not an IRC operator\n", "opti");
	else
	{
		std::vector<Client* >::iterator it;
		it = _find(entries[0], serv);
		if (it == serv->all_clients.end())
			reply = errNoSuchNick(entries[0], entries[0]);
		else
		{
			if (entries.size() >= 2)
				req_getComments(entries, 1);
			req_killingProcess((*it), serv);			
		}
	}
	return 0;
}

int Request::_oper(Client *cli, Server *serv)
{
	(void)cli;
	if (entries.size() == 2)
	{
		if (serv->opers[entries[0]] == entries[1])
		{
			reply = rpl_youreoper("381 " + _origin->setPrefix() + " :You are now an IRC operator\n", "op");
			_origin->setMode('o', true);
		}
		else
			reply = errPasswMismatch(_origin->setPrefix() + ":Password incorrect", "empty");
	}
	else
		reply = errNoOperHost(":No O-lines for your host\n", "op");
	serv->_chan_requests(*this);
	return 0;
}

int Request::_ping(Client *cli, Server *serv) /* For later */
{
	(void)serv;
	if (entries.size() < 1)
		reply = errNeedMoreParams(cli->getNickName(), _command);
	else
		reply = ":" + cli->setPrefix() + "PONG: " + entries[0] + "\r\n";
	return 0;
}

int Request::_whois(Client *cli, Server *serv) /* A modifier avec les bonnes replies */
{
	(void)serv;
	(void)cli;
	return 0;
}
