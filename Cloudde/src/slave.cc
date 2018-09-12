#include "slave.h"
Slave::Slave(const NodeInfo node_info)
{
    node_info_ = node_info;
}

Slave::~Slave()
{

}

int Slave::Initilize(IslandInfo island_info, ProblemInfo problem_info)
{
    problem_info_ = problem_info;

    island_info_ = island_info;

    EA_CPU_ = new DE_CPU();

    EA_CPU_->Initilize(problem_info_, 0);

    return 0;
}

int Slave::Unitilize()
{
	EA_CPU_->Unitilize();
    delete EA_CPU_;
    return 0;
}

int Slave::MapPopulationFromIsland(Population & population, MPI_Status &mpi_status)
{
    int message_length = 0;
    MPI_Get_count(&mpi_status, MPI_DOUBLE, &message_length);
    double * msg_recv = new double[message_length];
    MPI_Recv(msg_recv, message_length, MPI_DOUBLE, mpi_status.MPI_SOURCE, mpi_status.MPI_TAG, MPI_COMM_WORLD, &mpi_status);
    DeserialMsgToIndividual(population, msg_recv, message_length / (problem_info_.dim + 1));
    delete [] msg_recv;

    return 0;
}


int Slave::ReducePopulationToIsland(Population & population, MPI_Status &mpi_status)
{
    int message_length = population.size();
    double * msg_send = new double[message_length];
    for(int i = 0; i < message_length; i++)
        msg_send[i] = population[i].fitness_value;

    MPI_Send(msg_send, message_length, MPI_DOUBLE, mpi_status.MPI_SOURCE, mpi_status.MPI_TAG + 1, MPI_COMM_WORLD);
    delete [] msg_send;    
    return 0;
}



int Slave::Execute()
{
    MPI_Status mpi_status;
    int finish_flag = 0;
    int basic_tag = problem_info_.function_ID * 1000 +  10 * problem_info_.run_ID;
    while(finish_flag != 1)
    {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);

        if(mpi_status.MPI_TAG == basic_tag + 3)
        {
            Population population;
            MapPopulationFromIsland(population, mpi_status);
            EA_CPU_->EvaluateFitness(population);
            ReducePopulationToIsland(population, mpi_status);
        }
        else if (mpi_status.MPI_TAG == basic_tag)
        {

            MPI_Recv(&finish_flag, 1, MPI_INT, mpi_status.MPI_SOURCE, mpi_status.MPI_TAG, MPI_COMM_WORLD, &mpi_status);
        }
    }

	return 0;
}

int Slave::DeserialMsgToIndividual(vector<Individual> &individual, double *msg, int length)
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
