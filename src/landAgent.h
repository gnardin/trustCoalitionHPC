// AGENT
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
	std::vector<int> position;
	int strategy; // 0 = Random; 1 = TFT; 2 = pTFT
	std::vector<LandAgent*> neighbors;
	int numNeighbors;

	// Trust information
	bool considerTrust;
	double deltaTrust;
	double trustThreshold;

	// Status information
	bool isIndependent;
	bool isLeader;

	// Leader information
	std::vector<int> leaderPosition;
	double trustLeader;

	// Coalition information
	double coalitionPayoff;
	std::vector<std::vector<int> > coalitionMembers;

	// Agent information
	int action; // 0 = defect; 1 = cooperate
	double payoff;
	int prevDefectors;

	// Random
	repast::NumberGenerator* gen;

public:
	LandAgent(repast::AgentId _id, int _strategy, bool _considerTrust,
			double _deltaTrust, double _trustThreshold);

	LandAgent(repast::AgentId _id, int _x, int _y, int _strategy,
			bool _considerTrust, double _deltaTrust, double _trustThreshold,
			bool _isIndependent, bool _isLeader, int _leaderX, int _leaderY,
			double _trustLeader, int _action, double _payoff,
			int _prevDefectors);

	~LandAgent();

	/**
	 * AGENT'S SETTERS AND GETTERS
	 */

	repast::AgentId& getId();
	const repast::AgentId& getId() const;

	std::vector<int> getPosition();
	void setPosition(std::vector<int> _position);

	const std::vector<LandAgent*> getNeighbors() const;
	void setNeighbors(std::vector<LandAgent*> _neighbours);

	int getStrategy();
	void setStrategy(int _strategy);

	int getConsiderTrust();
	void setConsiderTrust(bool _considerTrust);

	double getDeltaTrust();
	void setDeltaTrust(double _deltaTrust);

	double getTrustThreshold();
	void setTrustThreshold(double _threshold);

	bool getIsIndependent();
	void setIsIndependent(bool _isIndependent);

	bool getIsLeader();
	void setIsLeader(bool _isLeader);

	std::vector<int> getLeaderPosition();
	void setLeaderPosition(std::vector<int> _leaderPosition);

	double getTrustLeader();
	void setTrustLeader(double _trustLeader);

	int getAction();
	void setAction(int _action);

	double getPayoff();
	void setPayoff(double _payoff);

	int getPrevDefectors();
	void setPrevDefectors(int _prevDefectors);

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
	void calculatePayoff(int _payoffT, int _payoffR, int _payoffP,
			int _payoffS);

	/**
	 * Leader agents receive its coalition members' payoff
	 */
	void addCoalitionPayoff(std::vector<int> _memberPosition, float _payoff);

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
		ar & strategy;
		ar & considerTrust;
		ar & deltaTrust;
		ar & trustThreshold;
		ar & isIndependent;
		ar & isLeader;
		ar & leaderX;
		ar & leaderY;
		ar & trustLeader;
		ar & action;
		ar & payoff;
		ar & prevDefectors;
	}

	int id;
	int proc;
	int type;
	int x;
	int y;
	int strategy;
	bool considerTrust;
	double deltaTrust;
	double trustThreshold;
	bool isIndependent;
	bool isLeader;
	int leaderX;
	int leaderY;
	double trustLeader;
	int action;
	double payoff;
	int prevDefectors;

	repast::AgentId getId() const {
		return repast::AgentId(id, proc, type);
	}
};

#endif // __LANDAGENT_H__
