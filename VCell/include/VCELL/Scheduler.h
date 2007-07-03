/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef SCHEDULER_H
#define SCHEDULER_H

class Simulation;

class Scheduler
{
public:
	Scheduler(Simulation *Asim);
	virtual void iterate()=0;
	virtual void    update();
	virtual void    reset();
	virtual bool initValues();
	virtual void    collectResults(int processRank);
	void solveFastSystem(int startVolIndex, int VolSize, int startMemIndex, int MemSize);
	bool hasFastSystem() { return bHasFastSystem; }
protected:
	Simulation *sim;
	bool    bFirstTime;
	bool    bHasFastSystem;
};

#endif