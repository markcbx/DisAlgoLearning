// TestRaft.cpp: 定义控制台应用程序的入口点。
//author: markchen 
//email:  542917644@qq.com
//

#include "stdafx.h"
#include "GroupMangement.h"
#include "SingleTon.h"
void TestGroupManage()
{
	Singleton<ClusterManage>::Tnstance(5, (void*)0);	
	for (int i = 0; i < 3; i++)
	{
		printf("No.%d test start****************\n", i + 1);
		Singleton<ClusterManage>::GetInstance()->InitClusterNode();
		Singleton<ClusterManage>::GetInstance()->WaitAllEnd();
		printf("No.%d test end****************\n", i + 1);
	}
	printf("all thread end\n");
}

int main()
{
	printf("Test Raft starting \n");
	TestGroupManage();
	getchar();
    return 0;
}

