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
/*排序类，其中包含两个静态成员*/
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
/***************************************数组的初始化和打乱顺序*************************************************/
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

	srand(1234567);//伪随机数种子，注意不能用时间做种子，否则两机无法同步
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
	cout << "初始化结束" << endl;
}
/***************************************串行求和*************************************************/
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
/***************************************串行求最大值*************************************************/
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
/***************************************串行快速排序*************************************************/
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
	DataType a[10000];
	int size;
}Stack;

void InitStack(Stack* s) {//初始化栈
	s->size = 0;
}

void StackPush(Stack* s, DataType val)
{//入栈
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
/***************************************单机串行主函数*************************************************/
void bubble::bubblebase()
{
	LARGE_INTEGER start, end;
	QueryPerformanceCounter(&start);//start  
	bubblesum();
	bubblemax();
	QuickSortNoR(rawFloatData, 0, DATANUM - 1);
	bool comp = true;
	for (size_t i = 0; i < DATANUM - 1; i++)//判断排序是否正确
	{

		if (rawFloatData[i] > rawFloatData[i + 1])
		{
			comp = false;
			break;
		}
	}
	if (comp)
	{
		cout << "输出求和结果" << sum << endl;
		cout << "最大数为:" << max << endl;
		cout << "输出排序正确" << endl;
	}
	else
	{
		cout << "输出求和结果" << sum << endl;
		cout << "最大数为:" << max << endl;
		cout << "输出排序错误" << endl;
	}
	QueryPerformanceCounter(&end);//end
	std::cout << "Time Consumed:" << (end.QuadPart - start.QuadPart) << endl;
	sum = 0.0;
	max = 0.0;
}
/***************************************多线程（64）排序*************************************************/

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
/***************************************多线程（64）求和*************************************************/

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
/***************************************多线程（64）求最大值*************************************************/
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
/*插入排序函数，是重排序的基本单元，对两个有序数组重排序*/
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
/***************************************重排序（当线程较多时，会递归执行），同时验证排序的正确性*************************************************/
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
			NULL,// 安全属性标记
			0,// 默认栈空间
			Threadsort,//回调函数
			&rawFloatData[i * SUBDATANUM],// 回调函数句柄
			0,
			NULL);
	}
	WaitForMultipleObjects(MAX_THREADS, hThreads, true, INFINITE);	//当所有线程返回时才能结束
	if (sortmerge(MAX_THREADS / 2))
	{
		cout << "输出求和结果" << sum << endl;
		cout << "最大数为:" << max << endl;
		cout << "输出排序正确" << endl;
	}
	else
	{
		cout << "输出求和结果" << sum << endl;
		cout << "最大数为:" << max << endl;
		cout << "输出排序错误" << endl;
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
	test1.bubblebase();//测试单机串行的运行情况
	bubble test2;
	test2.bubblethreads();//测试单机并行的情况
	return 0;
	//float a[DATANUM];//屏蔽以上代码，测试单机所能调用的最大数组
	//for (size_t i = 0; i < DATANUM; i++)
	//{
		//a[i] = float(i + 1);
	//}
}
