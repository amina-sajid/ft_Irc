#include "../Server.hpp"

/*
chain is a string that accumulates all the mode changes (+ to add, - to remove) applied during the execution of the mode_command.
It helps in keeping track of the changes so they can be applied in a single broadcast to all clients in the channel.*/
std::string Server::mode_toAppend(std::string chain, char opera, char mode)
{
	std::stringstream ss;

	ss.clear();
	char last = '\0';
	for(size_t i = 0; i < chain.size(); i++)
	{
		if(chain[i] == '+' || chain[i] == '-')
			last = chain[i];
	}
	if(last != opera)
		ss << opera << mode;
	else
		ss << mode;
	return ss.str();
}

std::string Server::invite_only(Channel *channel, char opera, std::string chain)
{
	std::string param;

	if(opera == '+' && !channel->getModeAtindex(0))
	{
		channel->setModeAtindex(0, true);
		channel->SetInvitOnly(1);
		param =  mode_toAppend(chain, opera, 'i');
	}
	else if (opera == '-' && channel->getModeAtindex(0))
	{
		channel->setModeAtindex(0, false);
		channel->SetInvitOnly(0);
		param =  mode_toAppend(chain, opera, 'i');
	}
	return param;
}



//This mode controls whether only channel operators (admins) can set the topic for the channel
std::string Server::topic_restriction(Channel *channel, char opera, std::string chain)
{
	std::string param;

	// Handle enabling topic restriction (+t)
	if (opera == '+' && !channel->getModeAtindex(1))
	{
		channel->setModeAtindex(1, true);
		channel->set_topicRestriction(true);
		param = mode_toAppend(chain, opera, 't');
	}
	// Handle disabling topic restriction (-t)
	else if (opera == '-' && channel->getModeAtindex(1))
	{
		channel->setModeAtindex(1, false);
		channel->set_topicRestriction(false);
		param = mode_toAppend(chain, opera, 't');
	}

	return param;
}




//The password_mode function is responsible for setting (+k mode) or removing (-k mode) the password on a channel.
bool validPassword(const std::string &password)
{
	if (password.empty())
		return false;
	for (size_t i = 0; i < password.size(); i++)
	{
		if(!std::isalnum(password[i]) && password[i] != '_')
			return false;
	}
	return true;
}

std::string Server::password_mode(const std::vector<std::string> &tokens, Channel *channel, size_t &pos, char opera, int fd, std::string &chain, std::string &arguments)
{
	std::string param;
	std::string pass;
    if (tokens.size() > pos)
    {
        pass = tokens[pos++];
    }

	// Error handling for missing password
	if (pass.empty())
	{
		sendreply(ERR_NEEDMODEPARM(channel->GetName(), "(k)"), fd);
		return param;
	}

	// Validate password
	if (!validPassword(pass))
	{
		sendreply(ERR_INVALIDMODEPARM(channel->GetName(), "(k)"), fd);
		return param;
	}

	// Handle setting the password (+k)
	if (opera == '+')
	{
		channel->setModeAtindex(2, true);
		channel->SetPassword(pass);

		if (!arguments.empty())
			arguments += " ";
		arguments += pass;

		param = mode_toAppend(chain, opera, 'k');
	}
	// Handle removing the password (-k)
	else if (opera == '-' && channel->getModeAtindex(2))
	{
		if (pass == channel->GetPassword())
		{
			channel->setModeAtindex(2, false);
			channel->SetPassword("");
			param = mode_toAppend(chain, opera, 'k');
		}
		else
		{
			sendreply(ERR_KEYSET(channel->GetName()), fd);
		}
	}

	return param;
}

//'o'-> is used to grant or revoke operator status to a user within a channel.

std::string Server::operator_privilege(const std::vector<std::string>& tokens, Channel* channel, size_t& pos, int fd, char opera, std::string chain, std::string& arguments) {
    if (pos >= tokens.size()) {
        sendreply(ERR_NEEDMODEPARM(channel->GetName(), "(o)"), fd);
        return "";
    }

    std::string user = tokens[pos++];
    if (!channel->clientInChannel(user)) {
        sendreply(ERR_NOSUCHNICK(channel->GetName(), user), fd);
        return "";
    }

    bool modeChanged = false;
    if (opera == '+') {
        channel->setModeAtindex(3, true);
        modeChanged = channel->change_clientToAdmin(user);
    } else if (opera == '-') {
        channel->setModeAtindex(3, false);
        modeChanged = channel->change_adminToClient(user);
    }

    if (modeChanged) {
        if (!arguments.empty()) arguments += " ";
        arguments += user;
        return mode_toAppend(chain, opera, 'o');
    }

    return "";
}









//'l' ->set or remove a limit on the number of users that can join a particular channel.

bool Server::isvalid_limit(const std::string& limit)
{
    return (!limit.empty() && limit.find_first_not_of("0123456789") == std::string::npos && std::atoi(limit.c_str()) > 0);
}

