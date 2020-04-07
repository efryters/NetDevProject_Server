/*
	Eric Fryters
	PROG2065 Ast 6
	March 31 - 2020
	TCP Server implementation - Source file
*/

// So the steps will be:
// Initialize Winsock
// Create socket
// Bind socket
// Listen
// Accept Connection (from client)
// Receive data, send data
// Shutdown or disconnect
// https://docs.microsoft.com/en-us/windows/win32/winsock/complete-server-code


// At: https://docs.microsoft.com/en-us/windows/win32/winsock/creating-a-basic-winsock-application
// In the left list of links, check Winsock Reference to read more about send, recv, bind, accept and other functions as needed.

/*
SQLite:
https://sqlite.org/cintro.html

The following link at tutorialspoint has complete examples of how to insert data using C language
https://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm
*/

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "../NetDev_Server/sqlite3.h"
#include "../NetDev_Server/Server.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
#define ACK_MSG "OK\r\n"

const char* dbFile = "C:\\xampp\\htdocs\\Project\\data.db";

int __cdecl main(void)
{
	punchInfo_s pi;
	ZeroMemory(&pi, sizeof(pi));

	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo* result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	/// TODO: Get SQLite stuff sorted out
	sqlite3* db;
	int dbRes = sqlite3_open_v2(dbFile, &db, SQLITE_OPEN_READWRITE, NULL);
	if (dbRes != SQLITE_OK) {
		printf("Error opening DB.");
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// No longer need server socket
	closesocket(ListenSocket);

	// Receive until the peer shuts down the connection
	do {

		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			*(recvbuf + iResult) = '\x00';
			printf("Bytes received: %d\n", iResult);
			char* localbuf = (char*)malloc(sizeof(char) * ++iResult);
			strcpy_s(localbuf, iResult, recvbuf);
			UnpackMessage(&pi, localbuf);
			free(localbuf);
			/// TODO: Got data, now process SQL statement.
			WritePunchToDB(db, &pi);
			dbRes = sqlite3_close(db);

			// Echo the buffer back to the sender
			iSendResult = send(ClientSocket, ACK_MSG, sizeof(ACK_MSG), 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
			printf("Bytes sent: %d\n", iSendResult);

		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}
		/* */

	} while (iResult > 0);

	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();
	return 0;
}

void UnpackMessage(punchInfo_s* pi, char* recvBuf)
{

	char* data = recvBuf;
	char delim[] = "&";
	char* ptr = data;

	int counter = 0;

	/* Count number of tokens */
	while ((ptr = strchr(ptr, '&')) != NULL)
	{
		counter++;
		ptr++;
	}
	counter++; // Add last token after final delimeter

	/// TODO: Could add verification that the correct # of tokens received, otherwise drop the message. 
	/// END

	char** dataArray = (char**)malloc(sizeof(char*) * (counter + 1));	// Allocate pointers for the tokens plus one for count
	*(dataArray) = (char*)malloc(sizeof(char));
	*(*(dataArray)) = counter;											// Set first element as # of values


	std::cout << "Original String: " << std::endl;
	puts(data);

	char* out = NULL;
	char* nextOut = NULL;
	out = strtok_s(data, delim, &nextOut);

	std::cout << std::endl << "Tokenized String:" << std::endl;

	int elLen = 0;
	int pos = 1;

	/* Process tokens */
	while (out != NULL)
	{
		elLen = strlen(out) + 1;									   // Determine size of the token then add one for terminating char
		puts(out);
		*(dataArray + pos) = (char*)malloc(sizeof(char) * elLen); // Allocate the memory for each token found
		strcpy_s(*(dataArray + pos), elLen, out);				   // And copy the data to the new memory

		out = strtok_s(NULL, delim, &nextOut);					   // Seek next token
		pos++;
	}

	pos = 0;

	/* Extract values from keys in each token */
	do
	{

		pos++;	// Skip first element
		int select = 0;
		char* buf;
		buf = (char*)malloc(sizeof(char) * MAX_FIELD);

		/* Keys */
		const char* kId = "id=\0";
		const char* kPin = "pin=\0";

		if (strstr((const char*) * ((dataArray)+pos), kId))
		{
			select = PI_ID;
		}
		else if (strstr((const char*) * ((dataArray)+pos), kPin))
		{
			select = PI_PIN;
		}

		strcpy_s(buf, MAX_FIELD, 1 + strchr(*((dataArray)+pos), '='));

		switch (select)
		{
		case PI_ID:
			pi->employeeID = buf;
			break;
		case PI_PIN:
			pi->employeePIN = buf;
			break;
		default:
			free(buf);
		}


	} while (pos < (int) * (*dataArray));

}