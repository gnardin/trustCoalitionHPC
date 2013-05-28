#ifndef  __MODEL_H__
#define  __MODEL_H__

#include <repast_hpc/AgentId.h>
#include <repast_hpc/AgentRequest.h>
#include <repast_hpc/initialize_random.h>
#include <repast_hpc/Point.h>
#include <repast_hpc/Properties.h>
#include <repast_hpc/Random.h>
#include <repast_hpc/RepastProcess.h>
#include <repast_hpc/Schedule.h>
#include <repast_hpc/SharedContext.h>
#include <repast_hpc/SharedNetwork.h>
#include <repast_hpc/SharedSpace.h>
#include <repast_hpc/SVDataSetBuilder.h>
#include <repast_hpc/Utilities.h>

#include "landAgent.h"
#include "dataSources.h"

namespace mpi = boost::mpi;

// Grid definition
const std::string GRID_MIN_X = "grid.min.x";
const std::string GRID_MIN_Y = "grid.min.y";
const std::string GRID_MAX_X = "grid.max.x";
const std::string GRID_MAX_Y = "grid.max.y";
const std::string GRID_BUFFER = "grid.buffer";

// Processes - Multiplication must be the total number of grid positions
const std::string PROC_X = "proc.per.x";
const std::string PROC_Y = "proc.per.y";

// Payoff matrix
const std::string PAYOFF_T = "payoff.temptation";
const std::string PAYOFF_R = "payoff.reward";
const std::string PAYOFF_P = "payoff.punishment";
const std::string PAYOFF_S = "payoff.sucker";

// Model attributes
const std::string MODEL_ROUNDS = "model.rounds";
const std::string MODEL_TAX = "model.tax";
const std::string MODEL_CONSIDER_TRUST = "model.consider-trust";
const std::string MODEL_DELTA_TRUST = "model.delta-trust";
const std::string MODEL_TRUST_THRESHOLD = "model.trust-threshold";
const std::string MODEL_STRATEGY_TYPE = "model.strategy-type";
const std::string MODEL_NEIGHBORHOOD = "model.neighborhood";

// Output attributes
const std::string OUTPUT_FILE = "output.file";
const std::string OUTPUT_SEPARATOR = "output.separator";
const std::string OUTPUT_FLUSH = "output.flush";

// Output
const std::string FIELD_NUMCOALITIONS = "numCoalitions";
const std::string FIELD_CREATEDCOALITIONS = "createdCoalitions";
const std::string FIELD_DESTROYEDCOALITIONS = "destroyedCoalitions";
const std::string FIELD_NUMINCHANGES = "numInChanges";
const std::string FIELD_NUMOUTCHANGES = "numOutChanges";
const std::string FIELD_NUMAGENTSCOALITIONS = "numAgentsCoalitions";
const std::string FIELD_NUMAGENTSINDEPENDENT = "numAgentsIndependent";
const std::string FIELD_NUMINDEPENDENTPTFT = "numIndependentpTFT";
const std::string FIELD_NUMINDEPENDENTTFT = "numIndependentTFT";
const std::string FIELD_NUMINDEPENDENTRANDOM = "numIndependentRandom";
const std::string FIELD_COALITIONPAYOFF = "coalitionPayoff";
const std::string FIELD_INDEPENDENTPAYOFF = "independentPayoff";

// Neighborhood
const int VON_NEUMANN = 0;
const int MOORE = 1;

// Agent Type
const int AGENT_TYPE = 0;

class ProviderReceiver;

class LandModel {

private:
	friend class ProviderReceiver;

	mpi::communicator* world;
	int rank;

	// Grid size
	int sizeX;
	int sizeY;

	// Process size
	int dimX;
	int dimY;

	// Payoff values
	int payoffT; //
	int payoffR; // reward
	int payoffP; // punishment
	int payoffS; // sucker

	// Model information
	int rounds;
	double tax;
	double considerTrust;
	double deltaTrust;
	double trustThreshold;
	int strategyType;
	int neighborhoodType;

	// Output information
	int numCoalitions;
	int createdCoalitions;
	int destroyedCoalitions;
	int numInChanges;
	int numOutChanges;
	int numAgentsCoalitions;
	int numAgentsIndependent;
	int numIndependentpTFT;
	int numIndependentTFT;
	int numIndependentRandom;
	double coalitionPayoff;
	double independentPayoff;

	// General
	repast::SharedContext<LandAgent> agents;
	repast::SharedGrids<LandAgent>::SharedWrappedGrid* grid;
	repast::SharedNetwork<LandAgent, repast::RepastEdge<LandAgent> >* net;
	repast::Properties props;
	repast::DataSet* dataset;

	// Random
	repast::NumberGenerator* genStrategy;
	repast::NumberGenerator* genConsiderTrust;

	std::vector<LandAgent*> neighborhood(LandAgent* _agent,
			int _neighborhoodType);

public:
	LandModel(const std::string& propsFile, int argc, char* argv[],
			mpi::communicator* world);
	virtual ~LandModel();
	void initDataCollection();
	void initSchedule();
	void step();
	void updateOutput();
	void synchAgents();

	/**
	 * Output methods
	 */
	int getNumCoalitions();
	int getCreatedCoalitions();
	int getDestroyedCoalitions();
	int getNumInChanges();
	int getNumOutChanges();
	int getNumAgentsCoalitions();
	int getNumAgentsIndependent();
	int getNumIndependentpTFT();
	int getNumIndependentTFT();
	int getNumIndependentRandom();
	double getCoalitionPayoff();
	double getIndependentPayoff();

	/**
	 * Grid methods
	 */
	LandAgent* createAgent(LandAgentPackage& _content);
	LandAgent* createAgent(const LandAgentPackage& content);
	void createAgents(std::vector<LandAgentPackage>& _contents,
			std::vector<LandAgent*>& _out);
	void provideContent(LandAgent* _agent, std::vector<LandAgentPackage>& _out);
	void provideContent(const repast::AgentRequest& _request,
			std::vector<LandAgentPackage>& _out);
	void updateAgent(const LandAgentPackage& _content);

	/**
	 * Network methods
	 */
	void provideEdgeContent(const repast::RepastEdge<LandAgent>* edge,
			std::vector<LandAgentEdge>& edgeContent);
	repast::RepastEdge<LandAgent>* createEdge(
			repast::Context<LandAgent>& context, LandAgentEdge& edge);
};

#endif // __MODEL_H__
