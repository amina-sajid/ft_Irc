#include "Server.hpp"

Server::Server()
{
    this->serv_socket_fd = -1;
}

Server::Server(Server const &src)
{
    *this = src;
}

Server &Server::operator=(Server const &src)
{
	if (this != &src)
    {

		this->port = src.port;
		this->serv_socket_fd = src.serv_socket_fd;
        this->password = src.password;
		this->fds = src.fds;
		this->clients = src.clients;
		this->channels = src.channels;
	}
	return *this;
}

Server::~Server()
{};

int Server::getPort() const
{
    return this->port;
}

void Server::setPort(int port)
{
    this->port = port;
}

std::string Server::getPassword() const
{
    return this->password;
}

void Server::setPassword(const std::string &password)
{
    this->password = password;
}

// int Server::getfd()
// {
//     return this->serv_socket_fd;
// }

// void Server::setfd(int fd)
// {
//     this->serv_socket_fd = fd;
// }

Client* Server::GetClient(int fd)
{
    for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->getfd() == fd) // Assuming getfd() returns the file descriptor of the client
        {
            return &(*it); // Return a pointer to the found Client object
        }
    }
    return NULL; // Return nullptr if no client with the given fd is found
}


Client *Server::GetClientNick(std::string nickname){
	for (size_t i = 0; i < this->clients.size(); i++){
		if (this->clients[i].GetNickName() == nickname)
			return &this->clients[i];
	}
	return NULL;
}

Channel *Server::GetChannel(std::string name)
{
	for (size_t i = 0; i < this->channels.size(); i++){
		if (this->channels[i].GetName() == name)
			return &channels[i];
	}
	return NULL;
}
//................REMOVES.........................................

void Server::rm_fromclient(int fd)
{
    for (std::vector<Client>::iterator it = this->clients.begin(); it != this->clients.end(); ++it) {
        if (it->getfd() == fd)
        {
            this->clients.erase(it); // Remove the client using the iterator
            return;
        }
    }
}

void Server::rm_fromfds(int fd)
{
    for (std::vector<struct pollfd>::iterator it = this->fds.begin(); it != this->fds.end(); ++it) {
        if (it->fd == fd)
        {
            this->fds.erase(it); // Remove the fd using the iterator
            return;
        }
    }
}



void	Server::RmChannels(int fd){
	for (size_t i = 0; i < this->channels.size(); i++){
		int flag = 0;
		if (channels[i].get_client(fd))
			{channels[i].remove_client(fd); flag = 1;}
		else if (channels[i].get_admin(fd))
			{channels[i].remove_admin(fd); flag = 1;}
		if (channels[i].GetClientsNumber() == 0)
			{channels.erase(channels.begin() + i); i--; continue;}
		if (flag){
			std::string rpl = ":" + GetClient(fd)->GetNickName() + "!~" + GetClient(fd)->GetUserName() + "@localhost QUIT Quit\r\n";
			channels[i].sendTo_all(rpl);
		}
	}
}




void Server::RemoveChannel(std::string name){
	for (size_t i = 0; i < this->channels.size(); i++){
		if (this->channels[i].GetName() == name)
			{this->channels.erase(this->channels.begin() + i); return;}
	}
}





void Server::sendreply(std::string response, int fd)
{
	if(send(fd, response.c_str(), response.size(), 0) == -1)
		std::cerr << "Response send() failed" << std::endl;
}

void Server::senderror(int code, std::string clientname, int fd, std::string msg)
{
	std::stringstream ss;
	ss << ":localhost " << code << " " << clientname << msg;
	std::string resp = ss.str();
	if(send(fd, resp.c_str(), resp.size(),0) == -1)
		std::cerr << "send() failed" << std::endl;
}

void Server::senderror(int code, std::string clientname, std::string channelname, int fd, std::string msg)
{
	std::stringstream ss;
	ss << ":localhost " << code << " " << clientname << " " << channelname << msg;
	std::string resp = ss.str();
	if(send(fd, resp.c_str(), resp.size(),0) == -1)
		std::cerr << "send() failed" << std::endl;
}

void Server::recieve_newdata(int fd)
{
	std::vector<std::string> cmd;
	char buff[1024];
	memset(buff, 0, sizeof(buff));
	Client *cli = GetClient(fd);
	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1 , 0);
	if(bytes <= 0)
	{
		std::cout << "Client <" << fd << "> Disconnected" << std::endl;
		RmChannels(fd);
		rm_fromclient(fd);
		rm_fromfds(fd);
		close(fd);
	}
	else
	{
		cli->setBuffer(buff);
		if(cli->getBuffer().find_first_of("\r\n") == std::string::npos)
			return;
		cmd = split_recievedBuffer(cli->getBuffer());
		for(size_t i = 0; i < cmd.size(); i++)
			this->parse_command(cmd[i], fd);
		if(GetClient(fd))
			GetClient(fd)->clearBuffer();
	}
}

std::vector<std::string> Server::split_cmd(std::string& cmd)
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

bool Server::notregistered(int fd)
{
	if (!GetClient(fd) || GetClient(fd)->GetNickName().empty() || GetClient(fd)->GetUserName().empty() || GetClient(fd)->GetNickName() == "*"  || !GetClient(fd)->GetLogedIn())
		return false;
	return true;
}

