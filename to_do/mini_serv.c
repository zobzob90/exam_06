/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_serv.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eric <eric@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 18:04:04 by eric              #+#    #+#             */
/*   Updated: 2026/01/18 14:07:23 by eric             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>

int count = 0;
int max_fd = 0;

int fds[100000];
char *msgs[100000];

fd_set rfds, wfds, afds;
char buf_write[1001];
char buf_read[1001];

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}

void fatal_error()
{
	write (2, "Fatal error\n", strlen("Fatal error\n"));
	exit (1);
}

void sendAll(int author, char *str)
{
	for (int fd = 0; fd <= max_fd; fd++)
		if (FD_ISSET(fd, &wfds) && fd != author)
			send(fd, str, strlen(str), 0);
}

void register_client(int fd)
{
	if (fd > max_fd)
		max_fd = fd;
	fds[fd] = count++;
	msgs[fd] = NULL;
	FD_SET(fd, &afds);
	sprintf(buf_write, "server: client %d just arrived\n", fds[fd]);
	sendAll(fd, buf_write);
}

void remove_client(int fd)
{
	sprintf(buf_write, "server: client %d just left\n", fds[fd]);
	sendAll(fd, buf_write);
	free(msgs[fd]);
	FD_CLR(fd, &afds);
	close(fd);
}

int create_socket()
{
	max_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (max_fd < 0)
		fatal_error();
	FD_SET(max_fd, &afds);
	return (max_fd);
}

void send_msg(int fd)
{
	char *msg;
	while (extract_message(&(msgs[fd]), &msg))
	{
		sprintf(buf_write, "client %d: ", fds[fd]);
		sendAll(fd, buf_write);
		sendAll(fd, msg);
		free(msg);
	}
}

void handle_new_connection(int sock_fd, struct sockaddr_in *servaddr)
{
	socklen_t add = sizeof(*servaddr);
	int client = accept(sock_fd, (struct sockaddr *)servaddr, &add);
	if (client >= 0)
		register_client(client);
}

void handle_client_message(int fd)
{
	int r = recv(fd, buf_read, 1000, 0);
	if (r <= 0)
	{
		remove_client(fd);
		return ;	
	}
	buf_read[r] = '\0';
	msgs[fd] = str_join(msgs[fd], buf_read);
	send_msg(fd); 
}

int main(int ac, char *av[])
{
	if (ac != 2)
	{
		write(2, "Wrong number of arguments\n", 26);
		exit (1);
	}
	FD_ZERO(&afds);
	int sock_fd = create_socket();
	struct sockaddr_in servaddr;        //
    bzero(&servaddr, sizeof(servaddr)); //
    servaddr.sin_family = AF_INET;                //
    servaddr.sin_addr.s_addr = htonl(2130706433); //
    servaddr.sin_port = htons(atoi(av[1]));       // replace 8080 //
	if (bind(sock_fd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) //
        fatal_error();
    if (listen(sock_fd, 10)) // 
        fatal_error();
	while (1)
	{
		rfds = wfds = afds;
		if (select(max_fd + 1, &rfds, &wfds, NULL, NULL) < 0)
			fatal_error();
		for (int fd = 0; fd <= max_fd; fd++)
		{
			if (!FD_ISSET(fd, &rfds))
				continue;
			if (fd == sock_fd)
				handle_new_connection(sock_fd, &servaddr);
			else
				handle_client_message(fd);
		}
	}
}
