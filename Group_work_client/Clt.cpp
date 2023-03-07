//TCP client

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib,"ws2_32.lib")
#include <WinSock2.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include "stdafx.h"
#include <Windows.h>
#include <math.h>
#include<cmath>
#include<thread>
#include <emmintrin.h>  //open look,look 128bit
#include <immintrin.h>  //open look,look 256bit
#include <zmmintrin.h>  //open look,look 512bit

#define MAX_THREADS 64
#define SUBDATANUM 1000000
#define DATANUM (SUBDATANUM*MAX_THREADS)
#define RECEIVEONCE DATANUM     //���ν���������

using namespace std;

//����������
float rawFloatData[DATANUM];

float receiveSorts[DATANUM + RECEIVEONCE];
float tmpSorts[2 * DATANUM];
float sortmid[DATANUM];
float summid[MAX_THREADS];
float maxmid[MAX_THREADS];

HANDLE hThreads[MAX_THREADS];
HANDLE hSemaphores[MAX_THREADS];
HANDLE g_hHandle;

float summ = 0.0f;
float maxx = 0.0f;

/***************************************���̣߳�64������*************************************************/
void Swap(float* a, float* b)
{//�������е��������ݽ��н���
	float temp = *b;
	*b = *a;
	*a = temp;
}
int MidNum(float* a, int b, int c, int d)
{
	float n_1 = a[b], n_2 = a[c], n_3 = a[d];
	float temp[3] = { n_1,n_2,n_3 };
	if (temp[0] > temp[1]) {
		Swap(&temp[0], &temp[1]);
	}
	if (temp[1] > temp[2]) {
		Swap(&temp[1], &temp[2]);
	}
	if (temp[0] > temp[1]) {
		Swap(&temp[0], &temp[1]);
	}
	if (temp[1] == a[b]) {
		return b;
	}
	if (temp[1] == a[c]) {
		return c;
	}
	return d;
}
int PartSort(float* a, int left, int right)
{
	int cur, prev, mid = MidNum(a, left, (left + right) / 2, right);//����ȡ��
	Swap(&a[left], &a[mid]);//����׼���ŵ��ײ�
	prev = left;//����ǰ��ָ��
	cur = prev + 1;
	while (cur <= right) {
		if (a[cur] < a[left] && ++prev != cur) {//cur�ұȱ�׼��С��,prev�ұȱ�׼�����Ȼ�󽻻�
			Swap(&a[cur], &a[prev]);
		}
		cur++;
	}
	Swap(&a[left], &a[prev]);//prev���ڵ�λ����Զ��С�ڱ�׼�������һλ,����׼�����佻�������м�
	return prev;
}
typedef int DataType;
typedef struct Stack {//����ջ
	DataType a[20000];
	int size;
}Stack;

void InitStack(Stack* s) {//��ʼ��ջ
	s->size = 0;
}

void StackPush(Stack* s, DataType val) {//��ջ
	s->a[s->size++] = val;
}

void StackPop(Stack* s) {//��ջ
	s->size--;
}

DataType StackTop(Stack* s) {//��ȡջ��Ԫ��
	return s->a[s->size - 1];
}
void QuickSortNoR(float* a, int left, int right)
{//�ǵݹ����
	if (left >= right) {//����Ԫ��С�ڵ���1ֱ���˳�
		return;
	}
	Stack s;//����ջ
	InitStack(&s);//��ʼ��ջ
	//������������յ���ջ,��Ϊջ��������������,Ϊ�˷������ʹ����right����,left����.
	StackPush(&s, right);
	StackPush(&s, left);
	while (0 != s.size) {//��ջ��Ϊ������ʼģ�µݹ���ý���ѭ������.
		int i = StackTop(&s);//�������е�����Ԫ��ȡ��,��Ϊ��һ�����俪ʼ���ÿ���.
		StackPop(&s);
		int j = StackTop(&s);
		StackPop(&s);
		int mid = PartSort(a, i, j);//����һ�ο���,�����ر�׼�����ڵ�λ��.
		if (mid > i + 1) {//�жϱ�׼������Ƿ񻹴�������,���ھͽ��������յ���ջ.
			StackPush(&s, mid - 1);
			StackPush(&s, i);
		}
		if (mid < j - 1) {//�жϱ�׼���Ҳ��Ƿ񻹴�������,���ھͽ��������յ���ջ.
			StackPush(&s, j);
			StackPush(&s, mid + 1);
		}
	}
}
DWORD WINAPI Threadsort(LPVOID lpParameter)
{
	float* p = (float*)lpParameter;
	QuickSortNoR(p, 0, SUBDATANUM - 1);
	/*for (size_t i = 0; i < SUBDATANUM - 1; i++)
	{
		for (size_t j = 0; j < SUBDATANUM - i - 1; j++)
		{
			if (p[j] > p[j + 1])
			{
				float tmp = p[j];
				p[j] = p[j + 1];
				p[j + 1] = tmp;
			}
		}
	}*/
	return 0;
}
/***************************************���̣߳�32�����*************************************************/

