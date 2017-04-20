/*
 * This application sends a grammar name to the SSI speech recognizer plugin
 * in order to test the grammar switching functionality.
 *
 * Currently, the port number and grammar names are hardcoded constants.
 *
 * author:  Kathrin Janowski
 * created: 2015/08
 */
#include <stdio.h>
#include <stdlib.h>
#include <WS2tcpip.h>	//sockets

//constants
int			SERVER_PORT = 4000;
const char* c_grammarNames[] =
			{"Grammar1.xml", "Grammar2.xml", "Grammar3.xml"};

//variables
SOCKET			m_socket;
SOCKADDR_IN		m_serverAddress;
char			m_buffer[1024];			

void sendMessage(const char* message)
{
	strcpy(m_buffer, message);

	int result = sendto(m_socket, m_buffer, strlen(m_buffer), 0,
		(SOCKADDR*) &m_serverAddress, sizeof(SOCKADDR_IN));

	if(result==SOCKET_ERROR)
	{
		printf("Error sending message (error code: %d)\n", WSAGetLastError());
	}
	else printf("Message sent: \"%s\" (%d bytes)\n", message, result);
}

int main(int argc, char* argv[])
{
	int result = 0;

	//start WinSock ----------------------------------------------------
	//printf("Starting WinSock...\n");
	WSADATA wsa;
	result = WSAStartup(MAKEWORD(2,0),&wsa);

	if(result != 0) //failure
	{
		printf("Could not start WinSock (error code: %d)\n", result);
		return 1;
	}
	//printf("WinSock started.\n");

	//create the socket ------------------------------------------------
	
	//printf("Creating socket...\n");
	m_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_socket == INVALID_SOCKET)
	{
		printf("Could not create socket (error code: %d)\n", WSAGetLastError());
		return 2;
	}
	//printf("Socket created.\n");

	//prepare the socket connection -----------------------------------
	m_serverAddress.sin_family=AF_INET;
	m_serverAddress.sin_port=htons(SERVER_PORT);
	inet_pton(AF_INET, "127.0.0.1", &(m_serverAddress.sin_addr));

	//printf("-----------------------------------------------------\n");

	boolean running=true;
	int input = 0;

	while(running)
	{
		printf("\nEnter a number to load a grammar (1-3) or quit (0): ");
		scanf("%d", &input);

		switch(input)
		{
			case 0: running=0; break;
			case 1: sendMessage(c_grammarNames[0]); break;
			case 2: sendMessage(c_grammarNames[1]); break;
			case 3: sendMessage(c_grammarNames[2]); break;
			default: printf("Ha ha. Seriously.\n");
		}

	}

	closesocket(m_socket);

	printf("-----------------------------------------------------\n");
	printf("Bye!\n");
	
	return 0;
}

