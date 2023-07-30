/******************************************************************************

   Copyright (C) 1993-1996 by id Software, Inc.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   DESCRIPTION:

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* TODO: Abstract this away. */
#if defined(__unix__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/ioctl.h>
#elif defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include "i_system.h"
#include "d_event.h"
#include "d_net.h"
#include "m_argv.h"

#include "doomstat.h"

#include "i_net.h"


/* NETWORKING */

static int     DOOMPORT =      666;

static int                     sendsocket;
static int                     insocket;

static struct  sockaddr_in     sendaddress[MAXNETNODES];


/* UDPsocket */
static int UDPsocket (void)
{
	int s;

	/* allocate a socket */
	s = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == -1)
#if defined(__unix__)
		I_Error ("can't create socket: %s",strerror(errno));
#elif defined(_WIN32)
		I_Error ("can't create socket: error code %d",WSAGetLastError());
#endif

	return s;
}

/* BindToLocalPort */
static void
BindToLocalPort
( int   s,
  int   port )
{
	int                 v;
	struct sockaddr_in  address;

	memset (&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = port;

	v = bind (s, (struct sockaddr *)&address, sizeof(address));
	if (v == -1)
#if defined(__unix__)
		I_Error ("BindToPort: bind: %s", strerror(errno));
#elif defined(_WIN32)
		I_Error ("BindToPort: bind: error code %d", WSAGetLastError());
#endif
}


/* PacketSend */
static void PacketSend (void)
{
	int         c;
	doomdata_t  sw;

	/* byte swap */
	sw.checksum = htonl(netbuffer->checksum);
	sw.player = netbuffer->player;
	sw.retransmitfrom = netbuffer->retransmitfrom;
	sw.starttic = netbuffer->starttic;
	sw.numtics = netbuffer->numtics;
	for (c=0 ; c< netbuffer->numtics ; c++)
	{
		sw.cmds[c].forwardmove = netbuffer->cmds[c].forwardmove;
		sw.cmds[c].sidemove = netbuffer->cmds[c].sidemove;
		sw.cmds[c].angleturn = htons(netbuffer->cmds[c].angleturn);
		sw.cmds[c].consistancy = htons(netbuffer->cmds[c].consistancy);
		sw.cmds[c].chatchar = netbuffer->cmds[c].chatchar;
		sw.cmds[c].buttons = netbuffer->cmds[c].buttons;
	}

   /* printf ("sending %i\n",gametic); */
	c = sendto (sendsocket , (const char*)&sw, doomcom->datalength
				,0,(struct sockaddr *)&sendaddress[doomcom->remotenode]
				,sizeof(sendaddress[doomcom->remotenode]));

	/*  if (c == -1) */
#if defined(__unix__)
	/*          I_Error ("SendPacket error: %s",strerror(errno)); */
#elif defined(_WIN32)
	/*          I_Error ("SendPacket error: error code %d",WSAGetLastError()); */
#endif
}


/* PacketGet */
static void PacketGet (void)
{
	int                 i;
	int                 c;
	struct sockaddr_in  fromaddress;
	socklen_t           fromlen;
	doomdata_t          sw;

	fromlen = sizeof(fromaddress);
	c = recvfrom (insocket, (char*)&sw, sizeof(sw), 0
				  , (struct sockaddr *)&fromaddress, &fromlen );
	if (c == -1 )
	{
	#if defined(__unix__)
		if (errno != EWOULDBLOCK)
			I_Error ("GetPacket: %s",strerror(errno));
	#elif defined(_WIN32)
		if (WSAGetLastError() != WSAEWOULDBLOCK)
			I_Error ("GetPacket: error code %d",WSAGetLastError());
	#endif
		doomcom->remotenode = -1;               /* no packet */
		return;
	}

	{
		static int first=1;
		if (first)
			printf("len=%d:p=[0x%x 0x%x] \n", c, *(int*)&sw, *((int*)&sw+1));
		first = 0;
	}

	/* find remote node number */
	for (i=0 ; i<doomcom->numnodes ; i++)
		if ( fromaddress.sin_addr.s_addr == sendaddress[i].sin_addr.s_addr )
			break;

	if (i == doomcom->numnodes)
	{
		/* packet is not from one of the players (new game broadcast) */
		doomcom->remotenode = -1;               /* no packet */
		return;
	}

	doomcom->remotenode = i;                    /* good packet from a game player */
	doomcom->datalength = c;

	/* byte swap */
	netbuffer->checksum = ntohl(sw.checksum);
	netbuffer->player = sw.player;
	netbuffer->retransmitfrom = sw.retransmitfrom;
	netbuffer->starttic = sw.starttic;
	netbuffer->numtics = sw.numtics;

	for (c=0 ; c< netbuffer->numtics ; c++)
	{
		netbuffer->cmds[c].forwardmove = sw.cmds[c].forwardmove;
		netbuffer->cmds[c].sidemove = sw.cmds[c].sidemove;
		netbuffer->cmds[c].angleturn = ntohs(sw.cmds[c].angleturn);
		netbuffer->cmds[c].consistancy = ntohs(sw.cmds[c].consistancy);
		netbuffer->cmds[c].chatchar = sw.cmds[c].chatchar;
		netbuffer->cmds[c].buttons = sw.cmds[c].buttons;
	}
}


/* I_InitNetwork */
void I_InitNetwork (void)
{
#if defined(__unix__)
	int
#elif defined(_WIN32)
	u_long
#endif
	                    trueval = d_true;
	int                 i;
	int                 p;
	struct hostent*     hostentry;      /* host information entry */

	doomcom = (doomcom_t*)malloc (sizeof (*doomcom) );
	memset (doomcom, 0, sizeof(*doomcom) );

	/* set up for network */
	i = M_CheckParm ("-dup");
	if (i && i< myargc-1)
	{
		doomcom->ticdup = myargv[i+1][0]-'0';
		if (doomcom->ticdup < 1)
			doomcom->ticdup = 1;
		if (doomcom->ticdup > 9)
			doomcom->ticdup = 9;
	}
	else
		doomcom-> ticdup = 1;

	if (M_CheckParm ("-extratic"))
		doomcom-> extratics = 1;
	else
		doomcom-> extratics = 0;

	p = M_CheckParm ("-port");
	if (p && p<myargc-1)
	{
		DOOMPORT = atoi (myargv[p+1]);
		printf ("using alternate port %i\n",DOOMPORT);
	}

	/* parse network game options, */
	/*  -net <consoleplayer> <host> <host> ... */
	i = M_CheckParm ("-net");
	if (!i)
	{
		/* single player game */
		netgame = d_false;
		doomcom->id = DOOMCOM_ID;
		doomcom->numplayers = doomcom->numnodes = 1;
		doomcom->deathmatch = d_false;
		doomcom->consoleplayer = 0;
		return;
	}

#ifdef _WIN32
	/* initialise WSA */
	{
		int error_code;
		WSADATA wsa_data;

		error_code = WSAStartup(MAKEWORD(2, 2), &wsa_data);

		if (error_code != 0)
			I_Error("WSAStartup failed with error code %d", error_code);
	}
#endif

	netgame = d_true;

	/* parse player number and host list */
	doomcom->consoleplayer = myargv[i+1][0]-'1';

	doomcom->numnodes = 1;      /* this node for sure */

	i++;
	while (++i < myargc && myargv[i][0] != '-')
	{
		sendaddress[doomcom->numnodes].sin_family = AF_INET;
		sendaddress[doomcom->numnodes].sin_port = htons(DOOMPORT);
		if (myargv[i][0] == '.')
		{
			sendaddress[doomcom->numnodes].sin_addr.s_addr
				= inet_addr (myargv[i]+1);
		}
		else
		{
			hostentry = gethostbyname (myargv[i]);
			if (!hostentry)
				I_Error ("gethostbyname: couldn't find %s", myargv[i]);
			sendaddress[doomcom->numnodes].sin_addr.s_addr
				= *(int *)hostentry->h_addr_list[0];
		}
		doomcom->numnodes++;
	}

	doomcom->id = DOOMCOM_ID;
	doomcom->numplayers = doomcom->numnodes;

	/* build message to receive */
	insocket = UDPsocket ();
	BindToLocalPort (insocket,htons(DOOMPORT));
#if defined(__unix__)
	ioctl
#elif defined(_WIN32)
	ioctlsocket
#endif
		(insocket, FIONBIO, &trueval);

	sendsocket = UDPsocket ();
}


void I_DeinitNetwork(void)
{
#ifdef _WIN32
	if (WSACleanup() == SOCKET_ERROR)
		I_Error("WSACleanup failed with error code %d", WSAGetLastError());
#endif
}


void I_NetCmd (void)
{
	switch (doomcom->command)
	{
		case CMD_SEND:
			PacketSend ();
			break;

		case CMD_GET:
			PacketGet ();
			break;

		default:
			I_Error ("Bad net cmd: %i",doomcom->command);
			break;

	}
}

