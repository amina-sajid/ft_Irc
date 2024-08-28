#include "../Server.hpp"

std::string GetQuitReason(const std::string& cmd) {
    std::istringstream stm(cmd);
    std::string command, reason;

    stm >> command;
    std::size_t pos = cmd.find(command);

    if (pos != std::string::npos) {
        reason = cmd.substr(pos + command.length());
        reason.erase(0, reason.find_first_not_of(' ')); // Remove leading spaces
    }

    if (reason.empty()) {
        return "Quit";
    }

    if (reason[0] != ':') {
        std::size_t space_pos = reason.find(' ');
        if (space_pos != std::string::npos) {
            reason = reason.substr(0, space_pos);
        }
        reason = ":" + reason;
    }

    return reason;
}

void Server::NotifyChannelsOfQuit(int fd, const std::string& reason)
{
    std::string quitMessage = ":" + GetClient(fd)->GetNickName() + "!~" +
                              GetClient(fd)->GetUserName() + "@localhost QUIT " +
                              reason + "\r\n";

    for (std::vector<Channel>::iterator it = channels.begin(); it != channels.end(); /* no increment here */) {
        if (it->get_client(fd)) {
            it->remove_client(fd);
            if (it->GetClientsNumber() == 0) {
                it = channels.erase(it);
            } else {
                it->sendTo_all(quitMessage);
                ++it;
            }
        } else if (it->get_admin(fd)) {
            it->remove_admin(fd);
            if (it->GetClientsNumber() == 0) {
                it = channels.erase(it);
            } else {
                it->sendTo_all(quitMessage);
                ++it;
            }
        } else {
            ++it;
        }
    }
}

void Server::QUIT(const std::string& cmd, int fd) {
    std::string reason = GetQuitReason(cmd);

    NotifyChannelsOfQuit(fd, reason);

    std::cout << "Client <" << fd << "> Disconnected" << std::endl;

    RmChannels(fd);    // Remove the client from any channels
    rm_fromclient(fd);  // Remove the client from the server's client list
    rm_fromfds(fd);     // Remove the client's file descriptor from tracking
    close(fd);         // Close the client's connection
}
