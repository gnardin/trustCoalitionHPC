#include "landModel.h"

LandModel::LandModel(const std::string& _propsFile, int _argc, char** _argv,
		boost::mpi::communicator* _world) :
		props(_propsFile, _argc, _argv, _world), agents(_world) {

	repast::initializeRandom(props, _world);

	repast::RepastProcess* rp = repast::RepastProcess::instance();
	rank = rp->rank();
	world = _world;

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
	topologyType = repast::strToInt(props.getProperty(MODEL_TOPOLOGY));

	genStrategy = repast::Random::instance()->getGenerator("strategy");
	genConsiderTrust = repast::Random::instance()->getGenerator(
			"considerTrust");

	// Grid size
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

	// Create Grid
	grid = new repast::SharedSpaces<LandAgent>::SharedWrappedDiscreteSpace(
			"grid ",
			repast::GridDimensions(repast::Point<double>(sizeX, sizeY)),
			procDim, gridBuffer, world);
	agents.addProjection(grid);

	// Grid size managed by each process
	dimX = sizeX / procX;
	dimY = sizeY / procY;

	int originX = grid->dimensions().origin().getX();
	int originY = grid->dimensions().origin().getY();

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
		agent->setXY((originX + (i / dimX)), (originY + (i % dimY)));
	}

	world->barrier();

	grid->initSynchBuffer(agents);
	grid->synchBuffer<LandAgentPackage>(agents, *this, *this);

	// Set agents neighbors
	for (int i = 0; i < (dimX * dimY); i++) {
		agent = grid->getObjectAt(
				repast::Point<int>(originX + (i / dimX), originY + (i % dimY)));

		agent->setNeighbors(neighborhood(agent));
	}

	// Request agents from other processes
	int numProcesses = repast::RepastProcess::instance()->worldSize();

	repast::AgentRequest request(rank);
	for (int p = 0; p < numProcesses; p++) {
		if (p != rank) {
			for (int i = 0; i < (dimX * dimY); i++) {
				request.addRequest(repast::AgentId(i, p, AGENT_TYPE));
			}
		}
	}
	repast::RepastProcess::instance()->requestAgents<LandAgent, LandAgentPackage>(
			agents, request, *this, *this, *this);

	repast::RepastProcess::instance()->synchronizeAgentStates<LandAgentPackage>(
			*this, *this);
}

LandModel::~LandModel() {
}

void LandModel::initDataCollection() {
	std::string outputFile = props.getProperty(OUTPUT_FILE);
	std::string outputSeparator = props.getProperty(OUTPUT_SEPARATOR);

	repast::ScheduleRunner& runner =
			repast::RepastProcess::instance()->getScheduleRunner();

	repast::SVDataSetBuilder builder(outputFile, outputSeparator,
			runner.schedule());

	builder.addDataSource(
			repast::createSVDataSource(FIELD_NUMCOALITIONS,
					new NumCoalitions(this), std::plus<int>()));

	builder.addDataSource(
			repast::createSVDataSource(FIELD_CREATEDCOALITIONS,
					new CreatedCoalitions(this), std::plus<int>()));

	builder.addDataSource(
			repast::createSVDataSource(FIELD_DESTROYEDCOALITIONS,
					new DestroyedCoalitions(this), std::plus<int>()));

	builder.addDataSource(
			repast::createSVDataSource(FIELD_NUMINCHANGES,
					new NumInChanges(this), std::plus<int>()));

	builder.addDataSource(
			repast::createSVDataSource(FIELD_NUMOUTCHANGES,
					new NumOutChanges(this), std::plus<int>()));

	builder.addDataSource(
			repast::createSVDataSource(FIELD_NUMAGENTSCOALITIONS,
					new NumAgentsCoalitions(this), std::plus<int>()));

	builder.addDataSource(
			repast::createSVDataSource(FIELD_NUMAGENTSINDEPENDENT,
					new NumAgentsIndependent(this), std::plus<int>()));

	builder.addDataSource(
			repast::createSVDataSource(FIELD_NUMINDEPENDENTPTFT,
					new NumIndependentpTFT(this), std::plus<int>()));

	builder.addDataSource(
			repast::createSVDataSource(FIELD_NUMINDEPENDENTTFT,
					new NumIndependentTFT(this), std::plus<int>()));

	builder.addDataSource(
			repast::createSVDataSource(FIELD_NUMINDEPENDENTRANDOM,
					new NumIndependentRandom(this), std::plus<int>()));

	builder.addDataSource(
			repast::createSVDataSource(FIELD_COALITIONPAYOFF,
					new CoalitionPayoff(this), std::plus<double>()));

	builder.addDataSource(
			repast::createSVDataSource(FIELD_INDEPENDENTPAYOFF,
					new IndependentPayoff(this), std::plus<double>()));

	dataset = builder.createDataSet();
}

