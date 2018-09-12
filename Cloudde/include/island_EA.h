#ifndef __ISLANDEA_H__
#define __ISLANDEA_H__
#pragma once
#include "config.h"
#include "random.h"
#include "EA_CPU.h"
#include <mpi.h>

class IslandEA
{
private:
	EA_CPU	*				EA_CPU_;
	Random                  random_;
	NodeInfo				node_info_;
	ProblemInfo				problem_info_;
	IslandInfo				island_info_;
	int 					start_slave_ID_;
	int 					slave_num_;
	int 					ReducePopulationToMaster(Population & population);
    int                     MapPopulationFromMaster(Population & population, MPI_Status &mpi_status);
	int 					ReducePopulationFromSlaves(Population & population);
	int 					MapPopulationToSlaves(Population & population);
	int 					SerialIndividualToMsg(double *msg, vector<Individual> &individual);
	int  					DeserialMsgToIndividual(vector<Individual> &individual, double *msg, int length);
	int 					Finish();
public:
							IslandEA(const NodeInfo node_info);
							~IslandEA();
	int 					Initilize(IslandInfo island_info, ProblemInfo problem_info);
	int 					Unitilize();
	int						Execute();


#ifdef DEBUG
	int                    	FprintPopulation();
#endif
};

#endif
