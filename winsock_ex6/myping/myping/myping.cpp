#include <stdio.h>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h> 
#include <time.h>
#include <algorithm>
#pragma comment (lib, "ws2_32.lib")

typedef struct icmp_header
{
	unsigned char icmp_type;    // 消息类型
	unsigned char icmp_code;    // 代码
	unsigned short icmp_checksum;    // 校验和
	unsigned short icmp_id;     // 用来唯一标识此请求的ID号，通常设置为进程ID
	unsigned short icmp_sequence;   // 序列号
} icmp_header;
#define ICMP_HEADER_SIZE sizeof(icmp_header)

//计算校验和函数
unsigned short checkSum(struct icmp_header* myIcmp, int len);

static int i = 0;
static int success = 0;
static int minTime = 1000, maxTime = 0, sumTime = 0;

int main(int argc, char** argv)
{
	printf("\n正在 Ping %s 具有 32 字节的数据:\n", argv[1]);

	//创建socket
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed");
		return 0;
	}
	SOCKET mysocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

	//获取目的地址
	struct addrinfo* result = NULL;
	struct addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;

	int dwRetval = getaddrinfo(argv[1], 0, &hints, &result);
	if (dwRetval != 0) {
		printf("getaddrinfo failed with error: %d\n", dwRetval);
		WSACleanup();
		return 0;
	}

	struct sockaddr_in* socketaddr;
	char ip_part[46];
	socketaddr = (struct sockaddr_in*) result->ai_addr;

	// 构造ICMP封包
	char icmpBuff[ICMP_HEADER_SIZE + 32];
	icmp_header* myIcmp = (icmp_header*)icmpBuff;
	myIcmp->icmp_type = 8;
	myIcmp->icmp_code = 0;
	myIcmp->icmp_sequence = 0;
	myIcmp->icmp_id = (unsigned short)GetCurrentProcessId();

	// 填充数据
	memcpy((icmpBuff + ICMP_HEADER_SIZE), "abcdefghijklmnopqrstuvwabcdefghi", 32);

	char recvBuff[1024];
	memset(recvBuff, 0, 1024);

	int sockLen = sizeof(SOCKADDR);

	while (i < 4)
	{
		myIcmp->icmp_id += 10;

		// 计算校验和
		myIcmp->icmp_checksum = 0;
		myIcmp->icmp_checksum = checkSum((struct icmp_header*)icmpBuff, sizeof(icmpBuff));

		DWORD start = GetTickCount();
		sendto(mysocket, icmpBuff, sizeof(icmpBuff), 0, result->ai_addr, sockLen);
		recvfrom(mysocket, recvBuff, MAXBYTE, 0, result->ai_addr, &sockLen);
		DWORD end = GetTickCount();
		DWORD time = end - start;

		if (time == 1000) {
			printf("请求超时。\n");
			WSACleanup();
			i++;
			continue;
		}

		success++;
		sumTime += time;
		minTime = min(minTime, time);
		maxTime = max(maxTime, time);

		// ip头部的第一个字节，前4位 表示 ip协议版本号，后4位表示IP头部长度
		char ipInfo = recvBuff[0];
		// ipv4头部的第9个字节为TTL的值
		unsigned char ttl = recvBuff[8];

		if (ipInfo >> 4 == 4) {	//ipv4，跨过ip协议头，得到ICMP协议头的位置，网络字节序。 
			icmp_header* icmp_rep = (icmp_header*)(recvBuff + ((char)(ipInfo << 4) >> 2));

			inet_ntop(AF_INET, &socketaddr->sin_addr, ip_part, sizeof(ip_part));
			if (icmp_rep->icmp_type == 0) { //获得应答报文 
				printf("来自 %s 的回复：字节=32 时间=%dms TTL=%d\n", ip_part, time, ttl);
			}
			else {
				printf("Wrong.type = %d\n", icmp_rep->icmp_type);
			}
		}
		else {
			printf("Can't resolve ipv6./n");
			WSACleanup();
			return 0;
		}

		Sleep(500);
		i++;
	}

	printf("\n%s 的 Ping 统计信息:\n", argv[1]);
	printf("    数据包: 已发送 = %d，已接收 = %d，丢失 = %d (%d%% 丢失)，\n", i, success, i - success, (i - success) * 100 / i);
	if (i - success >= 4) {
		return 0;
	}
	printf("往返行程的估计时间(以毫秒为单位):\n");
	printf("    最短 = %dms，最长 = %dms，平均 = %dms\n", minTime, maxTime, sumTime / success);
	return 0;
}

unsigned short checkSum(struct icmp_header* myIcmp, int len)
{
	long sum = 0;
	unsigned short* pusicmp = (unsigned short*)myIcmp;
	while (len > 1)
	{
		sum += *(pusicmp++);
		if (sum & 0x80000000)
		{
			sum = (sum & 0xffff) + (sum >> 16);
		}
		len -= 2;
	}
	if (len)
	{
		sum += (unsigned short) * (unsigned char*)pusicmp;
	}
	while (sum >> 16)
	{
		sum = (sum & 0xffff) + (sum >> 16);
	}
	return (unsigned short)~sum;
}