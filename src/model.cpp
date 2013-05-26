// MODEL
#include "model.h"

LandModel::LandModel(const std::string& _propsFile, int _argc, char* _argv[],
		mpi::communicator* _world) :
		props(_propsFile, _argc, _argv, _world), providerUpdater(agents) {

	repast::initializeRandom(props, _world);

	repast::RepastProcess* rp = repast::RepastProcess::instance();
	rank = rp->rank();
	world = _world;

	// Size of the grid
	sizeX = repast::strToInt(props.getProperty(GRID_MAX_X))
			- repast::strToInt(props.getProperty(GRID_MIN_X)) + 1;
	sizeY = repast::strToInt(props.getProperty(GRID_MAX_Y))
			- repast::strToInt(props.getProperty(GRID_MIN_Y)) + 1;

	int procX = repast::strToInt(props.getProperty(PROC_X));
	int procY = repast::strToInt(props.getProperty(PROC_Y));

	std::vector<int> procDim;
	procDim.push_back(procX);
	procDim.push_back(procY);

	int gridBuffer = repast::strToInt(props.getProperty(GRID_BUFFER));

	// Create the Grid
	grid = new repast::SharedGrids<LandAgent>::SharedWrappedGrid("grid ",
			repast::GridDimensions(repast::Point<int>(sizeX, sizeY)), procDim,
			gridBuffer, world);
	agents.addProjection(grid);

	// Size of the grid managed by each process
	dimX = sizeX / procX;
	dimY = sizeY / procY;

	int originX = grid->dimensions().origin().getX();
	int originY = grid->dimensions().origin().getY();

	// Payoff information
	payoffT = repast::strToInt(props.getProperty(PAYOFF_T));
	payoffR = repast::strToInt(props.getProperty(PAYOFF_R));
	payoffP = repast::strToInt(props.getProperty(PAYOFF_P));
	payoffS = repast::strToInt(props.getProperty(PAYOFF_S));

	// Model information
	rounds = repast::strToInt(props.getProperty(MODEL_ROUNDS));
	tax = repast::strToDouble(props.getProperty(MODEL_TAX));
	considerTrust = repast::strToDouble(
			props.getProperty(MODEL_CONSIDER_TRUST));
	deltaTrust = repast::strToDouble(props.getProperty(MODEL_DELTA_TRUST));
	trustThreshold = repast::strToDouble(
			props.getProperty(MODEL_TRUST_THRESHOLD));
	strategyType = repast::strToInt(props.getProperty(MODEL_STRATEGY_TYPE));
	neighborhoodType = repast::strToInt(props.getProperty(MODEL_NEIGHBORHOOD));

	genStrategy = repast::Random::instance()->getGenerator("strategy");
	genConsiderTrust = repast::Random::instance()->getGenerator(
			"considerTrust");

	// Create the agents
	int strategy = strategyType;
	bool cTrust;
	LandAgent* agent;
	for (int i = 0; i < (dimX * dimY); i++) {
		repast::AgentId id(i, rank, AGENT_TYPE);

		// Define the strategy
		if (strategyType == 3) {
			strategy = (int) genStrategy->next();
		}

		// Define if the agent should consider trust
		if (genConsiderTrust->next() <= considerTrust) {
			cTrust = true;
		} else {
			cTrust = false;
		}

		agent = new LandAgent(id, strategy, cTrust, deltaTrust, trustThreshold);
		agents.addAgent(agent);

		// Move the agent to the position in the grid
		grid->moveTo(agent,
				repast::Point<int>(originX + (i / dimX), originY + (i % dimY)));
		agent->setPosition(std::vector<int>(originX + (i / dimX), originY + (i % dimY)));
	}

	//grid->initSynchBuffer(agents);
	//grid->synchBuffer<LandAgentPackage>(agents, providerUpdater,
	//		providerUpdater);

	// Set agents neighbors
	for (int i = 0; i < (dimX * dimY); i++) {
		agent = grid->getObjectAt(
				repast::Point<int>(originX + (i / dimX), originY + (i % dimY)));
		agent->setNeighbors(neighborhood(agent, neighborhoodType));

		// Print on screen
		//std::cout << agent->getId() << " " << originX + (i / dimX) << " "
		//		<< originY + (i % dimY) << std::endl;
	}
	std::cout << "CREATED ALL" << std::endl;

	world->barrier();
}

LandModel::~LandModel() {
}

void LandModel::init() {
}

void LandModel::initSchedule(repast::ScheduleRunner& runner) {
	runner.scheduleStop(rounds);

	runner.scheduleEvent(1, 1,
			repast::Schedule::FunctorPtr(
					new repast::MethodFunctor<LandModel>(this,
							&LandModel::step)));

	//runner.scheduleEvent(1.05, 1,
	//	repast::Schedule::FunctorPtr(
	//		new repast::MethodFunctor<LandModel>(this,
	//			&LandModel::synchAgents)));
}