DWORD WINAPI Threadsum(LPVOID lpParameter)
{
	WaitForSingleObject(g_hHandle, INFINITE);
	float* p = (float*)lpParameter;
	long long int j = (p - &rawFloatData[0]) / SUBDATANUM;
	for (size_t i = 0; i < SUBDATANUM; i++)
	{
		summid[j] += log(sqrt(p[i]));
	}
	ReleaseMutex(g_hHandle);
	return 0;
}
/***************************************���̣߳�32�������ֵ*************************************************/
DWORD WINAPI Threadmax(LPVOID lpParameter)
{
	float* p = (float*)lpParameter;
	int j = (p - rawFloatData) / SUBDATANUM;
	WaitForSingleObject(g_hHandle, INFINITE);
	for (size_t i = 0; i < SUBDATANUM; i++)
	{
		if (maxmid[j] < p[i])
		{
			maxmid[j] = p[i];
		}
	}
	ReleaseMutex(g_hHandle);
	return 0;
}

/*��������������������Ļ�����Ԫ���������������������򣬱ȽϷ�������Ϊ�ǶԵ�һ������������ǵ�������
�ռ任ʱ�䡣��ô���������������д洢��ʵЧ�ʻ������*/

void sortinsert(float* p1, float* p2)
{
	long long int i = p2 - p1;
	float* p1_add = p1;
	float* p2_add = p2;
	for (int j = 0; j < i; j++)
	{
		if (*p1_add <= *p2_add)
		{
			p1_add++;
		}
		else
		{
			float tmp = *p2_add;
			for (size_t k = (p2_add - p1_add); k > 0; k--)
			{
				p1_add[k] = p1_add[k - 1];
			}
			*p1_add = tmp;
			if (p2_add - p2 + 1 == p2 - p1)
			{
				break;
			}
			p2_add++;
			p1_add++;
			j--;
		}
	}
}

/***************************************�����򣨵��߳̽϶�ʱ����ݹ�ִ�У�*************************************************/
void sortmerge(int Threads)
{
	if (Threads > 1)
	{
		sortmerge(Threads / 2);
	}
	for (size_t i = 0; i < MAX_THREADS / Threads; i += 2)
	{
		sortinsert(&rawFloatData[i * Threads * SUBDATANUM], &rawFloatData[(i + 1) * Threads * SUBDATANUM]);
	}
	
}

void joinsort(float a[], float b[], float c[], int lenth)
{
	int i = 0;
	int j = 0;
	while (i < lenth && j < lenth)
	{
		if (a[i] < b[j])
		{
			c[i + j] = a[i];
			i++;
			continue;
		}
		c[i + j] = b[j];
		j++;
	}
	while (i < lenth)
	{
		c[i + j] = a[i];
		i++;
	}
	while (j < lenth)
	{
		c[i + j] = b[j];
		j++;
	}
}

