#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <csignal>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <ctime>
#include "Client.hpp"
#include "Channel.hpp"
#include "messages.hpp"

class Client;
class Channel;
class Server
{
    private:
        int serv_socket_fd;
        int port;
        std::string password;
        static bool Signal;
        struct sockaddr_in add;
        struct sockaddr_in client_addr;
        struct pollfd clientsfd;

        // std::vector<std::string> cmd;
        std::vector<Client> clients; //-> vector of clients
	    std::vector<struct pollfd> fds;
		std::vector<Channel> channels;//-> vector of pollfd


    public:
        Server();
	    Server(Server const &src);
	    Server &operator=(Server const &src);
        ~Server();

		int getPort() const;
        void setPort(int port);
        std::string getPassword() const;
        void setPassword(const std::string &password);

		Client *GetClient(int fd);
		Channel *GetChannel(std::string name);
		Client *GetClientNick(std::string nickname);


		std::vector<std::string> split_recievedBuffer(std::string str);
	    std::vector<std::string> split_cmd(std::string &str);

        void serverinit(int port, std::string pass);
        void set_serversocket();
        void accept_newclient();
		void recieve_newdata(int client_socketfd);
        void parse_command(std::string &cmd,int fd);
        static void SignalHandler(int signum);

        void sendResponse(std::string response, int fd);

        void authenticate_user(int fd, std::string cmd);
        void set_nickname(std::string cmd, int fd);
        bool nickname_isvalid(std::string& nickname);
        bool nickname_inuse(std::string& nickname);
        void set_username(std::string& username, int fd);

       
        void sendreply(std::string response, int fd);
		void senderror(int code, std::string clientname, int fd, std::string msg);
		void senderror(int code, std::string clientname, std::string channelname, int fd, std::string msg);

		

        // int getfd();
        // void setfd(int serv_socket_fd);

		bool 	notregistered(int fd);
	//---------------------------//JOIN CMD
		void	JOIN(std::string cmd, int fd);
		int		SplitJoin(std::vector<std::pair<std::string, std::string> > &token, std::string cmd, int fd);
		void	ExistCh(std::vector<std::pair<std::string, std::string> >&token, int i, int j, int fd);
		void	NotExistCh(std::vector<std::pair<std::string, std::string> >&token, int i, int fd);
		int		SearchForClients(std::string nickname);

		//---------------------------//PART CMD
		void	PART(std::string cmd, int fd);
		int		SplitCmdPart(std::string cmd, std::vector<std::string> &tmp, std::string &reason, int fd);
	//---------------------------//CKIK CMD
		void	KICK(std::string cmd, int fd);
		std::string SplitCmdKick(std::string cmd, std::vector<std::string> &tmp, std::string &user, int fd);
	//---------------------------//PRIVMSG CMD
		void	PRIVMSG(std::string cmd, int fd);
		void SendMessageToRecipients(const std::vector<std::string>& recipients, const std::string& message, int fd);
		void ValidateRecipients(std::vector<std::string>& recipients, int fd);


	//---------------------------//QUITE CMD
		void QUIT(const std::string& cmd, int fd);
		void NotifyChannelsOfQuit(int fd, const std::string& reason);




	//---------------------------//MODE CMD
		void 		mode_command(std::string& cmd, int fd);
		void		getCmdArgs(std::string cmd,std::string& name, std::string& modeset ,std::string &params);
		std::string	mode_toAppend(std::string chain, char opera, char mode);
		std::string invite_only(Channel *channel, char opera, std::string chain);
		std::string topic_restriction(Channel *channel, char opera, std::string chain);
		std::string password_mode(const std::vector<std::string> &tokens, Channel *channel, size_t &pos, char opera, int fd, std::string &chain, std::string &arguments);
		std::string operator_privilege(const std::vector<std::string>& tokens, Channel* channel, size_t& pos, int fd, char opera, std::string chain, std::string& arguments);
		bool isvalid_limit(const std::string& limit);
		std::string channel_limit(const std::vector<std::string>& tokens, Channel *channel, size_t &pos, char opera, int fd, const std::string& chain, std::string& arguments);
		std::vector<std::string> splitParams(std::string params);

	//---------------------------//TOPIC CMD
		std::string tTopic();
		void Topic(std::string &cmd, int &fd);
		std::string gettopic(const std::string &input);
		void Invite(std::string &cmd, int &fd);	
		int getpos(const std::string &cmd);

// std::string gettopic(std::string &input);
// int getpos(std::string &cmd);

		//removes.............//
		void	rm_fromfds(int fd);
		void	rm_fromclient(int fd);
		void	RmChannels(int fd);
		void 	RemoveChannel(std::string name);
        void    close_fds();

};









#endif
