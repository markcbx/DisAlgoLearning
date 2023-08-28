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

//todo �ٶ�����ʱֻ��5���ڵ���ԡ�
//������ʱ����5��followerȡ���������ͬ�����⡣
//������Ҫ�ı�������߼�
//���Է���rand()�������߳���أ���ͬ���̣߳�������ȫ�����ͬ�����������
//ÿ���߳����ò�ͬ����������ӣ����Ա����������ĳ���
std::vector<int>  g_random_timeV = { 536,628,778,829,988 };
int GetFollowerRandomWaitTime(int base)
{
	int index = (rand() + base) % 5;
	return g_random_timeV[index];
}