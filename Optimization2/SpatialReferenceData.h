/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef SPATIALREFERENCEDATA_H
#define SPATIALREFERENCEDATA_H

#include <vector>
#include <string>
using namespace std;

class SpatialReferenceData {
private:
	vector<string> variableList;
	vector<double> timePointList;
	vector<double**> dataList;
	double* weights;

	int dataSize;
	int currentTimeIndex;
	int currentVarIndex;
	
public:
	SpatialReferenceData(int s);
	~SpatialReferenceData();

	int getNumVariables() { return (int)variableList.size(); }
	string& getVariable(int index) { return variableList.at(index); }
	int findVariable(string& varName);

	int getNumTimePoints();
	double getTimePoint(int index);

	const double* getData(int timeIndex, int varIndex);
	int getDataSize() { return dataSize; }

	void addVariable(string& var);
	void addTimePoint(double time);
	void addVariableData(int varIndex, double* data);

	void setWeights(double* w);
	void show();
};

#endif