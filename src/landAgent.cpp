#include "landAgent.h"

LandAgent::LandAgent(repast::AgentId _id, int _payoffT, int _payoffR,
		int _payoffP, int _payoffS, int _strategy, bool _considerTrust,
		double _deltaTrust, double _trustThreshold) {
	id = _id;
	x = 0;
	y = 0;
	numNeighbors = 0;

	payoffT = _payoffT;
	payoffR = _payoffR;
	payoffP = _payoffP;
	payoffS = _payoffS;

	strategy = _strategy;

	considerTrust = _considerTrust;
	deltaTrust = _deltaTrust;
	trustThreshold = _trustThreshold;

	isIndependent = true;
	isMember = false;
	isLeader = false;

	trustLeader = 0;

	coalitionPayoff = 0;

	action = 0;
	payoff = 0;
	numDefectors = 0;

	gen = repast::Random::instance()->getGenerator("decisionAction");
}

LandAgent::LandAgent(repast::AgentId _id, int _x, int _y, bool _isIndependent,
		bool _isMember, bool _isLeader, repast::AgentId _leaderId, int _action,
		double _payoff, double _coalitionPayoff) {
	id = _id;
	x = _x;
	y = _y;

	isIndependent = _isIndependent;
	isMember = _isMember;
	isLeader = _isLeader;

	leaderId = _leaderId;

	action = _action;
	payoff = _payoff;

	coalitionPayoff = _coalitionPayoff;
}

LandAgent::~LandAgent() {
}

repast::AgentId& LandAgent::getId() {
	return id;
}
const repast::AgentId& LandAgent::getId() const {
	return id;
}

int LandAgent::getX() {
	return x;
}
int LandAgent::getY() {
	return y;
}
void LandAgent::setXY(int _x, int _y) {
	x = _x;
	y = _y;
}

const std::vector<LandAgent*> LandAgent::getNeighbors() const {
	return neighbors;
}
void LandAgent::setNeighbors(std::vector<LandAgent*> _neighbors) {
	neighbors = _neighbors;
	numNeighbors = neighbors.size();
}

int LandAgent::getStrategy() {
	return strategy;
}
void LandAgent::setStrategy(int _strategy) {
	strategy = _strategy;
}

int LandAgent::getConsiderTrust() {
	return considerTrust;
}

void LandAgent::setConsiderTrust(bool _considerTrust) {
	considerTrust = _considerTrust;
}

double LandAgent::getDeltaTrust() {
	return deltaTrust;
}

void LandAgent::setDeltaTrust(double _deltaTrust) {
	deltaTrust = _deltaTrust;
}

double LandAgent::getTrustThreshold() {
	return trustThreshold;
}

void LandAgent::setTrustThreshold(double _trustThreshold) {
	trustThreshold = _trustThreshold;
}

bool LandAgent::getIsIndependent() {
	return isIndependent;
}

void LandAgent::setIsIndependent(bool _isIndependent) {
	isIndependent = _isIndependent;
}

bool LandAgent::getIsMember() {
	return isMember;
}

void LandAgent::setIsMember(bool _isMember) {
	isMember = _isMember;
}

bool LandAgent::getIsLeader() {
	return isLeader;
}

void LandAgent::setIsLeader(bool _isLeader) {
	isLeader = _isLeader;
}

repast::AgentId LandAgent::getLeaderId() {
	return leaderId;
}
void LandAgent::setLeaderId(repast::AgentId _leaderId) {
	leaderId = _leaderId;
}

double LandAgent::getTrustLeader() {
	return trustLeader;
}

void LandAgent::setTrustLeader(double _trustLeader) {
	trustLeader = _trustLeader;
}

int LandAgent::getAction() {
	return action;
}

void LandAgent::setAction(int _action) {
	action = _action;
}

double LandAgent::getPayoff() {
	return payoff;
}
void LandAgent::setPayoff(double _payoff) {
	payoff = _payoff;
}

double LandAgent::getCoalitionPayoff() {
	return coalitionPayoff;
}
void LandAgent::setCoalitionPayoff(double _coalitionPayoff) {
	coalitionPayoff = _coalitionPayoff;
}

int LandAgent::getNumDefectors() {
	return numDefectors;
}

void LandAgent::setNumDefectors(int _numDefectors) {
	numDefectors = _numDefectors;
}

