#include "landModel.h"

LandModel::LandModel(const std::string& _propsFile, int _argc, char* _argv[],
		mpi::communicator* _world) :
		props(_propsFile, _argc, _argv, _world) {

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
	grid = new repast::SharedGrids<LandAgent>::SharedWrappedGrid("grid ",
			repast::GridDimensions(repast::Point<int>(sizeX, sizeY)), procDim,
			gridBuffer, world);
	agents.addProjection(grid);

	// Create Network
	net = new repast::SharedNetwork<LandAgent, repast::RepastEdge<LandAgent> >(
			"network", true);
	agents.addProjection(net);

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

		agent = new LandAgent(id, payoffT, payoffR, payoffP, payoffS, strategy,
				cTrust, deltaTrust, trustThreshold);
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
		agent->setNeighbors(neighborhood(agent, neighborhoodType));
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
	repast::requestAgents<LandAgent, LandAgentPackage>(agents, request, *this,
			*this);

	// Build the network
	LandAgent* source;
	LandAgent* target;
	for (repast::SharedContext<LandAgent>::const_local_iterator local =
			agents.localBegin(); local != agents.localEnd(); local++) {

		source = local->get();
		for (repast::SharedContext<LandAgent>::const_iterator it =
				agents.begin(); it != agents.end(); ++it) {

			target = it->get();
			if (source->getId() != target->getId()) {
				net->addEdge(source, target);
			}
		}
	}

	repast::createComplementaryEdges<LandAgent, repast::RepastEdge<LandAgent>,
			LandAgentPackage, LandAgentEdge, LandModel, LandModel>(net, agents,
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

std::vector<LandAgent*> LandModel::neighborhood(LandAgent* agent,
		int neighborhoodType) {
	std::vector<LandAgent*> neighbors;
	LandAgent* out;
	int x = agent->getX();
	int y = agent->getY();

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

	return neighbors;
}

void LandModel::step() {
	repast::SharedContext<LandAgent>::const_local_iterator it;
	LandAgent* agent;
	LandAgent* leader;

	// Decide an action
	for (it = agents.localBegin(); it != agents.localEnd(); ++it) {
		(*it)->decideAction();
	}

	// Update buffer agents' action
	synchAgents();
	world->barrier();

	// Calculate Payoff
	for (it = agents.localBegin(); it != agents.localEnd(); ++it) {
		(*it)->calculatePayoff();
	}

	// Update buffer agents' payoff
	synchAgents();
	world->barrier();

	// Leaders collect their members' Payoff
	repast::SharedContext<LandAgent>::const_iterator member;
	for (it = agents.localBegin(); it != agents.localEnd(); ++it) {
		leader = it->get();
		if (leader->getIsLeader()) {
			for (member = agents.begin(); member != agents.end(); ++member) {

				if ((leader->getId() == (*member)->getLeaderId())
						&& ((*member)->getIsMember())) {
					leader->addCoalitionPayoff((*member)->getPayoff());
				}
			}
		}
	}

	// Leaders calculate theirs and their members payoff
	for (it = agents.localBegin(); it != agents.localEnd(); ++it) {
		agent = it->get();
		if (agent->getIsLeader()) {
			agent->calculateCoalitionPayoff(tax);
		}
	}

	// Update buffer agents' payoff
	synchAgents();
	world->barrier();

	// Members collect their payoff
	for (it = agents.localBegin(); it != agents.localEnd(); ++it) {
		agent = it->get();
		if (agent->getIsMember()) {
			leader = agents.getAgent(agent->getLeaderId());
			agent->setPayoff(leader->getCoalitionPayoff());
		}
	}

	// Update buffer agents' payoff
	synchAgents();
	world->barrier();

	// Independents and Members decide about the coalition
	for (it = agents.localBegin(); it != agents.localEnd(); ++it) {
		agent = it->get();
		if ((agent->getIsMember()) || (agent->getIsIndependent())) {
			agent->decideCoalition();
		}
	}

	// Update buffer agents' payoff
	synchAgents();
	world->barrier();

	// Update coalition status
	std::vector<LandAgent*> nodes;
	for (it = agents.localBegin(); it != agents.localEnd(); ++it) {
		agent = it->get();

		nodes.clear();
		net->successors(agent, nodes);

		agent->updateCoalitionStatus(nodes);
	}

	// Update buffer agents' payoff
	synchAgents();
}

void LandModel::updateOutput() {
	repast::SharedContext<LandAgent>::const_local_iterator it;
	LandAgent* agent;

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
	for (it = agents.localBegin(); it != agents.localEnd(); it++) {
		agent = it->get();

		if (agent->getIsLeader()) {
			numCoalitions++;
			numAgentsCoalitions++;
			coalitionPayoff += agent->getPayoff();
		} else if (agent->getIsMember()) {
			numAgentsCoalitions++;
			coalitionPayoff += agent->getPayoff();
		} else if (agent->getIsIndependent()) {
			numAgentsIndependent++;

			int strategy = agent->getStrategy();
			if (strategy == PTFT) {
				numIndependentpTFT++;
			} else if (strategy == TFT) {
				numIndependentTFT++;
			} else if (strategy == RANDOM) {
				numIndependentRandom++;
			}

			independentPayoff += agent->getPayoff();
		}
	}
}

void LandModel::synchAgents() {
	repast::syncAgents<LandAgentPackage>(*this, *this);
	grid->synchBuffer<LandAgentPackage>(agents, *this, *this);
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

/**
 * Network methods
 */
void LandModel::provideEdgeContent(const repast::RepastEdge<LandAgent>* edge,
		std::vector<LandAgentEdge>& edgeContent) {
	LandAgent* source = edge->source();
	LandAgent* target = edge->target();
	LandAgentPackage sourceContent = { source->getId().id(),
			source->getId().startingRank(), source->getId().agentType(),
			source->getX(), source->getY(), source->getIsIndependent(),
			source->getIsMember(), source->getIsLeader(),
			source->getLeaderId().id(), source->getLeaderId().startingRank(),
			source->getLeaderId().agentType(), source->getAction(),
			source->getPayoff(), source->getCoalitionPayoff() };

	LandAgentPackage targetContent = { target->getId().id(),
			target->getId().startingRank(), target->getId().agentType(),
			target->getX(), target->getY(), target->getIsIndependent(),
			target->getIsMember(), target->getIsLeader(),
			target->getLeaderId().id(), target->getLeaderId().startingRank(),
			target->getLeaderId().agentType(), target->getAction(),
			target->getPayoff(), target->getCoalitionPayoff() };

	LandAgentEdge content = { sourceContent, targetContent };
	edgeContent.push_back(content);
}

repast::RepastEdge<LandAgent>* LandModel::createEdge(
		repast::Context<LandAgent>& context, LandAgentEdge& edge) {
	repast::AgentId sourceId = edge.sourceContent.getId();
	repast::AgentId targetId = edge.targetContent.getId();

	return new repast::RepastEdge<LandAgent>(context.getAgent(sourceId),
			context.getAgent(targetId));
}
