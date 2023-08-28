#include "unit.h"
#include <random>
#include "time.h"

void SetRandom(int base)
{
	time_t t = time(0);
	tm  tmp;
	localtime_s(&tmp, &t);
	srand(tmp.tm_sec + tmp.tm_min+ base);
}

int GetRandomNum(int min, int max)
{
	int num = min + rand() % (max - min);
	return num;
}

//todo 假定测试时只用5个节点测试。
//这里临时处理5个follower取得随机数相同的问题。
//后续需要改变这里的逻辑
//调试发现rand()函数与线程相关，不同的线程，可以完全获得相同的随机数序列
//每个线程设置不同的随机数种子，可以避免此种情况的出现
std::vector<int>  g_random_timeV = { 536,628,778,829,988 };
int GetFollowerRandomWaitTime(int base)
{
	int index = (rand() + base) % 5;
	return g_random_timeV[index];
}