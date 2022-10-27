#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <iostream>
#include <string>

#include <WinSock2.h>
#include <fstream>
#pragma comment(lib,"ws2_32")
#define PORT 4578
#define PACKET_SIZE 1024

#include "stopwatch.h"
#include "network.h"
#include "video.h"


int BLOBSIZE(std::string weight);
void SENDKEY(SOCKET hClient, char* key, int size);
void RECVKEY(SOCKET hClient, char* key, int size);
void SENDMAT(Mat image, SOCKET hClient);
Mat RECVMAT(SOCKET hSocket);

/*
	 <coco dataset>
	 name
	 "../data/name/coco.names"

	 weight
	 "../data/weight/coco_v5n_320.onnx"
	 "../data/weight/coco_v5n_416.onnx"
	 "../data/weight/coco_v5n_640.onnx"

	 "../data/weight/coco_v5s_320.onnx"
	 "../data/weight/coco_v5s_416.onnx"
	 "../data/weight/coco_v5s_640.onnx"

	 "../data/weight/coco_v5m_320.onnx"
	 "../data/weight/coco_v5m_416.onnx"
	 "../data/weight/coco_v5m_640.onnx"

	 "../data/weight/coco_v5l_320.onnx"
	 "../data/weight/coco_v5l_416.onnx"
	 "../data/weight/coco_v5l_640.onnx"

	 "../data/weight/coco_v5x_320.onnx"
	 "../data/weight/coco_v5x_416.onnx"
	 "../data/weight/coco_v5x_640.onnx"


	 <chess dataset>
	 name
	 "../data/name/chess.names"

	 weight
	 "chess_v5n_320.onnx"
	 "chess_v5n_416.onnx"
	 "chess_v5s_416.onnx"
*/

int main()
{
	std::string name = "../data/name/coco.names";
	std::string videofile = "../data/video/ship.mp4";
	std::string imagefile = "../data/image/chess.jpg";
	std::string weight = "../data/weight/coco_v5n_320.onnx";

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET hListen;
	hListen = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);  // ���� ������� IPV4, ���������� ����, TCP �������� �ǹ�

	SOCKADDR_IN tListenAddr = {};
	tListenAddr.sin_family = AF_INET;    // ������
	tListenAddr.sin_port = htons(PORT);  // PORT �ּ�
	tListenAddr.sin_addr.s_addr = htonl(INADDR_ANY); // IP �ּ� (s_addr�� IPV4�� �ǹ�)

	bind(hListen, (SOCKADDR*)&tListenAddr, sizeof(tListenAddr));  // ���Ͽ� �ּ������� ����
	listen(hListen, SOMAXCONN);   // ������ ���� ��� ���·� ����, �ι�° ���ڴ� �Ѳ����� ��û ������ �ִ� ���ӽ��� ��

	SOCKADDR_IN tClntAddr = {};
	int iClntSize = sizeof(tClntAddr);
	SOCKET hClient = accept(hListen, (SOCKADDR*)&tClntAddr, &iClntSize); // ���� ������� ����, accept�� Ŭ���̾�Ʈ�� �ּ����� ����ü�� �ּҰ� �� ��, �ι�° ���ڷ� ���� ����ü�� ũ�⸦ �����ص� ������ �ּ�

	char cBuffer[PACKET_SIZE] = {};
	recv(hClient, cBuffer, PACKET_SIZE, 0);  // ��� �������κ��� ������ ������ �޾��ִ� ����
	printf("Recv Msg : %s\n", cBuffer);

	char cMsg[] = "Server Send";
	send(hClient, cMsg, strlen(cMsg), 0);  // ������ �޼����� Ŭ���̾�Ʈ���� ����

	Mat image;
	Netinf ServerNet = NetworkSetting(weight, name, BLOBSIZE(weight));
	char key[2];
	while (1)
	{
//		******* Image Recv and show *******
		image = RECVMAT(hClient);
		//vshow(image, ServerNet);
		imshow("Camera", image);

//		******* key Send *******
		key[0] = key[1] = 'n';
		SENDKEY(hClient, key, 2);
		printf("%c %c\n", key[0], key[1]);

		waitKey(1);
	}


	closesocket(hListen);
	closesocket(hClient);
	WSACleanup();
	return 0;
}
int BLOBSIZE(std::string weight)
{
	int iter = 0;
	int tmp = 0;
	int check = 0;
	int result = 0;

	while (1)
	{
		if (check)
		{
			if (check == 1)
				result += (weight[iter] - '0') * 100;
			else if (check == 2)
				result += (weight[iter] - '0') * 10;
			else if (check == 3)
				result += (weight[iter] - '0');
			else if (check == 4)
				break;

			check++;
		}

		if (weight[iter] == '_')
			if (++tmp == 2)
				check = 1;

		iter++;
	}

	return result;
}
void SENDKEY(SOCKET hClient, char* key, int size)
{
	if (GetAsyncKeyState('Q') & 0x8000)
	{
		key[0] = key[1] = 'q';

		send(hClient, key, 2, 0);
	}
	else
	{
		if (GetAsyncKeyState('W') & 0x8000)
			key[0] = 'w';
		else if (GetAsyncKeyState('S') & 0x8000)
			key[0] = 's';

		if (GetAsyncKeyState('A') & 0x8000)
			key[1] = 'a';
		else if (GetAsyncKeyState('D') & 0x8000)
			key[1] = 'd';

		send(hClient, key, size, 0);
	}
	fflush(stdin);
}
void RECVKEY(SOCKET hClient, char* key, int size)
{
	recv(hClient, key, size, 0);
}
void SENDMAT(Mat image, SOCKET hSocket)
{
	char rows[4];
	char cols[4];
	int size = image.rows * image.cols * image.channels();
	strcpy_s(rows, std::to_string(image.rows).c_str());
	strcpy_s(cols, std::to_string(image.cols).c_str());
	char* buffer = new char[size];

	for (int i = 0; i < 4 - std::to_string(image.rows).size(); i++)
		rows[3 - i] = -1;
	for (int i = 0; i < 4 - std::to_string(image.cols).size(); i++)
		cols[3 - i] = -1;
	memcpy(buffer, image.data, size);
	send(hSocket, rows, 4, 0);
	send(hSocket, cols, 4, 0);
	send(hSocket, buffer, size, 0);
	delete[]buffer;
}
Mat RECVMAT(SOCKET hSocket)
{
	char rows[4];
	char cols[4];
	char* buffer;
	int size, recvbytes, riter, citer, row, col;
	riter = citer = 1;
	recvbytes = row = col = 0;

	recv(hSocket, rows, 4, 0);
	recv(hSocket, cols, 4, 0);
	for (int i = 0; i < 4; i++)
	{
		if (rows[3 - i] != -1)
		{
			row += (rows[3 - i] - '0') * riter;
			riter *= 10;
		}
		if (cols[3 - i] != -1)
		{
			col += (cols[3 - i] - '0') * citer;
			citer *= 10;
		}
	}

	size = row * col * 3;
	buffer = new char[size];
	for (int i = 0; i < size; i += recvbytes)
	{
		if ((recvbytes = recv(hSocket, buffer + i, row * col * 3 - i, 0)) == -1)
			break;
	}
	recvbytes = 0;
	Mat  image = Mat::zeros(row, col, CV_8UC3);
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			image.at<cv::Vec3b>(i, j) = cv::Vec3b(buffer[recvbytes + 0], buffer[recvbytes + 1], buffer[recvbytes + 2]);
			recvbytes = recvbytes + 3;
		}
	}

	delete[]buffer;
	return image;
}