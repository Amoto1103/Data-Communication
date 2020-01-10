// GetHostName.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")// Used to link with Ws2_32.lib

#include <iostream>
using namespace std;

int main(int argc, char* argv[]) {
	WORD wVersion = MAKEWORD(2, 2); // Used to request version 2.2 of Windows sockets
	WSADATA wsaData;                // Data loaded by WSAStartup
	int iResult;                    // Error check if WSAStartup successful

	// Initialize Winsock
	iResult = WSAStartup(wVersion, &wsaData);
	if (iResult != 0) {
		cout << "WSAStartup failed: " << iResult << endl;
		return 1;
	}

	


	if (argc != 2) {
		char host_name[128];

		gethostname(host_name, sizeof(host_name));
		printf("Local Host Name : %s\n", host_name);
		WSACleanup();
		return 1;
	}

	DWORD dwRetval;

	struct sockaddr_in saGHN;
	char hostname[NI_MAXHOST];
	char servInfo[NI_MAXSERV];
	u_short port = 27015;

	//-----------------------------------------
		// Set up sockaddr_in structure which is passed
		// to the getnameinfo function
	saGHN.sin_family = AF_INET;
	inet_pton(AF_INET, argv[1], &saGHN.sin_addr.s_addr);
	saGHN.sin_port = htons(port);

	//-----------------------------------------
	// Call getnameinfo
	dwRetval = getnameinfo((struct sockaddr*) & saGHN,
		sizeof(struct sockaddr),
		hostname,
		NI_MAXHOST, servInfo, NI_MAXSERV, 0);

	if (dwRetval != 0) {
		printf("getnameinfo failed with error # %ld\n", WSAGetLastError());
		return 1;
	}
	else {
		printf("getnameinfo returned hostname = %s\n", hostname);
		return 0;
	}



	WSACleanup();


}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
