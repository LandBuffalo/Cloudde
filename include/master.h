#ifndef __MASTER_H__
#define __MASTER_H__
#pragma once
#include "config.h"
#include "random.h"
#include "EA_CPU.h"
#include <mpi.h>

class Master
{
private:
	IslandInfo 				island_info_;
	NodeInfo 				node_info_;
	ProblemInfo 			problem_info_;
	vector<Population> 		population_;
	vector<int> 			slave_num_island_;
	EA_CPU	*				EA_CPU_;
	Random 					random_;
	int 					min_size_;
	int 					u_max_;
	int						CheckAndCreatRecordFile();
	int						ReducePopulationFromIsland();
	int						MapPopulationToIsland();
	int						MigrationOccurrenceControl();
	int						CloudResourceBalancing();
	int						RecordResult(int current_FEs, double time_period, double communication_time);
	int						DeserialMsgToIndividual(vector<Individual> &individual, double *msg, int length);
	int						SerialIndividualToMsg(double *msg, vector<Individual> &individual);
	int 					Finish();
public:
							Master(const NodeInfo node_info);
							~Master();
	int 					Initilize(IslandInfo island_info, ProblemInfo problem_info);
	int 					Unitilize();
	int						Execute();

};

#endif
