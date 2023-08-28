#include "GroupMangement.h"
#include "windows.h"

std::map<int, RaftNode* > ClusterManage::m_NodeMap = {};

ClusterManage::ClusterManage(int nodeNum, void *configData)
{
	m_nodeNum = nodeNum;

}
ClusterManage::~ClusterManage()
{

}

void ClusterManage::InitClusterNode()
{
	for (int i = 0; i < m_nodeNum; i++)
	{
		RaftNode * node = new RaftNode(FOLLOWER, i + 1);
		m_NodeMap[node->GetNodeId()] = node;
		std::thread * threadState = new std::thread(RaftNodeThreadRun, node);
		m_listThread.push_back(threadState);
	}
}

void ClusterManage::Clear()
{
	m_NodeMap.clear();
	for (auto it = m_listThread.begin(); it != m_listThread.end(); it++)
	{
		std::thread *pThread = *it;
		delete pThread;
	}
	m_listThread.clear();
}



void ClusterManage::WaitAllEnd()
{
	Sleep(2000);
	std::list<RaftNode*> slist;
	ClusterManage::GetNodeExclude(0, slist);

	for (auto it = slist.begin(); it != slist.end(); it++)
	{
		(*it)->SetSimuFault(true);
	}
	for (auto it = m_listThread.begin(); it != m_listThread.end(); it++)
	{
		(*it)->join();
	}
	printf("all theard exit \n");
	Clear();
}
RaftNode* ClusterManage::GetNode(int nodeId)
{
	auto it = m_NodeMap.find(nodeId);
	if (it != m_NodeMap.end())
	{
		return it->second;
	}
	return 0;
}

void ClusterManage::GetNodeExclude(int nodeId, std::list<RaftNode*> &stateList)
{
	for (auto it = m_NodeMap.begin(); it != m_NodeMap.end(); it++)
	{
		if (it->first != nodeId)
		{
			stateList.push_back(it->second);
		}
	}
}

void ClusterManage::RemvoeNodeId(int nodeId)
{
	auto it = m_NodeMap.find(nodeId);
	if (it != m_NodeMap.end())
	{
		RaftNode * tmp = it->second;
		tmp->Clear();
		delete tmp;
		m_NodeMap.erase(it);
	}
}