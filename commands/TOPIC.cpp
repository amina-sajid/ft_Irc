#include "../Server.hpp"
/*
ERR_NEEDMOREPARAMS (461)
ERR_NOSUCHCHANNEL (403)
ERR_NOTONCHANNEL (442)
ERR_CHANOPRIVSNEEDED (482)
:localhost 442 #jj :You're not on that channel
:localhost 442 jj :You're Not a channel operator
*/

std::string Server::tTopic()
{
    std::time_t current = std::time(NULL);
    std::stringstream ss;
    ss << current;
    return ss.str();
}

std::string Server::gettopic(const std::string &input) {
    size_t pos = input.find(":");
    return (pos != std::string::npos) ? input.substr(pos) : "";
}

int Server::getpos(const std::string &cmd) {
    for (size_t i = 0; i < cmd.size(); ++i) {
        if (cmd[i] == ':' && (i > 0 && cmd[i - 1] == ' ')) {
            return i;
        }
    }
    return -1;
}


std::vector<std::string> SplitCmdTop(std::string& cmd)
{
	std::vector<std::string> vec;
	std::istringstream stm(cmd);
	std::string token;
	while(stm >> token)
	{
		vec.push_back(token);
		token.clear();
	}
	return vec;
}




void Server::Topic(std::string &cmd, int &fd) {
    std::vector<std::string> scmd = SplitCmdTop(cmd);
    if (scmd.size() < 2) {
        senderror(461, GetClient(fd)->GetNickName(), fd, " :Not enough parameters\r\n"); // ERR_NEEDMOREPARAMS
        return;
    }

    std::string nmch = scmd[1].substr(1); // Channel name without '#'
    Channel* channel = GetChannel(nmch);

    if (!channel) {
        senderror(403, "#" + nmch, fd, " :No such channel\r\n"); // ERR_NOSUCHCHANNEL
        return;
    }

    if (!channel->get_client(fd) && !channel->get_admin(fd)) {
        senderror(442, "#" + nmch, fd, " :You're not on that channel\r\n"); // ERR_NOTONCHANNEL
        return;
    }

    if (scmd.size() == 2) {
        if (channel->GetTopicName().empty()) {
            sendreply(": 331 " + GetClient(fd)->GetNickName() + " #" + nmch + " :No topic is set\r\n", fd); // RPL_NOTOPIC
        } else {
            std::string topicResponse = ": 332 " + GetClient(fd)->GetNickName() + " #" + nmch + " " + channel->GetTopicName() + "\r\n";
            std::string topicTimeResponse = ": 333 " + GetClient(fd)->GetNickName() + " #" + nmch + " " + GetClient(fd)->GetNickName() + " " + channel->GetTime() + "\r\n";
            sendreply(topicResponse, fd); // RPL_TOPIC
            sendreply(topicTimeResponse, fd); // RPL_TOPICWHOTIME
        }
        return;
    }

    std::string topic = gettopic(cmd);
    if (topic.empty() || (topic[0] == ':' && topic[1] == '\0')) {
        sendreply(": 331 " + GetClient(fd)->GetNickName() + " #" + nmch + " :No topic is set\r\n", fd); // RPL_NOTOPIC
        return;
    }

    bool isRestricted = channel->Gettopic_restriction();
    bool isAdmin = channel->get_admin(fd);
    if (isRestricted && !isAdmin) {
        senderror(482, "#" + nmch, fd, " :You're Not a channel operator\r\n"); // ERR_CHANOPRIVSNEEDED
        return;
    }

    channel->SetTime(tTopic());
    channel->SetTopicName(topic);

    std::string rpl = ":" + GetClient(fd)->GetNickName() + "!" + GetClient(fd)->GetUserName() + "@localhost TOPIC #" + nmch + " " + channel->GetTopicName() + "\r\n";
    channel->sendTo_all(rpl); // Send the updated topic to all users in the channel
}
