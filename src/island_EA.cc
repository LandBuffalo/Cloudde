#include "island_EA.h"
IslandEA::IslandEA(const NodeInfo node_info)
{
    node_info_ = node_info;
}

IslandEA::~IslandEA()
{

}


int IslandEA::Initilize(IslandInfo island_info, ProblemInfo problem_info)
{
    problem_info_ = problem_info;

    island_info_ = island_info;

    switch (node_info_.task_ID)
    {
        case 1:
            EA_CPU_ = new DE_CPU();
            break;
        case 2:
            EA_CPU_ = new PSO_CPU();
            break;
        case 3:
            EA_CPU_ = new GA_CPU();
            break;
        default:
            break;
    }

    EA_CPU_->Initilize(problem_info_, node_info_.node_ID - 1);

    return 0;
}

int IslandEA::Unitilize()
{    
	EA_CPU_->Unitilize();
    delete EA_CPU_;

    return 0;
}


int IslandEA::MapPopulationFromMaster(Population & population, MPI_Status &mpi_status)
{
    int message_length = 0;
    MPI_Get_count(&mpi_status, MPI_DOUBLE, &message_length);
    double * msg_recv = new double[message_length];
    MPI_Recv(msg_recv, message_length, MPI_DOUBLE, mpi_status.MPI_SOURCE, mpi_status.MPI_TAG, MPI_COMM_WORLD, &mpi_status);
    start_slave_ID_ = int(msg_recv[0]);
    slave_num_ = int(msg_recv[1]);
    //printf("node_id=%d--start_slave_ID_=%d--slave_num_=%d\n", node_info_.node_ID, start_slave_ID_, slave_num_);
    DeserialMsgToIndividual(population, msg_recv + 2, (message_length - 2) / (problem_info_.dim + 1));
    delete [] msg_recv;


    return 0;
}

int IslandEA::ReducePopulationToMaster(Population & population)
{
    MPI_Status mpi_status;
    int tag = problem_info_.function_ID * 1000 +  10 * problem_info_.run_ID + 2;
    int message_length = population.size() * (problem_info_.dim + 1);
    double * msg_send = new double[message_length];
    SerialIndividualToMsg(msg_send, population);
                    //printf("island_start_send-%d\n",node_info_.node_ID);

    MPI_Send(msg_send, message_length, MPI_DOUBLE, 0, tag, MPI_COMM_WORLD);
                //printf("island_end_send-%d\n",node_info_.node_ID);

    delete [] msg_send;

    return 0;
}

int IslandEA::ReducePopulationFromSlaves(Population & population)
{
		                    //printf("island_start_recv_from_slave-%d\n",node_info_.node_ID);
    int ave_individual_slave = population.size() / slave_num_;
    int tmp = population.size() % slave_num_;

    int tag = problem_info_.function_ID * 1000 + 10 * problem_info_.run_ID + 4;
    MPI_Status mpi_status;
    int count = 0;
    vector<int> start_individual_ID;
    for(int i = 0; i < slave_num_; i++)
    {
        start_individual_ID.push_back(count);
        if(i < tmp)
            count += ave_individual_slave + 1;
        else
            count += ave_individual_slave;
    }

	for(int i = 0; i < slave_num_; i++)
	{
		int message_length = 0;
		MPI_Probe(MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &mpi_status);
        int slave_ID = mpi_status.MPI_SOURCE - start_slave_ID_;
	    MPI_Get_count(&mpi_status, MPI_DOUBLE, &message_length);
	    double * msg_recv = new double[message_length];
	    MPI_Recv(msg_recv, message_length, MPI_DOUBLE, mpi_status.MPI_SOURCE, mpi_status.MPI_TAG, MPI_COMM_WORLD, &mpi_status);
	    for (int j = 0; j < message_length; j++)
	    	population[start_individual_ID[slave_ID] + j].fitness_value = msg_recv[j];
	    
	    delete [] msg_recv;
	}
                        //printf("island_end_recv_from_slave-%d\n",node_info_.node_ID);

    return 0;
}

int IslandEA::MapPopulationToSlaves(Population & population)
{
			                    //printf("island_start_send_to_slave-%d\n",node_info_.node_ID);

    MPI_Status mpi_status;
    int tag = problem_info_.function_ID * 1000 +  10 * problem_info_.run_ID + 3;
    int ave_individual_slave = population.size() / slave_num_;
    int tmp = population.size() % slave_num_;
    int length_ave_individual_slave = ave_individual_slave * (problem_info_.dim + 1);
    double * msg_send = new double[population.size() * (problem_info_.dim + 1)];
    SerialIndividualToMsg(msg_send, population);
    int count = 0;
    for(int i = 0; i < slave_num_; i++)
    {
    	int destination = i + start_slave_ID_;
    	if(i < tmp)
    	{
    		MPI_Send(msg_send + count, length_ave_individual_slave + problem_info_.dim + 1, MPI_DOUBLE, destination, tag, MPI_COMM_WORLD);
    		count += length_ave_individual_slave + problem_info_.dim + 1;
    	}
    	else
    	{
    		MPI_Send(msg_send + count, length_ave_individual_slave, MPI_DOUBLE, destination, tag, MPI_COMM_WORLD);
    		count += length_ave_individual_slave;
    	}
    }
                        //printf("island_end_send_to_slave-%d\n",node_info_.node_ID);

    delete [] msg_send;    
    return 0;
}


int IslandEA::Execute()
{
    MPI_Status mpi_status;
    int finish_flag = 0;
    int basic_tag = problem_info_.function_ID * 1000 +  10 * problem_info_.run_ID;
    while(finish_flag != 1)
    {
        MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
        if(mpi_status.MPI_TAG == basic_tag + 1)
        {
			Population sub_population, sub_candidate;
			MapPopulationFromMaster(sub_population, mpi_status);
			EA_CPU_->Reproduce(sub_candidate, sub_population);

			MapPopulationToSlaves(sub_candidate);
			ReducePopulationFromSlaves(sub_candidate);
			EA_CPU_->SelectSurvival(sub_population, sub_candidate);
			ReducePopulationToMaster(sub_population);
        }
        else if (mpi_status.MPI_TAG == basic_tag)
        {
            MPI_Recv(&finish_flag, 1, MPI_INT, mpi_status.MPI_SOURCE, mpi_status.MPI_TAG, MPI_COMM_WORLD, &mpi_status);
        }
    }





	return 0;
}

int IslandEA::DeserialMsgToIndividual(vector<Individual> &individual, double *msg, int length)
{
    int count = 0;
    for (int i = 0; i < length; i++)
    {
        Individual local_individual;
        for(int j = 0; j < problem_info_.dim; j++)
        {
            local_individual.elements.push_back(msg[count]);
            count++;
        }
        local_individual.fitness_value = msg[count];
        count++;
        individual.push_back(local_individual);
    }
    return 0;
}


int IslandEA::SerialIndividualToMsg(double *msg, vector<Individual> &individual)
{
    int count = 0;
    for (int i = 0; i < individual.size(); i++)
    {
        for (int j = 0; j < problem_info_.dim; j++)
        {
            msg[count] = individual[i].elements[j];
            count++;
        }
        msg[count] = individual[i].fitness_value;
        count++;
    }
    return 0;
}