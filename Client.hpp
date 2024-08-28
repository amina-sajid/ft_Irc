
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Server.hpp"
#include "Channel.hpp"

class Client
{
private:
	int fd;
	bool isOperator;
	bool registered;
	std::string nickname;
	bool logedin;
	std::string username;
	std::string buffer;
	std::string ip_address;
	std::vector<std::string> ChannelsInvite;
public:
	Client();
	Client(std::string nickname, std::string username, int fd);
	~Client();
	Client(Client const &src);
	Client &operator=(Client const &src);
	//---------------//Getters
	int getfd();
	void setfd(int fd);
	std::string getIpAddress();
	void setIpAddress(std::string ipadd);
	std::string getBuffer();
	void setBuffer(std::string recived);

	bool getRegistered();
	void setRegistered(bool value);

	std::string GetNickName();
	void SetNickname(std::string& nickName);
	std::string GetUserName();
	void SetUsername(std::string& username);
	
	bool GetLogedIn();
	void setLogedin(bool value);
	
	
	std::string getHostname();
	bool GetInviteChannel(std::string &ChName);
	
	
	
	void clearBuffer();
	void AddChannelInvite(std::string &chname);
	void RmChannelInvite(std::string &chname);
};

#endif
