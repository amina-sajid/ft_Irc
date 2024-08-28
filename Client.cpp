#include "Client.hpp"

Client::Client(){

	this->nickname = "";
	this->username = "";
	this->fd = -1;
	this->isOperator= false;
	this->registered = false;
	this->buffer = "";
	this->ip_address = "";
	this->logedin = false;
}

Client::Client(std::string nickname, std::string username, int fd) : fd(fd), nickname(nickname), username(username){}

Client::~Client(){}

Client::Client(Client const &src){*this = src;}

Client &Client::operator=(Client const &src){
	if (this != &src){
		this->nickname = src.nickname;
		this->username = src.username;
		this->fd = src.fd;
		this->ChannelsInvite = src.ChannelsInvite;
		this->buffer = src.buffer;
		this->registered = src.registered;
		this->ip_address = src.ip_address;
		this->logedin = src.logedin;
	}
	return *this;
}

int Client::getfd(){return this->fd;}

bool Client::getRegistered(){return registered;}



bool Client::GetInviteChannel(std::string &chName){
	for (size_t i = 0; i < this->ChannelsInvite.size(); i++){
		if (this->ChannelsInvite[i] == chName)
			return true;
	}
	return false;
}



std::string Client::GetNickName(){return this->nickname;}

bool Client::GetLogedIn(){return this->logedin;}

std::string Client::GetUserName(){return this->username;}

std::string Client::getBuffer(){return buffer;}

std::string Client::getIpAddress(){return ip_address;}

std::string Client::getHostname(){
	std::string hostname = this->GetNickName() + "!" + this->GetUserName();
	return hostname;
}

void Client::setfd(int fd){this->fd = fd;}

void Client::SetNickname(std::string& nickName){this->nickname = nickName;}

void Client::setLogedin(bool value){this->logedin = value;}

void Client::SetUsername(std::string& username){this->username = username;}

void Client::setBuffer(std::string recived){buffer += recived;}

void Client::setRegistered(bool value){registered = value;}

void Client::setIpAddress(std::string ipadd){this->ip_address = ipadd;}

void Client::clearBuffer(){buffer.clear();}

void Client::AddChannelInvite(std::string &chname){
	ChannelsInvite.push_back(chname);
}

void Client::RmChannelInvite(std::string &chname){
	for (size_t i = 0; i < this->ChannelsInvite.size(); i++){
		if (this->ChannelsInvite[i] == chname)
			{this->ChannelsInvite.erase(this->ChannelsInvite.begin() + i); return;}
	}
}
