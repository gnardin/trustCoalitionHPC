#include "dataSources.h"

NumCoalitions::NumCoalitions(LandModel* _model) {
	model = _model;
}

NumCoalitions::~NumCoalitions() {
}

int NumCoalitions::getData() {
	return model->getNumCoalitions();
}

CreatedCoalitions::CreatedCoalitions(LandModel* _model) {
	model = _model;
}

CreatedCoalitions::~CreatedCoalitions() {
}

int CreatedCoalitions::getData() {
	return model->getCreatedCoalitions();
}

DestroyedCoalitions::DestroyedCoalitions(LandModel* _model) {
	model = _model;
}

DestroyedCoalitions::~DestroyedCoalitions() {
}

int DestroyedCoalitions::getData() {
	return model->getDestroyedCoalitions();
}

NumInChanges::NumInChanges(LandModel* _model) {
	model = _model;
}

NumInChanges::~NumInChanges() {
}

int NumInChanges::getData() {
	return model->getNumInChanges();
}

NumOutChanges::NumOutChanges(LandModel* _model) {
	model = _model;
}

NumOutChanges::~NumOutChanges() {
}

int NumOutChanges::getData() {
	return model->getNumOutChanges();
}

NumAgentsCoalitions::NumAgentsCoalitions(LandModel* _model) {
	model = _model;
}

NumAgentsCoalitions::~NumAgentsCoalitions() {
}

int NumAgentsCoalitions::getData() {
	return model->getNumAgentsCoalitions();
}

NumAgentsIndependent::NumAgentsIndependent(LandModel* _model) {
	model = _model;
}

NumAgentsIndependent::~NumAgentsIndependent() {
}

int NumAgentsIndependent::getData() {
	return model->getNumAgentsIndependent();
}

NumIndependentpTFT::NumIndependentpTFT(LandModel* _model) {
	model = _model;
}

NumIndependentpTFT::~NumIndependentpTFT() {
}

int NumIndependentpTFT::getData() {
	return model->getNumIndependentpTFT();
}

NumIndependentTFT::NumIndependentTFT(LandModel* _model) {
	model = _model;
}

NumIndependentTFT::~NumIndependentTFT() {
}

int NumIndependentTFT::getData() {
	return model->getNumIndependentTFT();
}

NumIndependentRandom::NumIndependentRandom(LandModel* _model) {
	model = _model;
}

NumIndependentRandom::~NumIndependentRandom() {
}

int NumIndependentRandom::getData() {
	return model->getNumIndependentRandom();
}

CoalitionPayoff::CoalitionPayoff(LandModel* _model) {
	model = _model;
}

CoalitionPayoff::~CoalitionPayoff() {
}

double CoalitionPayoff::getData() {
	return model->getCoalitionPayoff();
}

IndependentPayoff::IndependentPayoff(LandModel* _model) {
	model = _model;
}

IndependentPayoff::~IndependentPayoff() {
}

double IndependentPayoff::getData() {
	return model->getIndependentPayoff();
}