void Server::parse_command(std::string &cmd, int fd)
{
	if(cmd.empty())
		return ;
	std::vector<std::string> splited_cmd = split_cmd(cmd);
	size_t found = cmd.find_first_not_of(" \t\v");
	if(found != std::string::npos)
		cmd = cmd.substr(found);
	if(splited_cmd.size() && (splited_cmd[0] == "PING" || splited_cmd[0] == "PONG"))
		return;
    if(splited_cmd.size() && (splited_cmd[0] == "PASS" || splited_cmd[0] == "pass"))
        authenticate_user(fd, cmd);
	else if (splited_cmd.size() && (splited_cmd[0] == "NICK" || splited_cmd[0] == "nick"))
		set_nickname(cmd,fd);
	else if(splited_cmd.size() && (splited_cmd[0] == "USER" || splited_cmd[0] == "user"))
		set_username(cmd, fd);
	else if (splited_cmd.size() && (splited_cmd[0] == "QUIT" || splited_cmd[0] == "quit"))
		QUIT(cmd,fd);
	else if(notregistered(fd))
	{
		if (splited_cmd.size() && (splited_cmd[0] == "KICK" || splited_cmd[0] == "kick"))
			KICK(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "JOIN" || splited_cmd[0] == "join"))
			JOIN(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "TOPIC" || splited_cmd[0] == "topic"))
			Topic(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "MODE" || splited_cmd[0] == "mode"))
			mode_command(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "PART" || splited_cmd[0] == "part"))
			PART(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "PRIVMSG" || splited_cmd[0] == "privmsg"))
			PRIVMSG(cmd, fd);
		else if (splited_cmd.size() && (splited_cmd[0] == "INVITE" || splited_cmd[0] == "invite"))
			Invite(cmd,fd);
		else if (splited_cmd.size())
			sendreply(ERR_CMDNOTFOUND(GetClient(fd)->GetNickName(),splited_cmd[0]),fd);
	}
	else if (!notregistered(fd))
		sendreply(ERR_NOTREGISTERED(std::string("*")),fd);
}



void Server::accept_newclient()
{
	Client cli;
	memset(&client_addr, 0, sizeof(client_addr));
	socklen_t client_len = sizeof(client_addr);
	int new_client_fd = accept(serv_socket_fd, (sockaddr *)&(client_addr), &client_len);
	if (new_client_fd == -1)
		{std::cout << "accept() failed" << std::endl; return;}
	if (fcntl(new_client_fd, F_SETFL, O_NONBLOCK) == -1)
		{std::cout << "fcntl() failed" << std::endl; return;}
	clientsfd.fd = new_client_fd;
	clientsfd.events = POLLIN;
	clientsfd.revents = 0;
	cli.setfd(new_client_fd);
	cli.setIpAddress(inet_ntoa((client_addr.sin_addr)));
	clients.push_back(cli);
	fds.push_back(clientsfd);
	std::cout << "Client <" << new_client_fd << "> Connected"   <<  std::endl;
}

bool Server::Signal = false;
void Server::SignalHandler(int signum)
{
	(void)signum;
	std::cout << std::endl << "Signal Received!" << std::endl;
	Server::Signal = true;
}


void Server::serverinit(int portnum, std::string pass)
{
    this->port = portnum;
    this->password = pass;

    set_serversocket();
    std::cout << "Server <" << serv_socket_fd << "> Connected"  << std::endl;
	std::cout << "Waiting to accept a connection...\n";
    while (Server::Signal == false)
	{
		if((poll(&fds[0],fds.size(),-1) == -1) && Server::Signal == false)
			throw(std::runtime_error("poll() faild"));
		for (size_t i = 0; i < fds.size(); i++)
		{
			if (fds[i].revents & POLLIN)
			{
				if (fds[i].fd == serv_socket_fd)
					this->accept_newclient();
				else
                    // std::cout << "Data ready to be read on client socket " << fds[i].fd << std::endl;
					this->recieve_newdata(fds[i].fd);
			}
		}
	}
	close_fds();

}

/*socket - creates a scoket
 setsocketopt - set the option to reuse the address if its used already
 fnctl = nonblockng mode,make the operations to return immediately without waiting
 bind - set the port number and ip address, server needs to know which port to listen
 listen = server needs to be in listening state to accept the client connections
 */

void Server::set_serversocket()
{
    int option = 1;
    add.sin_family = AF_INET;
    add.sin_addr.s_addr = INADDR_ANY;
    add.sin_port = htons(port);
    serv_socket_fd = socket(AF_INET,SOCK_STREAM,0);
    if(serv_socket_fd == -1)
        throw(std::runtime_error("faild to create socket"));
    if(setsockopt(serv_socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1)
		throw(std::runtime_error("failed to set option on socket"));
     if (fcntl(serv_socket_fd, F_SETFL, O_NONBLOCK) == -1)
		throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket"));
	if (bind(serv_socket_fd, (struct sockaddr *)&add, sizeof(add)) == -1)
		throw(std::runtime_error("faild to bind socket"));
	if (listen(serv_socket_fd, SOMAXCONN) == -1)
		throw(std::runtime_error("listen() faild"));
    clientsfd.fd = serv_socket_fd;
    clientsfd.events = POLLIN;
    clientsfd.revents = 0;
    fds.push_back(clientsfd);
}

std::vector<std::string> Server::split_recievedBuffer(std::string str)
{
	std::vector<std::string> vec;
	std::istringstream stm(str);
	std::string line;
	while(std::getline(stm, line))
	{
		size_t pos = line.find_first_of("\r\n");
		if(pos != std::string::npos)
			line = line.substr(0, pos);
		vec.push_back(line);
	}
	return vec;
}


void Server::close_fds()
{
    for (size_t i = 0; i < fds.size(); i++)
    {
        close(fds[i].fd);
    }
    fds.clear();
    std::cout << "All file descriptors closed, server shut down." << std::endl;
}