std::string Server::channel_limit(const std::vector<std::string>& tokens, Channel *channel, size_t &pos, char opera, int fd, const std::string& chain, std::string& arguments)
{
    std::string param;

    if (opera == '+')
    {
        if (pos < tokens.size())
        {
            std::string limit = tokens[pos++];
            if (isvalid_limit(limit))
            {
                channel->setModeAtindex(4, true);
                channel->SetLimit(std::atoi(limit.c_str()));
                if (!arguments.empty())
                    arguments += " ";
                arguments += limit;
                param = mode_toAppend(chain, opera, 'l');
            }
            else
            {
                sendreply(ERR_INVALIDMODEPARM(channel->GetName(), "(l)"), fd);
            }
        }
        else
        {
            sendreply(ERR_NEEDMODEPARM(channel->GetName(), "(l)"), fd);
        }
    }
    else if (opera == '-' && channel->getModeAtindex(4))
    {
        channel->setModeAtindex(4, false);
        channel->SetLimit(0);
        param = mode_toAppend(chain, opera, 'l');
    }

    return param;
}

// void Server::getCmdArgs(std::string cmd,std::string& name, std::string& modeset ,std::string &params)
// {
// 	std::istringstream stm(cmd);
// 	stm >> name;
// 	stm >> modeset;
// 	size_t found = cmd.find_first_not_of(name + modeset + " \t\v");//name + modeset + " \t\v" forms a string of characters that should be ignored.
// 	if(found != std::string::npos)
// 		params = cmd.substr(found);
//     std::cout << params << std::endl;
// }



void Server::getCmdArgs(std::string cmd, std::string& name, std::string& modeset, std::string& params) {
    std::istringstream stm(cmd);

    // Extract name
    stm >> name;

    // Extract modeset
    stm >> modeset;

    // Find the start of params by skipping over name and modeset
    std::string restOfCmd;
    std::getline(stm, restOfCmd);

    // Trim leading spaces from restOfCmd
    size_t start = restOfCmd.find_first_not_of(" \t\v");
    if (start != std::string::npos) {
        params = restOfCmd.substr(start);
    } else {
        params.clear();
    }

}


std::vector<std::string> Server::splitParams(std::string params)
{
	if(!params.empty() && params[0] == ':')
		params.erase(params.begin());//In some protocols (e.g., IRC), a colon at the beginning of a string indicates that the rest of the line is a parameter, so this line removes that leading colon if present.
	std::vector<std::string> tokens;
	std::string param;
	std::istringstream stm(params);
	while (std::getline(stm, param, ','))
	{
		tokens.push_back(param);
		param.clear();
	}
	return tokens;
}
void Server::mode_command(std::string& cmd, int fd)
{
    // Extract client and clean up the command
    Client *cli = GetClient(fd);
    cmd.erase(0, cmd.find_first_not_of("MODEmode \t\v"));
    if (cmd.empty()) {
        sendreply(ERR_NOTENOUGHPARAM(cli->GetNickName()), fd);
        return;
    }

    // Extract command arguments
    std::string channelName, modeset, params;
    getCmdArgs(cmd, channelName, modeset, params);
    std::vector<std::string> tokens = splitParams(params);

    // Validate channel name and existence
    if (channelName[0] != '#' || !GetChannel(channelName.substr(1))) {
        sendreply(ERR_CHANNELNOTFOUND(cli->GetUserName(), channelName), fd);
        return;
    }

    Channel *channel = GetChannel(channelName.substr(1));

    // Check if client is on the channel or is an admin
    if (!channel->get_client(fd) && !channel->get_admin(fd)) {
        senderror(442, cli->GetNickName(), channelName, fd, " :You're not on that channel\r\n");
        return;
    }

    // If no mode is provided, return current channel modes
    if (modeset.empty()) {
        sendreply(RPL_CHANNELMODES(cli->GetNickName(), channel->GetName(), channel->getModes()) +
                      RPL_CREATIONTIME(cli->GetNickName(), channel->GetName(), channel->get_creationtime()), fd);
        return;
    }

    // Check if the client is an admin
    if (!channel->get_admin(fd)) {
        sendreply(ERR_NOTOPERATOR(channel->GetName()), fd);
        return;
    }

    // Initialize mode processing
    std::string mode_chain;
    std::string arguments;
    char opera = '\0';
    size_t pos = 0;

    for (size_t i = 0; i < modeset.size(); ++i) {
        char mode = modeset[i];
        if (mode == '+' || mode == '-') {
            opera = mode;
        } else {
            switch (mode) {
                case 'i':
                    mode_chain += invite_only(channel, opera, mode_chain);
                    break;
                case 't':
                    mode_chain += topic_restriction(channel, opera, mode_chain);
                    break;
                case 'k':
                    mode_chain += password_mode(tokens, channel, pos, opera, fd, mode_chain, arguments);
                    break;
                case 'o':
                    mode_chain += operator_privilege(tokens, channel, pos, fd, opera, mode_chain, arguments);
                    break;
                case 'l':
                    mode_chain += channel_limit(tokens, channel, pos, opera, fd, mode_chain, arguments);
                    break;
                default:
                    sendreply(ERR_UNKNOWNMODE(cli->GetNickName(), channel->GetName(), mode), fd);
                    return;
            }
        }
    }

    if (!mode_chain.empty()) {
        channel->sendTo_all(RPL_CHANGEMODE(cli->getHostname(), channel->GetName(), mode_chain, arguments));
    }
}
