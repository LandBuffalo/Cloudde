#ifndef __SLAVE_H__
#define __SLAVE_H__
#pragma once
#include "config.h"
#include "random.h"
#include "EA_CPU.h"
#include <mpi.h>

class Slave
{
private:
	IslandInfo 				island_info_;
	ProblemInfo 			problem_info_;
	NodeInfo 				node_info_;
	EA_CPU * 				EA_CPU_;
	int 					island_ID_;
	int						MapPopulationFromIsland(Population & population, MPI_Status &mpi_status);
	int						ReducePopulationToIsland(Population & population, MPI_Status &mpi_status);
	int						DeserialMsgToIndividual(vector<Individual> &individual, double *msg, int length);

public:
							Slave(const NodeInfo node_info);
							~Slave();
	int 					Initilize(IslandInfo island_info, ProblemInfo problem_info);
	int 					Unitilize();
	int						Execute();

};

#endif
