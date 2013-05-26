#ifndef  __LANDAGENT_H__
#define  __LANDAGENT_H__

#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#include <repast_hpc/AgentId.h>
#include <repast_hpc/Random.h>

// Action
const int DEFECT = 0;
const int COOPERATE = 1;

// Strategy
const int PTFT = 0;
const int TFT = 1;
const int RANDOM = 2;

class LandAgent: public repast::Agent {

	friend class boost::serialization::access;

private:
	// Agent
	repast::AgentId id;
	int x;
	int y;
	std::vector<LandAgent*> neighbors;
	int numNeighbors;

	// Payoffs
	int payoffT;
	int payoffR;
	int payoffP;
	int payoffS;

	// Strategy
	int strategy;

	// Trust information
	bool considerTrust;
	double deltaTrust;
	double trustThreshold;

	// Status information
	bool isIndependent;
	bool isMember;
	bool isLeader;

	// Coalition information
	repast::AgentId leaderId;
	double trustLeader;

	std::vector<repast::AgentId> coalitionMembers;
	double coalitionPayoff;

	// Agent information
	int action;
	double payoff;
	int numDefectors;

	// Random
	repast::NumberGenerator* gen;

public:
	LandAgent(repast::AgentId _id, int _payoffT, int _payoffR, int _payoffP,
			int _payoffS, int _strategy, bool _considerTrust,
			double _deltaTrust, double _trustThreshold);

	LandAgent(repast::AgentId _id, int _x, int _y, bool _isIndependent,
			bool _isMember, bool _isLeader, repast::AgentId _leaderId,
			int _action, double _payoff, double _coalitionPayoff);

	~LandAgent();

	/**
	 * AGENT' SETTERS AND GETTERS
	 */

	repast::AgentId& getId();
	const repast::AgentId& getId() const;

	int getX();
	int getY();
	void setXY(int _x, int _y);

	const std::vector<LandAgent*> getNeighbors() const;
	void setNeighbors(std::vector<LandAgent*> _neighbours);

	int getStrategy();
	void setStrategy(int _strategy);

	int getConsiderTrust();
	void setConsiderTrust(bool _considerTrust);

	double getDeltaTrust();
	void setDeltaTrust(double _deltaTrust);

	double getTrustThreshold();
	void setTrustThreshold(double _trustThreshold);

	bool getIsIndependent();
	void setIsIndependent(bool _isIndependent);

	bool getIsMember();
	void setIsMember(bool _isMember);

	bool getIsLeader();
	void setIsLeader(bool _isLeader);

	repast::AgentId getLeaderId();
	void setLeaderId(repast::AgentId _leaderId);

	double getTrustLeader();
	void setTrustLeader(double _trustLeader);

	int getAction();
	void setAction(int _action);

	double getPayoff();
	void setPayoff(double _payoff);

	double getCoalitionPayoff();
	void setCoalitionPayoff(double _coalitionPayoff);

	int getNumDefectors();
	void setNumDefectors(int _prevDefectors);

	/**
	 * ACTIONS PERFORMED BY THE AGENTS AT EACH SIMULATION CYCLE
	 */

	/**
	 * Begin cycle
	 */
	void beginCycle();

	/**
	 * Agent decides its cycle action
	 */
	void decideAction();

	/**
	 * Agent calculates its payoff based on its own action and its neighbors' actions
	 */
	void calculatePayoff();

	/**
	 * Leader agents receive its coalition members' payoff
	 */
	void addCoalitionPayoff(repast::AgentId leaderId, float _payoff);

	/**
	 * Calculates the coalition leader payoff and return the split for each
	 */
	void calculateCoalitionPayoff(float tax);

	/**
	 * Agent decides to join/leave a coalition or stay as it is
	 */
	void decideCoalition();
};

struct LandAgentPackage {

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & id;
		ar & proc;
		ar & type;
		ar & x;
		ar & y;
		ar & isIndependent;
		ar & isMember;
		ar & isLeader;
		ar & leaderId;
		ar & leaderProc;
		ar & leaderType;
		ar & action;
		ar & payoff;
		ar & coalitionPayoff;
	}

	int id;
	int proc;
	int type;
	int x;
	int y;
	bool isIndependent;
	bool isMember;
	bool isLeader;
	int leaderId;
	int leaderProc;
	int leaderType;
	int action;
	double payoff;
	double coalitionPayoff;

	repast::AgentId getId() const {
		return repast::AgentId(id, proc, type);
	}

	repast::AgentId getLeaderId() const {
		return repast::AgentId(leaderId, leaderProc, leaderType);
	}
};

struct LandAgentEdge {

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		ar & sourceAgent;
		ar & targetAgent;
	}

	// source and target agents
	LandAgentPackage sourceAgent, targetAgent;
};

#endif // __LANDAGENT_H__
