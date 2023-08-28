#include  "raft.h"
#include "unit.h"
#include "GroupMangement.h"
#include "windows.h"
#define LEADER_HEARTBEAT_TIMEOUT   125


RaftNode::RaftNode(ROLE curRole, int nodeId):m_CurRole(curRole),m_NodeId(nodeId)
{
	m_SimulatedFault = false;
	m_ReElection = false;
	m_LeaderId = 0;
	m_CurTerm = 0;
}

RaftNode::~RaftNode()
{

}

void RaftNode::Clear()
{
	m_TaskList.clear();
}

void RaftNode::PushListMsgData(std::shared_ptr<MsgData> msgData)
{
	{
		std::unique_lock<std::mutex> lck(m_TaskMutex);
		m_TaskList.push_back(msgData);
	}
	m_TaskCV.notify_one();
}

std::shared_ptr<MsgData> RaftNode::GetListFrontAndRemove()
{
	std::shared_ptr<MsgData> msgData;
	
	if (m_TaskList.size() > 0)
	{
		msgData = m_TaskList.front();
		m_TaskList.pop_front();
	}
	
	return msgData;
}

bool RaftNode::WaitHeartbeatData(std::shared_ptr<MsgData> &msgData)
{
	bool bRes = true;
	int TimeOutNum = GetFollowerRandomWaitTime(m_NodeId);
	std::unique_lock<std::mutex> lck(m_TaskMutex);
	m_TaskCV.wait_for(lck, std::chrono::milliseconds(TimeOutNum), [this, &msgData, &bRes] {
		msgData = GetListFrontAndRemove();
		if (msgData)
		{
			bRes = true;
			
		}
		else
		{
			bRes = false;
		}
		return bRes;
	});
	return bRes;
}

void RaftNode::FollowerRun()
{
	std::shared_ptr<MsgData> msgData;
	if (WaitHeartbeatData(msgData))
	{
		//vote request
		if (msgData->msgType == 1)
		{
			printf("%d recv %d vote request\n", m_NodeId, msgData->nodeId);
			int voteRes = 0;
			auto it = m_VoteForMap.find(msgData->voteRequest.term);
			if (it == m_VoteForMap.end())
			{
				int curLogLen = m_LogVec.size();
				int curLastTerm = 0;
				int curLastIndex = 0;
				if (curLogLen > 0)
				{
					curLastTerm = m_LogVec[curLogLen - 1].term;
					curLastIndex = m_LogVec[curLogLen - 1].index;
				}
				if (msgData->voteRequest.lastTerm > curLastTerm ||
					(msgData->voteRequest.lastTerm == curLastTerm &&
						msgData->voteRequest.lastIndex >= curLastIndex))
				{
					voteRes = 1;
					m_VoteForMap[msgData->voteRequest.term] = msgData->voteRequest.nodeId;
				}
				std::shared_ptr<MsgData> pReponse(new MsgData);
				if (pReponse)
				{
					pReponse->msgType = 2;
					pReponse->nodeId = m_NodeId;
					pReponse->voteReponse.voteflag = voteRes;
					pReponse->voteReponse.term = m_CurTerm;
					RaftNode * reponseNode= ClusterManage::GetNode(msgData->nodeId);
					if (reponseNode)
					{
						reponseNode->PushListMsgData(pReponse);
					}
				}
			}
		}
		else if (msgData->msgType == 3)//win vote node leader heartbeat come.
		{
			if (msgData->leaderWinData.term < m_CurTerm)
			{
				//todo maybe some error.
			}
			else
			{
				m_CurTerm = msgData->leaderWinData.term;
				m_LeaderId = msgData->leaderWinData.nodeId;
			}
		}
	}
	else
	{
		SetCurRole(CANDIDATE);
	}
}

