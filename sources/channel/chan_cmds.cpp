/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   chan_cmds.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jcervoni <jcervoni@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/30 11:31:04 by jcervoni          #+#    #+#             */
/*   Updated: 2023/02/12 10:45:50 by jcervoni         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include "Request.hpp"
#include "Client.hpp"
#include "numeric_replies.hpp"

typedef std::string	(*err)(string, string);

void Channel::errInCmd(Request& request, string err)
{
	request.reply = err;
	request.status = treated;
}

void Channel::cmd_lexer(Request& request)
{
	string cmd_name[] = {"JOIN", "INVITE", "TOPIC", "PART", "PRIVMSG", "KICK", "NAMES"};
	for (size_t i = 0; i< _cmds.size(); i++){
		if (request._command == cmd_name[i])
			(this->*(_cmds[i]))(request);
	}
}

void Channel::reply_joining(Request& request)
{
	string rep = "";
	vector<Client *>::iterator it;

	if (this->_topic.size() > 0)
		rep += rpl_topic(this->getName(), this->getTopic());
	for (size_t i = 0; i < this->_users.size(); i++){
		if ((it=find(_operators.begin(), _operators.end(), _users[i])) != _operators.end())
			rep +="@";
		else
			rep +=":";
		rep += _users[i]->getNickName();
		if (i < _users.size() -1)
			rep += ", ";
	}
	rep += '\n';
	request.reply = rep;
	rep.clear();
}

void Channel::join(Request &request)
{
	string user = request._origin->getNickName();
	vector<Client *>::iterator it;
	// int matching_param;
	
	if (isInChanList((request._origin), _users))
		return (errInCmd(request, errUserOnChannel(user,this->getName())));
	// if (_mods['k'] == true)
	// {
	// 	for (size_t i = 0; i < request.entries.size(); i++){
	// 		if (request.entries[i] == _name)
	// 		{
	// 			matching_param = i;
	// 			break;
	// 		}
	// 	}
	// 	// will need position in protected chan list to snipe good key
	// }
	if (_mods['l'] && _onlineUsers == _maxUsers)
		return (errInCmd(request, errChannelIsFull(user, this->getName())));
	if (_mods['i'] == true)
	{
		if (!isInChanList((request._origin), _invited))
			return (errInCmd(request, errInviteOnlyChan(user, this->getName())));
		_invited.erase(it=find(_invited.begin(), _invited.end(), request._origin));
	}
	_onlineUsers += 1;
	request.target.insert(request.target.end(), _users.begin(), _users.end());
	request.response = ":" + request._origin->setPrefix() + " has join #" + this->getName() + '\n';
	reply_joining(request);
	_users.push_back(request._origin);
	request.status = treated;
}

void Channel::invite(Request& request)
{
	string user = request._origin->getNickName();
	vector<string>::iterator it;
	Client* to_add =found(request.entries[1], _invited);
	if (request.entries.size() < 2)
		return (errInCmd(request, errNeedMoreParams(user, request._command)));
	if (!isInServ(request.entries[1], this->_allUsers))
		return (errInCmd(request, errNoSuchNick(user, request.entries[1])));
	if (_mods['l'] && _onlineUsers == _maxUsers)
		return (errInCmd(request, errChannelIsFull(user, this->getName())));
	if (_mods['i'] == true)
	{
		if (!isInChanList((to_add), _operators))
			return (errInCmd(request, errChanPrivsNeeded(user, this->getName())));
		request.response = "@";
	}
	_invited.push_back(to_add);
	request.target.push_back(to_add);
	request.response += request._origin->setPrefix() + " INVITE " + request.entries[1] + " #" + this->getName() + '\n';
	request.reply = rpl_inviting(request.entries[1], this->getName());
	request.status = treated;
}

void Channel::topic(Request& request)
{
	size_t size = request.entries.size();
	string user = request._origin->getNickName();

	if (size == 1)
	{
		if (this->_topic.size() > 0)
			request.reply = rpl_topic(this->getName(), this->getTopic());
		else
			request.reply = rpl_notopic(this->getName(), this->getTopic());
	}
	else if (!isInChanList((request._origin), _operators))
		errInCmd(request, errChanPrivsNeeded(user, this->getName()));
	else // new topic and user is operator
	{
		if (request.entries[1].size() == 1)
			this->_topic = "";
		else
		{
			if (request.entries[1][0] != ':')
				errInCmd(request, errNeedMoreParams(user, request._command));
			else
			{
				this->_topic = request.entries[1];
				for (size_t i = 2; i < request.entries.size(); i++){
					this->_topic += " " + request.entries[i];
				}
			}
		}
	}
	request.status = treated;
}

void Channel::part(Request& request)
{
	string user = request._origin->getNickName();
	vector<Client*>::iterator it;

	if (!isInChanList((request._origin), _users))
		errInCmd(request, errNotOnChannel(user, this->getName()));
	else
	{
		_users.erase(it = find(_users.begin(), _users.end(), request._origin));
		if ((it = find(_operators.begin(), _operators.end(), request._origin)) != _operators.end())
			_operators.erase(it);
		if ((it = find(_vocal.begin(), _vocal.end(), request._origin)) != _vocal.end())
			_vocal.erase(it);
		_onlineUsers -= 1;
		request.target.insert(request.target.end(), _users.begin(), _users.end());
		request.response = user + " leaves #" + this->getName() + " " + request.message + '\n';
		request.status = treated;
	}
}

void Channel::privmsg(Request& request)
{
	string user = request._origin->getNickName();
	vector<Client*>::iterator it;

	if (!isInChanList((request._origin), _users))
		return (errInCmd(request, errNotOnChannel(user, this->getName())));
	request.response = ":" + request._origin->setPrefix() + " PRIVMSG "
	+  " " + request.message + '\n';
	request.target.insert(request.target.begin(), _users.begin(), _users.end());
	request.target.erase(it=find(request.target.begin(), request.target.end(), request._origin));
	request.status = treated;
}

// work in progress
void Channel::mode(Request& request)
{
	string user = request._origin->getNickName();
	if (!isInChanList((request._origin), this->_operators))
		return(errInCmd(request, errChanPrivsNeeded(user, this->getName())));
	addMode(request, request.entries);
}

void Channel::kick(Request& request)
{
	string user = request._origin->getNickName();
	Client* to_kick = found(request.entries[1], _users);
	vector<Client *>::iterator it;

	if (!isInChanList((request._origin), _users))
		return (errInCmd(request, errNotOnChannel(user, this->getName())));
	if (!isInChanList((request._origin), _operators))
		return(errInCmd(request, errChanPrivsNeeded(user, this->getName())));
	if ((it = find(_users.begin(), _users.end(), to_kick)) == _users.end())
		return(errInCmd(request, errNoSuchNick(user, request.entries[2])));
	_users.erase(it = find(_users.begin(), _users.end(), to_kick));
	if ((it = find(_operators.begin(), _operators.end(), to_kick)) != _operators.end())
		_operators.erase(it);
	if ((it = find(_vocal.begin(), _vocal.end(), to_kick)) != _vocal.end())
		_vocal.erase(it);
	_onlineUsers -= 1;
	request.response += request._origin->setPrefix() + " KICK " + this->getName() + " "
	+ request.entries[1] + " " + request.message + '\n';
	request.status = treated;
}

void Channel::names(Request& request)
{
	vector<Client*>::iterator it;

	request.reply += "#" + this->getName() + ":\n";
	for (it = _users.begin(); it != _users.end(); it++){
		if ((*it)->checkMode('i') == false)
		{
			if (find(_operators.begin(), _operators.end(), (*it)) != _operators.end())
				request.reply += '@';
			else
				request.reply += ':';
			request.reply += (*it)->getNickName() + ", ";
		}
	}
	request.reply.replace(request.reply.size() -2, 2, "\n");
}