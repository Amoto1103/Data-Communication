#include <string.h>
#include <stdio.h>
#include <winsock2.h>
#include <process.h>
#pragma comment(lib, "ws2_32.lib")

void do_service(void* client_s);            // Thread function

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 2);
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		return 0;
	}
	SOCKET serSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (serSocket == INVALID_SOCKET)
	{
		printf("socket error !");
		return 0;
	}
	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(8888);
	serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(serSocket, (sockaddr*)& serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		printf("bind error !");
		closesocket(serSocket);
		return 0;
	}
	
	while (1)
	{
		
		

		if (_beginthread(do_service, 4096, (void*)serSocket) < 0)
		{
			printf("ERROR - Unable to create thread \n");
			exit(1);
		}

		char sendbuf[255];
		gets_s(sendbuf);
		if (strcmp(sendbuf, "quit") == 0)
			break;
		else
		{
			const char* sep = " "; //可按多个字符来分割
			char* p=new char[256];
			p = strtok(sendbuf, sep);
			sockaddr_in sin;
			sin.sin_family = AF_INET;
			
			sin.sin_addr.S_un.S_addr = inet_addr(p);
			p = strtok(NULL, sep);
			
			sin.sin_port = htons(atoi(p));
			p = strtok(NULL, sep);
			int a = strlen(p);
			
			
			int len = sizeof(sin);
			sendto((unsigned int)serSocket, p, strlen(p)+1, 0, (sockaddr*)& sin, len);
		
			
			
		}

	}
	closesocket(serSocket);
	WSACleanup();
	return 0;
}

void do_service(void* serSocket)
{

	while (1)
	{
		sockaddr_in remoteAddr;
		int nAddrLen = sizeof(remoteAddr);
		char recvData[255];
		int ret = recvfrom((unsigned int)serSocket, recvData, 255, 0, (sockaddr*)& remoteAddr, &nAddrLen);
		if (ret > 0)
		{
			printf("from %s:%d,saying:%s\n", inet_ntoa(remoteAddr.sin_addr),ntohs(remoteAddr.sin_port),recvData);
			//printf("%s\n",recvData);
		}
		else
			break;
		
		
	}

	_endthread();
}