void LandModel::initSchedule() {
	repast::ScheduleRunner& runner =
			repast::RepastProcess::instance()->getScheduleRunner();

	runner.scheduleStop(rounds);

	runner.scheduleEvent(1, 1,
			repast::Schedule::FunctorPtr(
					new repast::MethodFunctor<LandModel>(this,
							&LandModel::step)));

	runner.scheduleEvent(1.1, 1,
			repast::Schedule::FunctorPtr(
					new repast::MethodFunctor<LandModel>(this,
							&LandModel::updateOutput)));

	runner.scheduleEvent(1.2, 1,
			repast::Schedule::FunctorPtr(
					new repast::MethodFunctor<repast::DataSet>(dataset,
							&repast::DataSet::record)));

	repast::Schedule::FunctorPtr dataWrite = repast::Schedule::FunctorPtr(
			new repast::MethodFunctor<repast::DataSet>(dataset,
					&repast::DataSet::write));

	int flush = repast::strToInt(props.getProperty(OUTPUT_FLUSH));
	runner.scheduleEvent(1.3, flush, dataWrite);

	runner.scheduleEndEvent(dataWrite);
}

std::vector<LandAgent*> LandModel::neighborhood(LandAgent* _agent) {
	std::vector<LandAgent*> neighbors;
	LandAgent* agent;
	int x = _agent->getX();
	int y = _agent->getY();

	if (topologyType == GRID) {
		// if true, then the agent has a neighbor to that direction:
		bool N = false;
		bool S = false;
		bool E = false;
		bool W = false;
		if ((x + 1) % sizeX) {
			agent = grid->getObjectAt(repast::Point<int>(x + 1, y));
			neighbors.push_back(agent);
			E = true;
		}

		if (x) {
			agent = grid->getObjectAt(repast::Point<int>(x - 1, y));
			neighbors.push_back(agent);
			W = true;
		}

		if ((y + 1) % sizeY) {
			agent = grid->getObjectAt(repast::Point<int>(x, y + 1));
			neighbors.push_back(agent);
			S = true;
		}

		if (y) {
			agent = grid->getObjectAt(repast::Point<int>(x, y - 1));
			neighbors.push_back(agent);
			N = true;
		}

		if (neighborhoodType == MOORE) {
			if (E && S) {
				agent = grid->getObjectAt(repast::Point<int>(x + 1, y + 1));
				neighbors.push_back(agent);
			}
			if (W && S) {
				agent = grid->getObjectAt(repast::Point<int>(x - 1, y + 1));
				neighbors.push_back(agent);
			}
			if (E && N) {
				agent = grid->getObjectAt(repast::Point<int>(x + 1, y - 1));
				neighbors.push_back(agent);
			}
			if (W && N) {
				agent = grid->getObjectAt(repast::Point<int>(x - 1, y - 1));
				neighbors.push_back(agent);
			}
		}
	} else if (topologyType == TORUS) {

		agent = grid->getObjectAt(repast::Point<int>((x + 1) % sizeX, y));
		neighbors.push_back(agent);
		agent = grid->getObjectAt(
				repast::Point<int>(((x - 1) + sizeX) % sizeX, y));
		neighbors.push_back(agent);
		agent = grid->getObjectAt(repast::Point<int>(x, (y + 1) % sizeY));
		neighbors.push_back(agent);
		agent = grid->getObjectAt(
				repast::Point<int>(x, ((y - 1) + sizeY) % sizeY));
		neighbors.push_back(agent);
		if (neighborhoodType == MOORE) {
			agent = grid->getObjectAt(
					repast::Point<int>((x + 1) % sizeX, (y + 1) % sizeY));
			neighbors.push_back(agent);
			agent = grid->getObjectAt(
					repast::Point<int>(((x - 1) + sizeX) % sizeX,
							(y + 1) % sizeY));
			neighbors.push_back(agent);
			agent = grid->getObjectAt(
					repast::Point<int>((x + 1) % sizeX,
							((y - 1) + sizeY) % sizeY));
			neighbors.push_back(agent);
			agent = grid->getObjectAt(
					repast::Point<int>(((x - 1) + sizeX) % sizeX,
							((y - 1) + sizeY) % sizeY));
			neighbors.push_back(agent);
		}
	}

	return neighbors;
}

