#ifndef __GROUP_MANGEMENT_HEADER__
#define __GROUP_MANGEMENT_HEADER__
#include "raft.h"
#include <thread>

//≤‚ ‘π‹¿Ì
class ClusterManage
{
public:
	ClusterManage(int nodeNum, void *configData);
	~ClusterManage();

	void InitClusterNode();
	void Clear();
	static RaftNode* GetNode(int nodeId);
	static void GetNodeExclude(int nodeId, std::list<RaftNode*> &nodeList);
	static void RemvoeNodeId(int nodeId);
	void WaitAllEnd();
private:
	int m_nodeNum;
	void* m_configData;
	static std::map<int, RaftNode* > m_NodeMap;
	std::list<std::thread *> m_listThread;
};





#endif //__GROUP_MANGEMENT_HEADER__
