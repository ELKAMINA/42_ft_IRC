/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jcervoni <jcervoni@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/23 15:06:37 by jcervoni          #+#    #+#             */
/*   Updated: 2023/02/10 15:44:06 by jcervoni         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <vector>
# include <iostream>
# include <string>
# include <algorithm>
# include <map>
# include "Colors.hpp"
# include "ServerSocket.hpp"
# include "Request.hpp"
# include "Client.hpp"
# include "numeric_replies.hpp"

using namespace std;

class Request;
class Client;

class Channel
{
	public:

	/* CONSTRUCTORS */
		Channel( vector<Client*>& allUsers, string channelName, Client& owner );
		Channel( vector<Client*>& allUsers, string channelName, string channelKey, Client& owner );
		Channel( const Channel& rhs );
		~Channel();

	/* OPERATORS OVERLOAD */
		Channel &operator=( const Channel& rhs );
	
	/* METHODS */


		/* MODES MANAGEMENT */
		void changeChanMode(Request& request, pair<string, string> command);
		void changeUserMode(Request& request, pair<string, string> command,vector<Client*>& target);
		int addMode(Request& request, vector<string>params);
		// void modeBan(Request& request, pair<string, string> command);
		void modeLimite(Request& request, pair<string, string> command);

		/* COMMUNICATION*/
		// void sendMessageToMembers(Request &message, Client &from);

		/* COMMANDS */
		void cmd_lexer(Request& request);
		void reply_joining(Request& request);
		void join(Request& request);
		void invite(Request& request);
		void topic(Request& request);
		void part(Request& request);
		void privmsg(Request& request);
		void mode(Request& request);
		void kick(Request& request);
		
		/* CHAN INFO CHECKERS */
		bool	isInChanList(Client const *user, vector<Client*>& list);
		Client*	found(string nickname, vector<Client*>&list);
		bool	isInServ(string const& user, vector<Client *>&users);

		/* CHAN MODE CHECKER */
		void errInCmd(Request& request, string err);
		bool activeMode(char mode);
		void initModes();
		/* COMMAND MANAGER */
		// void treatAndReturn(Request &request);

	/* ACCESSORS */
		string		getName() const;
		string		getTopic() const;
		int			getOnlineCount() const;
		std::string	getKey() const;
			
	private:

		char				_prefix;
		int					_onlineUsers;
		int					_maxUsers;
		string				_name;
		string				_key;
		string				_topic;
		vector<Client *>	_users;
		vector<Client *>	_operators; // separated from users or duplicated ?
		vector<Client *>	_vocal;
		// vector<Client *>	_banned;
		vector<Client *>	_invited;
		vector<Client*>&	_allUsers;
		map<char, bool>		_mods; //a définir

};

#endif
