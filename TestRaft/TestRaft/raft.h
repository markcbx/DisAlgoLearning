#ifndef __RAFT_HEADER__
#define __RAFT_HEADER__
#include <string>
#include <list>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <map>
#include <vector>
#include <memory>

enum ROLE {
	FOLLOWER = 1,
	CANDIDATE,
	LEADER,
};

struct ClientData {
	int add_x;
	int add_y;
};

struct LogItem {
	int term;
	int index;
	ClientData cdata;
};

struct VoteRequest {
	int nodeId;
	int term;
	int lastTerm;
	int lastIndex;
};

struct VoteReponse {
	int term;
	int voteflag;//1,win vote,0 lose vote	
};

struct WinVoteData {
	int nodeId;
	int term;
};

struct MsgData {
	int msgType;// 1:voteRequest, 2:voteReponse ,3:winVoteHeartbeat
	int nodeId; //sender id.
	union {
		VoteRequest voteRequest;
		VoteReponse voteReponse;
		WinVoteData leaderWinData;
	};
};

struct RaftNode {
	RaftNode(ROLE curRole, int nodeId);
	~RaftNode();

	bool GetSimuFault()
	{
		return m_SimulatedFault;
	}
	void SetSimuFault(bool fault)
	{
		m_SimulatedFault = fault;
	}

	int GetNodeId()
	{
		return m_NodeId;
	}

	ROLE GetCurRole()
	{
		return m_CurRole;
	}
	void SetCurRole(ROLE curRole)
	{
		m_CurRole = curRole;
	}

	bool WaitHeartbeatData(std::shared_ptr<MsgData> &msgData);
	ROLE StartElections();
	int SendHeartBeatData();

	std::shared_ptr<MsgData> GetListFrontAndRemove();
	void PushListMsgData(std::shared_ptr<MsgData> msgData);

	void FollowerRun();
	void CandidateRun();
	void LeaderRun();

	void Clear();

private:
	ROLE m_CurRole;
	int m_NodeId;
	bool m_SimulatedFault;
	bool m_ReElection;
	int m_LeaderId;

	std::mutex m_TaskMutex;
	std::condition_variable m_TaskCV;
	std::list<std::shared_ptr<MsgData> > m_TaskList;

	//all node need save state
	std::map<int, int > m_VoteForMap;
	int m_CurTerm;
	std::vector<LogItem> m_LogVec;


};

void RaftNodeThreadRun(RaftNode *node);










#endif //__RAFT_HEADER__
