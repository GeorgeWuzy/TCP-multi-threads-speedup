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
#define RECEIVEONCE DATANUM     //单次接收数据量

using namespace std;

//待测试数据
float rawFloatData[DATANUM];
float sortmid[DATANUM];
float summid[MAX_THREADS];
float maxmid[MAX_THREADS];
HANDLE hThreads[MAX_THREADS];
HANDLE hSemaphores[MAX_THREADS];
HANDLE g_hHandle;

float summ = 0.0f;
float maxx = 0.0f;

/***************************************多线程（64）排序*************************************************/
void Swap(float* a, float* b)
{//对数组中的两个数据进行交换
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
	int cur, prev, mid = MidNum(a, left, (left + right) / 2, right);//三数取中
	Swap(&a[left], &a[mid]);//将标准数放到首部
	prev = left;//定义前后指针
	cur = prev + 1;
	while (cur <= right) {
		if (a[cur] < a[left] && ++prev != cur) {//cur找比标准数小的,prev找比标准数大的然后交换
			Swap(&a[cur], &a[prev]);
		}
		cur++;
	}
	Swap(&a[left], &a[prev]);//prev所在的位置永远在小于标准数的最后一位,将标准数与其交换放在中间
	return prev;
}
typedef int DataType;
typedef struct Stack {//定义栈
	DataType a[20000];
	int size;
}Stack;

void InitStack(Stack* s) {//初始化栈
	s->size = 0;
}

void StackPush(Stack* s, DataType val) {//入栈
	s->a[s->size++] = val;
}

void StackPop(Stack* s) {//出栈
	s->size--;
}

DataType StackTop(Stack* s) {//获取栈顶元素
	return s->a[s->size - 1];
}
void QuickSortNoR(float* a, int left, int right)
{//非递归调用
	if (left >= right) {//数组元素小于等于1直接退出
		return;
	}
	Stack s;//创建栈
	InitStack(&s);//初始化栈
	//将数组的起点和终点入栈,因为栈是先入后出的特性,为了方便后续使用让right先入,left后入.
	StackPush(&s, right);
	StackPush(&s, left);
	while (0 != s.size) {//以栈空为条件开始模仿递归调用进行循环调用.
		int i = StackTop(&s);//将数组中的两个元素取出,作为第一个区间开始调用快排.
		StackPop(&s);
		int j = StackTop(&s);
		StackPop(&s);
		int mid = PartSort(a, i, j);//调用一次快排,并返回标准数所在的位置.
		if (mid > i + 1) {//判断标准数左侧是否还存在区间,存在就将其起点和终点入栈.
			StackPush(&s, mid - 1);
			StackPush(&s, i);
		}
		if (mid < j - 1) {//判断标准数右侧是否还存在区间,存在就将其起点和终点入栈.
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
/***************************************多线程（32）求和*************************************************/

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
/***************************************多线程（32）求最大值*************************************************/
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

/*插入排序函数，是重排序的基本单元，对两个有序数组重排序，比较繁琐，因为是对单一数组操作，考虑到可以用
空间换时间。那么将排序后的数组另行存储其实效率会大大提高*/

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

/***************************************重排序（当线程较多时，会递归执行）*************************************************/
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
	//数据初始化
	for (size_t i = 0; i < DATANUM; i++)
	{
		rawFloatData[i] = float(i + 1+DATANUM);

	}
	srand(1234567);//伪随机数种子，注意不能用时间做种子，否则两机无法同步
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
	cout << "初始化完成" << endl;
	//TCP连接
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);
	if (WSAStartup(DllVersion, &wsaData) != 0)
	{
		MessageBoxA(NULL, "WinSock startup error", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	SOCKADDR_IN addr;
	int addrlen = sizeof(addr);
	addr.sin_family = AF_INET; //IPv4 Socket
	addr.sin_port = htons(50010); // sever Port
	addr.sin_addr.s_addr = inet_addr("192.168.3.5"); //target PC

	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr));

	listen(sListen, SOMAXCONN);

	//while ()
	SOCKET newConnection; //build a new socket do new connection. the sListen is just listenning not used to exchange data
	newConnection = accept(sListen, (SOCKADDR*)&addr, &addrlen); //newConnection is used to exchange data with client

	//修改缓冲区大小
	int  optVal = DATANUM * 4 * 8;
	int optLen = sizeof(optVal);
	setsockopt(newConnection, SOL_SOCKET, SO_SNDBUF, (char*)&optVal, optLen);
	int optval2 = 0;
	getsockopt(newConnection, SOL_SOCKET, SO_SNDBUF, (char*)&optval2, &optLen);
	printf("send buf is %d\n", optval2);
	
	if (newConnection == 0)
	{
		cout << "bad connection." << endl;
	}
	else
	{
		int begin=0;//开始标志
		while (1)
		{
			recv(newConnection, (char*)&begin, sizeof(begin), NULL);//等待任务开始指令
			if (begin == 1000)
			{
				printf("test begin\n");
				LARGE_INTEGER start, end;
				QueryPerformanceCounter(&start);

				g_hHandle = CreateMutex(NULL, FALSE, NULL);
				for (size_t i = 0; i < MAX_THREADS; i++)
				{
					//hSemaphores[i] = CreateSemaphore(NULL, 0, 1, NULL);//CreateEvent(NULL,TRUE,FALSE)等价
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
						NULL,// 安全属性标记
						0,// 默认栈空间
						Threadsort,//回调函数
						&rawFloatData[i * SUBDATANUM],// 回调函数句柄
						0,
						NULL);
				}
				WaitForMultipleObjects(MAX_THREADS, hThreads, true, INFINITE); //当所有线程返回时才能结束

				sortmerge(MAX_THREADS / 2);
				//发送结果
				int sended;
				sended = send(newConnection, (char*)&rawFloatData, sizeof(rawFloatData), NULL);
				sended = send(newConnection, (char*)&summ, sizeof(float), NULL);
				sended = send(newConnection, (char*)&maxx, sizeof(float), NULL);

				printf("sended:%d", sended);
				QueryPerformanceCounter(&end);
				cout << "Time Consumed:" << (end.QuadPart - start.QuadPart) << endl;
				break;
			}
		}
		closesocket(newConnection);
		WSACleanup();
	}
	return 0;
}