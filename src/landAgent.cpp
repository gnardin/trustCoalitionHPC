// AGENT
#include "landAgent.h"

LandAgent::LandAgent(repast::AgentId _id, int _strategy, bool _considerTrust,
		double _deltaTrust, double _trustThreshold) {
	id = _id;
	position = std::vector<int>(0, 0);
	strategy = _strategy;
	numNeighbors = 0;

	considerTrust = _considerTrust;
	deltaTrust = _deltaTrust;
	trustThreshold = _trustThreshold;

	isIndependent = true;
	isLeader = false;

	leaderPosition = std::vector<int>(0, 0);
	trustLeader = 0;

	coalitionPayoff = 0;
	coalitionMembers = std::vector<std::vector<int> >();

	action = 0;
	payoff = 0;
	prevDefectors = 0;

	gen = repast::Random::instance()->getGenerator("decisionAction");
}

LandAgent::LandAgent(repast::AgentId _id, int _x, int _y, int _strategy,
		bool _considerTrust, double _deltaTrust, double _trustThreshold,
		bool _isIndependent, bool _isLeader, int _leaderX, int _leaderY,
		double _trustLeader, int _action, double _payoff, int _prevDefectors) {
	id = _id;
	position = std::vector<int>(_x, _y);
	strategy = _strategy;
	numNeighbors = 0;

	considerTrust = _considerTrust;
	deltaTrust = _deltaTrust;
	trustThreshold = _trustThreshold;

	isIndependent = _isIndependent;
	isLeader = _isLeader;

	leaderPosition = std::vector<int>(_leaderX, _leaderY);
	trustLeader = _trustLeader;

	coalitionPayoff = 0;
	coalitionMembers = std::vector<std::vector<int> >();

	action = _action;
	payoff = _payoff;
	prevDefectors = _prevDefectors;

	gen = repast::Random::instance()->getGenerator("decisionAction");
}

LandAgent::~LandAgent() {
}

repast::AgentId& LandAgent::getId() {
	return id;
}
const repast::AgentId& LandAgent::getId() const {
	return id;
}

std::vector<int> LandAgent::getPosition() {
	return position;
}
void LandAgent::setPosition(std::vector<int> _position) {
	position = _position;
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

bool LandAgent::getIsLeader() {
	return isLeader;
}

void LandAgent::setIsLeader(bool _isLeader) {
	isLeader = _isLeader;
}

std::vector<int> LandAgent::getLeaderPosition() {
	return leaderPosition;
}
void LandAgent::setLeaderPosition(std::vector<int> _leaderPosition) {
	leaderPosition = _leaderPosition;
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

int LandAgent::getPrevDefectors() {
	return prevDefectors;
}

void LandAgent::setPrevDefectors(int _prevDefectors) {
	prevDefectors = _prevDefectors;
}

/**
 * ACTIONS PERFORMED BY THE AGENTS AT EACH SIMULATION CYCLE
 */

void LandAgent::beginCycle() {
	action = 0;
	payoff = 0;

	coalitionPayoff = 0;
	coalitionMembers = std::vector<std::vector<int> >();
}

void LandAgent::decideAction() {
	// pTFT
	if (strategy == PTFT) {
		if ((prevDefectors / numNeighbors) > gen->next()) {
			action = DEFECT;
		} else {
			action = COOPERATE;
		}
		// TFT
	} else if (strategy == TFT) {
		if (prevDefectors > (numNeighbors / 2)) {
			action = DEFECT;
		} else {
			action = COOPERATE;
		}
		// Random
	} else if (strategy == RANDOM) {
		action = (int) gen->next();
	}
}

void LandAgent::calculatePayoff(int _payoffT, int _payoffR, int _payoffP,
		int _payoffS) {

	int numCooperate = 0;
	int numDefect = 0;
	int neighborAction = 0;
	std::vector<LandAgent*>::iterator it;

	// Independent or Coalition Member
	if (!isLeader) {

		// Coalition Member
		if (!isIndependent) {
			for (it = neighbors.begin(); it != neighbors.end(); ++it) {
				if (((*it)->getLeaderPosition()[0] == leaderPosition[0])
						&& ((*it)->getLeaderPosition()[1] == leaderPosition[1])) {
					payoff += _payoffR;
				} else {
					neighborAction = (*it)->getAction();
					if (neighborAction == COOPERATE) {
						payoff += _payoffT;
					} else if (neighborAction == DEFECT) {
						payoff += _payoffP;
						numDefect++;
					}
				}
			}
		}
		// Independent and Cooperated
		else if (action == COOPERATE) {
			for (it = neighbors.begin(); it != neighbors.end(); ++it) {
				neighborAction = (*it)->getAction();
				if (neighborAction == COOPERATE) {
					numCooperate++;
				} else if (neighborAction == DEFECT) {
					numDefect++;
				}
			}
			payoff = (numCooperate * _payoffR) + (numDefect * _payoffS);
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
			payoff = (numCooperate * _payoffT) + (numDefect * _payoffP);
		}

		payoff = payoff / (float) numNeighbors;
		prevDefectors = numDefect;
	}
}

void LandAgent::addCoalitionPayoff(std::vector<int> _memberPosition,
		float _payoff) {
	coalitionPayoff += _payoff;
	coalitionMembers.push_back(_memberPosition);
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