int main()
{
	//���ݳ�ʼ��
	for (size_t i = 0; i < DATANUM; i++)
	{
		rawFloatData[i] = float(i + 1);
	}
	srand(1234567);//α��������ӣ�ע�ⲻ����ʱ�������ӣ����������޷�ͬ��
	float  tmp;
	size_t  T = DATANUM;
	while (T--)
	{
		size_t i, j;
		i = rand() % DATANUM;
		j = rand() % DATANUM;
		tmp = rawFloatData[i];
		rawFloatData[i] = rawFloatData[j];
		rawFloatData[j] = tmp;
	}
	cout << "��ʼ�����" << endl;

	//TCP����
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);
	if (WSAStartup(DllVersion, &wsaData) != 0)
	{
		MessageBoxA(NULL, "Winsock startup error", "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	SOCKADDR_IN addr; //Adres przypisany do socketu Connection
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("192.168.3.5"); //Addres = localhost
	addr.sin_port = htons(50010); //Port = 1111
	addr.sin_family = AF_INET; //IPv4 Socket

	SOCKET newConnection = socket(AF_INET, SOCK_STREAM, NULL);
	if (connect(newConnection, (SOCKADDR*)&addr, sizeofaddr) != 0) //Connection
	{
		MessageBoxA(NULL, "Blad Connection", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}
	
	//�޸Ļ�������С
	int  optVal = DATANUM * 4 * 8;
	int optLen = sizeof(optVal);
	setsockopt(newConnection, SOL_SOCKET, SO_RCVBUF, (char*)&optVal, optLen);
	int optval2 = 0;
	getsockopt(newConnection, SOL_SOCKET, SO_RCVBUF, (char*)&optval2, &optLen);
	printf("recv buf is %d\n", optval2);

	if (newConnection == 0)
	{
		std::cout << "bad connection." << std::endl;
	}
	else {
		

		int begin = 1000;
		send(newConnection, (char*)&begin, sizeof(begin), NULL);
		printf("start signal sended\n");

		LARGE_INTEGER start, end;
		QueryPerformanceCounter(&start);//start  
		
		g_hHandle = CreateMutex(NULL, FALSE, NULL);
		for (size_t i = 0; i < MAX_THREADS; i++)
		{
			//hSemaphores[i] = CreateSemaphore(NULL, 0, 1, NULL);//CreateEvent(NULL,TRUE,FALSE)�ȼ�
			hThreads[i] = CreateThread(
				NULL,
				0,
				Threadsum,
				&rawFloatData[i * SUBDATANUM],
				0,
				NULL);
		}
		WaitForMultipleObjects(MAX_THREADS, hThreads, TRUE, INFINITE);
		for (size_t i = 0; i < MAX_THREADS; i++)
		{
			summ += summid[i];
		}
		for (size_t i = 0; i < MAX_THREADS; i++)
		{
			hThreads[i] = CreateThread(
				NULL,
				0,
				Threadmax,
				&rawFloatData[i * SUBDATANUM],
				0,
				NULL);
		}
		WaitForMultipleObjects(MAX_THREADS, hThreads, true, INFINITE);
		for (size_t i = 0; i < MAX_THREADS; i++)
		{
			if (maxx < maxmid[i])
			{
				maxx = maxmid[i];
			}
		}
		for (int i = 0; i < MAX_THREADS; i++)
		{

			hThreads[i] = CreateThread(
				NULL,// ��ȫ���Ա��
				0,// Ĭ��ջ�ռ�
				Threadsort,//�ص�����
				&rawFloatData[i * SUBDATANUM],// �ص��������
				0,
				NULL);
		}
		WaitForMultipleObjects(MAX_THREADS, hThreads, true, INFINITE);	//�������̷߳���ʱ���ܽ���
		sortmerge(MAX_THREADS / 2);
		//end = GetTickCount64();//200ms
		//��������
		int receivesuccess = 0;
		int received = 0;
		int i = 0;
		char* receivePin = (char*)receiveSorts;
		while (1) {

			receivesuccess = recv(newConnection, &receivePin[received], RECEIVEONCE * sizeof(float), NULL);
			//printf("��%d�ν��գ�������������%d\n", i, receivesuccess);
			i = i + 1;

			if (receivesuccess == -1)//��ӡ������Ϣ
			{
				int erron = WSAGetLastError();
				//printf("erron=%d\n", erron);
			}
			else
			{
				received = received + receivesuccess;
			}
			if (received >= (4 * DATANUM + 2))
			{
				//receivesuccess = recv(newConnection, (char*)&receivedsum, sizeof(float), NULL);
				//receivesuccess = recv(newConnection, (char*)&receivedmax, sizeof(float), NULL);
				break;
			}
		}

		//�ٹ鲢
		joinsort(receiveSorts, rawFloatData, tmpSorts, DATANUM);
		if (receiveSorts[DATANUM + 1] > maxx)
			maxx = receiveSorts[DATANUM + 1];
		summ += receiveSorts[DATANUM + 0];

		QueryPerformanceCounter(&end);//end


		//�������
		bool comp = true;
		for (size_t i = 0; i < DATANUM + RECEIVEONCE -1; i++)
		{
			if (rawFloatData[i] > rawFloatData[i+1])
			{
				comp = false;
			}
		}
		if (comp)
		{
			cout << "�����ͽ��" << summ << endl;
			cout << "�����Ϊ:" << maxx << endl;
			cout << "���������ȷ" << endl;
		}
		else
		{
			cout << "�����ͽ��" << summ << endl;
			cout << "�����Ϊ:" << maxx << endl;
			cout << "����������" << endl;
		}
		std::cout << "Time Consumed:" << (end.QuadPart - start.QuadPart) << endl;
	}
	closesocket(newConnection);
	WSACleanup();
	return 0;
}

