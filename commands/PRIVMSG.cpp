#include "../Server.hpp"

void SplitP(std::string cmd, std::string tofind, std::string &str)
{
	size_t i = 0;
	for (; i < cmd.size(); i++){
		if (cmd[i] != ' '){
			std::string tmp;
			for (; i < cmd.size() && cmd[i] != ' '; i++)
				tmp += cmd[i];
			if (tmp == tofind) break;
			else tmp.clear();
		}
	}
	if (i < cmd.size()) str = cmd.substr(i);
	i = 0;
	for (; i < str.size() && str[i] == ' '; i++);
	str = str.substr(i);
}

std::string SplitCmdPvt(std::string &cmd, std::vector<std::string> &tmp)
{
	std::stringstream ss(cmd);
	std::string str, msg;
	int count = 2;
	while (ss >> str && count--)
		tmp.push_back(str);
	if(tmp.size() != 2) return std::string("");
	SplitP(cmd, tmp[1], msg);
	return msg;
}

std::string ExtractMessage(std::string cmd, std::vector<std::string> &tmp)
{
	std::string str = SplitCmdPvt(cmd, tmp);
	if (tmp.size() != 2) {tmp.clear();return std::string("");}
	tmp.erase(tmp.begin());
	std::string str1 = tmp[0]; std::string str2; tmp.clear();
	for (size_t i = 0; i < str1.size(); i++){//split the first string by ',' to get the channels names
		if (str1[i] == ',')
			{
				tmp.push_back(str2);
				str2.clear();
			}
		else str2 += str1[i];
	}
	tmp.push_back(str2);
	for (size_t i = 0; i < tmp.size(); i++)//erase the empty strings
		{if (tmp[i].empty())tmp.erase(tmp.begin() + i--);}
	if (str[0] == ':') str.erase(str.begin());
	else //shrink to the first space
		{for (size_t i = 0; i < str.size(); i++){if (str[i] == ' '){str = str.substr(0, i);break;}}}
	return  str;
}

// Validates the recipient list and removes any invalid recipients
void Server::ValidateRecipients(std::vector<std::string>& recipients, int fd) {
    for (size_t i = 0; i < recipients.size(); ++i) {
        bool isChannel = (recipients[i][0] == '#');
        std::string recipientName = isChannel ? recipients[i].substr(1) : recipients[i];

        if (isChannel) {
            Channel* channel = GetChannel(recipientName);
            if (!channel) {
                senderror(401, recipients[i], GetClient(fd)->getfd(), " :No such nick/channel\r\n");
                recipients.erase(recipients.begin() + i);
                --i;
            } else if (!channel->GetClientInChannel(GetClient(fd)->GetNickName())) {
                senderror(404, GetClient(fd)->GetNickName(), recipients[i], GetClient(fd)->getfd(), " :Cannot send to channel\r\n");
                recipients.erase(recipients.begin() + i);
                --i;
            }
        } else if (!GetClientNick(recipientName)) {
            senderror(401, recipients[i], GetClient(fd)->getfd(), " :No such nick/channel\r\n");
            recipients.erase(recipients.begin() + i);
            --i;
        }
    }
}

// Handles the PRIVMSG command and sends a private message to recipients
void Server::PRIVMSG(std::string cmd, int fd) 
{
    std::vector<std::string> recipients;
    std::string message = ExtractMessage(cmd, recipients);

    if (recipients.empty()) {
        senderror(411, GetClient(fd)->GetNickName(), GetClient(fd)->getfd(), " :No recipient given (PRIVMSG)\r\n");
        return;
    }

    if (message.empty()) {
        senderror(412, GetClient(fd)->GetNickName(), GetClient(fd)->getfd(), " :No text to send\r\n");
        return;
    }

    if (recipients.size() > 10) {
        senderror(407, GetClient(fd)->GetNickName(), GetClient(fd)->getfd(), " :Too many recipients\r\n");
        return;
    }

    ValidateRecipients(recipients, fd);
    
    for (size_t i = 0; i < recipients.size(); ++i) {
        std::string resp;
        if (recipients[i][0] == '#') {
            std::string channelName = recipients[i].substr(1);
            resp = ":" + GetClient(fd)->GetNickName() + "!~" + GetClient(fd)->GetUserName() + "@localhost PRIVMSG " + recipients[i] + " :" + message + "\r\n";
            GetChannel(channelName)->sendTo_all(resp, fd);
        } else {
            resp = ":" + GetClient(fd)->GetNickName() + "!~" + GetClient(fd)->GetUserName() + "@localhost PRIVMSG " + recipients[i] + " :" + message + "\r\n";
            sendreply(resp, GetClientNick(recipients[i])->getfd());
        }
    }
}
