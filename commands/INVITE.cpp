#include "../Server.hpp"


std::vector<std::string> SplitCmdInv(std::string& cmd)
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




void Server::Invite(std::string &cmd, int &fd)
{
    std::vector<std::string> scmd = SplitCmdInv(cmd);
    
    if (scmd.size() < 3) {
        senderror(461, GetClient(fd)->GetNickName(), fd, " :Not enough parameters\r\n");
        return;
    }
    
    std::string channelName = scmd[2].substr(1);
    if (scmd[2][0] != '#' || !GetChannel(channelName)) {
        senderror(403, channelName, fd, " :No such channel\r\n");
        return;
    }
    
    Channel* channel = GetChannel(channelName);
    if (!channel->get_client(fd) && !channel->get_admin(fd)) {
        senderror(442, channelName, fd, " :You're not on that channel\r\n");
        return;
    }
    
    if (channel->GetClientInChannel(scmd[1])) {
        senderror(443, GetClient(fd)->GetNickName(), channelName, fd, " :is already on channel\r\n");
        return;
    }
    
    Client* client = GetClientNick(scmd[1]);
    if (!client) {
        senderror(401, scmd[1], fd, " :No such nick\r\n");
        return;
    }
    
    if (channel->GetInvitOnly() && !channel->get_admin(fd)) {
        senderror(482, GetClient(fd)->GetNickName(), scmd[1], fd, " :You're not channel operator\r\n");
        return;
    }
    
    if (channel->GetLimit() && channel->GetClientsNumber() >= channel->GetLimit()) {
        senderror(471, GetClient(fd)->GetNickName(), channelName, fd, " :Cannot invite to channel (+i)\r\n");
        return;
    }
    
    client->AddChannelInvite(channelName);
    std::string reply1 = ": 341 " + GetClient(fd)->GetNickName() + " " + client->GetNickName() + " " + scmd[2] + "\r\n";
    sendreply(reply1, fd);
    std::string reply2 = ":" + client->getHostname() + " INVITE " + client->GetNickName() + " " + scmd[2] + "\r\n";
    sendreply(reply2, client->getfd());
}
