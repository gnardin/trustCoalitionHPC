#ifndef DATASOURCES_H_INCLUDED
#define DATASOURCES_H_INCLUDED

#include <repast_hpc/TDataSource.h>

#include "landModel.h"

class LandModel;

class NumCoalitions: public repast::TDataSource<int> {

private:
	LandModel* model;

public:
	NumCoalitions(LandModel* _model);
	~NumCoalitions();

	int getData();
};

class CreatedCoalitions: public repast::TDataSource<int> {

private:
	LandModel* model;

public:
	CreatedCoalitions(LandModel* _model);
	~CreatedCoalitions();

	int getData();
};

class DestroyedCoalitions: public repast::TDataSource<int> {

private:
	LandModel* model;

public:
	DestroyedCoalitions(LandModel* _model);
	~DestroyedCoalitions();

	int getData();
};

class NumInChanges: public repast::TDataSource<int> {

private:
	LandModel* model;

public:
	NumInChanges(LandModel* _model);
	~NumInChanges();

	int getData();
};

class NumOutChanges: public repast::TDataSource<int> {

private:
	LandModel* model;

public:
	NumOutChanges(LandModel* _model);
	~NumOutChanges();

	int getData();
};

class NumAgentsCoalitions: public repast::TDataSource<int> {

private:
	LandModel* model;

public:
	NumAgentsCoalitions(LandModel* _model);
	~NumAgentsCoalitions();

	int getData();
};

class NumAgentsIndependent: public repast::TDataSource<int> {

private:
	LandModel* model;

public:
	NumAgentsIndependent(LandModel* _model);
	~NumAgentsIndependent();

	int getData();
};

class NumIndependentpTFT: public repast::TDataSource<int> {

private:
	LandModel* model;

public:
	NumIndependentpTFT(LandModel* _model);
	~NumIndependentpTFT();

	int getData();
};

class NumIndependentTFT: public repast::TDataSource<int> {

private:
	LandModel* model;

public:
	NumIndependentTFT(LandModel* _model);
	~NumIndependentTFT();

	int getData();
};

class NumIndependentRandom: public repast::TDataSource<int> {

private:
	LandModel* model;

public:
	NumIndependentRandom(LandModel* _model);
	~NumIndependentRandom();

	int getData();
};

class CoalitionPayoff: public repast::TDataSource<double> {

private:
	LandModel* model;

public:
	CoalitionPayoff(LandModel* _model);
	~CoalitionPayoff();

	double getData();
};

class IndependentPayoff: public repast::TDataSource<double> {

private:
	LandModel* model;

public:
	IndependentPayoff(LandModel* _model);
	~IndependentPayoff();

	double getData();
};

#endif // DATASOURCES_H_INCLUDED