/**
 * ACTIONS PERFORMED BY THE AGENTS AT EACH SIMULATION CYCLE
 */

void LandAgent::beginCycle() {
	action = 0;
	payoff = 0;

	coalitionPayoff = 0;
}

void LandAgent::decideAction() {
	// pTFT
	if (strategy == PTFT) {
		if ((numDefectors / numNeighbors) > gen->next()) {
			action = DEFECT;
		} else {
			action = COOPERATE;
		}
		// TFT
	} else if (strategy == TFT) {
		if (numDefectors > (numNeighbors / 2)) {
			action = DEFECT;
		} else {
			action = COOPERATE;
		}
		// Random
	} else if (strategy == RANDOM) {
		action = (int) gen->next();
	}
}

void LandAgent::calculatePayoff() {

	int numCooperate = 0;
	int numDefect = 0;
	int numMember = 0;
	int neighborAction = 0;
	std::vector<LandAgent*>::iterator it;

	if (!isLeader) {

		// Coalition Member
		if (isMember) {
			for (it = neighbors.begin(); it != neighbors.end(); ++it) {
				if ((*it)->getLeaderId() == leaderId) {
					numMember++;
				} else {
					neighborAction = (*it)->getAction();
					if (neighborAction == COOPERATE) {
						numCooperate++;
					} else if (neighborAction == DEFECT) {
						numDefect++;
					}
				}
			}
			payoff = (numMember * payoffR) + (numCooperate * payoffT)
					+ (numDefect * payoffP);
		} else if (isIndependent) {
			// Independent and Cooperated
			if (action == COOPERATE) {
				for (it = neighbors.begin(); it != neighbors.end(); ++it) {
					neighborAction = (*it)->getAction();
					if (neighborAction == COOPERATE) {
						numCooperate++;
					} else if (neighborAction == DEFECT) {
						numDefect++;
					}
				}
				payoff = (numCooperate * payoffR) + (numDefect * payoffS);
			}
			// Independent and Defected
			else if (action == DEFECT) {
				for (it = neighbors.begin(); it != neighbors.end(); ++it) {
					neighborAction = (*it)->getAction();
					if (neighborAction == COOPERATE) {
						numCooperate++;
					} else if (neighborAction == DEFECT) {
						numDefect++;
					}
				}
				payoff = (numCooperate * payoffT) + (numDefect * payoffP);
			}
		}

		payoff = payoff / (float) numNeighbors;
		numDefectors = numDefect;
	}
}

void LandAgent::addCoalitionPayoff(repast::AgentId _leaderId, float _payoff) {
	coalitionPayoff += _payoff;
	coalitionMembers.push_back(_leaderId);
}

void LandAgent::calculateCoalitionPayoff(float tax) {
	payoff = coalitionPayoff * ceil(tax);
	coalitionPayoff -= payoff;
}

void LandAgent::decideCoalition() {
	/**
	 if (x == leader[0] && y == leader[1]) { // Leaders cannot leave nor join coalition, neither they consider trust on a leader
	 return;
	 }
	 bool worstPayoff = true;
	 LandAgent* best = neighbors[0];
	 int nSize = neighbors.size();
	 for (it = neighbors.begin(); it != neighbors.end(); ++it) {
	 if ((*it)->getPayoff() < payoff) {
	 worstPayoff = false;
	 }
	 if ((*it)->getPayoff() > best->getPayoff()) {
	 best = (*it);
	 }
	 }
	 if (worstPayoff) {
	 if (leader[0] == -1) {
	 if (best->getLeaderX() == -1) {
	 best->setLeaderXY(best->getX(), best->getY());
	 best->setIsLeader(true);
	 best->setIsIndependent(false);
	 }
	 leader[0] = best->getLeaderX();
	 leader[1] = bets->getLeaderY();
	 isIndependent = false;
	 } else {
	 if (considerTrust) {
	 trustLeader += deltaTrust;
	 if (trustThreshold > trustLeader) {
	 leader[0] = -1;
	 leader[1] = -1;
	 }
	 } else {
	 leader[0] = -1;
	 leader[1] = -1;
	 }
	 }
	 } else if (leader[0] != -1) {
	 trustLeader += deltaTrust;
	 if (trustLeader > 100) {
	 trustLeader = 100;
	 }
	 }
	 */
}
