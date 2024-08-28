NAME = ircserv
CC = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRCS = main.cpp Server.cpp Client.cpp user_authenticate.cpp Channel.cpp\
		commands/INVITE.cpp commands/JOIN.cpp commands/KICK.cpp \
		commands/MODE.cpp commands/PRIVMSG.cpp commands/QUIT.cpp commands/TOPIC.cpp \
		commands/PART.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CXXFLAGS) -o $(NAME) $(OBJS)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
