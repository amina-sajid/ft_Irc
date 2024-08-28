#include "../Server.hpp"

int Server::SplitJoin(std::vector<std::pair<std::string, std::string> > &token, std::string cmd, int fd)
{
	std::vector<std::string> tmp;
	std::string ChStr, PassStr, buff;
	std::istringstream iss(cmd);
	while(iss >> cmd)
		tmp.push_back(cmd);
	if (tmp.size() < 2) {token.clear(); return 0;}
	tmp.erase(tmp.begin());
	ChStr = tmp[0]; tmp.erase(tmp.begin());
	if (!tmp.empty()) {PassStr = tmp[0]; tmp.clear();}
	for (size_t i = 0; i < ChStr.size(); i++){
		if (ChStr[i] == ',')
				{token.push_back(std::make_pair(buff, "")); buff.clear();}
		else buff += ChStr[i];
	}
	token.push_back(std::make_pair(buff, ""));
	if (!PassStr.empty()){
		size_t j = 0; buff.clear();
		for (size_t i = 0; i < PassStr.size(); i++){
			if (PassStr[i] == ',')
				{token[j].second = buff; j++; buff.clear();}
			else buff += PassStr[i];
		}
		token[j].second = buff;
	}
	for (size_t i = 0; i < token.size(); i++)//erase the empty channel names
		{if (token[i].first.empty())token.erase(token.begin() + i--);}
	for (size_t i = 0; i < token.size(); i++){//ERR_NOSUCHCHANNEL (403) // if the channel doesn't exist
		if (*(token[i].first.begin()) != '#')
			{senderror(403, GetClient(fd)->GetNickName(), token[i].first, GetClient(fd)->getfd(), " :No such channel\r\n"); token.erase(token.begin() + i--);}
		else
			token[i].first.erase(token[i].first.begin());
	}
	return 1;
}

int Server::SearchForClients(std::string nickname)
{
	int count = 0;
	for (size_t i = 0; i < this->channels.size(); i++){
		if (this->channels[i].GetClientInChannel(nickname))
			count++;
	}
	return count;
}

bool IsInvited(Client *cli, std::string ChName, int flag){
	if(cli->GetInviteChannel(ChName)){
		if (flag == 1)
			cli->RmChannelInvite(ChName);
		return true;
	}
	return false;
}

void Server::ExistCh(std::vector<std::pair<std::string, std::string> >& token, int i, int j, int fd)
{
    Client* client = GetClient(fd);
    const std::string& channelName = token[i].first;
    const std::string& password = token[i].second;

    // Check if the client is already in the channel
    if (this->channels[j].GetClientInChannel(client->GetNickName()))
        return;

    // Check if the client has joined too many channels
    if (SearchForClients(client->GetNickName()) >= 10)
    {
        senderror(405, client->GetNickName(), fd, " :You have joined too many channels\r\n");
        return;
    }

    // Check if the channel requires a password and validate it
    if (!this->channels[j].GetPassword().empty() && this->channels[j].GetPassword() != password)
    {
        if (!IsInvited(client, channelName, 0))
        {
            senderror(475, client->GetNickName(), "#" + channelName, fd, " :Cannot join channel (+k) - bad key\r\n");
            return;
        }
    }

    // Check if the channel is invite-only
    if (this->channels[j].GetInvitOnly() && !IsInvited(client, channelName, 1))
    {
        senderror(473, client->GetNickName(), "#" + channelName, fd, " :Cannot join channel (+i)\r\n");
        return;
    }

    // Check if the channel is full
    if (this->channels[j].GetLimit() && this->channels[j].GetClientsNumber() >= this->channels[j].GetLimit())
    {
        senderror(471, client->GetNickName(), "#" + channelName, fd, " :Cannot join channel (+l)\r\n");
        return;
    }

    // Add the client to the channel
    this->channels[j].add_client(*client);

    // Prepare and send the response
    std::string response = RPL_JOINMSG(client->getHostname(), client->getIpAddress(), channelName);
    if (!this->channels[j].GetTopicName().empty())
    {
        response += RPL_TOPICIS(client->GetNickName(), channelName, this->channels[j].GetTopicName());
    }
    response += RPL_NAMREPLY(client->GetNickName(), channelName, this->channels[j].clientChannel_list()) +
                RPL_ENDOFNAMES(client->GetNickName(), channelName);

    sendreply(response, fd);

    // Notify all clients in the channel
    this->channels[j].sendTo_all(response, fd);
}



void Server::NotExistCh(std::vector<std::pair<std::string, std::string> >& token, int i, int fd)
{
    Client* client = GetClient(fd);
    const std::string& channelName = token[i].first;

    // Check if the client has joined too many channels
    if (SearchForClients(client->GetNickName()) >= 10)
    {
        senderror(405, client->GetNickName(), fd, " :You have joined too many channels\r\n");
        return;
    }

    // Create and configure the new channel
    Channel newChannel;
    newChannel.SetName(channelName);
    newChannel.add_admin(*client);
    newChannel.set_createiontime();

    // Add the new channel to the server
    this->channels.push_back(newChannel);

    // Prepare the response
    std::string response = RPL_JOINMSG(client->getHostname(), client->getIpAddress(), channelName) +
                           RPL_NAMREPLY(client->GetNickName(), channelName, newChannel.clientChannel_list()) +
                           RPL_ENDOFNAMES(client->GetNickName(), channelName);

    // Send the response to the client
    sendreply(response, fd);
}

void Server::JOIN(std::string cmd, int fd)
{
	std::vector<std::pair<std::string, std::string> > token;
	// SplitJoin(token, cmd, fd);
	if (!SplitJoin(token, cmd, fd))// ERR_NEEDMOREPARAMS (461) // if the channel name is empty
		{senderror(461, GetClient(fd)->GetNickName(), GetClient(fd)->getfd(), " :Not enough parameters\r\n"); return;}
	if (token.size() > 10) //ERR_TOOMANYTARGETS (407) // if more than 10 Channels
		{senderror(407, GetClient(fd)->GetNickName(), GetClient(fd)->getfd(), " :Too many channels\r\n"); return;}
	for (size_t i = 0; i < token.size(); i++){
		bool flag = false;
		for (size_t j = 0; j < this->channels.size(); j++){
			if (this->channels[j].GetName() == token[i].first){
				ExistCh(token, i, j, fd);
				flag = true; break;
			}
		}
		if (!flag)
			NotExistCh(token, i, fd);
	}
}