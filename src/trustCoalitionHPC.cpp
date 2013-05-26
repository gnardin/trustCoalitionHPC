#include <iostream>
#include <time.h>

#include "landAgent.h"
#include "landModel.h"

#include <repast_hpc/RepastProcess.h>
#include <repast_hpc/Schedule.h>

namespace mpi = boost::mpi;

void usage(char* executable) {
	std::cerr << "usage: " << executable << " <config> <properties>"
			<< std::endl;
	std::cerr << "  <config>     - the path to the repast configuration file"
			<< std::endl;
	std::cerr << "  <properties> - the path to the model properties file"
			<< std::endl;
}

void runLandModel(const std::string propsFile, int argc, char* argv[],
		mpi::communicator* world) {

	if (world->rank() == 0) {
		Log4CL::instance()->get_logger("root").log(INFO,
				"Starting Model Execution...");
	}

	LandModel* landModel = new LandModel(propsFile, argc, argv, world);
	landModel->init();
	repast::ScheduleRunner& runner =
			repast::RepastProcess::instance()->getScheduleRunner();
	landModel->initSchedule(runner);

	world->barrier();

	runner.run();

	if (world->rank() == 0) {
		Log4CL::instance()->get_logger("root").log(INFO,
				"Terminating Model Execution");
	}
}

int main(int argc, char* argv[]) {
	// First check, if there are not enough arguments, warn the user and exit
	if (argc < 3) {
		usage(argv[0]);
		return -1;
	}

	mpi::environment env(argc, argv);
	mpi::communicator world;

	std::string config = argv[1];
	std::string props = argv[2];

	repast::RepastProcess::init(config, &world);
	clock_t start = clock();

	// Runs the model
	runLandModel(props, argc, argv, &world);

	// Wait for all the processes to finish the execution
	world.barrier();

	clock_t end = clock();
	if (world.rank() == 0) {
		long double diff = end - start;
		Log4CL::instance()->get_logger("root").log(INFO,
				"total execution, time: "
						+ boost::lexical_cast<std::string>(
								diff / CLOCKS_PER_SEC));
	}
	repast::RepastProcess::instance()->done();
	return 0;
}
