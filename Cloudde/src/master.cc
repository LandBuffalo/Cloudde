#include "master.h"
Master::Master(NodeInfo node_info)
{
    node_info_ = node_info;
}

Master::~Master()
{

}

int Master::CheckAndCreatRecordFile()
{
    string file_name;
    file_name = "./Results/CloudDE.csv";
    EA_CPU_ = new DE_CPU();
    ifstream exist_file;
    exist_file.open(file_name.c_str());
    ofstream file;

    if(!exist_file)
    {
        file.open(file_name.c_str());
        file<< "function_ID,run_ID,dim,best_fitness,time_period,communication_percentage,total_FEs,node_num,total_pop_size,island_num,interval,EA_parameters"<<endl;
        file.close();
    }
    else
        exist_file.close();

    return 0;

}

int Master::Initilize(IslandInfo island_info, ProblemInfo problem_info)
{
    island_info_ = island_info;
    problem_info_ = problem_info;
    CheckAndCreatRecordFile();
    EA_CPU_->Initilize(problem_info_, 0);
    for(int i = 0; i < island_info_.island_num; i++)
    {
    	Population tmp_population;
    	EA_CPU_->InitilizePopulation(tmp_population, island_info_.island_size);
    	population_.push_back(tmp_population);
    }
    min_size_ = 5;
    u_max_ = 7;
    int slave_num = node_info_.node_num - 1 - island_info_.island_num;
    for(int i = 0; i < island_info_.island_num; i++)
    	slave_num_island_.push_back(slave_num / island_info_.island_num);
    return 0;

}
int Master::Unitilize()
{
    EA_CPU_->Unitilize();
    slave_num_island_.clear();
    for(int i = 0; i < island_info_.island_num; i++)
        population_[i].clear();
    population_.clear();
    delete EA_CPU_;

    return 0;
}

int Master::ReducePopulationFromIsland()
{
    int tag = problem_info_.function_ID * 1000 + 10 * problem_info_.run_ID + 2;
    MPI_Status mpi_status;

    for(int i = 0; i < island_info_.island_num; i++)
    {
        int message_length = 0;
        MPI_Probe(MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &mpi_status);
        MPI_Get_count(&mpi_status, MPI_DOUBLE, &message_length);
        double * msg_recv = new double[message_length];
        MPI_Recv(msg_recv, message_length, MPI_DOUBLE, mpi_status.MPI_SOURCE, mpi_status.MPI_TAG, MPI_COMM_WORLD, &mpi_status);
        int island_ID = mpi_status.MPI_SOURCE - 1;
        population_[island_ID].clear();
        DeserialMsgToIndividual(population_[island_ID], msg_recv, message_length / (problem_info_.dim + 1));
        delete [] msg_recv;
    }

    return 0;

}

int Master::MapPopulationToIsland()
{
    MPI_Status mpi_status;
    int start_slave_ID = 1 + island_info_.island_num;
    int tag = problem_info_.function_ID * 1000 + 10 * problem_info_.run_ID + 1;


    for(int i = 0; i < island_info_.island_num; i++)
    {
        int message_length = population_[i].size() * (problem_info_.dim + 1)  + 2;
        double * msg_send = new double[message_length];
        msg_send[0] = start_slave_ID;
        msg_send[1] = slave_num_island_[i];

        start_slave_ID += slave_num_island_[i];
        SerialIndividualToMsg(msg_send + 2, population_[i]);
        MPI_Send(msg_send, message_length, MPI_DOUBLE, i + 1, tag, MPI_COMM_WORLD);
        delete []msg_send;
    }

    return 0;
}

int Master::Execute()
{
    int generation = 0;
    double time_period = 0;
    double current_time = 0;
    double start_time = MPI_Wtime();
    int current_FEs = 0;
    for(int i = 0; i < island_info_.island_num; i++)
        current_FEs += population_[i].size();
    long int total_FEs = problem_info_.max_base_FEs * problem_info_.dim;

#ifndef COMPUTING_TIME
    while(current_FEs < total_FEs)
#else
    while(MPI_Wtime() - start_time < problem_info_.computing_time / (120 + 0.0))
#endif   
    {
        double tmp_percentage = (MPI_Wtime() - start_time) / problem_info_.computing_time / (120 + 0.0);
        //printf("%d\n", current_FEs);
        double pm = 0.01 + 0.99 * (exp(10 * tmp_percentage) - 1.0) / (exp(10) - 1.0);
        if(random_.RandDoubleUnif(0,1) < pm)
        {
            //MigrationOccurrenceControl();
            //CloudResourceBalancing();
        }

        MapPopulationToIsland();
        ReducePopulationFromIsland();
        generation++;
        for(int i = 0; i < island_info_.island_num; i++)
            current_FEs += population_[i].size();
    }
    current_time = MPI_Wtime();
    RecordResult(current_FEs, current_time - start_time, 0);
    Finish();

    return 0;
}