void LandModel::step() {
	std::vector<LandAgent*> localAgents;
	std::vector<LandAgent*> remoteAgents;
	std::vector<LandAgent*>::iterator local;
	std::vector<LandAgent*>::iterator remote;
	LandAgent* leader;
	int nAgents = 0;

	// Decide an action
	agents.selectAgents(repast::SharedContext<LandAgent>::LOCAL, nAgents,
			localAgents);
	for (local = localAgents.begin(); local != localAgents.end(); local++) {
		(*local)->decideAction();
	}

	// Buffer synchronization
	grid->synchBuffer<LandAgentPackage>(agents, *this, *this);
	world->barrier();

	// Calculate Payoff
	agents.selectAgents(repast::SharedContext<LandAgent>::LOCAL, nAgents,
			localAgents);
	for (local = localAgents.begin(); local != localAgents.end(); local++) {
		(*local)->calculatePayoff(payoffT, payoffR, payoffP, payoffS);
	}

	// Synchronization
	repast::RepastProcess::instance()->synchronizeAgentStates<LandAgentPackage>(
			*this, *this, "REQUEST_AGENTS_ALL");
	world->barrier();

	// Leaders collect their members' Payoff
	std::vector<LandAgent*> members;
	std::vector<LandAgent*>::iterator member;
	agents.selectAgents(repast::SharedContext<LandAgent>::LOCAL, nAgents,
			localAgents);
	for (local = localAgents.begin(); local != localAgents.end(); local++) {
		if ((*local)->getIsLeader()) {
			leader = *local;
			leader->getCoalitionMembers(members);
			for (member = members.begin(); member != members.end(); ++member) {
				if ((leader->getId() == (*member)->getLeaderId())
						&& ((*member)->getIsMember())) {
					leader->addCoalitionPayoff((*member)->getPayoff());
				}
			}
		}
	}

	// Leaders calculate theirs and their members payoff
	agents.selectAgents(repast::SharedContext<LandAgent>::LOCAL, nAgents,
			localAgents);
	for (local = localAgents.begin(); local != localAgents.end(); local++) {
		if ((*local)->getIsLeader()) {
			(*local)->calculateCoalitionPayoff(tax);
		}
	}

	// Synchronization
	repast::RepastProcess::instance()->synchronizeAgentStates<LandAgentPackage>(
			*this, *this, "REQUEST_AGENTS_ALL");
	world->barrier();

	// Members collect their payoff
	agents.selectAgents(repast::SharedContext<LandAgent>::LOCAL, nAgents,
			localAgents);
	for (local = localAgents.begin(); local != localAgents.end(); local++) {
		if ((*local)->getIsMember()) {
			leader = agents.getAgent((*local)->getLeaderId());
			(*local)->setPayoff(leader->getCoalitionPayoff());
		}
	}

	// Synchronization
	repast::RepastProcess::instance()->synchronizeAgentStates<LandAgentPackage>(
			*this, *this, "REQUEST_AGENTS_ALL");
	world->barrier();

	// Independents and Members decide about the coalition
	agents.selectAgents(repast::SharedContext<LandAgent>::LOCAL, nAgents,
			localAgents);
	for (local = localAgents.begin(); local != localAgents.end(); local++) {
		if (((*local)->getIsMember()) || ((*local)->getIsIndependent())) {
			(*local)->decideCoalition();
		}
	}

	// Synchronization
	repast::RepastProcess::instance()->synchronizeAgentStates<LandAgentPackage>(
			*this, *this, "REQUEST_AGENTS_ALL");
	world->barrier();

	// Update coalition status
	agents.selectAgents(repast::SharedContext<LandAgent>::LOCAL, nAgents,
			localAgents);
	for (local = localAgents.begin(); local != localAgents.end(); local++) {

		members.clear();
		agents.selectAgents(repast::SharedContext<LandAgent>::NON_LOCAL,
				nAgents, remoteAgents);
		for (remote = remoteAgents.begin(); remote != remoteAgents.end();
				remote++) {

			if (((*local)->getId() != (*remote)->getId())
					&& ((*local)->getId() == (*remote)->getLeaderId())
					&& ((*remote)->getIsMember())) {
				members.push_back(*remote);
			}
		}

		(*local)->updateCoalitionStatus(members);
	}
}

void LandModel::updateOutput() {
	std::vector<LandAgent*> localAgents;
	std::vector<LandAgent*>::iterator local;
	int nAgents = 0;

	numCoalitions = 0;
	createdCoalitions = 0;
	destroyedCoalitions = 0;
	numInChanges = 0;
	numOutChanges = 0;
	numAgentsCoalitions = 0;
	numAgentsIndependent = 0;
	numIndependentpTFT = 0;
	numIndependentTFT = 0;
	numIndependentRandom = 0;
	coalitionPayoff = 0;
	independentPayoff = 0;

	agents.selectAgents(repast::SharedContext<LandAgent>::LOCAL, nAgents,
			localAgents);
	for (local = localAgents.begin(); local != localAgents.end(); local++) {
		if ((*local)->getIsLeader()) {
			numCoalitions++;
			numAgentsCoalitions++;
			coalitionPayoff += (*local)->getPayoff();
		} else if ((*local)->getIsMember()) {
			numAgentsCoalitions++;
			coalitionPayoff += (*local)->getPayoff();
		} else if ((*local)->getIsIndependent()) {
			numAgentsIndependent++;

			int strategy = (*local)->getStrategy();
			if (strategy == PTFT) {
				numIndependentpTFT++;
			} else if (strategy == TFT) {
				numIndependentTFT++;
			} else if (strategy == RANDOM) {
				numIndependentRandom++;
			}

			independentPayoff += (*local)->getPayoff();
		}
	}
}