std::vector<LandAgent*> LandModel::neighborhood(LandAgent* agent,
		int neighborhoodType) {
	std::vector<LandAgent*> neighbors;
	LandAgent* out;
	int x = agent->getPosition()[0];
	int y = agent->getPosition()[1];

	std::cout << x << " " << y << std::endl;
	/**
	// if true, then the agent has a neighbor to that direction:
	bool N = false;
	bool S = false;
	bool E = false;
	bool W = false;
	if ((x + 1) % sizeX) {
		out = grid->getObjectAt(repast::Point<int>(x + 1, y));
		neighbors.push_back(out);
		E = true;
	}

	if (x) {
		out = grid->getObjectAt(repast::Point<int>(x - 1, y));
		neighbors.push_back(out);
		W = true;
	}

	if ((y + 1) % sizeY) {
		out = grid->getObjectAt(repast::Point<int>(x, y + 1));
		neighbors.push_back(out);
		S = true;
	}

	if (y) {
		out = grid->getObjectAt(repast::Point<int>(x, y - 1));
		neighbors.push_back(out);
		N = true;
	}

	if (neighborhoodType == MOORE) {
		if (E && S) {
			out = grid->getObjectAt(repast::Point<int>(x + 1, y + 1));
			neighbors.push_back(out);
		}
		if (W && S) {
			out = grid->getObjectAt(repast::Point<int>(x - 1, y + 1));
			neighbors.push_back(out);
		}
		if (E && N) {
			out = grid->getObjectAt(repast::Point<int>(x + 1, y - 1));
			neighbors.push_back(out);
		}
		if (W && N) {
			out = grid->getObjectAt(repast::Point<int>(x - 1, y - 1));
			neighbors.push_back(out);
		}
	}
**/
	return neighbors;
}

void LandModel::step() {
	/**
	 repast::AgentId id;
	 LandAgent* agent;
	 LandAgent* leader;
	 std::vector<int> position;

	 // Decide an action
	 for (int i = 0; i < (dimX * dimY); i++) {
	 id = repast::AgentId(i, rank, AGENT_TYPE);
	 agents.getAgent(id)->decideAction();
	 }

	 // Update the buffer area for the payoff calculation
	 //synchAgents();

	 world->barrier();

	 // Calculate Payoff
	 for (int i = 0; i < (dimX * dimY); i++) {
	 id = repast::AgentId(i, rank, 0);
	 agents.getAgent(id)->calculatePayoff(payoffT, payoffR, payoffP,
	 payoffS);
	 }

	 world->barrier();

	 // Send coalition members' payoff to coalition leader
	 for (int i = 0; i < (dimX * dimY); i++) {
	 id = repast::AgentId(i, rank, 0);
	 agent = agents.getAgent(id);
	 if ((!agent->getIsIndependent()) && (!agent->getIsLeader())) {
	 position = agent->getLeaderPosition();
	 leader = grid->getObjectAt(repast::Point<int>(position));
	 leader->addCoalitionPayoff(agent->getPosition(),
	 agent->getPayoff());
	 }
	 }

	 world->barrier();

	 /*
	 // Agent decides to join a coalition, leave the coalition or stay as it is; Only non-leaders
	 for (int i = 0; i < dimX * dimY; i++) {
	 repast::AgentId id = repast::AgentId(i, rank, 0);
	 if (grid->getLocation(id, position))
	 agents.getAgent(id)->manageCoalition();
	 }
	 // Leaders check if coalition still has other members; Only leaders
	 for (int i = 0; i < dimX * dimY; i++) {
	 repast::AgentId id = repast::AgentId(i, rank, 0);
	 if (grid->getLocation(id, position)) {
	 LandAgent* lead = agents.getAgent(id);
	 if (lead->getX() == lead->getLeaderX() && lead->getY() == lead->getleaderY())
	 amIStillLeader(agents.getAgent(id));
	 }
	 }
	 **/
}

/**
void LandModel::applyTax(LandAgent* lead) {
	int total = 0;
	int coalSize;
	LandAgent* ag;
	std::vector<LandAgent*> coalition(0);
	repast::AgentId leadId = lead->getId();

	for (int i = 0; i < dimX; i++) {
		for (int j = 0; j < dimY; j++) {
			ag = grid->getObjectAt(repast::Point<int>(i, j));
			if (ag->getLeaderId() == leadId && ag->getId() != lead->getId()) {
				coalition.push_back(ag);
				total += ag->getPayoff();
			}
		}
	}
	coalSize = coalition.size();
	if (coalSize != 0) {
		float calcTax = total * (tax/100.0);
		lead->setPayoff(lead->getPayoff() + calcTax);
		total = (total - calcTax)/(1.0 * coalSize);
		for (std::vector<LandAgent*>::iterator it = coalition.begin(); it != coalition.end(); ++it) {
 	 		*it->setPayoff(total);
		}
	}
}

void LandModel::amIStillLeader(LandAgent* lead) {

	LandAgent* ag;
	int X = lead->getX();
	int Y = lead->getY();
	for (int i = 0; i < dimX; i++) {
		for (int j = 0; j < dimY; j++) {
			ag = grid->getObjectAt(repast::Point<int>(i, j));
			if (ag->getLeaderX() == X && ag->getLeaderY() == Y && ag->getId() != lead->getId()) {
				return;
			}
		}
	}
	lead->setLeaderId(NULL);
}
 **/

void LandModel::synchAgents() {
	repast::syncAgents<LandAgentPackage>(providerUpdater, providerUpdater);
	grid->synchBuffer<LandAgentPackage>(agents, providerUpdater,
			providerUpdater);
}
