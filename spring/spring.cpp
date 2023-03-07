#include <iostream>
#include<time.h>
#include<cmath>
#include<thread>
#include <windows.h>
#define MAX_THREADS 64
#define SUBDATANUM 200000
#define DATANUM (SUBDATANUM*MAX_THREADS)
using namespace std;
float rawFloatData[DATANUM];
float sortmid[DATANUM];
float summid[MAX_THREADS];
float maxmid[MAX_THREADS];
HANDLE hThreads[MAX_THREADS];
HANDLE hSemaphores[MAX_THREADS];
HANDLE g_hHandle;
/*�����࣬���а���������̬��Ա*/
class bubble {
public:
	static float sum;
	static float max;
	bubble();
	void bubblesum();
	void bubblemax();
	void bubblesort();
	void bubblebase();
	void bubblethreads();
	void sortinsert(float* p1, float* p2);
	bool sortmerge(int);
};
float bubble::sum = 0.0f;
float bubble::max = 0.0f;
/***************************************����ĳ�ʼ���ʹ���˳��*************************************************/
bubble::bubble()
{
	for (size_t i = 0; i < DATANUM; i++)
	{
		rawFloatData[i] = float(i + 1);
	}
	for (size_t i = 0; i < DATANUM; i++)
	{
		sortmid[i] = 0;
	}

	srand(1234567);//α��������ӣ�ע�ⲻ����ʱ�������ӣ����������޷�ͬ��
	float  tmp;
	size_t  T = DATANUM;
	while (T--)
	{
		size_t i, j;
		i = (size_t(rand()) * size_t(rand())) % DATANUM;
		j = (size_t(rand()) * size_t(rand())) % DATANUM;
		tmp = rawFloatData[i];
		rawFloatData[i] = rawFloatData[j];
		rawFloatData[j] = tmp;
	}
	cout << "��ʼ������" << endl;
}
/***************************************�������*************************************************/
void bubble::bubblesum()
{
	for (size_t i = 0; i < MAX_THREADS; i++)
	{
		summid[i] = 0.0f;
	}
	for (size_t j = 0; j < MAX_THREADS; j++)
	{
		for (size_t i = 0; i < SUBDATANUM; i++)
		{
			summid[j] += log(sqrt(rawFloatData[j * SUBDATANUM + i]));
		}
	}
	for (size_t i = 0; i < MAX_THREADS; i++)
	{
		sum += summid[i];
	}
}
/***************************************���������ֵ*************************************************/
void bubble::bubblemax()
{
	for (size_t i = 0; i < DATANUM; i++)
	{
		if (rawFloatData[i] > max)
		{
			max = rawFloatData[i];
		}
	}
}
/***************************************���п�������*************************************************/
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
	DataType a[10000];
	int size;
}Stack;

void InitStack(Stack* s) {//��ʼ��ջ
	s->size = 0;
}

void StackPush(Stack* s, DataType val)
{//��ջ
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
void bubble::bubblesort()
{
	for (size_t i = 0; i < DATANUM - 1; i++)
	{
		for (size_t j = 0; j < DATANUM - i - 1; j++)
		{
			if (rawFloatData[j] > rawFloatData[j + 1])
			{
				float tmp = rawFloatData[j];
				rawFloatData[j] = rawFloatData[j + 1];
				rawFloatData[j + 1] = tmp;
			}
		}
	}
}
/***************************************��������������*************************************************/
void bubble::bubblebase()
{
	LARGE_INTEGER start, end;
	QueryPerformanceCounter(&start);//start  
	bubblesum();
	bubblemax();
	QuickSortNoR(rawFloatData, 0, DATANUM - 1);
	bool comp = true;
	for (size_t i = 0; i < DATANUM - 1; i++)//�ж������Ƿ���ȷ
	{

		if (rawFloatData[i] > rawFloatData[i + 1])
		{
			comp = false;
			break;
		}
	}
	if (comp)
	{
		cout << "�����ͽ��" << sum << endl;
		cout << "�����Ϊ:" << max << endl;
		cout << "���������ȷ" << endl;
	}
	else
	{
		cout << "�����ͽ��" << sum << endl;
		cout << "�����Ϊ:" << max << endl;
		cout << "����������" << endl;
	}
	QueryPerformanceCounter(&end);//end
	std::cout << "Time Consumed:" << (end.QuadPart - start.QuadPart) << endl;
	sum = 0.0;
	max = 0.0;
}
/***************************************���̣߳�64������*************************************************/

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
/***************************************���̣߳�64�����*************************************************/

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
/***************************************���̣߳�64�������ֵ*************************************************/
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
/*��������������������Ļ�����Ԫ����������������������*/
void bubble::sortinsert(float* p1, float* p2)
{
	long long int length = p2 - p1;
	long long int i = 0;
	long long int j = 0;
	while (i < length && j < length)
	{
		if (p1[i] <= p2[j])
		{
			sortmid[i + j] = p1[i];
			i++;
		}
		else
		{
			sortmid[i + j] = p2[j];
			j++;
		}
	}
	while (i < length)
	{
		sortmid[i + j] = p1[i];
		i++;
	}
	while (j < length)
	{
		sortmid[i + j] = p2[j];
		j++;
	}
	for (size_t i = 0; i < 2 * length; i++)
	{
		p1[i] = sortmid[i];
	}
}
/***************************************�����򣨵��߳̽϶�ʱ����ݹ�ִ�У���ͬʱ��֤�������ȷ��*************************************************/
bool bubble::sortmerge(int Threads)
{
	bool comp = true;
	if (Threads > 1)
	{
		sortmerge(Threads / 2);
	}
	for (size_t i = 0; i < MAX_THREADS / Threads; i += 2)
	{
		sortinsert(&rawFloatData[i * Threads * SUBDATANUM], &rawFloatData[(i + 1) * Threads * SUBDATANUM]);
	}
	if (Threads == MAX_THREADS / 2)
	{
		for (size_t i = 0; i < DATANUM - 1; i++)
		{
			if (rawFloatData[i] > rawFloatData[i + 1])
			{
				comp = false;
				break;
			}
		}
	}
	return comp;
}
void bubble::bubblethreads()
{
	for (size_t i = 0; i < MAX_THREADS; i++)
	{
		summid[i] = 0.0f;
	}
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
		sum += summid[i];
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
		if (max < maxmid[i])
		{
			max = maxmid[i];
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
	if (sortmerge(MAX_THREADS / 2))
	{
		cout << "�����ͽ��" << sum << endl;
		cout << "�����Ϊ:" << max << endl;
		cout << "���������ȷ" << endl;
	}
	else
	{
		cout << "�����ͽ��" << sum << endl;
		cout << "�����Ϊ:" << max << endl;
		cout << "����������" << endl;
	}
	QueryPerformanceCounter(&end);//end
	std::cout << "Time Consumed:" << (end.QuadPart - start.QuadPart) << endl;
	for (size_t i = 0; i < MAX_THREADS; i++)
	{
		CloseHandle(hThreads[i]);
	}
	sum = 0.0f;
	max = 0.0f;
}

int main()
{

	bubble test1;
	test1.bubblebase();//���Ե������е��������
	bubble test2;
	test2.bubblethreads();//���Ե������е����
	return 0;
	//float a[DATANUM];//�������ϴ��룬���Ե������ܵ��õ��������
	//for (size_t i = 0; i < DATANUM; i++)
	//{
		//a[i] = float(i + 1);
	//}
}
