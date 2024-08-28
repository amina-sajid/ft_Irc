#include "Server.hpp"
#include "Client.hpp"


bool Server::nickname_isvalid(std::string& nickname)
{
	if(!nickname.empty() && (nickname[0] == '&' || nickname[0] == '#' || nickname[0] == ':'))
		return false;
	for(size_t i = 1; i < nickname.size(); i++)
	{
		if(!std::isalnum(nickname[i]) && nickname[i] != '_')
			return false;
	}
	return true;
}

bool Server::nickname_inuse(std::string& nickname)
{
	for (size_t i = 0; i < this->clients.size(); i++)
	{
		if (this->clients[i].GetNickName() == nickname)
			return true;
	}
	return false;
}

void Server::authenticate_user(int fd, std::string cmd)
{
	Client *cli = GetClient(fd);
	cmd = cmd.substr(4);
	size_t pos = cmd.find_first_not_of("\t\v ");
	if(pos < cmd.size())
	{
		cmd = cmd.substr(pos);
		if(cmd[0] == ':')
			cmd.erase(cmd.begin());
	}
	if(pos == std::string::npos || cmd.empty()) 
		sendreply(ERR_NOTENOUGHPARAM(std::string("*")), fd);
	else if(!cli->getRegistered())
	{
		std::string pass = cmd;
		if(pass == password)
			cli->setRegistered(true);
		else
            sendreply(ERR_INCORPASS(std::string("*")), fd);
	}
	else
        sendreply(ERR_ALREADYREGISTERED(GetClient(fd)->GetNickName()), fd);
}



// void Server::set_nickname(std::string cmd, int fd)
// {
// 	// Remove the first 4 characters ("NICK") from the command
// 	cmd = cmd.substr(4);

// 	// Trim leading whitespace
// 	size_t pos = cmd.find_first_not_of("\t\v ");
// 	if (pos < cmd.size())
// 	{
// 		cmd = cmd.substr(pos);
// 		if (cmd[0] == ':')
// 			cmd.erase(cmd.begin());
// 	}

// 	Client *cli = GetClient(fd);

// 	// If the nickname command is empty or only contains whitespace, send an error
// 	if (pos == std::string::npos || cmd.empty())
// 	{
// 		sendreply(ERR_NOTENOUGHPARAM(std::string("*")), fd);
// 		return;
// 	}

// 	// Check if the nickname is already in use
// 	if (nickname_inuse(cmd))
// 	{
// 		sendreply(ERR_NICKINUSE(std::string(cmd)), fd);
// 		return;
// 	}

// 	// Check if the nickname is valid according to IRC rules
// 	if (!nickname_isvalid(cmd))
// 	{
// 		sendreply(ERR_ERRONEUSNICK(std::string(cmd)), fd);
// 		return;
// 	}

// 	// If the client is registered, handle the nickname change
// 	if (cli && cli->getRegistered())
// 	{
// 		std::string oldnick = cli->GetNickName();
// 		cli->SetNickname(cmd);

// 		// If the old nickname is not empty and different from the new nickname, notify the change
// 		if (!oldnick.empty() && oldnick != cmd)
// 		{
// 			sendreply(RPL_NICKCHANGE(oldnick, cmd), fd);
// 		}
// 	}
// 	// If the client is not registered, send an error
// 	else if (cli && !cli->getRegistered())
// 	{
// 		sendreply(ERR_NOTREGISTERED(cmd), fd);
// 	}

// 	// If the client is now fully registered and logged in, send the connected response
// 	if (cli && cli->getRegistered() && !cli->GetUserName().empty() && !cli->GetNickName().empty() && !cli->GetLogedIn())
// 	{
// 		cli->setLogedin(true);
// 		sendreply(RPL_CONNECTED(cli->GetNickName()), fd);
// 	}
// }

void Server::set_nickname(std::string cmd, int fd)
{
	std::string inuse;
	cmd = cmd.substr(4);
	size_t pos = cmd.find_first_not_of("\t\v ");
	if(pos < cmd.size())
	{
		cmd = cmd.substr(pos);
		if(cmd[0] == ':')
			cmd.erase(cmd.begin());
	}
	Client *cli = GetClient(fd);
	if(pos == std::string::npos || cmd.empty())
		{sendreply(ERR_NOTENOUGHPARAM(std::string("*")), fd); return;}
	if (nickname_inuse(cmd) && cli->GetNickName() != cmd){
		inuse = "*";
		if(cli->GetNickName().empty())
			cli->SetNickname(inuse);
	    sendreply(ERR_NICKINUSE(std::string(cmd)), fd); 
		return;
	}
	if(!nickname_isvalid(cmd)) {
		sendreply(ERR_ERRONEUSNICK(std::string(cmd)), fd);
		return;
	}
	else
	{
		if(cli && cli->getRegistered())
		{
			std::string oldnick = cli->GetNickName();
			cli->SetNickname(cmd);
			if(!oldnick.empty() && oldnick != cmd)
			{
				if(oldnick == "*" && !cli->GetUserName().empty())
				{
					cli->setLogedin(true);
					sendreply(RPL_CONNECTED(cli->GetNickName()), fd);
					sendreply(RPL_NICKCHANGE(cli->GetNickName(),cmd), fd);
				}
				else
					sendreply(RPL_NICKCHANGE(oldnick,cmd), fd);
				return;
			}
			
		}
		else if (cli && !cli->getRegistered())
			sendreply(ERR_NOTREGISTERED(cmd), fd);
	}
	if(cli && cli->getRegistered() && !cli->GetUserName().empty() && !cli->GetNickName().empty() && cli->GetNickName() != "*" && !cli->GetLogedIn())
	{
		cli->setLogedin(true);
		sendreply(RPL_CONNECTED(cli->GetNickName()), fd);
	}
}


void	Server::set_username(std::string& cmd, int fd)
{
	std::vector<std::string> splited_cmd = split_cmd(cmd);

	Client *cli = GetClient(fd); 
	if((cli && splited_cmd.size() < 5))
		{sendreply(ERR_NOTENOUGHPARAM(cli->GetNickName()), fd); return; }
	if(!cli  || !cli->getRegistered())
		sendreply(ERR_NOTREGISTERED(std::string("*")), fd);
	else if (cli && !cli->GetUserName().empty())
		{sendreply(ERR_ALREADYREGISTERED(cli->GetNickName()), fd); return;}
	else
		cli->SetUsername(splited_cmd[1]);
	if(cli && cli->getRegistered() && !cli->GetUserName().empty() && !cli->GetNickName().empty() && cli->GetNickName() != "*"  && !cli->GetLogedIn())
	{
		cli->setLogedin(true);
		sendreply(RPL_CONNECTED(cli->GetNickName()), fd);
	}
}