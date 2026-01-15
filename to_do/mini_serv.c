/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_serv.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eric <eric@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 18:04:04 by eric              #+#    #+#             */
/*   Updated: 2026/01/15 18:26:58 by eric             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>

// DECLARE GLOBALE VARIABLE
int		count = 0; 	// Client count
int 	maxFds = 0; // Max number of fd use

int 	fds[100000];	//Array of fd 
char	*msgs[100000];	// Msgs

fd_set	rfds, wfds, afds; // everything needs for select() rfds ready to be reed socket, wfds ready to receive data
char	bufreads[1001]; // reed buffer
char	bufwrite[1001]; // write buffer


// CP GIVEN CODE

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

// FUNCTION TO DO

void fatal_error()
{
	write(2, "Fatal error\n", strlen("Fatal error\n"));
	exit(1);
}

void sendAll(int author, char *str)
{
	for (int fd = 0; fd <= maxFds; fd++)
		if (FD_ISSET(fd, &wfds) && fd != author)
			send(fd, str, strlen(str), 0);
}

void register_client(int fd)
{
	maxFds = fd > maxFds ? fd : maxFds;
	fds[fd] = count++;
	msgs[fd] = NULL;
	FD_SET(fd, &afds);
	sprintf(bufwrite, "server: client %d just arrived\n", fds[fd]);
	sendAll(fd, bufwrite);
}