ROLE RaftNode::StartElections()
{
	ROLE resRole = CANDIDATE;
	if (!m_ReElection)
	{
		m_CurTerm++;
	}

	int electionTimeOut = GetRandomNum(400, 500);
	printf("%d %d starting  curTerm %d vote timeout %d \n", m_NodeId, GetCurrentThreadId(), m_CurTerm, electionTimeOut);
	m_VoteForMap[m_CurTerm] = m_NodeId;
	std::list<RaftNode*> sList;
	ClusterManage::GetNodeExclude(m_NodeId, sList);
	int moreHalf = sList.size() / 2;
	int curVoteNum = 0;
	int curLastTerm = 0;
	int curLastIndex = 0;
	int curLoglen = m_LogVec.size();
	if (curLoglen > 0)
	{
		curLastTerm = m_LogVec[curLoglen - 1].term;
		curLastIndex = m_LogVec[curLoglen - 1].index;
	}

	std::shared_ptr<MsgData> pRequestVote(new MsgData);
	if (pRequestVote)
	{
		pRequestVote->msgType = 1;
		pRequestVote->nodeId = m_NodeId;
		pRequestVote->voteRequest.term = m_CurTerm;
		pRequestVote->voteRequest.nodeId = m_NodeId;
		pRequestVote->voteRequest.lastIndex = curLastIndex;
		pRequestVote->voteRequest.lastTerm = curLastTerm;
	}

	for (auto it = sList.begin(); it != sList.end(); it++)
	{
		(*it)->PushListMsgData(pRequestVote);
	}
	std::unique_lock<std::mutex> lck(m_TaskMutex);
	m_TaskCV.wait_for(lck, std::chrono::milliseconds(electionTimeOut), [this, &curVoteNum, moreHalf, &resRole] {
		std::shared_ptr<MsgData> taskData = GetListFrontAndRemove();
		if (taskData)
		{
			if (taskData->msgType != 2)
			{
				if (taskData->msgType == 3 && taskData->leaderWinData.term >= m_CurTerm)
				{
					//其他candidate 已经赢得选举，发送了心跳数据，收到后自己切换成follower.
					resRole = FOLLOWER;
					m_CurTerm = taskData->leaderWinData.term;
					m_LeaderId = taskData->leaderWinData.nodeId;
					return true;
				}
				//todo append item data come.
				return false;
			}
			printf("%d recv reponse data %d  type %d res:%d \n", m_NodeId, taskData->nodeId, taskData->msgType,
				taskData->voteReponse.voteflag);
			
			if (taskData->voteReponse.voteflag == 1 )
			{
				printf("%d %d win one vote\n", m_NodeId, taskData->nodeId);
				//if vote me 
				curVoteNum++;
				if (curVoteNum >= moreHalf)
				{
					resRole = LEADER;
					m_LeaderId = m_NodeId;
					//todo some init
					printf("%d win the vote term %d\n", m_NodeId, m_CurTerm);
					return true;
				}
			}
			else if (taskData->voteReponse.term > m_CurTerm)
			{
				//if lose vote. change to follower . 
				resRole = FOLLOWER;
				m_CurTerm = taskData->voteReponse.term;
				return true;
			}
		}
		return false;
	});
	if (resRole == CANDIDATE)
	{
		m_ReElection = true;
	}
	else
	{
		m_ReElection = false;
	}

	return resRole;
}

void RaftNode::CandidateRun()
{
	ROLE curRes = StartElections();
	SetCurRole(curRes);
}

//res not 0. change to follower.
int RaftNode::SendHeartBeatData()
{
	int nRes = 0;

	std::list<RaftNode*> sList;
	ClusterManage::GetNodeExclude(m_NodeId, sList);
	std::shared_ptr<MsgData> winData(new MsgData);
	if (winData)
	{
		winData->msgType = 3;
		winData->nodeId = m_NodeId;
		winData->leaderWinData.nodeId = m_NodeId;
		winData->leaderWinData.term = m_CurTerm;
		for (auto it = sList.begin(); it != sList.end(); it++)
		{
			(*it)->PushListMsgData(winData);
		}


		//todo copy data .

		std::unique_lock<std::mutex> lck(m_TaskMutex);
	    m_TaskCV.wait_for(lck, std::chrono::milliseconds(LEADER_HEARTBEAT_TIMEOUT), [this] {
			std::shared_ptr<MsgData> msgData = GetListFrontAndRemove();
			if (msgData)
			{
				//todo check some data;
				return true;
			}
			else
			{
				return false;
			}
		});
	}
	return nRes;
}

void RaftNode::LeaderRun()
{
	if (SendHeartBeatData())
	{
		//change to follower
		SetCurRole(FOLLOWER);
	}
}

void RaftNodeThreadRun(RaftNode *node)
{
	if (!node)
	{
		//param error
		return;
	}
	SetRandom(GetCurrentThreadId());
	while (!node->GetSimuFault())
	{
		switch (node->GetCurRole())
		{
		case FOLLOWER:
		{
			node->FollowerRun();
		}
			break;
		case CANDIDATE:
		{
			node->CandidateRun();
		}
			break;
		case LEADER:
		{
			node->LeaderRun();
		}
			break;
		default:
			break;
		}
	}
	printf("%d thread exit.\n", node->GetNodeId());
	ClusterManage::RemvoeNodeId(node->GetNodeId());	
}