/**
 * Output methods
 */
int LandModel::getNumCoalitions() {
	return numCoalitions;
}

int LandModel::getCreatedCoalitions() {
	return createdCoalitions;
}

int LandModel::getDestroyedCoalitions() {
	return destroyedCoalitions;
}

int LandModel::getNumInChanges() {
	return numInChanges;
}

int LandModel::getNumOutChanges() {
	return numOutChanges;
}

int LandModel::getNumAgentsCoalitions() {
	return numAgentsCoalitions;
}

int LandModel::getNumAgentsIndependent() {
	return numAgentsIndependent;
}

int LandModel::getNumIndependentpTFT() {
	return numIndependentpTFT;
}

int LandModel::getNumIndependentTFT() {
	return numIndependentTFT;
}

int LandModel::getNumIndependentRandom() {
	return numIndependentRandom;
}

double LandModel::getCoalitionPayoff() {
	return coalitionPayoff;
}

double LandModel::getIndependentPayoff() {
	return independentPayoff;
}

/**
 * Grid methods
 */
LandAgent* LandModel::createAgent(LandAgentPackage& content) {
	return new LandAgent(content.getId(), content.x, content.y,
			content.isIndependent, content.isMember, content.isLeader,
			content.getLeaderId(), content.action, content.payoff,
			content.coalitionPayoff);
}

LandAgent* LandModel::createAgent(const LandAgentPackage& content) {
	return new LandAgent(content.getId(), content.x, content.y,
			content.isIndependent, content.isMember, content.isLeader,
			content.getLeaderId(), content.action, content.payoff,
			content.coalitionPayoff);
}

void LandModel::createAgents(std::vector<LandAgentPackage>& contents,
		std::vector<LandAgent*>& out) {

	std::vector<LandAgentPackage>::iterator agent;
	for (agent = contents.begin(); agent != contents.end(); ++agent) {
		out.push_back(
				new LandAgent(agent->getId(), agent->x, agent->y,
						agent->isIndependent, agent->isMember, agent->isLeader,
						agent->getLeaderId(), agent->action, agent->payoff,
						agent->coalitionPayoff));
	}
}

void LandModel::provideContent(LandAgent* agent,
		std::vector<LandAgentPackage>& out) {

	LandAgentPackage package = { agent->getId().id(),
			agent->getId().startingRank(), agent->getId().agentType(),
			agent->getX(), agent->getY(), agent->getIsIndependent(),
			agent->getIsMember(), agent->getIsLeader(),
			agent->getLeaderId().id(), agent->getLeaderId().startingRank(),
			agent->getLeaderId().agentType(), agent->getAction(),
			agent->getPayoff(), agent->getCoalitionPayoff() };
	out.push_back(package);
}

void LandModel::provideContent(const repast::AgentRequest& request,
		std::vector<LandAgentPackage>& out) {

	const std::vector<repast::AgentId>& ids = request.requestedAgents();
	for (int i = 0, size = ids.size(); i < size; i++) {
		repast::AgentId id = ids[i];

		if (agents.contains(id)) {
			LandAgent* agent = agents.getAgent(id);
			LandAgentPackage content = { id.id(), id.startingRank(),
					id.agentType(), agent->getX(), agent->getY(),
					agent->getIsIndependent(), agent->getIsMember(),
					agent->getIsLeader(), agent->getLeaderId().id(),
					agent->getLeaderId().startingRank(),
					agent->getLeaderId().agentType(), agent->getAction(),
					agent->getPayoff(), agent->getCoalitionPayoff() };
			out.push_back(content);
		}
	}
}

void LandModel::updateAgent(const LandAgentPackage& content) {
	repast::AgentId id = content.getId();

	if (agents.contains(id)) {
		LandAgent* copy = agents.getAgent(id);
		copy->setXY(content.x, content.y);
		copy->setIsIndependent(content.isIndependent);
		copy->setIsMember(content.isMember);
		copy->setIsLeader(content.isLeader);
		copy->setLeaderId(content.getLeaderId());
		copy->setAction(content.action);
		copy->setPayoff(content.payoff);
		copy->setCoalitionPayoff(content.coalitionPayoff);
	}
}