int Master::Finish()
{
	int send_flag = 1;
    int tag = problem_info_.function_ID * 1000 + 10 * problem_info_.run_ID;
	for(int i = 1; i < node_info_.node_num; i++)
        MPI_Send(&send_flag, 1, MPI_INT, i, tag, MPI_COMM_WORLD);

    return 0;
}

int Master::MigrationOccurrenceControl()
{
    vector<double> mean_fitness_value;
    for(int i = 0; i < island_info_.island_num; i++)
    {
        double total_value = 0;
        for(int j = 0; j < population_[i].size(); j++)
            total_value += population_[i][j].fitness_value;
        mean_fitness_value.push_back(total_value / (population_[i].size() + 0.0));
    } 

    for(int i = 0; i < island_info_.island_num; i++)
    {
        for(int j = i + 1; j < island_info_.island_num; j++)
        {
            if(mean_fitness_value[i] < mean_fitness_value[j])
            {
                int tmp = random_.RandIntUnif(0, population_[j].size() - 1);

                if(population_[j].size() > min_size_)
                {
                    population_[i].push_back(population_[j][tmp]);
                    population_[j].erase(population_[j].begin() + tmp);                    
                }
            }                
            else
            {
                int tmp = random_.RandIntUnif(0, population_[i].size() - 1);

                if(population_[i].size() > min_size_)
                {
                    population_[j].push_back(population_[i][tmp]);
                    population_[i].erase(population_[i].begin() + tmp);                    
                }
            }
        }
    }

    return 0;
}

int Master::CloudResourceBalancing()
{
	int slave_num = node_info_.node_num - 1 - island_info_.island_num;
    double l = (island_info_.island_num * island_info_.island_size) / (slave_num + 0.0);
    double u = 0;
    for(int i = 0; i < island_info_.island_num; i++)
        u += (population_[i].size() / (slave_num_island_[i] + 0.0) - l) * (population_[i].size() / (slave_num_island_[i] + 0.0) - l);
    
    u = sqrt(u) / (island_info_.island_num - 1.0);
    int sum = 0;
    if(u > u_max_)
    {
        for(int i = 0; i < island_info_.island_num; i++)
        {
        	slave_num_island_[i] = floor(population_[i].size() / l + 0.5);
            if(slave_num_island_[i] == 0)
                slave_num_island_[i] = 1;	
            sum += slave_num_island_[i];    
        }
    }
    while(sum > slave_num)
    {
        int tmp = random_.RandIntUnif(0, island_info_.island_num - 1);
        if(slave_num_island_[tmp] > 1)
        {
            slave_num_island_[tmp]--;
            sum--;
        }
    }
    return 0;
}

int Master::RecordResult(int current_FEs, double time_period, double communication_time)
{
    string file_name;
    file_name = "./Results/CloudDE.csv";

    ofstream file;
    file.open(file_name.c_str(), ios::app);
    int total_pop_size = population_[0].size();
    double best_fitness = EA_CPU_->FindBestIndividual(population_[0]).fitness_value;
    for(int i = 1; i < island_info_.island_num; i++)
    {
        total_pop_size += population_[i].size();
        if(best_fitness > EA_CPU_->FindBestIndividual(population_[i]).fitness_value)
            best_fitness = EA_CPU_->FindBestIndividual(population_[i]).fitness_value;
    }
#ifdef COMPUTING_TIME
    file<<problem_info_.function_ID<<','<<problem_info_.run_ID<<','<<problem_info_.dim<<','<<best_fitness<<','<<problem_info_.computing_time<<','<<communication_time<<','<<current_FEs<<','<<node_info_.node_num<<','<<island_info_.island_num<<','<<total_pop_size<<endl;

#else
    file<<problem_info_.function_ID<<','<<problem_info_.run_ID<<','<<problem_info_.dim<<','<<best_fitness<<','<<time_period<<','<<communication_time<<','<<current_FEs<<','<<node_info_.node_num<<','<<island_info_.island_num<<','<<total_pop_size<<endl;
#endif
        file.close();
    return 0;
}


int Master::DeserialMsgToIndividual(vector<Individual> &individual, double *msg, int length)
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

int Master::SerialIndividualToMsg(double *msg, vector<Individual> &individual)
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
