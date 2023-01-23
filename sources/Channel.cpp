/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jcervoni <jcervoni@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/23 15:13:43 by jcervoni          #+#    #+#             */
/*   Updated: 2023/01/23 15:26:14 by jcervoni         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel( std::vector<Client>& allUsers, std::string channelName, Client& owner ) :
_allUsers(allUsers), _name(channelName)
{
	_operators.push_back(owner.getNickName()); // Client doesn't has a nickName getter yet
	_onlineUsers = 1;
	_topic = "";
	_key = "";
}

Channel::Channel( std::vector<Client>& allUsers, std::string channelName, std::string channelKey, Client& owner ) :
_allUsers(allUsers), _name(channelName), _key(channelKey)
{
	_operators.push_back(owner.getNickName()); // Client doesn't has a nickName getter yet
	_onlineUsers = 1;
	_topic = "";
}

Channel::Channel(const Channel& rhs) : _allUsers(rhs._allUsers)
{
	*this = rhs;
}

Channel& Channel::operator=(const Channel& rhs)
{
	if (this != &rhs)
	{
		this->_prefix = rhs._prefix;
		this->_onlineUsers = rhs._onlineUsers;
		this->_name = rhs._name;
		this->_key = rhs._key;
		this->_topic = rhs._topic;
		this->_users = rhs._users;
		this->_operators = rhs._operators;
		this->_vocal = rhs._vocal;
		this->_banned = rhs._banned;
	}
	return *this;
}

Channel::~Channel()
{
	this->_users.clear();
	this->_operators.clear();
	this->_vocal.clear();
	this->_banned.clear();
}
