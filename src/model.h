// MODEL
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
#include <repast_hpc/SharedSpace.h>
#include <repast_hpc/Utilities.h>

#include "landAgent.h"
#include "providerUpdater.h"

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

// Neighborhood
const int VON_NEUMANN = 0;
const int MOORE = 1;

// Agent Type
const int AGENT_TYPE = 0;

class LandModel {

private:
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

	// General
	repast::SharedContext<LandAgent> agents;
	repast::SharedGrids<LandAgent>::SharedWrappedGrid* grid;
	repast::Properties props;
	ProviderUpdater providerUpdater;

	// Random
	repast::NumberGenerator* genStrategy;
	repast::NumberGenerator* genConsiderTrust;

	std::vector<LandAgent*> neighborhood(LandAgent* _agent,
			int _neighborhoodType);

public:
	LandModel(const std::string& propsFile, int argc, char* argv[],
			mpi::communicator* world);

	virtual ~LandModel();

	void init();

	void initSchedule(repast::ScheduleRunner& runner);

	void step();

	void synchAgents();
};

#endif // __MODEL_H__
