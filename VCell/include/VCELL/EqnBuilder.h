/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef EQNBUILDER_H
#define EQNBUILDER_H

class Mesh;
class Variable;

class EqnBuilder
{
public:
	EqnBuilder(Variable *var, Mesh *mesh);

	virtual bool initEquation(double deltaTime, 
				int volumeIndexStart, int volumeIndexSize, 
				int membraneIndexStart, int membraneIndexSize)=0;
	virtual bool buildEquation(double deltaTime, 
				int volumeIndexStart, int volumeIndexSize, 
				int membraneIndexStart, int membraneIndexSize)=0;

	Mesh* getMesh() { return mesh; };

protected:
	Variable   *var;
	Mesh       *mesh;
};    

#endif